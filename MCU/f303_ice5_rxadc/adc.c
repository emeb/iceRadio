/*
 * adc.c - adc setup & isr for stm32f303
 * 07-04-2013 E. Brombaugh
 */

#include "adc.h"
#include "cyclesleep.h"

__IO uint16_t ADC_Buffer[ADC_BUFSZ];
__IO uint16_t ADC_Channels[ADC_CHANNELS];
__IO uint16_t calibration_value_1 = 0;

void setup_adc(void)
{
	GPIO_InitTypeDef       GPIO_InitStructure;
	DMA_InitTypeDef        DMA_InitStructure;
	ADC_InitTypeDef        ADC_InitStructure;

	/* Configure the ADC clock */
	RCC_ADCCLKConfig(RCC_ADC12PLLCLK_Div1);  
	
	/* Enable ADC1 clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ADC12, ENABLE);
		
	/* Enable GPIOA,C Periph clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);

	/* Configure ADC1/2 Channel1-4 as analog input */
	//GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3|GPIO_Pin_2|GPIO_Pin_1|GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure PC13 as 50MHz pp for diag pulse */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	/* Enable DMA1 clock */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	/* DMA1 Channel1 Init */
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&ADC_Buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = ADC_BUFSZ;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

	DMA_Init(DMA1_Channel1, &DMA_InitStructure);

	/* DMA interrupts */
	DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);

	/* enable DMA IRQ */
	NVIC_EnableIRQ(DMA1_Channel1_IRQn);

	/* ADC Calibration procedure */
	ADC_VoltageRegulatorCmd(ADC1, ENABLE);
  
	/* Insert delay equal to 10 ms */
	delay(10);
  
	ADC_SelectCalibrationMode(ADC1, ADC_CalibrationMode_Single);

	ADC_SelectCalibrationMode(ADC2, ADC_CalibrationMode_Single);
  
	while(ADC_GetCalibrationStatus(ADC1) != RESET );
	calibration_value_1 = ADC_GetCalibrationValue(ADC1);

	/* ADC setup */  
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_ContinuousConvMode = ADC_ContinuousConvMode_Enable;
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; 
	ADC_InitStructure.ADC_ExternalTrigConvEvent = ADC_ExternalTrigConvEvent_0;         
	ADC_InitStructure.ADC_ExternalTrigEventEdge = ADC_ExternalTrigEventEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_OverrunMode = ADC_OverrunMode_Disable;   
	ADC_InitStructure.ADC_AutoInjMode = ADC_AutoInjec_Disable;  
	ADC_InitStructure.ADC_NbrOfRegChannel = ADC_CHANNELS;
	ADC_Init(ADC1, &ADC_InitStructure);

	/* ADC1 regular channel configuration */ 
	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_601Cycles5);
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 2, ADC_SampleTime_601Cycles5);
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 3, ADC_SampleTime_601Cycles5);
	//ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 4, ADC_SampleTime_601Cycles5);
	
	/* Configures the ADC DMA */
	ADC_DMAConfig(ADC1, ADC_DMAMode_Circular);
		
	/* Enable the ADC DMA */
	ADC_DMACmd(ADC1, ENABLE);

	/* Enable ADC1 */
	ADC_Cmd(ADC1, ENABLE);

	/* wait for ADC1 ADRDY */
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_RDY));

	/* Enable the DMA channel */
	DMA_Cmd(DMA1_Channel1, ENABLE);

	/* Start ADC1 Software Conversion */ 
	ADC_StartConversion(ADC1);
	
}

/* Handles DMA1_Channel1 (ADC Buffer) interrupt request */
void DMA1_Channel1_IRQHandler(void)
{
	uint32_t accum[ADC_CHANNELS];
	int16_t i, j;
	
	/* Active ISR */
	GPIOC->BSRR |= (1<<13);

	/* Transfer Complete? */
	if(DMA1->ISR & DMA1_IT_TC1)
	{
#if 1
		/* clear accumulators */
		for(j=0;j<ADC_CHANNELS;j++)
			accum[j] = 0;
		
		/* invert & integrate samples */
		for(i=0;i<ADC_SAMPLES;i++)
		{
			for(j=0;j<ADC_CHANNELS;j++)
				accum[j] += ADC_Buffer[i*ADC_CHANNELS + j];
		}
		
		/* scale and save results */
		for(j=0;j<ADC_CHANNELS;j++)
			ADC_Channels[j] = (uint16_t)((accum[j]+8)>>4);
#else
		/* copy samples */
		for(i=0;i<ADC_CHANNELS;i++)
		{
            ADC_Channels[i] = ADC_Buffer[i];
		}
#endif
    }

	/* Clear DMA1_Channel1 interrupt */
	DMA1->IFCR = DMA_IFCR_CGIF1;

	/* Inactive ISR */
	GPIOC->BRR |= (1<<13);
}

