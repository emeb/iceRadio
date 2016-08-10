/*
 * audio.h - audio processing routines
 */

#ifndef __audio__
#define __audio__

#include "stm32f30x.h"
#include "arm_math.h"

#define DMA_BUFFSZ 128

void Audio_Init(void);
void Audio_SetFilter(uint8_t filter);
uint8_t Audio_GetFilter(void);
int16_t Audio_GetRSSI(void);
void Audio_SetDemod(uint8_t demod);
int8_t Audio_GetDemod(void);
void Audio_SetMute(uint8_t State);
uint16_t Audio_GetMute(void);
int16_t Audio_GetParam(void);
int16_t Audio_GetSyncFrq(void);
int16_t Audio_GetSyncSt(void);
void Audio_Proc(int16_t *src, int16_t *dst);

#endif

