/*
 * systick.c - 1ms system tick timer & related services.
 * 	- also handles switches, encoder and button debounce
 */

#include "systick.h"
#include "debounce.h"

uint32_t SysTick_Counter;
int16_t enc_val;
debounce_state dbs_btn, dbs_enc_A, dbs_enc_B;

/*
 * encoder details:
 * Phase A is GPIO PA1 - 0 state roughly centered between detents 
 * Phase B is GPIO PA2 - edge closest to detent
 * Button  is GPIO PA3
 * B = 1 on A^ -> CW
 * B = 0 on A^ -> CCW
 */

/*
 * SysTick_Init - sets up all the System Tick and UI state
 */
void SysTick_Init(void)
{
	GPIO_InitTypeDef    GPIO_InitStructure;

	/* GPIOA & GPIOF Periph clock enable */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	/* set up GPIO for encoder phase input */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* set up debounce objects for buttons & phases */
	init_debounce(&dbs_btn, 15);
	init_debounce(&dbs_enc_A, 2);
	init_debounce(&dbs_enc_B, 2);
    enc_val = 0;
    
	/* Init tick counter */
	SysTick_Counter = 0;
	
	/* start Tick IRQ */
	if(SysTick_Config(SystemCoreClock/1000))
	{
		/* Hang here to capture error */
		while(1);
	}
}

/*
 * SysTick_Handler - Called by System Tick IRQ @ 1ms intervals to update UI elements
 */
void SysTick_Handler(void)
{
	/* debounce button */
	debounce(&dbs_btn, (((~GPIOA->IDR) >> 3)&1));

	/* debounce encoder */
	debounce(&dbs_enc_A, (((GPIOA->IDR) >> 1)&1));
	debounce(&dbs_enc_B, (((GPIOA->IDR) >> 2)&1));

	/* if rising edge of clock then sample alternate phase for count dir */
	if(dbs_enc_A.re)
	{
		if(dbs_enc_B.state)
		{
			/* B = 1 on A^ -> CW */
			enc_val++;
		}
		else
		{
			/* B = 0 on A^ ->  CCW */
			enc_val--;
		}
	}
	
	/* Update SysTick Counter */
	SysTick_Counter++;
}

/*
 * get state of encoder button
 */
uint8_t SysTick_get_button(void)
{
	return dbs_btn.state;
}

/*
 * get state of encoder shaft
 */
int16_t SysTick_get_encoder(void)
{
	int16_t result = enc_val;
	enc_val = 0;
	return result;
}
