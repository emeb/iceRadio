#!/usr/bin/python3
#
# Digital DownConverter testbench
#
# 07-23-2015 E. Brombaugh

# Test out the DDC
import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import scipy.signal as signal
from scipy.fftpack import fft, ifft, fftfreq, fftshift
from ddc import ddc
       
# generate a signal
data_len = 2**18
Fs = 20e6
Ft = 7.125e6
data_bits = 10
data_scl = 2**(data_bits-1)-1
t = np.arange(data_len)/Fs
data_in = np.floor(data_scl/2 * (np.sin(2*np.pi*(Ft+1000)*t) + np.sin(2*np.pi*(Ft+25000)*t)) + 0.5)

# init the model
uut = ddc(data_bits)
uut.set_ftune(Ft/Fs)

# run the ddc
ddc_out = uut.calc(data_in)

# prepare to plot
data = ddc_out
rate = Fs/(uut.cic_i_inst.dec_rate*4)
data_len = len(data)
t = np.arange(data_len)/rate

# plot of time
fig = plt.figure(1)
plt.plot(t, np.real(data), label="real")
plt.plot(t, np.imag(data), label="imag")
plt.grid()
plt.xlabel("Time")
plt.ylabel("data")
plt.title("sinusoid - time")
plt.legend()

# plot of frequency
fig = plt.figure(2)
f = rate * fftshift(fftfreq(data_len))/1e3
win = signal.blackmanharris(data_len)
data_bhwin = data * win
bh_gain = sum(win)/data_len
data_dB = 20*np.log10(np.abs(fftshift(fft(data_bhwin)))/
                      (data_len*(data_scl/2)*bh_gain))
plt.plot(f, data_dB)
plt.grid()
plt.xlabel("Frequency (kHz)")
plt.ylabel("dB")
plt.title("sinusoid - freq")
#plt.xlim((0, (rate/1e3)/2))
plt.show()
