/*
 * platform.c
 *
 * Created: 4/11/2021 2:01:12 PM
 *  Author: Marc-Antoine
 */ 

#include "platform.h"

#include <stdio.h>
#include <string.h>

#include "hal_init.h"
#include "hal_gpio.h"
#include "hal_rand_sync.h"
#include "hal_mac_async.h"
#include "hal_usb_device.h"
#include "cdcdf_acm.h"
#include "cdcdf_acm_desc.h"


#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Device UID / SN registers. Refer to datasheet page 59 section 9.6 */
#define UID_WORD0 0x008061FC
#define UID_WORD1 0x00806010
#define UID_WORD2 0x00806014
#define UID_WORD3 0x00806018

static char deviceNameUnique[DEVICE_NAME_UNIQUE_LENGTH_BYTES];


/************************************************************/



/* RNG doesn't need a semaphore because it is synchronous */
struct rand_sync_desc RNG;

/* ETH_MAC doesn't need a semaphore because it is garanteed to be used only be the FreeRTOS IP task. */
struct mac_async_descriptor ETH_MAC;


/**************************************************************/



static void vRNGInit();
//static void vRNGDeinit();






/************************************************************/


void vHardwareInit()
{
	init_mcu();
	vUSBInit();
	vRNGInit();
}

uint32_t genRand32()
{
	return rand_sync_read32(&RNG);
}


void vValidateConfig()
{
	/* In file "hpl_usb.c" */
	//configASSERT( CONF_USB_D_CLK_SRC == USB_CLK_SRC_DPLL);
}



char * pcGetDeviceNameUnique()
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






/******************************************************/



void vGMACInit()
{
	/* Init clocks */
	hri_mclk_set_AHBMASK_GMAC_bit(MCLK);
	hri_mclk_set_APBCMASK_GMAC_bit(MCLK);
	/* Init peripheral */
	mac_async_init(&ETH_MAC, GMAC);
	/* Init GPIO */
	gpio_set_pin_function(PIN_PA20, PINMUX_PA20L_GMAC_GMDC);
	gpio_set_pin_function(PIN_PA21, PINMUX_PA21L_GMAC_GMDIO);
	gpio_set_pin_function(PIN_PA13, PINMUX_PA13L_GMAC_GRX0);
	gpio_set_pin_function(PIN_PA12, PINMUX_PA12L_GMAC_GRX1);
	gpio_set_pin_function(PIN_PC20, PINMUX_PC20L_GMAC_GRXDV);
	gpio_set_pin_function(PIN_PA18, PINMUX_PA18L_GMAC_GTX0);
	gpio_set_pin_function(PIN_PA19, PINMUX_PA19L_GMAC_GTX1);
	gpio_set_pin_function(PIN_PA14, PINMUX_PA14L_GMAC_GTXCK);
	gpio_set_pin_function(PIN_PA17, PINMUX_PA17L_GMAC_GTXEN);
}

void vGMACDeinit()
{
	/* DeInit GPIO */
	gpio_set_pin_function(PIN_PA20, GPIO_PIN_FUNCTION_OFF);
	gpio_set_pin_function(PIN_PA21, GPIO_PIN_FUNCTION_OFF);
	gpio_set_pin_function(PIN_PA13, GPIO_PIN_FUNCTION_OFF);
	gpio_set_pin_function(PIN_PA12, GPIO_PIN_FUNCTION_OFF);
	gpio_set_pin_function(PIN_PC20, GPIO_PIN_FUNCTION_OFF);
	gpio_set_pin_function(PIN_PA18, GPIO_PIN_FUNCTION_OFF);
	gpio_set_pin_function(PIN_PA19, GPIO_PIN_FUNCTION_OFF);
	gpio_set_pin_function(PIN_PA14, GPIO_PIN_FUNCTION_OFF);
	gpio_set_pin_function(PIN_PA17, GPIO_PIN_FUNCTION_OFF);
	/* DeInit peripheral */
	mac_async_deinit(&ETH_MAC);
	/* DeInit clocks */
	hri_mclk_clear_AHBMASK_GMAC_bit(MCLK);
	hri_mclk_clear_APBCMASK_GMAC_bit(MCLK);
}

/**************************************************************/


static uint8_t single_desc_bytes[] = { CDCD_ACM_DESCES_LS_FS };
#define CDCD_ECHO_BUF_SIZ CONF_USB_CDCD_ACM_DATA_BULKIN_MAXPKSZ
static struct usbd_descriptors single_desc[] = {{single_desc_bytes, single_desc_bytes + sizeof(single_desc_bytes)}};
/** Ctrl endpoint buffer */
static uint8_t ctrl_buffer[64];


/**
 * \brief Callback invoked when bulk OUT data received
 */
static bool usb_device_cb_bulk_out(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
	//cdcdf_acm_write((uint8_t *)usbd_cdc_buffer, count);

	/* No error. */
	return false;
}

