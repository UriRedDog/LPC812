/*
 * @brief Multi-Rate Timer (MRT) example
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2012
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"

/** @defgroup EXAMPLES_PERIPH_8XX_MRT LPC8xx Multi-Rate Timer example
 * @ingroup EXAMPLES_PERIPH_8XX
 * <b>Example description</b><br>
 * The MRT example demonstrates using the Multi-Rate Timer API functions.
 * This example configures timers 0 and 1 as periodic interrupts that
 * blink LED0. Timer 2 is configured as a one-shot interrupt that gets
 * reset in the interrupt handler. Timer 3 is setup as a one-shot interrupt
 * that gets reset in the background loop.<br>
 *
 * <b>Special connection requirements</b><br>
 * There are no special connection requirements for this example.<br>
 *
 * <b>Build procedures:</b><br>
 * @ref LPCOPEN_8XX_BUILDPROCS_IAR<br>
 * @ref LPCOPEN_8XX_BUILDPROCS_KEIL<br>
 * @ref LPCOPEN_8XX_BUILDPROCS_XPRESSO<br>
 *
 * <b>Supported boards and board setup:</b><br>
 * @ref LPCOPEN_8XX_BOARD_XPRESSO_812<br>
 *
 * <b>Submitting LPCOpen issues:</b><br>
 * @ref LPCOPEN_COMMUNITY
 * @{
 */

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

static volatile bool t3Fired;

volatile unsigned int mRedLedBrightness;
volatile unsigned int mBlueLedBrightness;
volatile unsigned int mGreenLedBrightness;

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/* Interrupt fire reference counters for t0/1, t2, and t3 */
uint32_t t0, t1, t2, t3;
volatile bool mReload;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/* Setup a timer for a periodic (repeat mode) rate */
void setupMRT(uint8_t ch, MRT_MODE_T mode, uint32_t rate)
{
	LPC_MRT_CH_T *pMRT;

	/* Get pointer to timer selected by ch */
	pMRT = Chip_MRT_GetRegPtr(ch);

	/* Setup timer with rate based on MRT clock */
	Chip_MRT_SetInterval(pMRT, (Chip_Clock_GetSystemClockRate() / rate) |
						 MRT_INTVAL_LOAD);

	/* Timer mode */
	Chip_MRT_SetMode(pMRT, mode);

	/* Clear pending interrupt and enable timer */
	Chip_MRT_IntClear(pMRT);
	Chip_MRT_SetEnabled(pMRT);
}

/*****************************************************************************
 * Public functions
 ****************************************************************************/
void SetReloadTimers(bool state)
{
	mReload = state;
}

bool GetReloadTimers(void)
{
	return mReload;
}

// based upon the temperature, we will set the led brightness
// blue is cold, green is warm, red is hot
void SetBrightness(uint32_t temperature)
{


}

void ExecuteTimers(count)
{
	Board_LED_Set(0, true);
	setupMRT(1, MRT_MODE_ONESHOT, mRedLedBrightness+count);
	Board_LED_Set(1, true);
	setupMRT(2, MRT_MODE_ONESHOT, mBlueLedBrightness+count);
	Board_LED_Set(2, true);
	setupMRT(3, MRT_MODE_ONESHOT, mGreenLedBrightness+count);
}

/**
 * @brief	Handle interrupt from MRT
 * @return	Nothing
 */
void MRT_IRQHandler(void)
{
	uint32_t int_pend;

	/* Get interrupt pending status for all timers */
	int_pend = Chip_MRT_GetIntPending();
	Chip_MRT_ClearIntPending(int_pend);

	/* Channel 0 */
	if (int_pend & (MRTn_INTFLAG(0))) {
		SetReloadTimers(true);
		t0++;
	}

	if (int_pend & (MRTn_INTFLAG(1))) {
		Board_LED_Set(0, false);
		t1++;
	}

	/* Channel 2 is single shot, reset it here */
	if (int_pend & (MRTn_INTFLAG(2))) {
		Board_LED_Set(1, false);
		t2++;
	}

	/* Channel 3 is single shot, set flag so background loop resets it */
	if (int_pend & (MRTn_INTFLAG(3))) {
		Board_LED_Set(2, false);
		t3++;
	}
}

/**
 * @brief	MRT example main function
 * @return	Status (This function will not return)
 */
void InitMrt(void)
{
	int mrtch;

	SetReloadTimers(false);

	mRedLedBrightness = 250;
	mBlueLedBrightness = 325;
	mGreenLedBrightness = 312;
	/* Generic Initialization */

	/* MRT Initialization and disable all timers */
	Chip_MRT_Init();
	for (mrtch = 0; mrtch < MRT_CHANNELS_NUM; mrtch++) {
		Chip_MRT_SetDisabled(Chip_MRT_GetRegPtr(mrtch));
	}

	/* Enable the interrupt for the MRT */
	NVIC_EnableIRQ(MRT_IRQn);

	/* Enable timer 0 in repeat mode  */
	setupMRT(0, MRT_MODE_REPEAT, 100); // 10hz gives pwm of 100ms

	Board_LED_Set(0, false);
	Board_LED_Set(1, false);
	Board_LED_Set(2, false);

}

/**
 * @}
 */
