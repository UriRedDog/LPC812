/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include "board.h"

#include <cr_section_macros.h>

// TODO: insert other include files here

#include "9804.h"
#include "mrt.h"

uint8_t gReadBuffer[4];
uint8_t gWriteBuffer[4];

// TODO: insert other definitions and declarations here


static volatile int intErrCode;

/* SysTick rate in Hz */
#define TICKRATE_HZ (10)

/* Current state for LED control via I2C cases */
static volatile int state;
/* Initializes pin muxing for I2C interface - note that SystemInit() may
   already setup your pin muxing at system startup */
static void Init_I2C_PinMux(void)
{
#if defined(BOARD_NXP_XPRESSO_812)
	/* Enable the clock to the Switch Matrix */
	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_SWM);

	/* Connect the I2C_SDA and I2C_SCL signals to port pins(P0.10, P0.11) */
	Chip_SWM_MovablePinAssign(SWM_I2C_SDA_IO, 10);
	Chip_SWM_MovablePinAssign(SWM_I2C_SCL_IO, 11);

#if (I2C_BITRATE > 400000)
	/* Enable Fast Mode Plus for I2C pins */
	Chip_IOCON_PinSetI2CMode(LPC_IOCON, IOCON_PIO10, PIN_I2CMODE_FASTPLUS);
	Chip_IOCON_PinSetI2CMode(LPC_IOCON, IOCON_PIO11, PIN_I2CMODE_FASTPLUS);
#endif

	/* Disable the clock to the Switch Matrix to save power */
	Chip_Clock_DisablePeriphClock(SYSCTL_CLOCK_SWM);

#endif
}




/**
 * @brief	Handle interrupt from SysTick timer
 * @return	Nothing
 */
void SysTick_Handler(void)
{
	static int ticks = 0;

	ticks++;
	if (ticks > TICKRATE_HZ)
	{
		ticks = 0;
		state = 1;
	}
}



int main(void)
{
	ErrorCode_t errorCode;
	int count = 0;
#ifdef __USE_LPCOPEN
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();

    // Set up and initialize all required blocks and functions
    // related to the board hardware
    Board_Init();

    // Set the LED to the state of "Off"
    Board_LED_Set(0, false);

	/* Setup I2C pin muxing */
	Init_I2C_PinMux();

	/* Allocate I2C handle, setup I2C rate, and initialize I2C
	   clocking */
	setupI2CMaster();


	/* Enable SysTick Timer */
	SysTick_Config(SystemCoreClock / TICKRATE_HZ);

#endif

    // TODO: insert code here

	gWriteBuffer[0] = 0x5;
	gWriteBuffer[1] = 0x50;
	errorCode = writeRegister(AlertLower, gWriteBuffer);
	errorCode = readRegister(AlertLower, gReadBuffer);

	errorCode = readRegister(ManufactureID, gReadBuffer);
	errorCode = readRegister(Configuration, gReadBuffer);

    while(1)
    {
		if(GetReloadTimers() == true)
		{
			SetReloadTimers(false);
			count += 20;
			if(count > 1000)
				count = 0;
			ExecuteTimers(count);
		}
		if(state)
		{
			state = 0;
			SetBrightness(GetTemperatureInt());
		}

		/* Sleep until a state change occurs in SysTick */
		__WFI();
    }
    return 0 ;
}
