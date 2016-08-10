#!/usr/bin/python3
#
# 1/4-cycle sine table
#
# 07-11-2016 E. Brombaugh

import numpy as np
import math as math
import matplotlib as mpl
import matplotlib.pyplot as plt
import scipy.signal as signal
from write_meminit import write_meminit
from write_memh import write_memh

# fixed params
sine_len = 1024
sine_bits = 16
scl = 2**(sine_bits-1)-1
LUT = np.zeros(sine_len, dtype=np.int)

# stuff into array
for i in np.arange(sine_len):
    LUT[i] = np.floor(math.sin((i+0.5)*math.pi/(2*sine_len))*scl + 0.5)

# Plot combined coeffs
if 1:
    wave = np.zeros(4*sine_len, dtype=np.int)
    wave[0:sine_len] = LUT;
    wave[sine_len:sine_len*2] = LUT[np.arange(sine_len-1,-1,-1)];
    wave[sine_len*2:sine_len*3] = -LUT;
    wave[sine_len*3:sine_len*4] = -LUT[np.arange(sine_len-1,-1,-1)];
    plt.figure()
    plt.plot(wave)
    plt.grid()
    plt.xlabel("index")
    plt.ylabel("value")
    plt.title("Sine Table (close to continue)")
    plt.show()

# Dump Coefficients to FPGA mem file
if 0:
    # Xilinx mem init file
    write_meminit("sine_table_1k.v", LUT)
else:
    # Verilog readmemh file
    write_memh("sine_table_1k.memh", LUT)
