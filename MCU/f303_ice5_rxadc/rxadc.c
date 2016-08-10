/*
 * rxadc.c - low level interfaces to rxadc FPGA design
 * 07-19-14 E. Brombaugh
 */

#include "stm32f30x.h"
#include "rxadc.h"
#include "ice5.h"

/*
 * lo read 
 */
uint32_t rxadc_get_lo(void)
{
	uint32_t data;
    uint64_t temp;
    ICE5_FPGA_Slave_Read(16, &data);
    temp = ((uint64_t)data)*40000000;
    return temp>>26;
}

/*
 * lo write
 */
uint32_t rxadc_set_lo(uint32_t freq)
{
    uint64_t temp = ((uint64_t)freq)<<26;
    temp = temp / 40000000;
    freq = temp;
    ICE5_FPGA_Slave_Write(16, freq);
    return freq;
}

/*
 * audio mux read 
 */
uint8_t rxadc_get_mux(void)
{
    uint32_t mux;
    ICE5_FPGA_Slave_Read(17, &mux);
    return mux & 1;
}

/*
 * audio mux write 
 */
void rxadc_set_mux(uint8_t mux)
{
    ICE5_FPGA_Slave_Write(17, mux);
}

/*
 * noise_shape write 
 */
void rxadc_set_ns(uint8_t ns)
{
    ICE5_FPGA_Slave_Write(18, ns);
}