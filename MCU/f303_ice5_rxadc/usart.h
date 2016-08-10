/*
 * usart.h - usart printf stuff
 * 12-24-12 E. Brombaugh
 */

#ifndef __usart__
#define __usart__

void setup_usart1(void);
int get_usart(void);
int outbyte(int ch);
int inbyte(void);

#endif
