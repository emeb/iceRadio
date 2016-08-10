/*
 * i2s.c - generic I2S full-duplex slave
 * 07-16-16 E. Brombaugh
 */
  
#include "i2s.h"
#include "audio.h"

/*-----------------------------------
Hardware Configuration defines parameters
-----------------------------------------*/                 
/* I2S peripheral configuration defines */
#define I2S_SPI                     SPI2
#define I2S_EXT                     I2S2ext
#define I2S_CLK                     RCC_APB1Periph_SPI2
#define I2S_ADDRESS                 (uint32_t)&(I2S_SPI->DR)
#define I2S_EXT_ADDRESS             (uint32_t)&(I2S_EXT->DR)
#define I2S_GPIO_AF                 GPIO_AF_5
#define I2S_GPIO_CLOCK              RCC_AHBPeriph_GPIOB
#define I2S_WS_PIN                  GPIO_Pin_12
#define I2S_SCK_PIN                 GPIO_Pin_13
#define I2S_DOUT_PIN                GPIO_Pin_15
#define I2S_DIN_PIN                 GPIO_Pin_14
#define I2S_WS_PINSRC               GPIO_PinSource12
#define I2S_SCK_PINSRC              GPIO_PinSource13
#define I2S_DOUT_PINSRC             GPIO_PinSource15
#define I2S_DIN_PINSRC              GPIO_PinSource14
#define I2S_DOUT_GPIO               GPIOB
#define I2S_DIN_GPIO                GPIOB
#define I2S_SCK_GPIO                GPIOB
#define I2S_WS_GPIO                 GPIOB

/* Select the interrupt preemption priority and subpriority for the DMA interrupt */
#define I2S_IRQ_PREPRIO             0   /* Select the preemption priority level(0 is the highest) */
#define I2S_IRQ_SUBRIO              0   /* Select the sub-priority level (0 is the highest) */

/* I2S DMA Channel definitions */
#define I2S_DMA                     DMA1
#define I2S_DMA_CLOCK               RCC_AHBPeriph_DMA1
#define I2S_TX_DMA_CHANNEL          DMA1_Channel5
#define I2S_TX_DMA_IRQ              DMA1_Channel5_IRQn
#define I2S_TX_DMA_FLAG_TC          DMA1_FLAG_TC5
#define I2S_TX_DMA_FLAG_HT          DMA1_FLAG_HT5
#define I2S_TX_DMA_FLAG_TE          DMA1_FLAG_TE5
#define I2S_RX_DMA_CHANNEL          DMA1_Channel4
#define I2S_RX_DMA_IRQ              DMA1_Channel4_IRQn
#define I2S_RX_DMA_FLAG_TC          DMA1_FLAG_TC4
#define I2S_RX_DMA_FLAG_HT          DMA1_FLAG_HT4
#define I2S_RX_DMA_FLAG_TE          DMA1_FLAG_TE4

#define I2S_TX_IRQHandler           DMA1_Channel5_IRQHandler
#define I2S_RX_IRQHandler           DMA1_Channel4_IRQHandler
#define I2S_DMA_PERIPH_DATA_SIZE    DMA_PeripheralDataSize_HalfWord
#define I2S_DMA_MEM_DATA_SIZE       DMA_MemoryDataSize_HalfWord

/* uncomment this for diagnostic GPIO */
#define I2S_DIAG
#ifdef I2S_DIAG
#define I2S_DIAG_GPIO_CLOCK         RCC_AHBPeriph_GPIOA
#define I2S_DIAG_GPIO               GPIOA
#define I2S_DIAG_PIN                GPIO_Pin_4
#define I2S_DIAG_SET(x)             GPIO_WriteBit(I2S_DIAG_GPIO,I2S_DIAG_PIN,x)
#else
#define I2S_DIAG_SET(x)
#endif

/* DMA buffers for I2S */
__IO int16_t tx_buffer[DMA_BUFFSZ], rx_buffer[DMA_BUFFSZ];

/*
 * Do all hardware setup for I2S
 */
