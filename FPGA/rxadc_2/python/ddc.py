#!/usr/bin/python3
#
# Digital DownConverter and support functions
#
# 07-23-2015 E. Brombaugh

import numpy as np
import matplotlib as mpl
import matplotlib.pyplot as plt
import scipy.signal as signal
from scipy.fftpack import fft, ifft, fftfreq, fftshift
from write_meminit import write_meminit

# tuner function - IF real to baseband complex
class tuner:
    # init an instance of the model
    def __init__(self, tune_bits, lut_bits, lo_bits):
        self.tune_bits = tune_bits
        self.lut_bits = lut_bits
        self.lo_scl = 2**(lo_bits-1)
        self.ftune = 0

    # set the tuner frequency
    def set_ftune(self, Ft):
        self.ftune = -np.floor((2**self.tune_bits)*Ft)

    # compute the model
    def calc(self, x):
        # phase accumulator with fixed bitwidth
        accum = (np.arange(len(x))*self.ftune) % 2**self.tune_bits
        
        # truncate phase to LUT address bits
        addr = np.floor(accum/(2**(self.tune_bits-self.lut_bits)))

        # look up sine table
        phs = addr / (2**self.lut_bits);
        lo = (self.lo_scl-1)*np.exp(2*np.pi*phs*1j)
        lo_i = np.floor(np.real(lo)+0.5)
        lo_q = np.floor(np.imag(lo)+0.5)
        lo = lo_i + lo_q * 1j

        # complex multiply
        y = (lo * x)/self.lo_scl
        y_i = np.floor(np.real(y)+0.5)
        y_q = np.floor(np.imag(y)+0.5)
        y = y_i + y_q * 1j
        return y

# CIC decimator
class cic_dec:
    # init an instance
    def __init__(self, num_stages, dec_rate, x_bits):
        self.num_stages = num_stages
        self.dec_rate = dec_rate
        acc_bits = x_bits + self.num_stages * np.log2(self.dec_rate)
        self.sat_val = 2**acc_bits
        
    # compute the model
    def calc(self, x):
        # setup
        y_len = np.floor(len(x)/self.dec_rate)
        new_intg = np.zeros(self.num_stages, dtype=np.int)
        intg = np.zeros(self.num_stages, dtype=np.int)
        comb = np.zeros(self.num_stages, dtype=np.int)
        y = np.zeros(y_len, dtype=np.int)

        # iterate over output samples
        for y_idx in np.arange(y_len):
            # Integrators
            for x_idx in np.arange(self.dec_rate):
                new_intg[0] = ((intg[0] + x[(y_idx * self.dec_rate) + x_idx] + self.sat_val/2) % self.sat_val) - self.sat_val/2
                
                for stage in np.arange(1,self.num_stages):
                    new_intg[stage] = (intg[stage] + intg[stage-1]) % self.sat_val
                    
                intg = 1*new_intg # copy, don't just set the pointer!
                
            # Combs
            temp = intg[self.num_stages-1]
            for stage in np.arange(self.num_stages):
                diff = ((temp - comb[stage]+self.sat_val/2) % self.sat_val) - self.sat_val/2
                comb[stage] = temp
                temp = 1*diff

            # output sample
            y[y_idx] = temp

        # done
        return y

# FIR decimator
class fir4dec:
    # init an instance
    def __init__(self, fir_len, fir_bits, tb_width):
        tb_ctr = 1/(2*4)
        pass_corner = tb_ctr - (tb_ctr*tb_width/2)
        stop_corner = tb_ctr + (tb_ctr*tb_width/2)
        fir_bands = [0, pass_corner, stop_corner, 0.5]
        b = signal.remez(fir_len, fir_bands, [1, 0])
        coeff_scl = 2**(fir_bits-1)
        self.fir_coeff = np.floor(b*coeff_scl + 0.5)
        
        # Dump Coefficients?
        if 1:
            write_meminit("fir4dec_coeff.v", self.fir_coeff)

        self.fir_coeff = self.fir_coeff/coeff_scl;
        
        # plot FIR response?
        if 1:
            W, H = signal.freqz(self.fir_coeff)
            plt.figure()
            plt.plot(W/(2*np.pi), 20*np.log10(np.abs(H)))
            plt.grid()
            plt.xlabel("Freq (normalized)")
            plt.ylabel("dB")
            plt.title("fir4dec response (close to continue sim)")
            plt.show()

    # compute the model
    def calc(self, x):
        # filter & quantize
        full_out = signal.lfilter(self.fir_coeff, 1, x)
        
        # Decimate by 4
        out = full_out[::4]

        # convert back to complex
        out_i = np.floor(np.real(out)+0.5)
        out_q = np.floor(np.imag(out)+0.5)
        out = out_i + out_q * 1j

        # done
        return out

# DDC class
class ddc:
    # init an instance
    def __init__(self, data_bits):
        self.x_bits = data_bits

        # Tuner setup
        self.tuner_inst = tuner(24, 8, data_bits)
        
        # CIC setup
        self.cic_i_inst = cic_dec(4, 256, data_bits)
        self.cic_q_inst = cic_dec(4, 256, data_bits)
        self.cic_scl = 2**(self.cic_i_inst.num_stages * np.ceil(np.log2(self.cic_i_inst.dec_rate)) - 
                        1 - np.floor(np.log2(self.cic_i_inst.dec_rate)/2))
        
        # FIR setup
        self.fir_inst = fir4dec(250, 18, 0.2)

    # set the tuner frequency
    def set_ftune(self, Ft):
        self.tuner_inst.set_ftune(Ft)
        
    # compute the model
    def calc(self, x):
        # tune input
        tuner_out = self.tuner_inst.calc(x)

        # First decimation stage
        cic_i = self.cic_i_inst.calc(np.real(tuner_out))
        cic_q = self.cic_i_inst.calc(np.imag(tuner_out))
        cic_out = cic_i + cic_q * 1j

        # scale back the CIC output
        cic_scaled_raw = cic_out / self.cic_scl
        cic_scaled_i = np.floor(np.real(cic_scaled_raw) + 0.5)
        cic_scaled_q = np.floor(np.imag(cic_scaled_raw) + 0.5)
        cic_scaled = cic_scaled_i + cic_scaled_q * 1j

        # FIR cleanup filter
        fir_out = self.fir_inst.calc(cic_scaled)
                
        # done
        #return cic_scaled
        return fir_out
