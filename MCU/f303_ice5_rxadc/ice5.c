/*
 * ice5.c - interface routines for STM32F303 SPI to ice5 FPGA
 * 07-02-16 E. Brombaugh
 */
 
#include "ice5.h"
#include "cyclesleep.h"

/* fool the compiler */
#define UNUSED(x) ((void)(x))
/**
  * @brief  SPI Interface pins
  */
#define ICE5_SPI                SPI1
#define ICE5_SPI_CLK            RCC_APB2Periph_SPI1
#define ICE5_SPI_SCK_PIN        GPIO_Pin_5
#define ICE5_SPI_SCK_GPIO_PORT  GPIOA
#define ICE5_SPI_SCK_GPIO_CLK   RCC_AHBPeriph_GPIOA
#define ICE5_SPI_SCK_SOURCE     GPIO_PinSource5
#define ICE5_SPI_SCK_AF         GPIO_AF_5

#define ICE5_SPI_MISO_PIN       GPIO_Pin_6
#define ICE5_SPI_MISO_GPIO_PORT GPIOA
#define ICE5_SPI_MISO_GPIO_CLK  RCC_AHBPeriph_GPIOA
#define ICE5_SPI_MISO_SOURCE    GPIO_PinSource6
#define ICE5_SPI_MISO_AF        GPIO_AF_5

#define ICE5_SPI_MOSI_PIN       GPIO_Pin_7
#define ICE5_SPI_MOSI_GPIO_PORT GPIOA
#define ICE5_SPI_MOSI_GPIO_CLK  RCC_AHBPeriph_GPIOA
#define ICE5_SPI_MOSI_SOURCE    GPIO_PinSource7
#define ICE5_SPI_MOSI_AF        GPIO_AF_5

#define ICE5_SPI_CS_PIN         GPIO_Pin_0
#define ICE5_SPI_CS_GPIO_PORT   GPIOB
#define ICE5_SPI_CS_GPIO_CLK    RCC_AHBPeriph_GPIOB

#define ICE5_CDONE_PIN          GPIO_Pin_2
#define ICE5_CDONE_GPIO_PORT    GPIOB
#define ICE5_CDONE_GPIO_CLK     RCC_AHBPeriph_GPIOB

#define ICE5_CRST_PIN           GPIO_Pin_1
#define ICE5_CRST_GPIO_PORT     GPIOB
#define ICE5_CRST_GPIO_CLK      RCC_AHBPeriph_GPIOB

#define ICE5_SPI_CS_LOW()		GPIO_ResetBits(ICE5_SPI_CS_GPIO_PORT, ICE5_SPI_CS_PIN)
#define ICE5_SPI_CS_HIGH()      GPIO_SetBits(ICE5_SPI_CS_GPIO_PORT, ICE5_SPI_CS_PIN)
#define ICE5_CRST_LOW()         GPIO_ResetBits(ICE5_CRST_GPIO_PORT, ICE5_CRST_PIN)
#define ICE5_CRST_HIGH()        GPIO_SetBits(ICE5_CRST_GPIO_PORT, ICE5_CRST_PIN)
#define ICE5_CDONE_GET()        GPIO_ReadInputDataBit(ICE5_CDONE_GPIO_PORT, ICE5_CDONE_PIN)
#define ICE5_SPI_DUMMY_BYTE     0xFF

