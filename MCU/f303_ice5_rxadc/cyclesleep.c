/*
 * cyclesleep.c - zyp's cycle counter sleep routines
 * 12-20-12 E. Brombaugh
 */
 
#include "cyclesleep.h"

volatile uint32_t* demcr = (uint32_t*)0xE000EDFC;
volatile uint32_t* dwt_ctrl = (uint32_t*)0xe0001000;
volatile uint32_t* dwt_cyccnt = (uint32_t*)0xe0001004;
uint32_t DelayCyc1s; 

/* get sysclock freq */
uint32_t get_sysclk(void)
{
	RCC_ClocksTypeDef RCC_Clocks;
	
	/* Compute Delay amount */
	RCC_GetClocksFreq(&RCC_Clocks);
	return RCC_Clocks.SYSCLK_Frequency;
}

// turn on cycle counter
void cyccnt_enable()
{
	*demcr |= (1<<24);
    *dwt_ctrl |= 1;
	DelayCyc1s = get_sysclk();
}

uint32_t cyclegoal(uint32_t cycles)
{
	return cycles + *dwt_cyccnt;
}

uint32_t cyclegoal_ms(uint32_t ms)
{
	return ms*(DelayCyc1s/1000) + *dwt_cyccnt;
}

uint32_t cyclecheck(uint32_t goal)
{
	return *dwt_cyccnt < goal;
}

// sleep for a certain number of cycles
void cyclesleep(uint32_t cycles)
{
    uint32_t goal = cyclegoal(cycles);
    
    while(cyclecheck(goal));
}

// sleep for a certain number of milliseconds
void delay(uint32_t ms)
{
	cyclesleep(ms*(DelayCyc1s/1000));
}
