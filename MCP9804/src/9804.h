/*
 * 9804.h
 *
 *  Created on: Nov 18, 2013
 *      Author: dthedens
 */


#ifndef _9804_H_
#define _9804_H_

#include "board.h"

// given A0 A1 A2 are tied to ground
#define ADDRESS 0x30  // 0b0011000w/r
/* 100Kbps I2C bit-rate */
#define I2C_BITRATE             (100000)

/** 7-bit and 10-bit I2C addresses */
#define I2C_ADDR_7BIT     (0x48)
#define I2C_ADDR_10BIT    (0x2CA)

/* Enable the following definition to build this example for interrupt mode */
#define INTERRUPTMODE

// 9804 internal register definition
typedef enum RegisterDefinition
{
	ReservedForFuture = 0x0,
	Configuration,
	AlertUpper,
	AlertLower,
	Critical,
	Temperature,
	ManufactureID,
	DeviceID,
	Resoulution
} registers_t;

unsigned char lastRegister;

// only one I2C on the LPC800
void InitI2c();

void setupI2CMaster(void);
ErrorCode_t readRegister(registers_t which, uint8_t *buffer);
ErrorCode_t writeRegister(registers_t which, uint8_t *buffer);
float GetTemperatureFloat();
uint32_t GetTemperatureInt();

#endif /* 9804_H_ */
