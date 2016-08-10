/*
 * audio_lib.c - miscellaneous audio processing functions
 */

#include "audio_lib.h"
#include "stdlib.h"

/**
  * @brief  Split interleaved stereo into two separate buffers
  * @param  sz -  samples per input buffer (divisible by 2)
  * @param  src - pointer to source buffer
  * @param  ldst - pointer to left dest buffer (even samples)
  * @param  rdst - pointer to right dest buffer (odd samples)
  * @retval none
  */
void audio_split_stereo(int16_t sz, int16_t *src, int16_t *ldst, int16_t *rdst)
{
	while(sz)
	{
		*ldst++ = *src++;
		sz--;
		*rdst++ = *src++;
		sz--;
	}
}

/**
  * @brief  sums left and right into left and right with saturation
  * @param  sz -  samples per input buffer (divisible by 2)
  * @param  lsrc - pointer to left source/dest buffer
  * @param  rsrc - pointer to right source/dest buffer
  * @retval none
  */
void audio_sum_stereo(int16_t sz, int16_t *lsrc, int16_t *rsrc)
{
	int32_t sum;

	while(sz)
	{
		sum = *lsrc + *rsrc;
		int16_t sat = audio_sat(sum);
		*rsrc++ = sat;
		*lsrc++ = sat;
		sz--;
	}
}

/**
  * @brief  copies src to dst
  * @param  sz -  samples per input buffer (divisible by 2)
  * @param  dst - pointer to dest buffer
  * @param  src - pointer to source buffer
  * @retval none
  */
void audio_copy(int16_t sz, int16_t *dst, int16_t *src)
{
	while(sz)
	{
		*dst++ = *src++;
		sz--;
	}
}

/**
  * @brief  computes dst = dst*g0 + src1*g1 + src2*g2
  * @param  sz -  samples per input buffer (divisible by 2)
  * @param  dst - pointer to dest buffer
  * @param  src1 - pointer to source1 buffer
  * @param  src2 - pointer to source2 buffer
  * @param  g0 - gain for dst
  * @param  g1 - gain for src1
  * @param  g2 - gain for src2
  * @retval none
  */
void audio_sop3(int16_t sz, int16_t *dst, int16_t *src1, int16_t *src2, float32_t g0,
	float32_t g1, float32_t g2)
{
	float32_t f_sum;

	while(sz)
	{
		f_sum = (float32_t)*dst * g0;
		f_sum += (float32_t)*src1++ * g1;
		f_sum += (float32_t)*src2++ * g2;
		*dst++ = audio_sat(f_sum);
		sz--;
	}
}

/**
  * @brief  computes dst = dst*g0 + src1*g1
  * @param  sz -  samples per input buffer (divisible by 2)
  * @param  dst - pointer to dest buffer
  * @param  src1 - pointer to source1 buffer
  * @param  g0 - gain for dst
  * @param  g1 - gain for src1
  * @retval none
  */
void audio_sop2(int16_t sz, int16_t *dst, int16_t *src1, float32_t g0, float32_t g1)
{
	float32_t f_sum;

	while(sz)
	{
		f_sum = (float32_t)*dst * g0;
		f_sum += (float32_t)*src1++ * g1;
		*dst++ = audio_sat(f_sum);
		sz--;
	}
}

/**
  * @brief  check a buffer for clipping
  * @param  sz -  samples per input buffer (divisible by 2)
  * @param  src - pointer to source buffer
  * @retval 1 if clipping detected
  */
uint8_t audio_clip(int16_t sz, int16_t *src, int16_t thresh)
{
	uint8_t clip = 0;

	while(sz)
	{
		if(abs(*src++)>thresh)
			clip = 1;
		sz--;
	}

	return clip;
}

/**
  * @brief  copy src to dst with gain
  * @param  sz -  samples per buffer
  * @param  dst - pointer to source buffer
  * @param  src - pointer to dest buffer
  * @param  gain - float gain coeff.
  * @retval none
  */
