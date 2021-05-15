/*
 * platform.c
 *
 * Created: 4/11/2021 2:01:12 PM
 *  Author: Marc-Antoine
 */ 

#include "platform.h"

#include "atmel_start.h"
#include "atmel_start_pins.h"

#include "FreeRTOSConfig.h"

extern struct rand_sync_desc RAND_0;
extern struct i2c_m_sync_desc I2C_0;
extern struct usart_sync_descriptor USART_0;

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
