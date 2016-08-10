/*
 * st7735.c - interface routines for ST7735 LCD.
 * shamelessly ganked from Adafruit_ST7735 library
 * 12-20-12 E. Brombaugh
 */

#include "shared_spi.h"
#include "st7735.h"
#include "cyclesleep.h"
#include "font_8x8.h"

#define ST7735_CMD 0x100
#define ST7735_DLY 0x200
#define ST7735_END 0x400

void ST7735_write(uint16_t dat)
{
	if((dat&ST7735_CMD) == ST7735_CMD)
		LCD_DC_CMD();
	else
		LCD_DC_DATA();

	LCD_CS_LOW();

	SPI_WriteByte(dat&0xff);

	LCD_CS_HIGH();
}

// Initialization command sequence
const static uint16_t
  initlst[] = {                  // Init for 7735R (Red tab)
    ST7735_SWRESET | ST7735_CMD, // * Software reset, 0 args, w/delay
	  150 | ST7735_DLY,          //   150 ms delay
    ST7735_SLPOUT | ST7735_CMD,  // * Out of sleep mode, 0 args, w/delay
      500 | ST7735_DLY,          //   500 ms delay
    ST7735_FRMCTR1 | ST7735_CMD, // * Frame rate ctrl - normal mode, 3 args:
      0x01, 0x2C, 0x2D,          //   Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR2 | ST7735_CMD, // * Frame rate control - idle mode, 3 args:
      0x01, 0x2C, 0x2D,          //   Rate = fosc/(1x2+40) * (LINE+2C+2D)
    ST7735_FRMCTR3 | ST7735_CMD, // * Frame rate ctrl - partial mode, 6 args:
      0x01, 0x2C, 0x2D,          //   Dot inversion mode
      0x01, 0x2C, 0x2D,          //   Line inversion mode
    ST7735_INVCTR | ST7735_CMD,  // * Display inversion ctrl, 1 arg, no delay:
      0x07,                      //   No inversion
    ST7735_PWCTR1 | ST7735_CMD,  // * Power control, 3 args, no delay:
      0xA2,
      0x02,                      //   -4.6V
      0x84,                      //   AUTO mode
    ST7735_PWCTR2 | ST7735_CMD,  // * Power control, 1 arg, no delay:
      0xC5,                      //   VGH25 = 2.4C VGSEL = -10 VGH = 3 * AVDD
    ST7735_PWCTR3 | ST7735_CMD,  // * Power control, 2 args, no delay:
      0x0A,                      //   Opamp current small
      0x00,                      //   Boost frequency
    ST7735_PWCTR4 | ST7735_CMD,  // * Power control, 2 args, no delay:
      0x8A,                      //   BCLK/2, Opamp current small & Medium low
      0x2A,  
    ST7735_PWCTR5 | ST7735_CMD,  // * Power control, 2 args, no delay:
      0x8A, 0xEE,
    ST7735_VMCTR1 | ST7735_CMD,  // * Power control, 1 arg, no delay:
      0x0E,
    ST7735_INVOFF | ST7735_CMD,  // * Don't invert display, no args, no delay
    ST7735_MADCTL | ST7735_CMD,  // * Memory access control (directions), 1 arg:
      0xC8,                      //   row addr/col addr, bottom to top refresh
    ST7735_COLMOD | ST7735_CMD,  // * set color mode, 1 arg, no delay:
      0x05,                      //   16-bit color
    ST7735_CASET | ST7735_CMD,   // * Column addr set, 4 args, no delay:
      0x00, 0x00,                //   XSTART = 0
      0x00, 0x7F,                //   XEND = 127
    ST7735_RASET | ST7735_CMD,   // * Row addr set, 4 args, no delay:
      0x00, 0x00,                //   XSTART = 0
      0x00, 0x9F,                //   XEND = 159
    ST7735_GMCTRP1 | ST7735_CMD, // * Magical unicorn dust, 16 args, no delay:
      0x02, 0x1c, 0x07, 0x12,
      0x37, 0x32, 0x29, 0x2d,
      0x29, 0x25, 0x2B, 0x39,
      0x00, 0x01, 0x03, 0x10,
    ST7735_GMCTRN1 | ST7735_CMD, // * Sparkles and rainbows, 16 args, no delay:
      0x03, 0x1d, 0x07, 0x06,
      0x2E, 0x2C, 0x29, 0x2D,
      0x2E, 0x2E, 0x37, 0x3F,
      0x00, 0x00, 0x02, 0x10,
    ST7735_NORON | ST7735_CMD,   // * Normal display on, no args, w/delay
      10 | ST7735_DLY,           //   10 ms delay
    ST7735_DISPON | ST7735_CMD,  // * Main screen turn on, no args w/delay
      100 | ST7735_DLY,          //   100 ms delay
	ST7735_END };                // END

// LCD state
uint8_t rowstart, colstart;
uint8_t _width, _height, rotation;
uint16_t colortab[256];
	  
