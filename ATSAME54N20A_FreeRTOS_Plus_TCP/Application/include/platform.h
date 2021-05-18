/*
 * platform.h
 *
 * Created: 4/11/2021 2:01:00 PM
 *  Author: Marc-Antoine
 */ 
#include <stdbool.h>
#include <stdint.h>

#ifndef PLATFORM_H_
#define PLATFORM_H_

#define DEVICE_NAME "ATSAME54"
#define DEVICE_NAME_UNIQUE cGetDeviceNameUnique()
#define DEVICE_UID_LENGTH_BYTES 16
#define DEVICE_NAME_UNIQUE_LENGTH_BYTES ( ( uint8_t ) ( sizeof( DEVICE_NAME ) + ( 2 * DEVICE_UID_LENGTH_BYTES ) + 2 ) )




void vToggleLED();
void vWritePin(const uint8_t pin, const bool level);
void vHardwareInit();

void genRand32(uint32_t* randValue);

void vValidateConfig();

/*
 * Write "len" bytes of "data" to "address".
 */
int32_t lI2CWrite( int16_t address, const uint8_t * data, const uint16_t len );

/*
 * Read "len" bytes from "address" into "buf".
 */
int32_t lI2CRead( int16_t address, const uint8_t * buf, const uint16_t len );


uint16_t uGetBaudRegisterValue(uint32_t baudrate);

char * cGetDeviceNameUnique();



#endif /* PLATFORM_H_ */