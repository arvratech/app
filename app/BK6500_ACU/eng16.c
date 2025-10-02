#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmpc.h"
#include "lcdc.h"
#include "eng16.h"

#include "gfont_eng_16.c"

unsigned char eng_v16_pitch[95] = {
 6, 6, 7,14,10,14,13, 6, 7, 7, 8, 9, 6, 9, 6, 8,
 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 6, 6,10, 9,10, 9,
17,11,11,12,11,11,10,13,11, 4, 9,11,10,14,12,13,
11,13,11,11,10,12,11,15,12,11,11, 9,15, 9, 9, 7,
 6,10,10,10,10,10, 6,10,10, 4, 5, 9, 4,14,10,11,
10,10, 6,10, 6,10, 9,13,10, 9,10,10,10,10,13
};

extern unsigned char font_eng_20[][60];


void _GetEng16Font(int c, unsigned char **font, int *w)
{
	int		idx;

	if(c >= 0x20 && c <= 0x7e) idx = c - 0x20;
	else	idx = 0x1f;	// '?'
	if(!idx) *font = font_eng_20[0];
	else	 *font = font_eng_16[idx-1];
	*w = eng_v16_pitch[idx];
}

// support only Eng font
int bmpcPutEng16Char(BMPC *bmpD, int x, int y, int c, int Color)
{
	BMPC	*bmpS, _bmpS;
	unsigned char	*font;
	int		w;

	bmpS = &_bmpS;
	_GetEng16Font(c, &font, &w);
	bmpcInit(bmpS, font, 16, 16);
	bmpcPutMaskEx(bmpD, x, y, w, 16, Color, bmpS);
	return w;
}

int bmpcPutEng16Text(BMPC *bmpD, int x, int y, char *string, int Color)
{
	BMPC	*bmpS, _bmpS;
	char	*s;
	unsigned char	*font;
	int		c, x1, w, h, tw, dw;

	if(!string || !string[0]) {
		bmpD->DoRect.x = bmpD->DoRect.y = bmpD->DoRect.Width = bmpD->DoRect.Height = 0;
		tw = 0;
	} else {
		bmpS = &_bmpS;
		h = 16;
		x1 = bmpD->Width;
		tw = dw = 0;
		s = string;
		while(*s) {
			c = *s++;
			_GetEng16Font(c, &font, &w); 
			bmpcInit(bmpS, font, 16, h);
			bmpcPutMaskEx(bmpD, x, y, w, h, Color, bmpS);
			x += w; tw += w;
			if(x1 > bmpD->DoRect.x) x1 = bmpD->DoRect.x;
			dw += bmpD->DoRect.Width;
		}
//cprintf("Tw=%d Do=%d %d\n", tw, dw, GetStringWidth(string));
		bmpcSetDoRect(bmpD, x1, bmpD->DoRect.y, dw, bmpD->DoRect.Height);	
	}
	return tw;
}

void _lcdRefresh(void);

int lcdPutEng16Char(int x, int y, int c, int Color)
{
	int		w;

	w = bmpcPutEng16Char(lcdGetBmp(), x, y, c, Color);
	_lcdRefresh();	
	return w;
}

int lcdPutEng16Text(int x, int y, char *string, int Color)
{
	int		w;

	w = bmpcPutEng16Text(lcdGetBmp(), x, y, string, Color);
	_lcdRefresh();
	return w;
}
