/*
 * platform.h
 *
 * Created: 4/11/2021 2:01:00 PM
 *  Author: Marc-Antoine
 */ 


#ifndef PLATFORM_H_
#define PLATFORM_H_

#include <stdbool.h>
#include <stdint.h>


#define DEVICE_NAME						"ATSAME54"
#define DEVICE_NAME_UNIQUE				pcGetDeviceNameUnique()
#define DEVICE_UID_LENGTH_BYTES			16
#define DEVICE_NAME_UNIQUE_LENGTH_BYTES ( ( uint8_t ) ( sizeof( DEVICE_NAME ) + ( 2 * DEVICE_UID_LENGTH_BYTES ) + 2 ) )


void vHardwareInit();

uint32_t genRand32();

void vValidateConfig();

uint16_t uGetBaudRegisterValue(uint32_t baudrate);

char * pcGetDeviceNameUnique();


void vGMACInit();
void vGMACDeinit();

void vUSBInit();
void vUSBDeinit();



#endif /* PLATFORM_H_ */