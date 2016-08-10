/*
	main.c
	
	Part of f303_ice5 - stm32f303 & ice5lp4k FPGA
	Copyright 07-02-2016 E. Brombaugh
*/
#include <stdio.h>
#include <stdlib.h>
#include "stm32f30x.h"
#include "cyclesleep.h"
#include "systick.h"
#include "usart.h"
#include "led.h"
#include "adc.h"
#include "ice5.h"
#include "cmd.h"
#include "shared_spi.h"
#include "st7735.h"
#include "diskio.h"
#include "ff.h"
#include "audio.h"
#include "i2s.h"
#include "rxadc.h"
#include "menu.h"

/* uncomment this line for FPGA bitstream in flash */
//#define FLASH_FPGA

/* uncomment this line for serial command line */
//#define SERIAL_CMD

#ifdef FLASH_FPGA
/* FPGA bitstream */
extern uint8_t _binary_bitmap_bin_start;
extern uint8_t _binary_bitmap_bin_end;
#endif

/*
 * start 
 */
int main(void)
{
#ifdef FLASH_FPGA
	uint32_t bitmap_size = &_binary_bitmap_bin_end - &_binary_bitmap_bin_start;
#else
    char *bitmap_name = "rxadc_23.bin";
#endif
	int rxchar, i;
	uint32_t delaygoal;
	uint8_t result;
    uint8_t buffer[17];
    
	/* start cycle counter */
	cyccnt_enable();
	
	/* init LEDs & Switches */
	SysTick_Init();
	LEDInit();
	
	/* init the adc */
	setup_adc();

	/* Setup USART diag output */
	setup_usart1();
	printf("\niceRadio\n");
	
	/* enable shared spi */
	setup_shared_spi();
	printf("Shared SPI configured\n");

	/* init LCD */
    ST7735_init();
    ST7735_fillScreen(ST7735_BLACK);
    ST7735_drawstr(0, 0, (uint8_t *)"Hello World!", ST7735_WHITE, ST7735_BLACK);
    printf("LCD initialized\n");
    
	/* Setup FPGA */
	ICE5_Init();
#ifdef FLASH_FPGA
	printf("Configuring %d bytes....", (unsigned int)bitmap_size);
	result = ICE5_FPGA_Config(&_binary_bitmap_bin_start, bitmap_size);
	if(result)
        printf("FPGA configure error: %d.\n", result);
    else
    {
        uint32_t Data;
        printf("FPGA configured.\n");
        ICE5_FPGA_Slave_Read(0, &Data);
        printf("ID = 0x%08X\n", (int)Data);
    }
#else
	printf("Configuring FPGA from %s...\n", bitmap_name);
    {
        FRESULT fres;
        FATFS Fatfs;
        FIL File;
        uint32_t Data;
        
        /* mount the SD card */
        fres = f_mount(0, &Fatfs);
        
        /* try to open the file */
        fres = f_open(&File, bitmap_name, FA_READ);
        if(!fres)
        {
            /* try to configure FPGA */
            result = ICE5_FPGA_Config_File(&File);
            
            if(result)
                printf("ERROR - ICE5_FPGA_Config_File returned %d\r\n", result);
            else
            {
                printf("FPGA configured.\n");
                ICE5_FPGA_Slave_Read(0, &Data);
                printf("ID = 0x%08X\n", (int)Data);
            }
        }
        else
        {
            printf("ERROR - Can't open file %s\r\n", bitmap_name);
        }
    }
#endif

    /* init the audio handler */
    Audio_Init();
    printf("Audio initialized\n");
    
    /* init the I2S slave port -- AFTER FPGA is running ! */
    i2s_init();
    printf("I2S initialized\n");
    
#ifdef SERIAL_CMD
    printf("Initializing Serial Command Processor...\n");
    init_cmd();
#else
	/* init the GUI menu */
    delay(100);
	menu_init();
	printf("Menu initialized...\n");
#endif
    
	/* loop forever */
	delaygoal = cyclegoal_ms(100);
	while(1)
	{
		/* Blink the heartbeat LED */
		if(!cyclecheck(delaygoal))
		{
			LEDToggle();
        
#ifdef SERIAL_CMD
            /* dump out ADC channels to LCD */
            for(i=0;i<ADC_CHANNELS;i++)
            {
                sprintf((char *)buffer, "ADC[% 1d]: % 5d ", i, ADC_Channels[i]);
                ST7735_drawstr(0, 8*(i+1), buffer, ST7735_GREEN, ST7735_BLACK);
            }
            
            /* Tune Frequency */
            sprintf((char *)buffer, "Freq: % 8d ", (int)rxadc_get_lo());
            ST7735_drawstr(0, 8*8, buffer, ST7735_RED, ST7735_BLACK);
            
            /* Tune Frequency */
            sprintf((char *)buffer, "AGC: % 4d ", (int)Audio_GetRSSI());
            ST7735_drawstr(0, 8*9, buffer, ST7735_BLUE, ST7735_BLACK);
#endif
            
            /* goal for next cycle */
			delaygoal = cyclegoal_ms(100);
		}
		
#ifdef SERIAL_CMD
		/* UART command processing */
		if((rxchar = get_usart())!= EOF)
		{
			/* Parse commands */
			cmd_parse(rxchar);
		}
#else
        /* GUI menu */
		menu_update();
#endif
    }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif
