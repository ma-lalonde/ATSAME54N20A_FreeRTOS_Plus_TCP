/*
 * app_utils.h
 *
 * Created: 5/6/2021 4:22:21 PM
 *  Author: Marc-Antoine
 */ 


#ifndef APP_UTILS_H_
#define APP_UTILS_H_

#include "FreeRTOS.h"
#include "task.h"

#if ( defined( ipconfigSUPPORT_OUTGOING_PINGS ) && ( ipconfigSUPPORT_OUTGOING_PINGS == 1 ) )
BaseType_t vSendPing( const uint8_t * ipAddress );
#endif /* ( defined( ipconfigSUPPORT_OUTGOING_PINGS ) && ( ipconfigSUPPORT_OUTGOING_PINGS == 1 ) ) */

/**
 * @brief The timer query function provided to the MQTT context.
 *
 * @return Time in milliseconds.
 */
uint32_t ulGetTimeMs( uint32_t initial_timestamp );

#endif /* APP_UTILS_H_ */