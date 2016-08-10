#!/usr/bin/python3
#
# FIR coeffs for 8x decimator
#
# 07-17-2016 E. Brombaugh

# Test out the DDC
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import scipy.signal as signal
from scipy.fftpack import fft, ifft, fftfreq, fftshift
from write_meminit import write_meminit
from write_memh import write_memh

# fixed params
fir_len = 246
fir_bits = 19 # leave @ 19 for better quant - correct in HW scaling
Fs = 40e6 / 256
LUT = np.zeros(256, dtype=np.int)

# per-filter params
tb_Fctr = [ Fs/16, 4000, 2800, 500 ]
tb_width = [ 0.4, 0.8, 1.5, 3]
coeff_scl = 2**(fir_bits-1)

# pick one filter for this pass
i = 0 # Fs/16 for 8x decimation = 9765Hz

tb_ctr = tb_Fctr[i]/Fs
pass_corner = tb_ctr - (tb_ctr*tb_width[i]/2)
stop_corner = tb_ctr + (tb_ctr*tb_width[i]/2)
fir_bands = [0, pass_corner, stop_corner, 0.5]
b = signal.remez(fir_len, fir_bands, [1, 0])
fir_coeff = np.floor(b*coeff_scl + 0.5)

# compute coeff word size
coeff_bits = 1+np.ceil(np.log2(np.max(np.abs(fir_coeff))))
print('Max coeff bits = ', coeff_bits)

# compute worst-case sum for hardware sizing
acc_sum = np.sum(32768*np.abs(fir_coeff))
acc_bits = 1+np.ceil(np.log2(acc_sum))
print('Max accumulator bits = ', acc_bits)

# plot FIR response?
if 1:
    W, H = signal.freqz(fir_coeff)
    passband_max = np.max(np.abs(H))
    stopband_idx = np.nonzero(W/np.pi > (stop_corner+0.1))
    stopband_max = np.max(np.abs(H[stopband_idx]))
    print('Stopband Atten = ', 20*np.log10(passband_max/stopband_max), 'dB')
    plt.figure()
    plt.plot((Fs)*W/(2*np.pi), 20*np.log10(np.abs(H)))
    plt.grid()
    plt.xlabel("Freq (Hz)")
    plt.ylabel("dB")
    plt.title("fir4dec response (close to continue)")
    plt.show()

# stuff into array
LUT[0:fir_len] = fir_coeff

# Plot combined coeffs
if 1:
    plt.figure()
    plt.plot(LUT)
    plt.grid()
    plt.xlabel("index")
    plt.ylabel("value")
    plt.title("Combined coeffs (close to continue)")
    plt.show()

# Dump Coefficients to FPGA mem file
if 0:
    # Xilinx mem init file
    write_meminit("fir8dec_coeff.v", LUT)
else:
    # Verilog readmemh file
    write_memh("fir8dec_coeff.memh", LUT)
