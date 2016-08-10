/*
 * ice5.c - interface routines for STM32F303 SPI to ice5 FPGA
 * 07-02-16 E. Brombaugh
 */
 
#ifndef __ICE5__
#define __ICE5__

#include "stm32f30x.h"
#include "ff.h"

void ICE5_Init(void);
uint8_t ICE5_FPGA_Config(uint8_t *bitmap, uint32_t size);
uint8_t ICE5_FPGA_Config_File(FIL *File);
void ICE5_FPGA_Slave_Write(uint8_t Reg, uint32_t Data);
void ICE5_FPGA_Slave_Read(uint8_t Reg, uint32_t *Data);

#endif
