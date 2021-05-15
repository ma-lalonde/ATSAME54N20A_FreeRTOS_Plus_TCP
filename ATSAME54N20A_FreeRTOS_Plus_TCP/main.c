#include <atmel_start.h>

#include "FreeRTOS.h"
#include "task.h"

#include "FreeRTOS_IP.h"

#include "platform.h"
#include "led.h"
#include "ethernet_config.h"



int main(void)
{
	/* Initializes MCU, drivers and middleware */
	vHardwareInit();
	
	FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
	
	LED_vStart1HzBlinkTask();
	

	
	vTaskStartScheduler();
	/* Replace with your application code */
	while (1) {
	}
}

