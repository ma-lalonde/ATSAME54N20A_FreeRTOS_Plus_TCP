/*
 * platform.c
 *
 * Created: 4/11/2021 2:01:12 PM
 *  Author: Marc-Antoine
 */ 

#include "platform.h"

#include <stdio.h>
#include <string.h>

#include "atmel_start.h"
#include "atmel_start_pins.h"

#include "FreeRTOSConfig.h"

/* Device UID / SN registers. Refer to datasheet page 59 section 9.6 */
#define UID_WORD0 0x008061FC
#define UID_WORD1 0x00806010
#define UID_WORD2 0x00806014
#define UID_WORD3 0x00806018

static char deviceNameUnique[DEVICE_NAME_UNIQUE_LENGTH_BYTES];

extern struct rand_sync_desc RAND_0;


void vToggleLED()
{
	gpio_toggle_pin_level(LED);
}


void vWritePin(const uint8_t pin, const bool level)
{
	gpio_set_pin_level(pin, level);
	
}


void vHardwareInit()
{
	atmel_start_init();
	
	rand_sync_enable(&RAND_0);
}


void genRand32(uint32_t* randValue)
{
	rand_sync_read_buf32(&RAND_0, randValue, 1);
}


void vValidateConfig()
{
	/* In file "hpl_usb.c" */
	//configASSERT( CONF_USB_D_CLK_SRC == USB_CLK_SRC_DPLL);
}


char * cGetDeviceNameUnique()
{
	if (deviceNameUnique[0] == 0)
	{
		char uid_byte_hex[9];
		
		strcpy(deviceNameUnique, DEVICE_NAME "_");
		sprintf(uid_byte_hex, "%08lX", *((uint32_t *)UID_WORD0));
		strcat(deviceNameUnique, uid_byte_hex);
		sprintf(uid_byte_hex, "%08lX", *((uint32_t *)UID_WORD1));
		strcat(deviceNameUnique, uid_byte_hex);
		sprintf(uid_byte_hex, "%08lX", *((uint32_t *)UID_WORD2));
		strcat(deviceNameUnique, uid_byte_hex);
		sprintf(uid_byte_hex, "%08lX", *((uint32_t *)UID_WORD3));
		strcat(deviceNameUnique, uid_byte_hex);
	}

	return deviceNameUnique;
}