/**
 * \brief Callback invoked when bulk IN data received
 */
static bool usb_device_cb_bulk_in(const uint8_t ep, const enum usb_xfer_code rc, const uint32_t count)
{
	/* Echo data. */
	//cdcdf_acm_read((uint8_t *)usbd_cdc_buffer, sizeof(usbd_cdc_buffer));

	/* No error. */
	return false;
}

/**
 * \brief Callback invoked when Line State Change
 */
static bool usb_device_cb_state_c(usb_cdc_control_signal_t state)
{
	if (state.rs232.DTR) {
		/* Callbacks must be registered after endpoint allocation */
		cdcdf_acm_register_callback(CDCDF_ACM_CB_READ, (FUNC_PTR)usb_device_cb_bulk_out);
		cdcdf_acm_register_callback(CDCDF_ACM_CB_WRITE, (FUNC_PTR)usb_device_cb_bulk_in);
		/* Start Rx */
		//cdcdf_acm_read((uint8_t *)usbd_cdc_buffer, sizeof(usbd_cdc_buffer));
	}

	/* No error. */
	return false;
}




void vUSBInit()
{
	/* Init clocks */
	hri_gclk_write_PCHCTRL_reg(GCLK, USB_GCLK_ID, CONF_GCLK_USB_SRC | GCLK_PCHCTRL_CHEN);
	hri_mclk_set_AHBMASK_USB_bit(MCLK);
	hri_mclk_set_APBBMASK_USB_bit(MCLK);
	/* Init peripherals */
	usb_d_init();
	/* Init GPIO */
	gpio_set_pin_direction(	PIN_PA24, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(     PIN_PA24, false);
	gpio_set_pin_pull_mode( PIN_PA24, GPIO_PULL_OFF);
	gpio_set_pin_function(  PIN_PA24, PINMUX_PA24H_USB_DM);
	gpio_set_pin_direction( PIN_PA25, GPIO_DIRECTION_OUT);
	gpio_set_pin_level(     PIN_PA25, false);
	gpio_set_pin_pull_mode( PIN_PA25, GPIO_PULL_OFF);
	gpio_set_pin_function(  PIN_PA25, PINMUX_PA25H_USB_DP);
	
	/* usb stack init */
	usbdc_init(ctrl_buffer);

	/* usbdc_register_funcion inside */
	cdcdf_acm_init();

	usbdc_start(single_desc);
	usbdc_attach();

	cdcdf_acm_register_callback(CDCDF_ACM_CB_STATE_C, (FUNC_PTR)usb_device_cb_state_c);
}

void vUSBDeinit()
{
	usbdc_detach();
	usbdc_stop();
	cdcdf_acm_deinit();
	usbdc_deinit();
	
	/* DeInit GPIO */
	gpio_set_pin_direction( PIN_PA24, GPIO_DIRECTION_OFF);
	gpio_set_pin_level    ( PIN_PA24, false);
	gpio_set_pin_function ( PIN_PA24, GPIO_PIN_FUNCTION_OFF);
	gpio_set_pin_direction( PIN_PA25, GPIO_DIRECTION_OFF);
	gpio_set_pin_level    ( PIN_PA25, false);
	gpio_set_pin_function ( PIN_PA25, GPIO_PIN_FUNCTION_OFF);
	/* Init peripherals */
	usb_d_deinit();
	/* Init clocks */
	hri_mclk_clear_AHBMASK_USB_bit(MCLK);
	hri_mclk_clear_APBBMASK_USB_bit(MCLK);
	hri_gclk_write_PCHCTRL_reg(GCLK, USB_GCLK_ID, CONF_GCLK_USB_SRC & ~GCLK_PCHCTRL_CHEN);
}

/* The USB module requires a GCLK_USB of 48 MHz ~ 0.25% clock
 * for low speed and full speed operation. */
#if (CONF_GCLK_USB_FREQUENCY > (48000000 + 48000000 / 400)) || (CONF_GCLK_USB_FREQUENCY < (48000000 - 48000000 / 400))
#warning USB clock should be 48MHz ~ 0.25% clock, check your configuration!
#endif

/******************************************************/


/******************************************************/

static void vRNGInit()
{
	/* Init clock */
	hri_mclk_set_APBCMASK_TRNG_bit(MCLK);
	/* Init peripheral */
	rand_sync_init(&RNG, TRNG);
	/* Enable peripheral */
	rand_sync_enable(&RNG);
}


//static void vRNGDeinit()
//{
	///* Disable peripheral */
	//rand_sync_disable(&RNG);
	///* DeInit peripheral */
	//rand_sync_deinit(&RNG);
	///* DeInit clock */
	//hri_mclk_clear_APBCMASK_TRNG_bit(MCLK);
//}

/*********************************************************/