void i2s_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2S_InitTypeDef I2S_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
	//NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable I2S GPIO clocks */
	RCC_AHBPeriphClockCmd(I2S_GPIO_CLOCK, ENABLE);

	/* Connect pins to I2S peripheral  */
	GPIO_PinAFConfig(I2S_WS_GPIO, I2S_WS_PINSRC, I2S_GPIO_AF);
	GPIO_PinAFConfig(I2S_SCK_GPIO, I2S_SCK_PINSRC, I2S_GPIO_AF);
	GPIO_PinAFConfig(I2S_DIN_GPIO, I2S_DIN_PINSRC, I2S_GPIO_AF);
	GPIO_PinAFConfig(I2S_DOUT_GPIO, I2S_DOUT_PINSRC, I2S_GPIO_AF);

	/* I2S pins configuration -------------------------*/
	GPIO_InitStructure.GPIO_Pin =   I2S_SCK_PIN;
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd =  GPIO_PuPd_NOPULL;
	GPIO_Init(I2S_SCK_GPIO, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =   I2S_DIN_PIN;
	GPIO_Init(I2S_DIN_GPIO, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =   I2S_DOUT_PIN;
	GPIO_Init(I2S_DOUT_GPIO, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin =   I2S_WS_PIN;
	GPIO_Init(I2S_WS_GPIO, &GPIO_InitStructure);
		
#ifdef I2S_DIAG
    /* Configure ISR diag Pin */
	RCC_AHBPeriphClockCmd(I2S_DIAG_GPIO_CLOCK, ENABLE);
	GPIO_InitStructure.GPIO_Pin =  I2S_DIAG_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(I2S_DIAG_GPIO, &GPIO_InitStructure);
#endif

	/* Enable the I2S peripheral clock */
	RCC_APB1PeriphClockCmd(I2S_CLK, ENABLE);

	/* Deinitialize SPI_I2S peripheral */
	SPI_I2S_DeInit(I2S_SPI);

	/* I2S peripheral configuration */
	SPI_I2S_DeInit(I2S_SPI);
    I2S_StructInit(&I2S_InitStructure);
	I2S_InitStructure.I2S_Standard = I2S_Standard_Phillips;
	I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
	I2S_InitStructure.I2S_CPOL = I2S_CPOL_High;
	I2S_InitStructure.I2S_Mode = I2S_Mode_SlaveTx;

	/* Initialize the I2S peripheral with the structure above */
	I2S_Init(I2S_SPI, &I2S_InitStructure);

	/* Initialize the I2S extended channel for RX */
	I2S_FullDuplexConfig(I2S_EXT, &I2S_InitStructure);

	/* Enable the DMA clock */
	RCC_AHBPeriphClockCmd(I2S_DMA_CLOCK, ENABLE); 

	/* Configure the TX DMA Channel */
	DMA_Cmd(I2S_TX_DMA_CHANNEL, DISABLE);
	DMA_DeInit(I2S_TX_DMA_CHANNEL);
	DMA_InitStructure.DMA_PeripheralBaseAddr = I2S_ADDRESS;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&tx_buffer;
	DMA_InitStructure.DMA_BufferSize = (uint32_t)DMA_BUFFSZ;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = I2S_DMA_PERIPH_DATA_SIZE;
	DMA_InitStructure.DMA_MemoryDataSize = I2S_DMA_MEM_DATA_SIZE; 
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_Init(I2S_TX_DMA_CHANNEL, &DMA_InitStructure);  

	/* Enable the selected DMA interrupts */
	DMA_ITConfig(I2S_TX_DMA_CHANNEL, DMA_IT_TC | DMA_IT_HT, ENABLE);

	/* Enable the I2S DMA request */
	SPI_I2S_DMACmd(I2S_SPI, SPI_I2S_DMAReq_Tx, ENABLE);

	/* Configure the RX DMA Channel */
	DMA_Cmd(I2S_RX_DMA_CHANNEL, DISABLE);
	DMA_DeInit(I2S_RX_DMA_CHANNEL);
	DMA_InitStructure.DMA_PeripheralBaseAddr = I2S_EXT_ADDRESS;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)&rx_buffer;
	DMA_InitStructure.DMA_BufferSize = (uint32_t)DMA_BUFFSZ;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = I2S_DMA_PERIPH_DATA_SIZE;
	DMA_InitStructure.DMA_MemoryDataSize = I2S_DMA_MEM_DATA_SIZE; 
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_Init(I2S_RX_DMA_CHANNEL, &DMA_InitStructure);  

	/* Enable the selected DMA interrupts */
	DMA_ITConfig(I2S_RX_DMA_CHANNEL, DMA_IT_TC | DMA_IT_HT, ENABLE);

	/* Enable the I2S DMA request */
	SPI_I2S_DMACmd(I2S_EXT, SPI_I2S_DMAReq_Rx, ENABLE);

#if 0
	// Fancy priority IRQ enable
	/* I2S DMA IRQ Channel configuration */
	NVIC_InitStructure.NVIC_IRQChannel = I2S_TX_DMA_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = I2S_IRQ_PREPRIO;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = I2S_IRQ_SUBRIO;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#else
	// Simpler IRQ enable
	NVIC_EnableIRQ(I2S_RX_DMA_IRQ);
#endif

	/* Enable the tx I2S DMA Channel */
	DMA_Cmd(I2S_TX_DMA_CHANNEL, ENABLE);
	/* Enable the rx I2S DMA Channel */
	DMA_Cmd(I2S_RX_DMA_CHANNEL, ENABLE);
    
    /* wait for the external WS line to go high as per ref manual */
    while(GPIO_ReadInputDataBit(I2S_WS_GPIO,I2S_WS_PIN) != SET);
	
	/* If the I2S peripheral is still not enabled, enable it */
    if ((I2S_SPI->I2SCFGR & 0x0400) == 0)
    {
      I2S_Cmd(I2S_SPI, ENABLE);
    }    
    if ((I2S_EXT->I2SCFGR & 0x0400) == 0)
    {
      I2S_Cmd(I2S_EXT, ENABLE);
    } 
}

/**
  * @brief  This function handles I2S RX DMA block interrupt. 
  * @param  None
  * @retval none
  */
void I2S_RX_IRQHandler(void)
{ 
	int16_t *src, *dst;
	
	/* Raise activity flag */
	I2S_DIAG_SET(1);

	/* Transfer complete interrupt */
	if (I2S_DMA->ISR & I2S_RX_DMA_FLAG_TC)
	{
		/* Point to 2nd half of buffers */
		src = (int16_t *)(rx_buffer) + DMA_BUFFSZ/2;
		dst = (int16_t *)(tx_buffer) + DMA_BUFFSZ/2;
		
		/* Handle 2nd half */  
		Audio_Proc(src, dst);    

		/* Clear the Interrupt flag */
		DMA_ClearFlag(I2S_RX_DMA_FLAG_TC);
	}

	/* Half Transfer complete interrupt */
	if (I2S_DMA->ISR & I2S_RX_DMA_FLAG_HT)
	{
		/* Point to 1st half of buffers */
		src = (int16_t *)(rx_buffer);
		dst = (int16_t *)(tx_buffer);

		/* Handle 1st half */  
		Audio_Proc(src, dst);    

		/* Clear the Interrupt flag */
		DMA_ClearFlag(I2S_RX_DMA_FLAG_HT);    
	}
	
	/* Lower activity flag */
	I2S_DIAG_SET(0);
;
}
