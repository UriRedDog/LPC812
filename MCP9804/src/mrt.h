/*
 * mrt.h
 *
 *  Created on: Nov 20, 2013
 *      Author: dthedens
 */

#ifndef MRT_H_
#define MRT_H_


extern void MRT_IRQHandler(void);
extern bool GetReloadTimers(void);
extern void SetReloadTimers(bool state);
extern void setupMRT(uint8_t ch, MRT_MODE_T mode, uint32_t rate);
extern void ExecuteTimers(unsigned int count);
extern void SetBrightness(uint32_t temperature);

#endif /* MRT_H_ */
