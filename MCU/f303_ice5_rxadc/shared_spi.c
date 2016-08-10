/*
 * shared_spi.c - interface routines for F303_ICE5 SPI
 * 07-05-16 E. Brombaugh
 */
 
#include "shared_spi.h"

/* fool the compiler */
#define UNUSED(x) ((void)(x))

void setup_shared_spi(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;

	/* GPIO Periph clock enables */
	RCC_AHBPeriphClockCmd(LCD_CS_GPIO_CLK | LCD_DC_GPIO_CLK | 
		LCD_LITE_GPIO_CLK | SD_CS_GPIO_CLK | SD_SPI_MOSI_GPIO_CLK |
		SD_SPI_MISO_GPIO_CLK | SD_SPI_SCK_GPIO_CLK, ENABLE);

	/* SD_SPI Periph clock enable */
	RCC_APB1PeriphClockCmd(SD_SPI_CLK, ENABLE); 

	/* Configure SD_SPI pins: SCK */
	GPIO_InitStructure.GPIO_Pin = SD_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_Init(SD_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	/* Configure SD_SPI pins: MISO */
	GPIO_InitStructure.GPIO_Pin = SD_SPI_MISO_PIN;
	GPIO_Init(SD_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	/* Configure SD_SPI pins: MOSI */
	GPIO_InitStructure.GPIO_Pin = SD_SPI_MOSI_PIN;
	GPIO_Init(SD_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/* Configure SD_SPI_CS_PIN pin: SD Card CS pin */
	GPIO_InitStructure.GPIO_Pin = SD_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(SD_CS_GPIO_PORT, &GPIO_InitStructure);
	SD_CS_HIGH();
	
	/* Configure LCD_CS_PIN pin: LCD CS pin */
	GPIO_InitStructure.GPIO_Pin = LCD_CS_PIN;
	GPIO_Init(LCD_CS_GPIO_PORT, &GPIO_InitStructure);
	LCD_CS_HIGH();
	
	/* Configure LCD_DC_PIN pin: LCD D/C pin */
	GPIO_InitStructure.GPIO_Pin = LCD_DC_PIN;
	GPIO_Init(LCD_DC_GPIO_PORT, &GPIO_InitStructure);
	LCD_DC_CMD();

	/* Configure LCD_RST_PIN pin: LCD Reset pin */
	GPIO_InitStructure.GPIO_Pin = LCD_RST_PIN;
	GPIO_Init(LCD_RST_GPIO_PORT, &GPIO_InitStructure);
	LCD_RST_HIGH();
	
	/* Configure LCD_LITE_PIN pin: LCD Lite pin */
	GPIO_InitStructure.GPIO_Pin = LCD_LITE_PIN;
	GPIO_Init(LCD_LITE_GPIO_PORT, &GPIO_InitStructure);
	LCD_LITE_HIGH();
	
	/* Connect PXx to SD_SPI_SCK */
	GPIO_PinAFConfig(SD_SPI_SCK_GPIO_PORT, SD_SPI_SCK_SOURCE, SD_SPI_SCK_AF);

	/* Connect PXx to SD_SPI_MISO */
	GPIO_PinAFConfig(SD_SPI_MISO_GPIO_PORT, SD_SPI_MISO_SOURCE, SD_SPI_MISO_AF); 

	/* Connect PXx to SD_SPI_MOSI */
	GPIO_PinAFConfig(SD_SPI_MOSI_GPIO_PORT, SD_SPI_MOSI_SOURCE, SD_SPI_MOSI_AF);  

	/* SD_SPI Config */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
#if 0
	// original setup from ST eval code - works
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
#else
	// Martin Thomas setup - works too
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
#endif
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SD_SPI, &SPI_InitStructure);

	SPI_RxFIFOThresholdConfig(SD_SPI, SPI_RxFIFOThreshold_QF);

	SPI_Cmd(SD_SPI, ENABLE); /* SD_SPI enable */
	
	SPI_InitDMA(); /* setup DMA structure for fast ops */
}

void SPI_WriteByte(uint8_t Data)
{
	/* Wait until the transmit buffer is empty */
	//while(SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_TXE) == RESET)
	while((SD_SPI->SR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET)
	{
	}

	/* Send the byte */
	//SPI_SendData8(SD_SPI, Data);
	//SD_SPI->DR = (uint16_t)Data;
	*(__IO uint8_t *) ((uint32_t)SD_SPI+0x0C) = Data;
	
	/*!< Wait to receive a byte*/
	//while(SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
	while((SD_SPI->SR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET)
	{
	}

	/* Return the byte read from the SPI bus */ 
	//return SPI_ReceiveData8(SD_SPI);
	//return *(__IO uint8_t *) ((uint32_t)SD_SPI+0x0C);
	uint8_t dummy = *(__IO uint8_t *) ((uint32_t)SD_SPI+0x0C);
    UNUSED(dummy); /* To avoid GCC warning */

}

uint8_t SPI_WriteReadByte(uint8_t Data)
{
	/* Wait until the transmit buffer is empty */
	//while(SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_TXE) == RESET)
	while((SD_SPI->SR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET)
	{
	}

	/*!< Send the byte */
	//SPI_SendData8(SD_SPI, Data);
	//SD_SPI->DR = (uint16_t)Data;
	*(__IO uint8_t *) ((uint32_t)SD_SPI+0x0C) = Data;
	
	/* Wait to receive a byte*/
	//while(SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
	while((SD_SPI->SR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET)
	{
	}

	/* Return the byte read from the SPI bus */ 
	//return SPI_ReceiveData8(SD_SPI);
	return *(__IO uint8_t *) ((uint32_t)SD_SPI+0x0C);
}

uint8_t SPI_ReadByte(void)
{
	uint8_t Data = 0;

	/* Wait until the transmit buffer is empty */
	while (SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_TXE) == RESET)
	{
	}
	/* Send the byte */
	SPI_SendData8(SD_SPI, SD_DUMMY_BYTE);

	/* Wait until a data is received */
	while (SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_RXNE) == RESET)
	{
	}
	/* Get the received data */
	Data = SPI_ReceiveData8(SD_SPI);

	/* Return the shifted data */
	return Data;
}

DMA_InitTypeDef DMA_InitStructure;

void SPI_InitDMA(void)
{
	// turn on DMA1 clock
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);
	
    DMA_Cmd(DMA2_Channel2, DISABLE);
    DMA_DeInit(DMA2_Channel2);
 
    // Common
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&(SD_SPI->DR);
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
}

void SPI_start_DMA_WriteBytes(uint8_t *buffer, uint16_t len)
{
    /* Setup buffer loc / len */
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)buffer;
    DMA_InitStructure.DMA_BufferSize = len;
    DMA_Init(DMA2_Channel2, &DMA_InitStructure);
 
    /* Enable SPI_DMA_TX */
    DMA_Cmd(DMA2_Channel2, ENABLE);
 
    /* Enable SPI_DMA TX request */
    SPI_I2S_DMACmd(SD_SPI, SPI_I2S_DMAReq_Tx, ENABLE);
}

void SPI_end_DMA_WriteBytes(void)
{
    /* Wait until SPI_DMA_TX Complete */
    while (DMA_GetFlagStatus(DMA1_FLAG_TC3) == RESET);
 
    /* DISABLE SPI_DMA_TX */
    DMA_Cmd(DMA2_Channel2, DISABLE);
    SPI_I2S_DMACmd(SD_SPI, SPI_I2S_DMAReq_Tx, DISABLE);
 
    /* Clear DMA TransferComplete Flag */
	DMA_ClearFlag(DMA2_FLAG_TC2);
    //DMA_ClearITPendingBit(DMA1_IT_TC3);
 
    /* Wait to receive a byte */
    while (SPI_I2S_GetFlagStatus(SD_SPI, SPI_I2S_FLAG_RXNE) == RESET);
}
 
void SPI_real_DMA_WriteBytes(uint8_t *buffer, uint16_t len)
{
	SPI_start_DMA_WriteBytes(buffer, len);
	SPI_end_DMA_WriteBytes();
}
