/*
 * led.c
 *
 * Created: 4/9/2021 11:11:50 PM
 *  Author: Marc-Antoine
 */ 

#include "led.h"

#include "atmel_start_pins.h"

#include "FreeRTOS.h"
#include "task.h"

#include "platform.h"


static TaskHandle_t LED_x1HzBlinkHandle = NULL;

static void LED_v1HzBlinkTask( void * pvParameters );

void LED_vStart1HzBlinkTask()
{
	if (LED_x1HzBlinkHandle == NULL)
	{
		xTaskCreate(LED_v1HzBlinkTask,       	// Function that implements the task.
					"LED_1HzBlink",     		// Text name for the task.
					configMINIMAL_STACK_SIZE,	// Stack size in words, not bytes.
					( void * ) 1,    			// Parameter passed into the task.
					tskIDLE_PRIORITY + 1,		// Priority at which the task is created.
					&LED_x1HzBlinkHandle );		// Used to pass out the created task's handle.
	}
}


void LED_v1HzBlinkTask( void * pvParameters )
{
	for(;;)
	{
		vWritePin(LED, 1);
		vTaskDelay(5);
		vWritePin(LED, 0);
		vTaskDelay(995);
	}
}