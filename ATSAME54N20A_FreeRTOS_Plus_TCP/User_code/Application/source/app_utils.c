/*
 * app_utils.c
 *
 * Created: 4/10/2021 1:44:05 AM
 *  Author: Marc-Antoine
 */ 

#include "app_utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "platform.h"
#include "cdcdf_acm.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"


#if ( defined( ipconfigSUPPORT_OUTGOING_PINGS ) && ( ipconfigSUPPORT_OUTGOING_PINGS == 1 ) )
extern QueueHandle_t xPingReplyQueue;

BaseType_t vSendPing( const uint8_t * ipAddress )
{
uint16_t usRequestSequenceNumber, usReplySequenceNumber;
uint32_t ulIPAddress;

	BaseType_t xReturn = pdFAIL;

	if (xPingReplyQueue != NULL)
	{
		/* Convert the IP Address into the required 32-bit format. */
		ulIPAddress = FreeRTOS_inet_addr_quick( ipAddress[0], ipAddress[1], ipAddress[2], ipAddress[3] );

		/* Send a ping containing 8 data bytes.  Wait (in the Blocked state) a
		maximum of 100ms for a network buffer into which the generated ping request
		can be written and sent. */
		usRequestSequenceNumber = FreeRTOS_SendPingRequest( ulIPAddress, 8, 100 / portTICK_PERIOD_MS );

		if( usRequestSequenceNumber == pdFAIL )
		{
			/* The ping could not be sent because a network buffer could not be
			obtained within 100ms of FreeRTOS_SendPingRequest() being called. */
		}
		else
		{
			/* The ping was sent.  Wait 200ms for a reply.  The sequence number from
			each reply is sent from the vApplicationPingReplyHook() on the
			xPingReplyQueue queue (this is not standard behaviour, but implemented in
			the example function above).  It is assumed the queue was created before
			this function was called! */
			if( xQueueReceive( xPingReplyQueue,
							   &usReplySequenceNumber,
							   200 / portTICK_PERIOD_MS ) == pdPASS )
			{
				/* A ping reply was received.  Was it a reply to the ping just sent? */
				if( usRequestSequenceNumber == usReplySequenceNumber )
				{
					/* This was a reply to the request just sent. */
					xReturn = pdPASS;
				}
			}
		}
	}
	
	return xReturn;
}
#endif /* ( defined( ipconfigSUPPORT_OUTGOING_PINGS ) && ( ipconfigSUPPORT_OUTGOING_PINGS == 1 ) ) */

BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )
{
	*pulNumber = genRand32();
	
	return pdTRUE;
}


void vLoggingPrintf( const char * pcFormatString, ...)
{
	//static char message[256];
	//memset(message, 0, 256);
	//
	//va_list ap;	
	//sprintf(message, pcFormatString, ap);
//
	//uint16_t i = 0;
	//while (message[i] != '\0')
	//{
		//i += 1;
	//}
	//message[i++] = '\r';
	//message[i++] = '\n';
//
	//cdcdf_acm_write((uint8_t *)message, i);
}

/* Stubs of standard IO read and write. Not used, can be replaced if wanted. */
void _read(int fd, void * buf, size_t nbyte )
{
	
}

void _write(int fd, const void * buf, size_t nbyte )
{
	
}

uint32_t ulGetTimeMs( uint32_t initial_timestamp )
{
    TickType_t xTickCount = 0;
    uint32_t ulTimeMs = 0UL;

    /* Get the current tick count. */
    xTickCount = xTaskGetTickCount();

    /* Convert the ticks to milliseconds. */
    ulTimeMs = ( uint32_t ) xTickCount * (1000 / configTICK_RATE_HZ);

    /* Reduce ulGlobalEntryTimeMs from obtained time so as to always return the
     * elapsed time in the application. */
    ulTimeMs = ( uint32_t ) ( ulTimeMs - initial_timestamp );

    return ulTimeMs;
}