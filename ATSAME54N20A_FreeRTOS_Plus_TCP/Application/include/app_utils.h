/*
 * app_utils.h
 *
 * Created: 5/16/2021 10:57:24 PM
 *  Author: Marc-Antoine
 */ 


#ifndef APP_UTILS_H_
#define APP_UTILS_H_

#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOSIPConfig.h"

#if ( defined( ipconfigSUPPORT_OUTGOING_PINGS ) && ( ipconfigSUPPORT_OUTGOING_PINGS == 1 ) )
	BaseType_t vSendPing( const uint8_t * ipAddress );
#endif



#endif /* APP_UTILS_H_ */