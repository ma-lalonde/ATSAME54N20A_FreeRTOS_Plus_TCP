/*
 * FreeRTOS+TCP V2.3.2
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */


/* Standard includes */
#include <stdbool.h>
#include <stdint.h>

/* Atmel ASF includes */
#include "hal_mac_async.h"
#include "ethernet_phy.h"
#include "hal_delay.h"
#include "hpl_gmac_config.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+TCP includes */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"

#define KSZ8081RND    0

/* Make sure someone takes care of the CRC calculation */
#if ( (CONF_GMAC_NCFGR_RXCOEN == 0 )  && ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 ) )
	#error Receive CRC offloading should be enabled.
#endif

#if ( ( CONF_GMAC_DCFGR_TXCOEN == 0 ) && ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM == 1 ) )
	#error Transmit CRC offloading should be enabled.
#endif

/* The Ethernet MAC and PHY instances created by ASF */
extern struct mac_async_descriptor ETHERNET_MAC_0;
extern struct ethernet_phy_descriptor ETHERNET_PHY_0_desc;

/* Holds the handle of the task used as a deferred interrupt processor.  The
 * handle is used so direct notifications can be sent to the task for all EMAC/DMA
 * related interrupts. */
TaskHandle_t xEMACTaskHandle = NULL;

/*static QueueHandle_t xTxBufferQueue; */
/*int tx_release_count[ 4 ]; */

/* xTXDescriptorSemaphore is a counting semaphore with
 * a maximum count of GMAC_TX_BUFFERS, which is the number of
 * DMA TX descriptors. */
/*static SemaphoreHandle_t xTXDescriptorSemaphore = NULL; */

static bool phy_link_state = pdFALSE;
static int32_t phy_rst;

static volatile BaseType_t xGMACSwitchRequired = pdTRUE;


void xRxCallback( void );
static void phy_link_reset();
static void prvEMACDeferredInterruptHandlerTask( void * pvParameters );

/* Start a new autonegotiation on the PHY link and wait until link is up.
 * Will also start a new negotiation every 10 seconds if link is still donw by then. */
static void phy_link_reset()
{
    configASSERT( INCLUDE_xTaskGetSchedulerState == 1 );
    BaseType_t schedulerState = xTaskGetSchedulerState();

    do
    {
        uint8_t retry_count = 0;

        /* Restart an auto-negotiation */
        phy_rst = ethernet_phy_restart_autoneg( &ETHERNET_PHY_0_desc );
        configASSERT( phy_rst == ERR_NONE );

        /* Wait for PHY link up */
        do
        {
            if( schedulerState == taskSCHEDULER_RUNNING )
            {
                vTaskDelay( 100 );
            }
            else
            {
                delay_ms( 100 );
            }

            phy_rst = ethernet_phy_get_link_status( &ETHERNET_PHY_0_desc, &phy_link_state );
        } while( phy_rst == ERR_NONE && phy_link_state != pdTRUE && retry_count < 100 );
    } while( phy_rst != ERR_NONE || phy_link_state != pdTRUE );
}


