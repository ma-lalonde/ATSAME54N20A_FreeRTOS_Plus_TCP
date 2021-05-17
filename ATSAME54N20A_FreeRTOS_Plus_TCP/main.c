#include <atmel_start.h>

#include "FreeRTOS.h"
#include "task.h"

#include "FreeRTOS_IP.h"

#include "platform.h"
#include "led.h"
#include "ethernet_config.h"
#include "app_utils.h"

TaskHandle_t xPingHandle = NULL;

void vPingTestTask(void * pvParameters);

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	vHardwareInit();
	
	FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
	
	LED_vStart1HzBlinkTask();
	
	
	xTaskCreate( vPingTestTask,				/* Function that implements the task. */
				"PingTask",                 /* Text name for the task. */
				256,                        /* Stack size in words, not bytes. */
				( void * ) 1,               /* Parameter passed into the task. */
				configMAX_PRIORITIES - 3,   /* Priority at which the task is created. */
				&xPingHandle );             /* Used to pass out the created task's handle. */

	
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