// initialize 256 color LUT
void ST7735_color_init(void)
{
	uint16_t idx, hue, sat;
	
	for(idx = 0;idx<256;idx++)
	{
		hue = (idx&0xE0)>>5;
		sat = idx&0x1F;
		
		switch(hue)
		{
			case 0:		// red
				colortab[idx] = sat << 11;
				break;
			case 1:		// green
				colortab[idx] = (sat << 6);
				break;
			case 2:		// blue
				colortab[idx] = sat;
				break;
			case 3:		// cyan
				colortab[idx] = sat | (sat << 6);
				break;
			case 4: 	// magenta
				colortab[idx] = sat | (sat << 11);
				break;
			case 5:		// yellow
				colortab[idx] = (sat << 6) | (sat << 11);
				break;
			case 6: 	// white
				colortab[idx] = sat | (sat << 6) | (sat << 11);
				break;
			case 7:		// black
				colortab[idx] = 0;
		}
	}
}
	  
// Initialization for ST7735R red tab screens
void ST7735_init(void)
{
	// default settings
	colstart = 0;
	rowstart = 0;
	_width  = ST7735_TFTWIDTH;
	_height = ST7735_TFTHEIGHT;
	rotation = 0;
	ST7735_color_init();
	
	// Reset it
	LCD_RST_LOW();
	delay(10);
	LCD_RST_HIGH();
	delay(10);

	// Send init command list
	uint16_t *addr = (uint16_t *)initlst, ms;
	while(*addr != ST7735_END)
	{
		if((*addr & ST7735_DLY) != ST7735_DLY)
			ST7735_write(*addr++);
		else
		{
			ms = (*addr++)&0x1ff;        // strip delay time (ms)
			delay(ms);
		}	
	}
}

// opens a window into display mem for bitblt
void ST7735_setAddrWindow(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
	ST7735_write(ST7735_CASET | ST7735_CMD); // Column addr set
	ST7735_write(0x00);
	ST7735_write(x0+colstart);     // XSTART 
	ST7735_write(0x00);
	ST7735_write(x1+colstart);     // XEND

	ST7735_write(ST7735_RASET | ST7735_CMD); // Row addr set
	ST7735_write(0x00);
	ST7735_write(y0+rowstart);     // YSTART
	ST7735_write(0x00);
	ST7735_write(y1+rowstart);     // YEND

	ST7735_write(ST7735_RAMWR | ST7735_CMD); // write to RAM
}

// fill screen w/ single color
void ST7735_fillScreen(uint16_t color)
{
	uint8_t x, y, hi = color >> 8, lo = color;

	ST7735_setAddrWindow(0, 0, _width-1, _height-1);

	LCD_DC_DATA();
	LCD_CS_LOW();

	for(y=_height; y>0; y--)
	{
		for(x=_width; x>0; x--)
		{
			SPI_WriteByte(hi);
			SPI_WriteByte(lo);
		}
	}

	LCD_CS_HIGH();
}

// send out color value
void ST7735_pushColor(uint16_t color)
{
	LCD_DC_DATA();
	LCD_CS_LOW();

	SPI_WriteByte(color >> 8);
	SPI_WriteByte(color);

	LCD_CS_HIGH();
}

// draw single pixel
void ST7735_drawPixel(int16_t x, int16_t y, uint16_t color)
{

	if((x < 0) ||(x >= _width) || (y < 0) || (y >= _height)) return;

	ST7735_setAddrWindow(x,y,x+1,y+1);

	ST7735_pushColor(color);
}

// fast vert line
void ST7735_drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color)
{
	// Rudimentary clipping
	if((x >= _width) || (y >= _height)) return;
	if((y+h-1) >= _height) h = _height-y;
	ST7735_setAddrWindow(x, y, x, y+h-1);

	uint8_t hi = color >> 8, lo = color;
	LCD_DC_DATA();
	LCD_CS_LOW();
	while (h--)
	{
		SPI_WriteByte(hi);
		SPI_WriteByte(lo);
	}
	LCD_CS_HIGH();
}

// fast horiz line
void ST7735_drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color)
{
	// Rudimentary clipping
	if((x >= _width) || (y >= _height)) return;
	if((x+w-1) >= _width)  w = _width-x;
	ST7735_setAddrWindow(x, y, x+w-1, y);

	uint8_t hi = color >> 8, lo = color;
	LCD_DC_DATA();
	LCD_CS_LOW();
	while (w--)
	{
		SPI_WriteByte(hi);
		SPI_WriteByte(lo);
	}
	LCD_CS_HIGH();
}

// fill a rectangle
void ST7735_fillRect(int16_t x, int16_t y, int16_t w, int16_t h,
	uint16_t color)
{
	// rudimentary clipping (drawChar w/big text requires this)
	if((x >= _width) || (y >= _height)) return;
	if((x + w - 1) >= _width)  w = _width  - x;
	if((y + h - 1) >= _height) h = _height - y;

	ST7735_setAddrWindow(x, y, x+w-1, y+h-1);

	uint8_t hi = color >> 8, lo = color;
	LCD_DC_DATA();
	LCD_CS_LOW();
	for(y=h; y>0; y--)
	{
		for(x=w; x>0; x--)
		{
			SPI_WriteByte(hi);
			SPI_WriteByte(lo);
		}
	}
	LCD_CS_HIGH();
}

