/*
 * adc.h - adc setup & isr for stm32f303
 * 01-28-2013 E. Brombaugh
 */

#ifndef __adc__
#define __adc__

#include "stm32f30x.h"

#define ADC_CHANNELS 4
#define ADC_SAMPLES 16
#define ADC_BUFSZ (ADC_CHANNELS*ADC_SAMPLES)

extern __IO uint16_t ADC_Channels[ADC_CHANNELS];

void setup_adc(void);

#endif
