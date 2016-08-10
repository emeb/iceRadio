/*
 * menu.c - UI menu driver for f303_ice5_rxadc project
 * 07-19-14 E. Brombaugh
 */

#include <stdio.h>
#include "menu.h"
#include "systick.h"
#include "audio.h"
#include "cyclesleep.h"
#include "st7735.h"
#include "rxadc.h"

/* Filter bandwidths */
const uint16_t filter_bw[] = {
	8000,
	6000,
	4000,
	2000,
	1000,
	500
};

/* Demod strings */
#define MAX_DEMODS 8
const char *demod_str[] = {
	"I/Q",
	" AM",
    "SYN",
	"USB",
	"LSB",
    "U/L",
    "NFM",
    "RAW"
};

/* local vars used by menu system */
int16_t prev_button;
char txt_buff[17];
uint32_t Flo;
uint8_t cursor_loc, cursor_type, menu_demod_type, filter_num;
uint32_t rssi_goal;

/*
 * demod read 
 */
uint8_t menu_get_demod(void)
{
    
    if(rxadc_get_mux() == 0)
        return 0;
    else
        return Audio_GetDemod()+1;
}

/*
 * demod write
 */
void menu_set_demod(uint8_t demod)
{
    /* limit to legal range */
    demod = demod % MAX_DEMODS;
    
    if(demod == 0)
    {
        /* Bypass processing and send raw I/Q to DAC */
        rxadc_set_mux(0);
    }
    else
    {
        /* local processing */
        rxadc_set_mux(1);
        Audio_SetDemod(demod-1);
    }        
}

/*
 * draw LO frequency
 */
void menu_LO_draw(void)
{
	snprintf(txt_buff,17," LO: %8u Hz", (unsigned int)Flo);
	ST7735_drawstr(0,16,(uint8_t *)txt_buff, ST7735_WHITE, ST7735_BLACK);
}

/*
 * draw filter BW
 */
void menu_filter_draw(void)
{
	snprintf(txt_buff,17,"Fltr:   % 5d Hz", filter_bw[filter_num]);
	ST7735_drawstr(0,26,(uint8_t *)txt_buff, ST7735_WHITE, ST7735_BLACK);
}

/*
 * draw mod type
 */
void menu_demod_draw(void)
{
	snprintf(txt_buff,17,"Mod:      %s", demod_str[menu_demod_type]);
	ST7735_drawstr(0,36,(uint8_t *)txt_buff, ST7735_WHITE, ST7735_BLACK);
}

/*
 * draw rssi
 */
void menu_rssi_draw(void)
{
	int16_t rssi_dBm = Audio_GetRSSI();
	int8_t w, wt;
    
    /* text */
	//printf("rssi: %d dBm\n", rssi_dBm);
	snprintf(txt_buff,17,"RSSI:   % 4d dBm", rssi_dBm);
	ST7735_drawstr(0,50,(uint8_t *)txt_buff, ST7735_WHITE, ST7735_BLACK);
    
    /* bargraph */
    w = rssi_dBm + 140;
    if(w>127)
        w = 127;
    if(w<0)
        w = 0;
    
#if 1
    /* colors */
    wt = w;
    if(wt>95)
    {
        ST7735_fillRect(95, 60, wt-96, 4, ST7735_RED);
        wt=95;
    }
    if(wt>63)
    {
        ST7735_fillRect(63, 60, wt-64, 4, ST7735_YELLOW);
        wt=63;
    }
    ST7735_fillRect(0, 60, wt, 4, ST7735_GREEN);
#else
    /* white */
    ST7735_fillRect(0, 60, w, 4, ST7735_WHITE);
#endif
    ST7735_fillRect(w, 60, 127-w, 4, ST7735_BLACK);

}

/*
 * draw sync freq
 */
void menu_sync_draw(void)
{
	if(menu_demod_type==2)
    {
        if(Audio_GetSyncSt()<2)
            snprintf(txt_buff,17,"Sync: Unlocked  ");
        else
            snprintf(txt_buff,17,"Sync: % 4d Hz  ", Audio_GetSyncFrq());
    }
    else
    {
        snprintf(txt_buff,17,"Sync: Off       ");
    }
    ST7735_drawstr(0,70,(uint8_t *)txt_buff, ST7735_WHITE, ST7735_BLACK);
}

/*
 * draw the cursor
 */
void menu_draw_cursor(uint8_t draw)
{
	uint8_t x=0, y=0, w=8;
	
	/* compute location of cursor */
	if(cursor_loc < 8)
	{
		/* In LO digits */
		x = (cursor_loc + 5)*8;
		y = 16;
	}
	else if(cursor_loc == 8)
	{
		/* in Filter num */
		x = 72;
		y = 26;
        w = 32;
	}
	else if(cursor_loc == 9)
	{
		/* in Mod type */
		x = 80;
		y = 36;
        w = 24;
	}
    y = y+7;
    
	if(draw)
        /* Draw it */
        ST7735_fillRect(x, y, w, 2, (cursor_type ? ST7735_RED : ST7735_GREEN));
    else
        /* Erase it */
        ST7735_fillRect(x, y, w, 2, ST7735_BLACK);
}

