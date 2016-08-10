# ##############################################################################

# iCEcube SDC

# Version:            2016.02.27810

# File Generated:     Jul 8 2016 15:53:44

# ##############################################################################

####---- CreateClock list ----1
create_clock  -period 50.00 -waveform {0.00 25.00} -name {SPI_SCLK} [get_ports {SPI_SCLK}] 
create_clock  -period 25.00 -waveform {0.00 12.50} -name {rxadc_clk} [get_ports {rxadc_clk}] 

