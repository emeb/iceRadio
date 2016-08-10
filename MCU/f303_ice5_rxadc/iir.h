/*
 * iir.c - Cascaded Infinite Impulse Response filter routine
 * 07-14-2015 E. Brombaugh
 */

#ifndef __iir__
#define __iir__

#include <stdint.h>

/* define this for linux */
typedef float float32_t;

/* iir types */
typedef struct
{
	float32_t state[2];	/* pipeline for input signal		*/
} bq_state;

typedef struct
{
	float32_t num[3];	/* Num coef in order of ze-1		*/
	float32_t den[3]; 	/* Denom coef in order of ze-1		*/
	float32_t gain;		/* Overall filter gain */
} bq_coeffs;

typedef struct
{
	uint8_t	num_bq;		/* Number of biquad sections in a filter */
	bq_state  *bqs;		/* pointer to array of biquad states	*/
	bq_coeffs *bqc;		/* pointer to array of biquad coeffs	*/
} iir;

/* iir functions */
void iir_init(iir *is, bq_state *s, bq_coeffs *c, uint8_t n);
float32_t iir_calc(iir *is, float32_t input);

#endif
