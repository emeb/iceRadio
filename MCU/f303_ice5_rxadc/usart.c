/*
 * usart.c - serial i/o routines
 * 12-24-12 E. Brombaugh
 */
 
#include <stdio.h>
#include "stm32f30x.h"

uint8_t RX_buffer[256];
uint8_t *RX_wptr, *RX_rptr;

/* USART1 setup */
void setup_usart1(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	/* init RX buffer write/read pointers*/
	RX_wptr = &RX_buffer[0];
	RX_rptr = &RX_buffer[0];

	/* Setup USART */
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	/* Connect PB6 to USARTx_Tx */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_7);

	/* Connect PB7 to USARTx_Rx */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource7, GPIO_AF_7);

	/* Configure USART Tx as alternate function push-pull */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* Configure USART Rx as alternate function push-pull */
	/* RX not used */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	/* USART configuration */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	/* USART = 115k-8-N-1 */
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART1, &USART_InitStructure);
	
	/* Enable RX interrupt */
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	/* Enable the USART6 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	/* Enable USART */
	USART_Cmd(USART1, ENABLE);
}

int get_usart(void)
{
#if 0
	/* Non-interrupt version */
	if(USART_GetFlagStatus(USART6, USART_FLAG_RXNE) == SET)
		return USART_ReceiveData(USART6);
	else
		return EOF;
#else
	/* interrupt version */
	int retval;
	
	/* check if there's data in the buffer */
	if(RX_rptr != RX_wptr)
	{
		/* get the data */
		retval = *RX_rptr++;
		
		/* wrap the pointer */
		if((RX_rptr - &RX_buffer[0])>=256)
			RX_rptr = &RX_buffer[0];
	}
	else
		retval = EOF;

	return retval;
#endif
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
int outbyte(int ch)
{
	/* Place your implementation of fputc here */
	/* e.g. write a character to the USART */
	USART_SendData(USART1, (uint8_t) ch);

	/* Loop until transmit data register is empty */
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
	{}

	return ch;
}

/**
  * @brief  Retargets the C library printf function to the USART.
  * @param  None
  * @retval None
  */
int inbyte(void)
{
	/* nothing happening yet */
	return 0;
}

/*
 * USART IRQ handler - used only for Rx for now
 */
void USART1_IRQHandler(void)
{
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		/* get the character */
		uint8_t rxchar = USART_ReceiveData(USART1);

		/* check if there's room in the buffer */
		if((RX_wptr != RX_rptr-1) &&
           (RX_wptr - RX_rptr != 255))
		{
			/* Yes - Queue the new char */
			*RX_wptr++ = rxchar;
	
			/* Wrap pointer */
			if((RX_wptr - &RX_buffer[0])>=256)
				RX_wptr = &RX_buffer[0];
		}
	}

	if(USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
	{
	}
}