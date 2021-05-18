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


/* Atmel ASF includes */
#include "hal_mac_async.h"
#include "hpl_gmac_config.h"
#include "hal_gpio.h"
#include "ethernet_phy.h"
#include "ieee8023_mii_standard_config.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+TCP includes */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_IP_Private.h"
#include "NetworkBufferManagement.h"
#include "phyHandling.h"




/***********************************************/
/*           Configuration variables           */
/***********************************************/

/* Set to 1 is PHY part number and crystal frequency match. */
#define KSZ8081RND_25MHZ    0

/* Make sure someone takes care of the CRC calculation */
#if ( (CONF_GMAC_NCFGR_RXCOEN == 0 ) && ( ipconfigDRIVER_INCLUDED_RX_IP_CHECKSUM == 1 ) )
	#error Receive CRC offloading should be enabled.
#endif
#if ( ( CONF_GMAC_DCFGR_TXCOEN == 0 ) && ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM == 1 ) )
	#error Transmit CRC offloading should be enabled.
#endif

#if ( defined( ipconfigUSE_LLMNR ) && ( ipconfigUSE_LLMNR == 1 ) )
	static const uint8_t ucLLMNR_MAC_address[] = { 0x01, 0x00, 0x5E, 0x00, 0x00, 0xFC };
#endif


/***********************************************/
/*              FreeRTOS variables             */
/***********************************************/

/* Copied from FreeRTOS_IP.c */
#define ipCORRECT_CRC           0xffffU

/* Also copied from FreeRTOS_IP.c */
/** @brief If ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES is set to 1, then the Ethernet
 * driver will filter incoming packets and only pass the stack those packets it
 * considers need processing.  In this case ipCONSIDER_FRAME_FOR_PROCESSING() can
 * be #-defined away.  If ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES is set to 0
 * then the Ethernet driver will pass all received packets to the stack, and the
 * stack must do the filtering itself.  In this case ipCONSIDER_FRAME_FOR_PROCESSING
 * needs to call eConsiderFrameForProcessing.
 */
#if ipconfigETHERNET_DRIVER_FILTERS_FRAME_TYPES == 0
    #define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer )    eConsiderFrameForProcessing( ( pucEthernetBuffer ) )
#else
    #define ipCONSIDER_FRAME_FOR_PROCESSING( pucEthernetBuffer )    eProcessBuffer
#endif

/* Holds the handle of the task used as a deferred interrupt processor.  The
 * handle is used so direct notifications can be sent to the task for all EMAC/DMA
 * related interrupts. */
TaskHandle_t xEMACTaskHandle = NULL;

/* The PING response queue */ 
#if ( defined( ipconfigSUPPORT_OUTGOING_PINGS ) && ( ipconfigSUPPORT_OUTGOING_PINGS == 1 ) )
	QueueHandle_t xPingReplyQueue = NULL;
#endif

void xRxCallback( void );
static void prvEMACDeferredInterruptHandlerTask( void * pvParameters );

/***********************************************/
/*                GMAC variables               */
/***********************************************/

/* The Ethernet MAC instance created by ASF4 */
extern struct mac_async_descriptor ETHERNET_MAC_0;

static void prvGMACInit( void );


/***********************************************/
/*                PHY variables                */
/***********************************************/

/* The Ethernet PHY instance created by ASF4 */
extern struct ethernet_phy_descriptor ETHERNET_PHY_0_desc;

static bool phy_link_state = pdFALSE;

static void prvPHYLinkReset( void );
static void prvPHYInit( void );









/*********************************************************************/
/*                      FreeRTOS+TCP functions                       */
/*********************************************************************/

