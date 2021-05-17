/*
 * app_utils.c
 *
 * Created: 5/16/2021 10:57:34 PM
 *  Author: Marc-Antoine
 */ 

#include "app_utils.h"




#if ( defined( ipconfigSUPPORT_OUTGOING_PINGS ) && ( ipconfigSUPPORT_OUTGOING_PINGS == 1 ) )

#include "queue.h"
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"



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
#endif