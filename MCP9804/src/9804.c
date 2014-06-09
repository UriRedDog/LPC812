/*
 * 9804.c
 *
 *  Created on: Nov 18, 2013
 *      Author: dthedens
 */


#include "9804.h"

/* Use a buffer size larger than the expected return value of
   i2c_get_mem_size() for the static I2C handle type */
static uint32_t i2cMasterHandleMEM[0x20];

/* I2C master handle and memory for ROM API */
static I2C_HANDLE_T *i2cHandleMaster;

// address with A0, A1, A2 attached to ground
static uint8_t gAddress = ADDRESS;

volatile ErrorCode_t gErrorCode ;

	I2C_RESULT_T result;
/* Turn on LED to indicate an error */
static void errorI2C(void)
{
	Board_LED_Set(0, true);
	while (1) {}
}


/* Setup I2C handle and parameters */
void setupI2CMaster()
{
	/* Enable I2C clock and reset I2C peripheral - the boot ROM does not
	   do this */
	Chip_I2C_Init();

	/* Perform a sanity check on the storage allocation */
	if (LPC_I2CD_API->i2c_get_mem_size() > sizeof(i2cMasterHandleMEM)) {
		/* Example only: this should never happen and probably isn't needed for
		   most I2C code. */
		errorI2C();
	}

	/* Setup the I2C handle */
	i2cHandleMaster = LPC_I2CD_API->i2c_setup(LPC_I2C_BASE, i2cMasterHandleMEM);
	if (i2cHandleMaster == NULL) {
		errorI2C();
	}

	/* Set I2C bit rate */
	if (LPC_I2CD_API->i2c_set_bitrate(i2cHandleMaster, Chip_Clock_GetSystemClockRate(), I2C_BITRATE) != LPC_OK) {
		errorI2C();
	}
	/* Enable the interrupt for the I2C */
	NVIC_EnableIRQ(I2C_IRQn);
}

/* I2C interrupt callback, called on completion of I2C operation when in
   interrupt mode. Called in interrupt context. */
static void cbI2CComplete(uint32_t err_code, uint32_t n)
{
	gErrorCode = err_code;
}



ErrorCode_t writeRegister(registers_t which, uint8_t *buffer)
{
	uint8_t SendData[4];
	I2C_PARAM_T param;

	SendData[0] = gAddress & 0xFE; // make sure bit 0 is 0
	SendData[1] = (uint8_t)which;
	SendData[2] = *buffer++;
	SendData[3] = *buffer;

	param.num_bytes_send    = 4;
	param.buffer_ptr_send   = SendData;
	param.num_bytes_rec     = 0;
	//param.buffer_ptr_rec    = 0;
	param.stop_flag         = 1;
	param.func_pt           = cbI2CComplete;
	/* Set timeout (much) greater than the transfer length */
	gErrorCode = -1;
	LPC_I2CD_API->i2c_set_timeout(i2cHandleMaster, 10000);
	LPC_I2CD_API->i2c_master_transmit_intr(i2cHandleMaster, &param, &result);
	//gErrorCode = LPC_I2CD_API->i2c_master_tx_rx_poll(i2cHandleMaster, &param, &result);
	while (gErrorCode == -1)
	{
		__WFI();
	}
	return gErrorCode;
}


ErrorCode_t readRegister(registers_t which, uint8_t *buffer)
{
	uint8_t SendData[4];
	I2C_PARAM_T param;
	SendData[0] = gAddress & 0xFE;  // bit 0 is 0
	SendData[1] = (uint8_t)which;

	*buffer = gAddress | 0x1;
	param.num_bytes_send    = 2;
	param.buffer_ptr_send   = SendData;
	param.num_bytes_rec     = 3;
	param.buffer_ptr_rec    = buffer;
	param.stop_flag         = 1;
	param.func_pt           = cbI2CComplete;
	/* Set timeout (much) greater than the transfer length */
	gErrorCode = -1;
	LPC_I2CD_API->i2c_set_timeout(i2cHandleMaster, 10000);
	LPC_I2CD_API->i2c_master_transmit_intr(i2cHandleMaster, &param, &result);
	//gErrorCode = LPC_I2CD_API->i2c_master_tx_rx_poll(i2cHandleMaster, &param, &result);
	while (gErrorCode == -1)
	{
		__WFI();
	}
	return gErrorCode;
}



uint32_t GetTemperatureInt()
{
	uint8_t buf[2];
	int temp;
	readRegister(Temperature, buf);
	temp = ((buf[0] <<8) + buf[1]) & 0x01FFF;  // clear flags

	if((temp & 0x1000) == 0x1000)
		temp |= 0xFFFFF000;
	return temp>>4;
}

float GetTemperatureFloat()
{
	uint8_t buf[2];
	int temp;
	readRegister(Temperature, buf);
	temp = ((buf[0] <<8) + buf[1]) & 0x01FFF;  // clear flags

	if((temp & 0x1000) == 0x1000)
		temp |= 0xFFFFF000;
	return 0.0625f * temp;
}


/**
 * @brief	I2C interrupt handler
 * @return	Nothing
 */
void I2C_IRQHandler(void)
{
	/* Call I2C ISR function in ROM with the I2C handle */
	LPC_I2CD_API->i2c_isr_handler(i2cHandleMaster);
}