static void prvEMACDeferredInterruptHandlerTask( void * pvParameters )
{
    NetworkBufferDescriptor_t * pxBufferDescriptor;
    size_t xBytesReceived;
    size_t xBytesRead;

    /* Used to indicate that xSendEventStructToIPTask() is being called because
     * of an Ethernet receive event. */
    IPStackEvent_t xRxEvent;

    for( ; ; )
    {
        phy_rst = ethernet_phy_get_link_status( &ETHERNET_PHY_0_desc, &phy_link_state );

        if( phy_link_state == pdTRUE )
        {
            /* Wait for the Ethernet MAC interrupt to indicate that another packet
             * has been received.  The task notification is used in a similar way to a
             * counting semaphore to count Rx events, but is a lot more efficient than
             * a semaphore. */
            ulTaskNotifyTake( pdFALSE, pdMS_TO_TICKS( 1000 ) );

            /* See how much data was received.  Here it is assumed ReceiveSize() is
             * a peripheral driver function that returns the number of bytes in the
             * received Ethernet frame. */
            xBytesReceived = mac_async_read_len( &ETHERNET_MAC_0 );
            if( xBytesReceived > 0 )
            {
                /* Allocate a network buffer descriptor that points to a buffer
                 * large enough to hold the received frame.  As this is the simple
                 * rather than efficient example the received data will just be copied
                 * into this buffer. */
                pxBufferDescriptor = pxGetNetworkBufferWithDescriptor( xBytesReceived, 0 );

                if( pxBufferDescriptor != NULL )
                {
                    /* pxBufferDescriptor->pucEthernetBuffer now points to an Ethernet
                     * buffer large enough to hold the received data.  Copy the
                     * received data into pcNetworkBuffer->pucEthernetBuffer.  Here it
                     * is assumed ReceiveData() is a peripheral driver function that
                     * copies the received data into a buffer passed in as the function's
                     * parameter.  Remember! While is is a simple robust technique -
                     * it is not efficient.  An example that uses a zero copy technique
                     * is provided further down this page. */
                    xBytesRead = mac_async_read( &ETHERNET_MAC_0, pxBufferDescriptor->pucEthernetBuffer, xBytesReceived );
                    pxBufferDescriptor->xDataLength = xBytesRead;

                    /* See if the data contained in the received Ethernet frame needs
                     * to be processed.  NOTE! It is preferable to do this in
                     * the interrupt service routine itself, which would remove the need
                     * to unblock this task for packets that don't need processing. */
                    if( eConsiderFrameForProcessing( pxBufferDescriptor->pucEthernetBuffer )
                        == eProcessBuffer )
                    {
                        /* The event about to be sent to the TCP/IP is an Rx event. */
                        xRxEvent.eEventType = eNetworkRxEvent;

                        /* pvData is used to point to the network buffer descriptor that
                         * now references the received data. */
                        xRxEvent.pvData = ( void * ) pxBufferDescriptor;

                        /* Send the data to the TCP/IP stack. */
                        if( xSendEventStructToIPTask( &xRxEvent, 0 ) == pdFALSE )
                        {
                            /* The buffer could not be sent to the IP task so the buffer
                             * must be released. */
                            vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );

                            /* Make a call to the standard trace macro to log the
                             * occurrence. */
                            iptraceETHERNET_RX_EVENT_LOST();
                        }
                        else
                        {
                            /* The message was successfully sent to the TCP/IP stack.
                             * Call the standard trace macro to log the occurrence. */
                            iptraceNETWORK_INTERFACE_RECEIVE();
                        }
                    }
                    else
                    {
                        /* The Ethernet frame can be dropped, but the Ethernet buffer
                         * must be released. */
                        vReleaseNetworkBufferAndDescriptor( pxBufferDescriptor );
                    }
                }
                else
                {
                    /* The event was lost because a network buffer was not available.
                     * Call the standard trace macro to log the occurrence. */
                    iptraceETHERNET_RX_EVENT_LOST();
                }
            }
        }
        else
        {
            phy_link_reset();
        }
    }
}



