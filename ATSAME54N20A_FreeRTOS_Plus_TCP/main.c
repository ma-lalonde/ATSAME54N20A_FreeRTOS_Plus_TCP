#include <atmel_start.h>

#include "FreeRTOS.h"
#include "task.h"

#include "FreeRTOS_IP.h"

#include "platform.h"
#include "app_config.h"
#include "app_utils.h"

/* The MAC address array is not declared const as the MAC address will
normally be read from an EEPROM and not hard coded (in real deployed
applications).*/
const uint8_t ucMACAddress[ 6 ] = { configMAC_ADDR0, configMAC_ADDR1, configMAC_ADDR2, configMAC_ADDR3, configMAC_ADDR4, configMAC_ADDR5 };
const uint8_t ucIPAddress[ 4 ] = { configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3 };
const uint8_t ucNetMask[ 4 ] = { configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3 };
const uint8_t ucGatewayAddress[ 4 ] = { configGATEWAY_ADDR0, configGATEWAY_ADDR1, configGATEWAY_ADDR2, configGATEWAY_ADDR3 };
const uint8_t ucDNSServerAddress[ 4 ] = { configDNS_SERVER_ADDR0, configDNS_SERVER_ADDR1, configDNS_SERVER_ADDR2, configDNS_SERVER_ADDR3 };

TaskHandle_t xPingHandle = NULL;

void vPingTestTask(void * pvParameters);

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	vHardwareInit();
	
	FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
	
	
	//xTaskCreate( vPingTestTask,				/* Function that implements the task. */
				//"PingTask",                 /* Text name for the task. */
				//256,                        /* Stack size in words, not bytes. */
				//( void * ) 1,               /* Parameter passed into the task. */
				//configMAX_PRIORITIES - 3,   /* Priority at which the task is created. */
				//&xPingHandle );             /* Used to pass out the created task's handle. */

	
	vTaskStartScheduler();
	/* Replace with your application code */
	while (1) {
	}
}

void vPingTestTask(void * pvParameters)
{
	uint8_t ipAddress[4] = {192, 168, 1, 133};
	
	for(;;)
	{
		vSendPing( ipAddress );
		vTaskDelay( 5000 );
		
	}
}