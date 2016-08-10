/*
 * cmd.h - Command parsing routines for STM32F303 breakout SPI to ice5 FPGA
 * 05-11-16 E. Brombaugh
 */
 
#ifndef __cmd__
#define __cmd__

void init_cmd(void);
void cmd_parse(char ch);

#endif
