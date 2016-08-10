#!/usr/bin/python3
#
# CIC decimator test bench
#
# 07-23-2015 E. Brombaugh

import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import scipy.signal as signal
from scipy.fftpack import fft, ifft, fftfreq, fftshift
from ddc import cic_dec

# generate a signal
data_len = 2**16
Fs = 40e6
Fc = 20e3
data_bits = 12
data_scl = 2**(data_bits-1)-1
t = np.arange(data_len)/Fs
data_in = np.floor(data_scl * np.sin(2*np.pi*Fc*t) + 0.5)

# set system parameters and create an instance of the cic
cic_stages = 4;
cic_rate = 256;
uut = cic_dec(cic_stages, cic_rate, data_bits)

# run the model
cic_out = uut.calc(data_in)

# prepare to plot
if 0:
    data = data_in
    rate = Fs
else:
    data = cic_out
    rate = Fs/cic_rate

data_len = len(data)
t = np.arange(data_len)/rate

# plot of time
fig = plt.figure(1)
plt.plot(t, np.real(data))
plt.grid()
plt.xlabel("Time")
plt.ylabel("data")
plt.title("sinusoid - time")

# plot of frequency
fig = plt.figure(2)
f = Fs * fftshift(fftfreq(data_len))/1e6
win = signal.blackmanharris(data_len)
data_bhwin = data * win
bh_gain = sum(win)/data_len
data_dB = 20*np.log10(np.abs(fftshift(fft(data_bhwin)))/
                      (data_len*(data_scl/2)*bh_gain))
plt.plot(f, data_dB)
plt.grid()
plt.xlabel("Frequency (MHz)")
plt.ylabel("dB")
plt.title("sinusoid - freq")
plt.xlim((0, (Fs/1e6)/2))
plt.show()

