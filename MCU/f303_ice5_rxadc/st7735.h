/*
 * st7735.h - interface routines for ST7735 LCD.
 * shamelessly ganked from Adafruit_ST7735 library
 * 12-20-12 E. Brombaugh
 */


#ifndef __ST7735__
#define __ST7735__

#include <stdint.h>

// some flags for initR() :(
#define INITR_GREENTAB 0x0
#define INITR_REDTAB   0x1

#define ST7735_TFTWIDTH  128
#define ST7735_TFTHEIGHT 160

#define ST7735_NOP     0x00
#define ST7735_SWRESET 0x01
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_SLPIN   0x10
#define ST7735_SLPOUT  0x11
#define ST7735_PTLON   0x12
#define ST7735_NORON   0x13

#define ST7735_INVOFF  0x20
#define ST7735_INVON   0x21
#define ST7735_DISPOFF 0x28
#define ST7735_DISPON  0x29
#define ST7735_CASET   0x2A
#define ST7735_RASET   0x2B
#define ST7735_RAMWR   0x2C
#define ST7735_RAMRD   0x2E

#define ST7735_PTLAR   0x30
#define ST7735_COLMOD  0x3A
#define ST7735_MADCTL  0x36

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

// Color definitions
#define	ST7735_BLACK   0x0000
#define	ST7735_RED     0x001F
#define	ST7735_BLUE    0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_YELLOW  0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_CYAN    0xFFE0  
#define ST7735_WHITE   0xFFFF

void ST7735_write(uint16_t dat);
void ST7735_init(void);
void ST7735_setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void ST7735_fillScreen(uint16_t color);
void ST7735_pushColor(uint16_t color);
void ST7735_drawPixel(int16_t x, int16_t y, uint16_t color);
void ST7735_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void ST7735_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void ST7735_fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
	uint16_t color);
uint16_t ST7735_Color565(uint8_t r, uint8_t g, uint8_t b);
void ST7735_drawchar(int16_t x, int16_t y, uint8_t chr, 
	uint16_t fg, uint16_t bg);
void ST7735_drawstr(int16_t x, int16_t y, uint8_t *str, 
	uint16_t fg, uint16_t bg);
void ST7735_setRotation(uint8_t m);
void ST7735_draw_framebuffer(uint8_t *buffer, uint8_t w, uint8_t h);

#endif
