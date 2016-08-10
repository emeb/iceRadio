/*
 * audio.c - Audio processing routines
 */

#include <math.h>
#include "audio.h"
#include "audio_lib.h"
#include "iir.h"

/* Stereo buffers */
#define STEREO_BUFSZ (DMA_BUFFSZ/2)
#define MONO_BUFSZ (STEREO_BUFSZ/2)
int16_t	buffer1[MONO_BUFSZ], buffer2[MONO_BUFSZ];

/* sine LUT */
float32_t sine_lut[256];

/* state for demods */
float32_t i_dc_acc, q_dc_acc, am_dc_acc;
float32_t pll_intg, pll_frq, pll_phs;
float32_t mag_sq;
float32_t alpha, logR, f_pwr, agc_acc, agc_gain;
float32_t nbfm_pphs, nbfm_de_acc;
float32_t rssi;
float32_t freq_norm;
int32_t phs, frq;
uint16_t mute_state, mute_queued, pll_count;
uint8_t demod_type, pll_state;

/* IIR lowpass filters */
#include "iir_coeffs.h"
uint8_t filter_num;
bq_state i_iir_s[3], q_iir_s[3];
iir i_iir, q_iir;

/* SSB Hilbert IIRs */
#define SHIFT_STAGES 6
const float32_t c_ahi[] =
{
	0.5131884F,
	0.8133175F,
	0.9359722F,
	0.9791145F,
	0.9934793F,
	0.9989305F
};
	
const float32_t c_alo[] =
{
	0.2755710F,
	0.6922636F,
	0.8896328F,
	0.9633075F,
	0.9882633F,
	0.9965990F
};
float32_t ahi[SHIFT_STAGES], alo[SHIFT_STAGES];
struct
{	
	/* Allpass state */
	float32_t state_hi_i[SHIFT_STAGES][2];
	float32_t state_hi_o[SHIFT_STAGES][2], dly;
	float32_t state_lo_i[SHIFT_STAGES][2];
	float32_t state_lo_o[SHIFT_STAGES][2];
	
	/* DC block state */
	float32_t lo_dc, hi_dc;
	
} fb;


#define DC_SCALE (1.0F/1000.0F)
#define PLL_P_WIDE 1.0e-3F
#define PLL_I_WIDE 1.0e-6F
#define PLL_P_NARR 5.0e-6F
#define PLL_I_NARR 1.0e-8F
#define PLL_LOCK_THRESH 0.01F
#define NBFM_DEV_SCL ((19531.25F/2500.0F)/(2.0F*PI))
#define NBFM_DE_SCALE ((2.0F*PI*300)/19531.25F)

/*
 * Set up audio processing
 */
void Audio_Init(void)
{
	int16_t i;
	
	/* fill the sine LUT */
	for(i=0;i<256;i++)
		sine_lut[i] = sinf(6.2832F*(float32_t)i/256.0F);

	/* setup input DC block */
	i_dc_acc = q_dc_acc = am_dc_acc = 0.0F;
    
    /* init the sync AM pll */
    pll_phs = 0.0F;
    pll_intg = 0.0F;
	pll_state = 0;
    pll_count = 0;
    
    /* Narrowband FM state */
    nbfm_pphs = nbfm_de_acc = 0.0F;
    
	/* setup the IIR filters */
	Audio_SetFilter(0);
	
	/* init the AGC */
	alpha = 0.001F;
	logR = logf(0.01F);
	f_pwr = 0.0F;
	agc_acc = 0.0F;
	agc_gain = 1.0F;

	/* init the signal strength */
	rssi = 1.0F;
	
	/* set up SSB demod */
	/* compute the allpass coeffs */
	for(i=0;i<SHIFT_STAGES;i++)
	{
		ahi[i] = c_ahi[i]*c_ahi[i];
		alo[i] = c_alo[i]*c_alo[i];
	}

	/* clear allpass state memory */
	for(i=0;i<SHIFT_STAGES;i++)
	{
		fb.state_hi_i[i][0] = 0.0F;
		fb.state_hi_i[i][1] = 0.0F;
		fb.state_hi_o[i][0] = 0.0F;
		fb.state_hi_o[i][1] = 0.0F;
		fb.state_lo_i[i][0] = 0.0F;
		fb.state_lo_i[i][1] = 0.0F;
		fb.state_lo_o[i][0] = 0.0F;
		fb.state_lo_o[i][1] = 0.0F;
	}
	fb.dly = 0.0F;
	
	/* init DC blocks */
	fb.lo_dc = fb.hi_dc = 0.0F;
	
	/* Init mute state */
	mute_state = 0;
	
	/* Init Demod Mode */
	demod_type = 0; /* 0 =AM */
}

