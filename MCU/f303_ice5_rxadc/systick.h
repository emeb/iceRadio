/*
 * systick.h - 1ms system tick setup
 */

#ifndef __systick__
#define __systick__

#include "stm32f30x.h"

void SysTick_Init(void);
uint8_t SysTick_get_button(void);
int16_t SysTick_get_encoder(void);

#endif