// Pass 8-bit (each) R,G,B, get back 16-bit packed color
uint16_t ST7735_Color565(uint8_t r, uint8_t g, uint8_t b)
{
	return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Draw character direct to the display
void ST7735_drawchar(int16_t x, int16_t y, uint8_t chr, 
	uint16_t fg, uint16_t bg)
{
	uint16_t i, j, col;
	uint8_t d;
	
	ST7735_setAddrWindow(x, y, x+7, y+7);
	
	LCD_DC_DATA();
	LCD_CS_LOW();
	for(i=0;i<8;i++)
	{
		d = fontdata[(chr<<3)+i];
		for(j=0;j<8;j++)
		{
			if(d&0x80)
				col = fg;
			else
				col = bg;
			
			SPI_WriteByte((col>>8)&0xFF);
			SPI_WriteByte(col&0xFF);			
			
			// next bit
			d <<= 1;
		}
	}
	LCD_CS_HIGH();
}

// draw a string to the display
void ST7735_drawstr(int16_t x, int16_t y, uint8_t *str, 
	uint16_t fg, uint16_t bg)
{
	uint8_t c;
	
	while((c=*str++))
	{
		ST7735_drawchar(x, y, c, fg, bg);
		x += 8;
		if(x>120)
			break;
	}
}

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x08
#define MADCTL_MH  0x04

// set orientation of display
void ST7735_setRotation(uint8_t m)
{
	ST7735_write(ST7735_MADCTL | ST7735_CMD);
	rotation = m % 4; // can't be higher than 3
	switch (rotation)
	{
		case 0:
			ST7735_write(MADCTL_MX | MADCTL_MY | MADCTL_RGB);
			_width  = ST7735_TFTWIDTH;
			_height = ST7735_TFTHEIGHT;
			break;
		case 1:
			ST7735_write(MADCTL_MY | MADCTL_MV | MADCTL_RGB);
			_width  = ST7735_TFTHEIGHT;
			_height = ST7735_TFTWIDTH;
			break;
		case 2:
			ST7735_write(MADCTL_RGB);
			_width  = ST7735_TFTWIDTH;
			_height = ST7735_TFTHEIGHT;
			break;
		case 3:
			ST7735_write(MADCTL_MX | MADCTL_MV | MADCTL_RGB);
			_width  = ST7735_TFTHEIGHT;
			_height = ST7735_TFTWIDTH;
			break;
	}
}

// intermediate DMA buffer for writing in 1/4 screen sections
uint8_t SPI_DMABUF0 [16 * 128 * 2], SPI_DMABUF1 [16 * 128 * 2];
uint8_t nfirst_pass = 0;

// copy SRAM byte buffer to display as words
void ST7735_draw_framebuffer(uint8_t *buffer, uint8_t w, uint8_t h)
{
#if 0
	// PIO framebuffer write w/ on-the-fly conversion to 16bpp
	uint8_t x, y;
	uint16_t color;
	
	ST7735_setAddrWindow(0, 0, w-1, h-1);

	LCD_DC_DATA();
	LCD_CS_LOW();
	for(y=h; y>0; y--)
	{
		for(x=w; x>0; x--)
		{
			color = *buffer++;
			color = color | (color << 6) | (color << 11);
			
			// hi byte = 0
			while((SD_SPI->SR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
			*(__IO uint8_t *) ((uint32_t)SD_SPI+0x0C) = (color>>8)&0xFF;
			
			// low byte = data
			while((SD_SPI->SR & SPI_I2S_FLAG_TXE) == (uint16_t)RESET);
			*(__IO uint8_t *) ((uint32_t)SD_SPI+0x0C) = color&0xFF;
		}
	}
	LCD_CS_HIGH();
#else
	// DMA framebuffer with intermediate conversion
	uint8_t i, x, y, *curr_dest, *dest;
	uint16_t color;
	
	// iterate 4x over sections
	for(i=0;i<8;i++)
	{
		// swap buffers
		curr_dest = ((i&1) == 1) ? SPI_DMABUF1 : SPI_DMABUF0;
		
		// first convert 8bpp framebuffer data to 16bpp in DMA buffer
		dest = curr_dest;
		for(y=h/8; y>0; y--)
		{
			for(x=w; x>0; x--)
			{
				color = colortab[*buffer++];
				*dest++ = (color>>8)&0xFF;
				*dest++ = color&0xFF;
			}
		}
		
		// don't finish previous on first pass
		//if(i!=0)
		if(nfirst_pass!=0)
		{
			SPI_end_DMA_WriteBytes();
			LCD_CS_HIGH();
		}
		else
			nfirst_pass = 1;
		
		// setup window for LCD
		ST7735_setAddrWindow(0, i*16, w-1, h-1);
		
		// send DMA buffer
		LCD_DC_DATA();
		LCD_CS_LOW();
		SPI_start_DMA_WriteBytes(curr_dest, 2*w*h/8);
		//SPI_end_DMA_WriteBytes();
		//LCD_CS_HIGH();
	}
	
	// wait for final DMA to finish
	//SPI_end_DMA_WriteBytes();
	//LCD_CS_HIGH();
#endif
}