/*
 * generate a power-of-10 digit increment
 */
int32_t menu_diginc(int16_t incdec, uint8_t digit)
{
	int32_t result=1;
	
	while(digit--)
		result *=10;
	
	return result*incdec;
}

/*
 * initialize the menu system
 */
void menu_init(void)
{
	/* init the state */
    Flo = 1440000;
	prev_button = -100;
	cursor_loc = 0;
	cursor_type = 0;
	menu_demod_type = 1;
	filter_num = 2;
    
    /* default startup */
    rxadc_set_ns(1);
	rxadc_set_lo(Flo);
	menu_set_demod(menu_demod_type);
    Audio_SetFilter(filter_num);
    
	/* init the display */
	ST7735_drawstr(0, 0,(uint8_t *)"    iceRadio    ", ST7735_MAGENTA, ST7735_BLACK);
    ST7735_drawFastHLine(0, 10, 128, ST7735_CYAN);	menu_LO_draw();
    ST7735_drawFastHLine(0, 46, 128, ST7735_CYAN);	menu_LO_draw();
	menu_filter_draw();
	menu_demod_draw();
	menu_rssi_draw();
	ST7735_drawstr(0,56,(uint8_t *)"                ", ST7735_WHITE, ST7735_BLACK);
	menu_draw_cursor(1);
	
	rssi_goal = cyclegoal_ms(100);
}

/*
 * update the menu
 */
void menu_update(void)
{
	int16_t new_encoder, new_button;
	uint32_t tempLO;
	
	/* update location / value */
	new_encoder = SysTick_get_encoder();
	if(new_encoder != 0)
	{
		printf("Encoder update: %d\n", new_encoder);
		if(cursor_type)
		{
			/* cursor type 1 => update value */
			if(cursor_loc<8)
			{
				/* Locations 0-7 are LO digits */
				tempLO = Flo + menu_diginc(new_encoder, 7-cursor_loc);
				if(tempLO < 20000000)
				{
                    Flo = tempLO;
					rxadc_set_lo(Flo);
					printf("Flo (actual): %u\n", (unsigned int)rxadc_get_lo());
					menu_LO_draw();
					menu_draw_cursor(1);
				}
			}
			else if(cursor_loc == 8)
			{
				/* Location 13 is filter bw */
				filter_num = filter_num+new_encoder;
				filter_num = (filter_num == 6) ? 0 : filter_num;
				filter_num = (filter_num == 255) ? 5 : filter_num;
				Audio_SetFilter(filter_num);
				printf("Filter: %d\n", filter_num);
				menu_filter_draw();
				menu_draw_cursor(1);
			}
			else if(cursor_loc == 9)
			{
				/* Location 14 is mod type */
                int8_t temp = menu_demod_type;
 				printf("Before Mod: %d\n", menu_demod_type);
				temp = temp+new_encoder;
 				printf("temp: %d\n", temp);
				temp = (temp >= MAX_DEMODS) ? 0 : temp;
				temp = (temp < 0 ) ? MAX_DEMODS-1 : temp;
 				printf("temp: %d\n", temp);
                menu_demod_type = temp;
				menu_set_demod(menu_demod_type);
				printf("After Mod: %d\n", menu_demod_type);
				menu_demod_draw();
				menu_draw_cursor(1);
			}
		}
		else
		{
			/* cursor type 0 => update location */
			menu_draw_cursor(0);
			cursor_loc += new_encoder;
			if(cursor_loc == 10)
				cursor_loc = 0;
			if(cursor_loc == 255)
				cursor_loc = 9;
			printf("cursor_loc = %d\n", cursor_loc);
			menu_draw_cursor(1);
		}
	}
	
	/* Switch cursor types */
	new_button = SysTick_get_button();
	if(new_button != prev_button)
	{
		printf("Button update: %d\n", new_button);
		prev_button = new_button;
		snprintf(txt_buff, 17, "btn = % 6d", new_button);
		if(new_button)
		{
			menu_draw_cursor(0);
			cursor_type = 1^cursor_type;
			menu_draw_cursor(1);
		}
		printf("cursor_type: %d\n", cursor_type);
	}
	
	/* Update RSSI */
	if(!cyclecheck(rssi_goal))
	{
		/* update RSSI */
		menu_rssi_draw();
        
        /* update sync freq */
		menu_sync_draw();
        
		/* reset the goal for another 100ms */
		rssi_goal = cyclegoal_ms(100);
	}
}