void audio_gain(int16_t sz, int16_t *dst, int16_t *src, float32_t gain)
{
	float32_t f_sum;

	while(sz--)
	{
		/* apply gain */
		f_sum = (float32_t)*src++ * gain;

		/* saturate and save to destination */
		*dst++ = audio_sat(f_sum);
	}
}

/**
  * @brief  sum src to dst with gain
  * @param  sz -  samples per buffer
  * @param  dst - pointer to source buffer
  * @param  src - pointer to dest buffer
  * @param  gain - float gain coeff.
  * @retval none
  */
void audio_gain_sum(int16_t sz, int16_t *dst, int16_t *src, float32_t gain)
{
	float32_t f_sum;

	while(sz--)
	{
		f_sum = (float32_t)*src++ * gain + (float32_t)*dst;

		/* saturate and save to destination */
		*dst++ = audio_sat(f_sum);
	}
}

/**
  * @brief  combine two separate buffers into interleaved stereo
  * @param  sz -  samples per output buffer (divisible by 2)
  * @param  dst - pointer to source buffer
  * @param  lsrc - pointer to left dest buffer (even samples)
  * @param  rsrc - pointer to right dest buffer (odd samples)
  * @retval none
  */
void audio_comb_stereo(int16_t sz, int16_t *dst, int16_t *lsrc, int16_t *rsrc)
{
	while(sz)
	{
		*dst++ = *lsrc++;
		sz--;
		*dst++ = *rsrc++;
		sz--;
	}
}

/**
  * @brief  morph a to b into destination
  * @param  sz -  samples per buffer
  * @param  dst - pointer to source buffer
  * @param  asrc - pointer to dest buffer
  * @param  bsrc - pointer to dest buffer
  * @param  morph - float morph coeff. 0 = a, 1 = b
  * @retval none
  */
void audio_morph(int16_t sz, int16_t *dst, int16_t *asrc, int16_t *bsrc,
				float32_t morph)
{
	float32_t morph_inv = 1.0F - morph, f_sum;

	while(sz--)
	{
		f_sum = (float32_t)*asrc++ * morph_inv + (float32_t)*bsrc++ * morph;

		/* save to destination */
		*dst++ = audio_sat(f_sum);
	}
}

#if 0
/**
  * @brief  constant power mix a & b into destination
  * @param  sz -  samples per buffer
  * @param  dst - pointer to source buffer
  * @param  asrc - pointer to dest buffer
  * @param  bsrc - pointer to dest buffer
  * @param  morph - 0-4095 mix index 0 = a, 4095 = b
  * @retval none
  */
void audio_cpmix(int16_t sz, int16_t *dst, int16_t *asrc, int16_t *bsrc,
				int16_t mix)
{
	float32_t morph, morph_inv, f_sum;

	/* get gain coeffs from pre-computed table */
	morph = gaintab[mix];
	morph_inv = gaintab[4095-mix];

	while(sz--)
	{
		f_sum = (float32_t)*asrc++ * morph_inv + (float32_t)*bsrc++ * morph;

		/* save to destination */
		*dst++ = audio_sat(f_sum);
	}
}
#endif

/**
  * @brief  approximate constant power mix a & b into destination
  * @param  sz -  samples per buffer
  * @param  dst - pointer to source buffer
  * @param  asrc - pointer to dest buffer
  * @param  bsrc - pointer to dest buffer
  * @param  mix - float mix value 0 = a, 1.0 = b
  * @retval none
  */
void audio_cp2mix(int16_t sz, int16_t *dst, int16_t *asrc, int16_t *bsrc,
				float32_t mix)
{
	float32_t morph, morph_inv, f_sum;

	/* get gain coeffs with approx function */
	morph = 1.0F-((1.0F-mix)*(1.0F-mix));
	morph_inv = 1.0F-(mix*mix);

	while(sz--)
	{
		f_sum = (float32_t)*asrc++ * morph_inv + (float32_t)*bsrc++ * morph;

		/* save to destination */
		*dst++ = audio_sat(f_sum);
	}
}
