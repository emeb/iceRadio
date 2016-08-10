/*
 * cyclesleep.h - zyp's cycle counter sleep routines
 * 12-20-12 E. Brombaugh
 */

#ifndef __cyclesleep__
#define __cyclesleep__

#include "stm32f30x.h"

#define CYCLES_PER_SEC 0x044aa200

void cyccnt_enable();
void cyclesleep(uint32_t cycles);
uint32_t cyclegoal(uint32_t cycles);
uint32_t cyclegoal_ms(uint32_t ms);
uint32_t cyclecheck(uint32_t goal);
void delay(uint32_t ms);

#endif
