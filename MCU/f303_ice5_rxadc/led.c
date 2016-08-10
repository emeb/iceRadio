/*
 * led.c - F303_ice5 LED setup
 */

#include "led.h"

/*
 * Initialize the breakout board LED
 */
void LEDInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Enable GPIO A Clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	
	/* Enable PA9 for output */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/*
 * Turn on LED
 */
void LEDOn(void)
{
	GPIOA->BSRR = (1<<9);
}

/*
 * Turn off LED
 */
void LEDOff(void)
{
	GPIOA->BRR = (1<<9);
}

/*
 * Toggle LED
 */
void LEDToggle(void)
{
	GPIOA->ODR ^= (1<<9);
}