void ICE5_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	SPI_InitTypeDef   SPI_InitStructure;

	/* GPIO Periph clock enables */
	RCC_AHBPeriphClockCmd(ICE5_CDONE_GPIO_CLK | 
		ICE5_SPI_CS_GPIO_CLK | ICE5_SPI_MOSI_GPIO_CLK |
		ICE5_SPI_MISO_GPIO_CLK | ICE5_SPI_SCK_GPIO_CLK, ENABLE);

	/* ICE5_SPI Periph clock enable */
	RCC_APB2PeriphClockCmd(ICE5_SPI_CLK, ENABLE); 

	/* Configure ICE5_SPI pin: SCK */
	GPIO_InitStructure.GPIO_Pin = ICE5_SPI_SCK_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_Init(ICE5_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

	/* Configure ICE5_SPI pin: MISO */
	GPIO_InitStructure.GPIO_Pin = ICE5_SPI_MISO_PIN;
	GPIO_Init(ICE5_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

	/* Configure ICE5_SPI pin: MOSI */
	GPIO_InitStructure.GPIO_Pin = ICE5_SPI_MOSI_PIN;
	GPIO_Init(ICE5_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

	/* Configure ICE5_SPI_CS_PIN pin */
	GPIO_InitStructure.GPIO_Pin = ICE5_SPI_CS_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(ICE5_SPI_CS_GPIO_PORT, &GPIO_InitStructure);
	ICE5_SPI_CS_HIGH();
	
	/* Configure ICE5_CRST_PIN pin */
	GPIO_InitStructure.GPIO_Pin = ICE5_CRST_PIN;
	GPIO_Init(ICE5_CRST_GPIO_PORT, &GPIO_InitStructure);
	ICE5_CRST_HIGH();
	
	/* leave ICE5_CDONE_PIN pin as input with pullup */
	GPIO_InitStructure.GPIO_Pin = ICE5_CDONE_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(ICE5_CDONE_GPIO_PORT, &GPIO_InitStructure);

	/* Connect PXx to ICE5_SPI_SCK */
	GPIO_PinAFConfig(ICE5_SPI_SCK_GPIO_PORT, ICE5_SPI_SCK_SOURCE, ICE5_SPI_SCK_AF);

	/* Connect PXx to ICE5_SPI_MISO */
	GPIO_PinAFConfig(ICE5_SPI_MISO_GPIO_PORT, ICE5_SPI_MISO_SOURCE, ICE5_SPI_MISO_AF); 

	/* Connect PXx to ICE5_SPI_MOSI */
	GPIO_PinAFConfig(ICE5_SPI_MOSI_GPIO_PORT, ICE5_SPI_MOSI_SOURCE, ICE5_SPI_MOSI_AF);  

	/* ICE5_SPI Config */
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
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;

	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(ICE5_SPI, &SPI_InitStructure);

	SPI_RxFIFOThresholdConfig(ICE5_SPI, SPI_RxFIFOThreshold_QF);

	SPI_Cmd(ICE5_SPI, ENABLE); /* ICE5_SPI enable */
}

void ICE5_SPI_WriteByte(uint8_t Data)
{
	/* Wait until the transmit buffer is empty */
	//while(SPI_I2S_GetFlagStatus(ICE5_SPI, SPI_I2S_FLAG_TXE) == RESET)
	while((ICE5_SPI->SR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET)
	{
	}

	/* Send the byte */
	//SPI_SendData8(ICE5_SPI, Data);
	//ICE5_SPI->DR = (uint16_t)Data;
	*(__IO uint8_t *) ((uint32_t)ICE5_SPI+0x0C) = Data;
	
	/*!< Wait to receive a byte*/
	//while(SPI_I2S_GetFlagStatus(ICE5_SPI, SPI_I2S_FLAG_RXNE) == RESET)
	while((ICE5_SPI->SR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET)
	{
	}

	/* Return the byte read from the SPI bus */ 
	//return SPI_ReceiveData8(ICE5_SPI);
	//return *(__IO uint8_t *) ((uint32_t)ICE5_SPI+0x0C);
	uint8_t dummy = *(__IO uint8_t *) ((uint32_t)ICE5_SPI+0x0C);
    UNUSED(dummy); /* To avoid GCC warning */

}

uint8_t ICE5_SPI_WriteReadByte(uint8_t Data)
{
	/* Wait until the transmit buffer is empty */
	//while(SPI_I2S_GetFlagStatus(ICE5_SPI, SPI_I2S_FLAG_TXE) == RESET)
	while((ICE5_SPI->SR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET)
	{
	}

	/*!< Send the byte */
	//SPI_SendData8(ICE5_SPI, Data);
	//ICE5_SPI->DR = (uint16_t)Data;
	*(__IO uint8_t *) ((uint32_t)ICE5_SPI+0x0C) = Data;
	
	/* Wait to receive a byte*/
	//while(SPI_I2S_GetFlagStatus(ICE5_SPI, SPI_I2S_FLAG_RXNE) == RESET)
	while((ICE5_SPI->SR & SPI_I2S_FLAG_RXNE) == (uint16_t)RESET)
	{
	}

	/* Return the byte read from the SPI bus */ 
	//return SPI_ReceiveData8(ICE5_SPI);
	return *(__IO uint8_t *) ((uint32_t)ICE5_SPI+0x0C);
}

/*
 * Read a byte from SPI with dummy write
 */
uint8_t ICE5_SPI_ReadByte(void)
{
	uint8_t Data = 0;

	/* Wait until the transmit buffer is empty */
	while (SPI_I2S_GetFlagStatus(ICE5_SPI, SPI_I2S_FLAG_TXE) == RESET)
	{
	}
	/* Send the byte */
	SPI_SendData8(ICE5_SPI, ICE5_SPI_DUMMY_BYTE);

	/* Wait until a data is received */
	while (SPI_I2S_GetFlagStatus(ICE5_SPI, SPI_I2S_FLAG_RXNE) == RESET)
	{
	}
	/* Get the received data */
	Data = SPI_ReceiveData8(ICE5_SPI);

	/* Return the shifted data */
	return Data;
}

/*
 * Write a block of bytes to the ICE5 SPI
 */
void ICE5_SPI_WriteBlk(uint8_t *Data, uint32_t Count)
{
	while(Count--)
	{
		ICE5_SPI_WriteByte(*Data++);
	}
}

/*
 * private function to start the config process
 */
uint8_t ICE5_FPGA_Config_start(void)
{
	uint32_t timeout;
	
	/* drop CS bit to signal slave mode */
	ICE5_SPI_CS_LOW();
	
	/* drop reset bit */
	ICE5_CRST_LOW();
	
	/* delay */
	delay(1);
	
	/* Wait for done bit to go inactive */
	timeout = 100;
	while(timeout && (ICE5_CDONE_GET()==Bit_SET))
	{
		timeout--;
	}
	if(!timeout)
	{
		/* Done bit didn't respond to Reset */
        ICE5_CRST_HIGH();
        ICE5_SPI_CS_HIGH();
		return 1;
	}
    
	/* raise reset */
	ICE5_CRST_HIGH();
	
	/* delay to allow FPGA to reset */
	delay(1);
    
    return 0;
}

/*
 * private function to finish the config process
 */
uint8_t ICE5_FPGA_Config_finish(void)
{
	uint32_t timeout;
	
	/* send clocks while waiting for DONE to assert */
	timeout = 100;
	while(timeout && (ICE5_CDONE_GET()==Bit_RESET))
	{
		ICE5_SPI_WriteByte(ICE5_SPI_DUMMY_BYTE);
		timeout--;
	}
	if(!timeout)
	{
		/* FPGA didn't configure correctly */
		return 2;
	}
	
	/* send at least 49 more clocks */
	ICE5_SPI_WriteByte(ICE5_SPI_DUMMY_BYTE);
	ICE5_SPI_WriteByte(ICE5_SPI_DUMMY_BYTE);
	ICE5_SPI_WriteByte(ICE5_SPI_DUMMY_BYTE);
	ICE5_SPI_WriteByte(ICE5_SPI_DUMMY_BYTE);
	ICE5_SPI_WriteByte(ICE5_SPI_DUMMY_BYTE);
	ICE5_SPI_WriteByte(ICE5_SPI_DUMMY_BYTE);
	ICE5_SPI_WriteByte(ICE5_SPI_DUMMY_BYTE);

	/* Raise CS bit for subsequent slave transactions */
	ICE5_SPI_CS_HIGH();
	
	/* no error handling for now */
	return 0;
}

/*
 * configure the FPGA from MCU memory
 */
uint8_t ICE5_FPGA_Config(uint8_t *bitmap, uint32_t size)
{
    /* start configuration */
    if(ICE5_FPGA_Config_start())
    {
        /* error */
        return 1;
    }
    
	/* send the bitstream */
	ICE5_SPI_WriteBlk(bitmap, size);
	
    /* finish the configuration */
    return ICE5_FPGA_Config_finish();
}

/*
 * configure the FPGA from a FatFS file
 */
uint8_t ICE5_FPGA_Config_File(FIL *File)
{
    uint8_t buffer[512];
    FRESULT fres;
    uint32_t br;
    
    /* start configuration */
    if(ICE5_FPGA_Config_start())
    {
        /* error */
        return 1;
    }
    
	/* iterate over blocks */
    while((fres = f_read(File, buffer, 512, (UINT *)&br))==FR_OK)
    {
        /* done? */
        if(br == 0)
            break;
        
        /* send the block */
        ICE5_SPI_WriteBlk(buffer, br);
    }
    
    /* check for error */
    if(fres)
    {
        /* file error */
        ICE5_CRST_HIGH();
        ICE5_SPI_CS_HIGH();
        return 3;
    }
    
    /* finish the configuration */
    return ICE5_FPGA_Config_finish();
}

/*
 * Write a long to the FPGA SPI slave
 */
void ICE5_FPGA_Slave_Write(uint8_t Reg, uint32_t Data)
{
	/* Drop CS */
	ICE5_SPI_CS_LOW();
	
	/* msbit of byte 0 is 0 for write */
	ICE5_SPI_WriteByte(Reg & 0x7f);

	/* send next four bytes */
	ICE5_SPI_WriteByte((Data>>24) & 0xff);
	ICE5_SPI_WriteByte((Data>>16) & 0xff);
	ICE5_SPI_WriteByte((Data>> 8) & 0xff);
	ICE5_SPI_WriteByte((Data>> 0) & 0xff);
	
	/* Raise CS */
	ICE5_SPI_CS_HIGH();
}

/*
 * Read a long from the FPGA SPI slave
 */
void ICE5_FPGA_Slave_Read(uint8_t Reg, uint32_t *Data)
{
	uint8_t rx[4];
	
	/* Drop CS */
	ICE5_SPI_CS_LOW();
	
	/* msbit of byte 0 is 1 for write */
	ICE5_SPI_WriteByte(Reg | 0x80);

	/* get next four bytes */
	rx[0] = ICE5_SPI_ReadByte();
	rx[1] = ICE5_SPI_ReadByte();
	rx[2] = ICE5_SPI_ReadByte();
	rx[3] = ICE5_SPI_ReadByte();
	
	/* assemble result */
	*Data = (rx[0]<<24) | (rx[1]<<16) | (rx[2]<<8) | rx[3];
	
	/* Raise CS */
	ICE5_SPI_CS_HIGH();
}

