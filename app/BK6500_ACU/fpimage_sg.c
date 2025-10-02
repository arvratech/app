#include <string.h>
#include "bmpc.h"
#include "lcdc.h"
#include "fpimage.h"

static BMPC		*bmpFP, _bmpFP;
static unsigned char FPHalfBuffer[19500];	// 130 * 150


void FPImageInitialize(void)
{
	bmpFP = &_bmpFP;
	bmpcInit(bmpFP, FPHalfBuffer, 130, 150);	// 150, 172
}

void FPImageDrawBoundary(int x, int y, int Color)
{
	int		i, j, w, h;

	w = bmpFP->Width + 4; h = bmpFP->Height + 4;
	lcdFillRect(x,  y,  w, 2, Color);
	y += 2; h -= 2; w -= 4;
	lcdFillRect(x,  y,  2, h, Color);
	x += 2;
	lcdFillRect(x+w,y,  2, h, Color);
	h -= 2;
	lcdFillRect(x,  y+h,w, 2, Color);
}

void FPImageDrawImage(int x, int y, unsigned char *Buffer)
{
	unsigned char	*s, *d;
	unsigned long	uval;
	int		i, j, w, h, sw, c;

	s = Buffer; d = (unsigned char *)bmpFP->Buffer;
	w = bmpFP->Width; h = bmpFP->Height;
	sw = w + w;
	for(i = 0;i < h;i++) {
		for(j = 0;j < w;j++) {
			*d++ = (*s + *(s+1) + *(s+sw) + *(s+sw+1)) >> 2;
			s += 2;
		}
		s += sw;
	}
	lcdPutGray(x+2, y+2, bmpFP);
}

void FPImageDrawPrompt(int x, int y)
{
	memset(bmpFP->Buffer, 0xff, 19500);
	lcdPutGray(x+2, y+2, bmpFP);
}