/*
 * Set the filter bandwidth
 */
void Audio_SetFilter(uint8_t filter)
{
    filter_num = filter;
    
	/* compute coeff index */
	filter_num = (filter_num > 5) ? 5 : filter_num;
	filter = filter_num * 3;
	
	/* init the filters with specified filter */
	iir_init(&i_iir, i_iir_s, (bq_coeffs *)&c[filter], 3);
	iir_init(&q_iir, q_iir_s, (bq_coeffs *)&c[filter], 3);
}

uint8_t Audio_GetFilter(void)
{
    return filter_num;
}

/*
 * Get RSSI
 * Calibrated for 200mVpp @ RXADC input = -10dBm
 */
int16_t Audio_GetRSSI(void)
{
	int16_t rssi_dBm;
    float32_t temp = 1.0F/agc_gain;
    rssi_dBm = 10.0F*log10f(temp*temp)-24.0F;
    return rssi_dBm;
}

/*
 * Set the demodulator type
 */
void Audio_SetDemod(uint8_t demod)
{
	demod_type = demod % 7;
}

/*
 * Get the demodulator type
 */
int8_t Audio_GetDemod(void)
{
	return demod_type;
}

/*
 * Mute control
 */
void Audio_SetMute(uint8_t State)
{
	if(State==0)
	{
		/* want unmute */
		if(mute_state == 512)
		{
			/* currently muted, so start ramping up */
			mute_state = 0;
		}
	}
	else
	{
		/* want mute */
		if(mute_state == 256)
		{
			/* currently unmuted, so start ramping down */
			mute_state++;
		}
	}
}

uint16_t Audio_GetMute(void)
{
	return mute_state;
}

/*
 * Observable for debug
 */
int16_t Audio_GetParam(void)
{
    return 1000*am_dc_acc;
}

/*
 * Observable for debug
 */
int16_t Audio_GetSyncFrq(void)
{
    return (int16_t)(pll_frq*19531.25F+0.5F);
}

/*
 * Observable for debug
 */
int16_t Audio_GetSyncSt(void)
{
    return pll_state;
}

/*
 * interpolating LUT-based sine wavetable
 */
float32_t sine_wave(float32_t phs)
{
    float32_t iphs;
    phs = modff(phs, &iphs); /* modulo 1.0 */
	phs *= 256.0F;
	uint32_t phs_int = (uint32_t)phs;
	float32_t phs_frac = phs - (float32_t)phs_int;
	float32_t result = sine_lut[phs_int]*(1.0F-phs_frac);
	return result + sine_lut[(phs_int+1)&0xFF]*phs_frac;
}

/*
 * Audio processing ISR
 */
