/*
 * iir.c - Cascaded Infinite Impulse Response filter routine
 * 07-14-2015 E. Brombaugh
 */

#include "iir.h"

/*
 * initialize the iir structure
 */
void iir_init(iir *is, bq_state *s, bq_coeffs *c, uint8_t n)
{
	uint8_t i;
	
	/* set up the structure */
	is->num_bq = n;
	is->bqs = s;
	is->bqc = c;
	
	/* clear the state */
	for(i=0;i<n;i++)
	{
		s->state[0] = 0;
		s->state[1] = 0;
		s++;
	}
}

/*
 * compute the iir
 */
float32_t iir_calc(iir *is, float32_t input)
{
	uint8_t i;
	float32_t xin = input, yout;
	bq_state  *s = is->bqs;
	bq_coeffs *c = is->bqc;

	/* iterate over the number of biquads */
	for(i=0;i<is->num_bq;++i)
	{
		/* transpose direct form II biquad  */
		yout = c->num[0]*xin + s->state[0];
		s->state[0] = s->state[1] + c->num[1] * xin - c->den[1] * yout;
		s->state[1] = c->num[2] * xin - c->den[2] * yout;

		/* ouput of the previous biquad is input to next, with gain applied */
		xin = yout * c->gain;
		
		/* update pointers */
		s++;
		c++;
	}
	return xin;
}