BaseType_t xNetworkInterfaceInitialise( void )
{
    /*
     * Perform the hardware specific network initialization here.  Typically
     * that will involve using the Ethernet driver library to initialize the
     * Ethernet (or other network) hardware, initialize DMA descriptors, and
     * perform a PHY auto-negotiation to obtain a network link. */

    if( xEMACTaskHandle == NULL )
    {
		/* Initialize MAC and PHY */
		prvGMACInit();
		prvPHYInit();

		/* (Re)set PHY link */
		prvPHYLinkReset();
		
		/* Initialize PING capability */
		#if ( defined( ipconfigSUPPORT_OUTGOING_PINGS ) && ( ipconfigSUPPORT_OUTGOING_PINGS == 1 ) )
			xPingReplyQueue = xQueueCreate(ipconfigPING_QUEUE_SIZE, sizeof(uint16_t));
		#endif
		
        /* Create event handler task */
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


static void prvEMACDeferredInterruptHandlerTask( void * pvParameters )
{
    NetworkBufferDescriptor_t * pxBufferDescriptor;
    size_t xBytesReceived;
    size_t xBytesRead;

	uint16_t xICMPChecksumResult = ipCORRECT_CRC;
	const IPPacket_t * pxIPPacket;


    /* Used to indicate that xSendEventStructToIPTask() is being called because
     * of an Ethernet receive event. */
    IPStackEvent_t xRxEvent;

    for( ; ; )
    {
        ethernet_phy_get_link_status( &ETHERNET_PHY_0_desc, &phy_link_state );

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


					#if ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM == 1 )
					{
						/* the Atmel SAM GMAC peripheral does not support hardware CRC offloading for ICMP packets.
						 * It must therefore be implemented in software. */
						pxIPPacket = ipCAST_CONST_PTR_TO_CONST_TYPE_PTR( IPPacket_t, pxBufferDescriptor->pucEthernetBuffer );
						if (pxIPPacket->xIPHeader.ucProtocol == ( uint8_t ) ipPROTOCOL_ICMP )
						{
							xICMPChecksumResult = usGenerateProtocolChecksum( pxBufferDescriptor->pucEthernetBuffer, pxBufferDescriptor->xDataLength, pdFALSE );
						}
						else
						{
							xICMPChecksumResult = ipCORRECT_CRC; // Reset the result value in case this is not an ICMP packet.
						}
					}
					#endif
					
                    /* See if the data contained in the received Ethernet frame needs
                     * to be processed.  NOTE! It is preferable to do this in
                     * the interrupt service routine itself, which would remove the need
                     * to unblock this task for packets that don't need processing. */
                    if( ipCONSIDER_FRAME_FOR_PROCESSING( pxBufferDescriptor->pucEthernetBuffer ) == eProcessBuffer 
					    && xICMPChecksumResult == ipCORRECT_CRC)
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
            prvPHYLinkReset();
        }
    }
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
    ethernet_phy_get_link_status( &ETHERNET_PHY_0_desc, &phy_link_state );

    if( phy_link_state == pdTRUE )
    {
		
		#if ( ipconfigDRIVER_INCLUDED_TX_IP_CHECKSUM == 1 )
		{
			/* the Atmel SAM GMAC peripheral does not support hardware CRC offloading for ICMP packets.
			 * It must therefore be implemented in software. */
			const IPPacket_t * pxIPPacket = ipCAST_CONST_PTR_TO_CONST_TYPE_PTR( IPPacket_t, pxDescriptor->pucEthernetBuffer );
			if (pxIPPacket->xIPHeader.ucProtocol == ( uint8_t ) ipPROTOCOL_ICMP )
			{
				( void ) usGenerateProtocolChecksum( pxDescriptor->pucEthernetBuffer, pxDescriptor->xDataLength, pdTRUE );
			}
		}
		#endif
		
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
    vTaskNotifyGiveFromISR( xEMACTaskHandle, 0 );
}


/*********************************************************************/
/*                          GMAC functions                           */
/*********************************************************************/

/* Initializes the GMAC peripheral. This function is based on ASF4 GMAC initialization
 * and REPLACES the Atmel START- generated code, typically located in "driver_init.h".
 * It is compatible with modifications made in Atmel START afterwards because the 
 * configuration is saved in "hpl_gmac_config.h". */
static void prvGMACInit()
{
	/* Call the ASF4 generated initialization code */
	/* Initialize GMAC clock */
	hri_mclk_set_AHBMASK_GMAC_bit(MCLK);
	hri_mclk_set_APBCMASK_GMAC_bit(MCLK);
	
	/* Apply Atmel START base configuration */
	mac_async_init(&ETHERNET_MAC_0, GMAC);
	
	/* Initialize GMAC port */
	gpio_set_pin_function(GPIO(GPIO_PORTA, 20), PINMUX_PA20L_GMAC_GMDC);
	gpio_set_pin_function(GPIO(GPIO_PORTA, 21), PINMUX_PA21L_GMAC_GMDIO);
	gpio_set_pin_function(GPIO(GPIO_PORTA, 13), PINMUX_PA13L_GMAC_GRX0);
	gpio_set_pin_function(GPIO(GPIO_PORTA, 12), PINMUX_PA12L_GMAC_GRX1);
	gpio_set_pin_function(GPIO(GPIO_PORTC, 20), PINMUX_PC20L_GMAC_GRXDV);
	gpio_set_pin_function(GPIO(GPIO_PORTA, 18), PINMUX_PA18L_GMAC_GTX0);
	gpio_set_pin_function(GPIO(GPIO_PORTA, 19), PINMUX_PA19L_GMAC_GTX1);
	gpio_set_pin_function(GPIO(GPIO_PORTA, 14), PINMUX_PA14L_GMAC_GTXCK);
	gpio_set_pin_function(GPIO(GPIO_PORTA, 17), PINMUX_PA17L_GMAC_GTXEN);
	
	/* Set GMAC Filtering */
	struct mac_async_filter mac_filter;
	memcpy(mac_filter.mac, ipLOCAL_MAC_ADDRESS, ipMAC_ADDRESS_LENGTH_BYTES);
	mac_filter.tid_enable = false;
	mac_async_set_filter(&ETHERNET_MAC_0, 0, &mac_filter);
	#if ( defined( ipconfigUSE_LLMNR ) && ( ipconfigUSE_LLMNR == 1 ) )
	{
		/* Set hardware filter for LLMNR capability. */
		memcpy(mac_filter.mac, ucLLMNR_MAC_address, ipMAC_ADDRESS_LENGTH_BYTES);
		/* LLMNR requires responders to listen to both TCP and UDP protocols. */
		mac_filter.tid_enable = false;
		mac_async_set_filter(&ETHERNET_MAC_0, 1, &mac_filter);
	}
	#endif
	
	
	
	/* Set GMAC interrupt priority to be compatible with FreeRTOS API */
	NVIC_SetPriority( GMAC_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY >> (8 - configPRIO_BITS) );
	
	/* Register callbacks */
	mac_async_disable_irq( &ETHERNET_MAC_0 );
	mac_async_register_callback( &ETHERNET_MAC_0, MAC_ASYNC_RECEIVE_CB, ( FUNC_PTR ) xRxCallback );
	mac_async_enable_irq( &ETHERNET_MAC_0 );
}


/*********************************************************************/
/*                           PHY functions                           */
/*********************************************************************/


/* Initializes the PHY hardware. Based on ASF4 generated code. */
static void prvPHYInit()
{
	mac_async_enable(&ETHERNET_MAC_0);
	ethernet_phy_init(&ETHERNET_PHY_0_desc, &ETHERNET_MAC_0, CONF_ETHERNET_PHY_0_IEEE8023_MII_PHY_ADDRESS);
	#if CONF_ETHERNET_PHY_0_IEEE8023_MII_CONTROL_REG0_SETTING == 1
		ethernet_phy_write_reg(&ETHERNET_PHY_0_desc, MDIO_REG0_BMCR, CONF_ETHERNET_PHY_0_IEEE8023_MII_CONTROL_REG0);
	#endif /* CONF_ETHERNET_PHY_0_IEEE8023_MII_CONTROL_REG0_SETTING */
	
	/* By default, the KSZ8081RND expects a 50 MHz crystal, unlike the KSZ8081RNA */
	#if ( defined( KSZ8081RND_25MHZ ) && KSZ8081RND_25MHZ == 1 )
		uint16_t phy_reg_pcr2_val = 0;
		ethernet_phy_read_reg( &ETHERNET_PHY_0_desc, KSZ8081RNA_PCR2, &phy_reg_pcr2_val );
		phy_reg_pcr2_val = phy_reg_pcr2_val & ( 0x1 << 7 );
		ethernet_phy_write_reg( &ETHERNET_PHY_0_desc, KSZ8081RNA_PCR2, phy_reg_pcr2_val );
	#endif /* ( defined( KSZ8081RND_25MHZ ) && KSZ8081RND_25MHZ == 1 ) */
}


/* Start a new autonegotiation on the PHY link and wait until link is up.
 * Will also start a new negotiation every 5 seconds if link is still down by then. */
static void prvPHYLinkReset()
{
    do
    {
        uint16_t retry_count = 0;

        /* Restart an auto-negotiation */
        ethernet_phy_restart_autoneg( &ETHERNET_PHY_0_desc );

        /* Wait for PHY link up */
        for(retry_count = 0; retry_count < 5000 && !phy_link_state; retry_count++)
        {
            ethernet_phy_get_link_status( &ETHERNET_PHY_0_desc, &phy_link_state );
			vTaskDelay( 1 );
        }
    } while( phy_link_state != pdTRUE );
}