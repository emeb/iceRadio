/*
 * led.h - F303_ice5 LED setup
 */

#ifndef __led__
#define __led__

#include "stm32f30x.h"

void LEDInit(void);
void LEDOn(void);
void LEDOff(void);
void LEDToggle(void);

#endif