BaseType_t xNetworkInterfaceInitialise( void )
{
    /*
     * Perform the hardware specific network initialization here.  Typically
     * that will involve using the Ethernet driver library to initialize the
     * Ethernet (or other network) hardware, initialize DMA descriptors, and
     * perform a PHY auto-negotiation to obtain a network link.
     *
     * This example assumes InitialiseNetwork() is an Ethernet peripheral driver
     * library function that returns 0 if the initialization fails.
     */

    if( xEMACTaskHandle == NULL )
    {
        /* ASF4 initializes the MAC and PHY in the atmel startup routine,
         * which should be called before initializing Ethernet, like so:
         *
         * ETHERNET_MAC_0_CLOCK_init();
         * mac_async_init(&ETHERNET_MAC_0, GMAC);
         * ETHERNET_MAC_0_PORT_init();
         *
         * mac_async_enable(&ETHERNET_MAC_0);
         * ethernet_phy_init(&ETHERNET_PHY_0_desc, &ETHERNET_MAC_0, CONF_ETHERNET_PHY_0_IEEE8023_MII_PHY_ADDRESS);
         #if CONF_ETHERNET_PHY_0_IEEE8023_MII_CONTROL_REG0_SETTING == 1
         *  ethernet_phy_write_reg(&ETHERNET_PHY_0_desc, MDIO_REG0_BMCR, CONF_ETHERNET_PHY_0_IEEE8023_MII_CONTROL_REG0);
         #endif */

        /* The following is to use a 25 MHz crystal input with the KSZ8081RND */
        #if ( defined( KSZ8081RND ) && KSZ8081RND == 1 )
            uint16_t phy_reg_pcr2_val = 0;
            ethernet_phy_read_reg( &ETHERNET_PHY_0_desc, KSZ8081RNA_PCR2, &phy_reg_pcr2_val );
            phy_reg_pcr2_val = phy_reg_pcr2_val & ( 0x1 << 7 );
            ethernet_phy_write_reg( &ETHERNET_PHY_0_desc, KSZ8081RNA_PCR2, phy_reg_pcr2_val );
        #endif

        /* Enable and initialize interrupts */
        mac_async_disable_irq( &ETHERNET_MAC_0 );
        int32_t xResult = mac_async_register_callback( &ETHERNET_MAC_0, MAC_ASYNC_RECEIVE_CB, ( FUNC_PTR ) xRxCallback );
		NVIC_SetPriority( GMAC_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - configPRIO_BITS) );
        mac_async_enable_irq( &ETHERNET_MAC_0 );
        configASSERT( xResult == ERR_NONE );
        /* Create event handler tasks */
        phy_link_reset();

        xTaskCreate( prvEMACDeferredInterruptHandlerTask, /* Function that implements the task. */
                     "EMACInt",                           /* Text name for the task. */
                     256,                                 /* Stack size in words, not bytes. */
                     ( void * ) 1,                        /* Parameter passed into the task. */
                     configMAX_PRIORITIES - 1,            /* Priority at which the task is created. */
                     &xEMACTaskHandle );                  /* Used to pass out the created task's handle. */

        configASSERT( xEMACTaskHandle );
    }

    return phy_link_state;
}


BaseType_t xNetworkInterfaceOutput( NetworkBufferDescriptor_t * const pxDescriptor,
                                    BaseType_t xReleaseAfterSend )
{
    /* Simple network interfaces (as opposed to more efficient zero copy network
     * interfaces) just use Ethernet peripheral driver library functions to copy
     * data from the FreeRTOS+TCP buffer into the peripheral driver's own buffer.
     * This example assumes SendData() is a peripheral driver library function that
     * takes a pointer to the start of the data to be sent and the length of the
     * data to be sent as two separate parameters.  The start of the data is located
     * by pxDescriptor->pucEthernetBuffer.  The length of the data is located
     * by pxDescriptor->xDataLength. */
    phy_rst = ethernet_phy_get_link_status( &ETHERNET_PHY_0_desc, &phy_link_state );

    if( phy_link_state == pdTRUE )
    {
        mac_async_write( &ETHERNET_MAC_0, pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength );

        /* Call the standard trace macro to log the send event. */
        iptraceNETWORK_INTERFACE_TRANSMIT();

        if( xReleaseAfterSend != pdFALSE )
        {
            /* It is assumed SendData() copies the data out of the FreeRTOS+TCP Ethernet
             * buffer.  The Ethernet buffer is therefore no longer needed, and must be
             * freed for re-use. */
            vReleaseNetworkBufferAndDescriptor( pxDescriptor );
        }
    }

    return pdTRUE;
}

void xRxCallback( void )
{
    /*if( ( ( ulStatus & GMAC_RSR_REC ) != 0 ) && ( xEMACTaskHandle != NULL ) ) */
    /*{ */
    /* Only an RX interrupt can wakeup prvEMACHandlerTask. */
    vTaskNotifyGiveFromISR( xEMACTaskHandle, 0 );
    /*} */
}
