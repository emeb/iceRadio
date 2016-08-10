/*
 * debounce.c - simple button debouncer.
 */

#include "debounce.h"

void init_debounce(debounce_state *dbs, uint8_t len)
{
	dbs->pipe = 0;
	dbs->state = 0;
	dbs->prev_state = 0;
	dbs->re = 0;
	dbs->fe = 0;
	dbs->mask = (1<<len)-1;
}

void debounce(debounce_state *dbs, uint32_t in)
{
	/* update debounce pipeline */
	dbs->pipe = (dbs->pipe << 1) | (in & 1);

	/* update old state */
	dbs->prev_state = dbs->state;

	/* update current state */
	if(dbs->state)
	{
		/* 1 state - look for all zero in the pipe */
		if((dbs->pipe & dbs->mask) == 0)
			dbs->state = 0;
	}
	else
	{
		/* 0 state - look for all one in the pipe */
		if((dbs->pipe & dbs->mask) == dbs->mask)
			dbs->state = 1;
	}

	/* detect rising edge (button pushed) */
	if(dbs->prev_state == 0 && dbs->state == 1)
		dbs->re = 1;
	else
		dbs->re = 0;

	/* detect falling edge (button released) */
	if(dbs->prev_state == 1 && dbs->state == 0)
		dbs->fe = 1;
	else
		dbs->fe = 0;
}