void Audio_Proc(int16_t *src, int16_t *dst)
{
#if 0
	int i;
    
    /* Split Stereo */
	audio_split_stereo(STEREO_BUFSZ, src, buffer1, buffer2);
    
    /* scale the data */
    for(i=0;i<MONO_BUFSZ;i++)
    {
        buffer1[i] >>= 1;
        buffer2[i] >>= 1;
    }
    
	/* Combine stereo */
	audio_comb_stereo(STEREO_BUFSZ, dst, buffer1, buffer2);
#else
	uint16_t index, m;
	float32_t i_in, q_in;
	float32_t i_dcb, q_dcb;
	float32_t i_filter, q_filter;
	float32_t am_raw, am_dcb;
	float32_t hilb_in, hilb_out;
	float32_t ap_i, ap_q;
	float32_t ssb_i, ssb_q;
	float32_t i_det = 0.0F, q_det = 0.0F;
	float32_t mute_gain, i_mute, q_mute;
    
	/* process I2S data */
	for(index=0;index<MONO_BUFSZ;index++)
	{
		/* get input from FPGA & convert to float */
		i_in = (float32_t)*src++/32768.0F;
		q_in = (float32_t)*src++/32768.0F;

		/* Input DC blocker ------------------------ */
		i_dcb = i_in - i_dc_acc;
		i_dc_acc += (i_dcb * DC_SCALE);
		q_dcb = q_in - q_dc_acc;
		q_dc_acc += (q_dcb * DC_SCALE);
		
        if((demod_type==0)||(demod_type==1))
        {
            /* bypass for AM to avoid distortion when carrier @ DC */
            i_dcb = i_in;
            q_dcb = q_in;
        }
        
		/* filter ---------------------------------- */
		i_filter = iir_calc(&i_iir, i_dcb);
		q_filter = iir_calc(&q_iir, q_dcb);
		
		/* AGC  -------------------------------------*/
		i_filter = i_filter * agc_gain;
		q_filter = q_filter * agc_gain;
		mag_sq = i_filter*i_filter + q_filter*q_filter;
#if 0
        /* single slope */
		f_pwr = 0.99*f_pwr + 0.01*mag_sq;
#else
        /* dual slope */
        if(f_pwr > mag_sq)
            /* decay */
            f_pwr = 0.99F * f_pwr + 0.01F * mag_sq;
        else
            /* attack 10x faster */
            f_pwr = 0.9F * f_pwr + 0.1F * mag_sq;
#endif
		/*
		 * detector - one of
		 *  0 AM (mag)
		 *  1 Sync AM (mix w/ PLL)
		 *  2 SSB upper (phasing)
		 *  3 SSB lower (phasing)
		 *  4 SSB upper + lower (phasing)
         *  5 raw I&Q with filter
		 */
		switch(demod_type)
		{
			case 0:	/* AM */
			case 1:	/* Sync AM */
				if(demod_type == 0)
                {
                    /* AM mag detector */
                    am_raw = sqrtf(mag_sq);
                }
                else
                {
                    /* AM Sync detector */
                    float32_t pll_i_lo, pll_q_lo, pll_i_bb, pll_q_bb, pll_err;
                    
                    /* get LO */
                    pll_i_lo = sine_wave(pll_phs+0.25F);
                    pll_q_lo = sine_wave(pll_phs);
                    
                    /* conjugate mix down */
                    pll_i_bb = i_filter * pll_i_lo + q_filter * pll_q_lo;
                    pll_q_bb = q_filter * pll_i_lo - i_filter * pll_q_lo;
                    
                    /* error is angle or imag of baseband */
                    pll_err = atan2f(pll_q_bb, pll_i_bb);
                    
                    /* check for DC estimate ramp up after PLL lock */
                    if((pll_state == 0) && (am_dc_acc >= PLL_LOCK_THRESH))
                    {
                        /* Start timer */
                        pll_state = 1;
                        pll_count = 5000;
                    }
                    
                    /* Reset BW if lost lock */
                    if((pll_state == 2) && (am_dc_acc < (PLL_LOCK_THRESH/2.0F)))
                        pll_state = 0;
                    
                    /* timeout? */
                    if(pll_state == 1)
                    {
                        if(pll_count == 0)
                            pll_state = 2;
                        else
                            pll_count = pll_count - 1;
                    }
                    
                    /* loop filter */
                    if(pll_state != 2)
                    {
                        /* Wide bandwidth */
                        pll_frq = pll_intg + PLL_P_WIDE * pll_err;
                        pll_intg += PLL_I_WIDE * pll_err;
                        
                        /* output normal demod */
                        am_raw = sqrtf(mag_sq);
                    }
                    else
                    {
                        /* Narrow bandwidth */
                        pll_frq = pll_intg + PLL_P_NARR * pll_err;
                        pll_intg += PLL_I_NARR * pll_err;
                        
                        /* output Sync demod */
                        am_raw = pll_i_bb;
                    }
                    
                    /* nco */
                    pll_phs += pll_frq;
                    if(pll_phs > 1.0F)
                        pll_phs -=1.0F;
                    else if(pll_phs < 0.0F)
                        pll_phs += 1.0F;
                }
                
				/* AM DC Block */
				am_dcb = am_raw - am_dc_acc;
				am_dc_acc += (am_dcb * DC_SCALE);
				
				/* output with makeup gain */
				i_det = 2.0F * am_dcb;
                q_det = i_det;
				break;
			
			case 2: /* SSB upper */
			case 3: /* SSB lower */
			case 4: /* SSB upper + lower */
				/* Hi filters */
				hilb_in = i_filter;
				for(m=0;m<SHIFT_STAGES;m++)
				{
					/* compute AP */
					hilb_out = ahi[m]*(hilb_in+fb.state_hi_o[m][1])-fb.state_hi_i[m][1];
					
					/* advance delays */
					fb.state_hi_i[m][1] = fb.state_hi_i[m][0];
					fb.state_hi_i[m][0] = hilb_in;
					fb.state_hi_o[m][1] = fb.state_hi_o[m][0];
					fb.state_hi_o[m][0] = hilb_out;
					
					/* input for next AP = out from current */
					hilb_in = hilb_out;
				}
				
				/* one sample delay on hi side output */
				ap_i = fb.dly;
				fb.dly = hilb_out;
				
				/* Lo filters */
				hilb_in = q_filter;
				for(m=0;m<SHIFT_STAGES;m++)
				{
					/* compute AP */
					hilb_out = alo[m]*(hilb_in+fb.state_lo_o[m][1])-fb.state_lo_i[m][1];
					
					/* advance delays */
					fb.state_lo_i[m][1] = fb.state_lo_i[m][0];
					fb.state_lo_i[m][0] = hilb_in;
					fb.state_lo_o[m][1] = fb.state_lo_o[m][0];
					fb.state_lo_o[m][0] = hilb_out;
					
					/* input for next AP = out from current */
					hilb_in = hilb_out;
				}
				ap_q = hilb_out;
				
				/* SSB DC blocks */
				ssb_i = ap_i - fb.hi_dc;
				fb.hi_dc += (ssb_i * DC_SCALE);
				ssb_q = ap_q - fb.lo_dc;
				fb.lo_dc += (ssb_q * DC_SCALE);
				
				/* choose sideband for output */
				if(demod_type == 2)
					i_det = q_det = ssb_i - ssb_q;	/* upper */
				else if(demod_type == 3)
					i_det = q_det = ssb_i + ssb_q;	/* lower */
				else if(demod_type == 4)
				{
                    i_det = ssb_i - ssb_q;	/* upper */
                    q_det = ssb_i + ssb_q;	/* lower */
                }
				break;
            
            case 5: /* Narrowband FM */
                {
                    float32_t nbfm_phs, nbfm_raw, nbfm_out;
                    
                    /* get phase */
                    nbfm_phs = atan2f(i_filter, q_filter);
                    
                    /* differentiate */
                    nbfm_raw = nbfm_phs - nbfm_pphs;
                    nbfm_pphs = nbfm_phs;
                    
                    /* unwrap */
                    if(nbfm_raw > PI)
                        nbfm_raw -= 2.0*PI;
                    else if(nbfm_raw < -PI)
                        nbfm_raw += 2.0*PI;
                    
                    /* deviation adj to 10% full scale */
                    nbfm_raw *= (NBFM_DEV_SCL/10.0F);
                    
#if 1
                    /* de-emphasis with leak */
                    nbfm_de_acc = (nbfm_de_acc*0.99F) + nbfm_raw;
                    nbfm_out = nbfm_de_acc * NBFM_DE_SCALE;
#else
                    /* no de-emphasis */
                    nbfm_out = nbfm_raw;
#endif

                    i_det = q_det = nbfm_out;
                }
                break;
                
            case 6: /* raw I & Q with AGC & filter */
                i_det = i_filter;
                q_det = q_filter;
				break;
                
		}	
		
		/* Muting */
		mute_gain = 0.0F;
		if(mute_state<256)
		{
			/* ramping up */
			mute_gain = (float32_t)mute_state/256.0F;
			mute_state++;
		}
		else if(mute_state == 256)
		{
			/* holding, umuted */
			mute_gain = 1.0F;
		}
		else if((mute_state > 256) && (mute_state<512))
		{
			mute_gain = (float32_t)(511-mute_state)/256.0F;
			mute_state++;
		}
		i_mute = i_det * mute_gain;
		q_mute = q_det * mute_gain;

		/* Saturate to integer and send to DAC */
		*dst++ = audio_sat(32768*i_mute);
		*dst++ = audio_sat(32768*q_mute);
	}
	
	/* update AGC */
	agc_acc = agc_acc + alpha * (logR - logf(f_pwr));
	agc_acc = agc_acc > 10.0F ? 10.0F : agc_acc;
	agc_acc = agc_acc < -10.0F ? -10.0F : agc_acc;
	agc_gain = expf(agc_acc);
#endif
}


