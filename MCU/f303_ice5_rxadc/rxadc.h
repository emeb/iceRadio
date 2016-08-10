/*
 * rxadc.h - low level interfaces to rxadc FPGA design
 * 07-19-14 E. Brombaugh
 */

#ifndef __rxadc__
#define __rxadc__

uint32_t rxadc_get_lo(void);
uint32_t rxadc_set_lo(uint32_t freq);
uint8_t rxadc_get_mux(void);
void rxadc_set_mux(uint8_t mux);
void rxadc_set_ns(uint8_t ns);

#endif
