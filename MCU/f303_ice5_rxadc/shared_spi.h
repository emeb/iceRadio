/*
 * shared_spi.c - interface routines for F303_ICE5 SPI
 * 07-05-16 E. Brombaugh
 */
 
#ifndef __SHARED_SPI__
#define __SHARED_SPI_

#include "stm32f30x.h"

/*
 * SPI Interface pins
 */
#define SD_SPI                           SPI3
#define SD_SPI_CLK                       RCC_APB1Periph_SPI3
#define SD_SPI_SCK_PIN                   GPIO_Pin_3
#define SD_SPI_SCK_GPIO_PORT             GPIOB
#define SD_SPI_SCK_GPIO_CLK              RCC_AHBPeriph_GPIOB
#define SD_SPI_SCK_SOURCE                GPIO_PinSource3
#define SD_SPI_SCK_AF                    GPIO_AF_6

#define SD_SPI_MISO_PIN                  GPIO_Pin_4
#define SD_SPI_MISO_GPIO_PORT            GPIOB
#define SD_SPI_MISO_GPIO_CLK             RCC_AHBPeriph_GPIOB
#define SD_SPI_MISO_SOURCE               GPIO_PinSource4
#define SD_SPI_MISO_AF                   GPIO_AF_6

#define SD_SPI_MOSI_PIN                  GPIO_Pin_5
#define SD_SPI_MOSI_GPIO_PORT            GPIOB
#define SD_SPI_MOSI_GPIO_CLK             RCC_AHBPeriph_GPIOB
#define SD_SPI_MOSI_SOURCE               GPIO_PinSource5
#define SD_SPI_MOSI_AF                   GPIO_AF_6

#define SD_CS_PIN                        GPIO_Pin_15
#define SD_CS_GPIO_PORT                  GPIOA
#define SD_CS_GPIO_CLK                   RCC_AHBPeriph_GPIOA

#define LCD_LITE_PIN                     GPIO_Pin_8
#define LCD_LITE_GPIO_PORT               GPIOB
#define LCD_LITE_GPIO_CLK                RCC_AHBPeriph_GPIOB

#define LCD_CS_PIN                       GPIO_Pin_9
#define LCD_CS_GPIO_PORT                 GPIOB
#define LCD_CS_GPIO_CLK                  RCC_AHBPeriph_GPIOB

#define LCD_DC_PIN                       GPIO_Pin_14
#define LCD_DC_GPIO_PORT                 GPIOC
#define LCD_DC_GPIO_CLK                  RCC_AHBPeriph_GPIOC

#define LCD_RST_PIN                      GPIO_Pin_15
#define LCD_RST_GPIO_PORT                GPIOC
#define LCD_RST_GPIO_CLK                 RCC_AHBPeriph_GPIOC

#define SD_CS_LOW()     GPIO_ResetBits(SD_CS_GPIO_PORT, SD_CS_PIN)
#define SD_CS_HIGH()    GPIO_SetBits(SD_CS_GPIO_PORT, SD_CS_PIN)
#define LCD_CS_LOW()    GPIO_ResetBits(LCD_CS_GPIO_PORT, LCD_CS_PIN)
#define LCD_CS_HIGH()   GPIO_SetBits(LCD_CS_GPIO_PORT, LCD_CS_PIN)
#define LCD_DC_CMD()    GPIO_ResetBits(LCD_DC_GPIO_PORT, LCD_DC_PIN)
#define LCD_DC_DATA()   GPIO_SetBits(LCD_DC_GPIO_PORT, LCD_DC_PIN)
#define LCD_RST_LOW()   GPIO_ResetBits(LCD_RST_GPIO_PORT, LCD_RST_PIN)
#define LCD_RST_HIGH()  GPIO_SetBits(LCD_RST_GPIO_PORT, LCD_RST_PIN)
#define LCD_LITE_LOW()  GPIO_ResetBits(LCD_LITE_GPIO_PORT, LCD_LITE_PIN)
#define LCD_LITE_HIGH() GPIO_SetBits(LCD_LITE_GPIO_PORT, LCD_LITE_PIN)
#define SD_DUMMY_BYTE   0xFF

void setup_shared_spi(void);
void SPI_WriteByte(uint8_t Data);
uint8_t SPI_WriteReadByte(uint8_t Data);
uint8_t SPI_ReadByte(void);
void SPI_fake_DMA_WriteBytes(uint8_t *buffer, uint16_t len);
void SPI_InitDMA(void);
void SPI_start_DMA_WriteBytes(uint8_t *buffer, uint16_t len);
void SPI_end_DMA_WriteBytes(void);
void SPI_real_DMA_WriteBytes(uint8_t *buffer, uint16_t len);

#endif
