#include <string.h>
#include "bmpc.h"
#include "lcdc.h"
#include "..\..\BioFP\FPAlg\mtx.h"
#include "..\..\BioFP\FPAlg\img.h"
#include "unit.h"
#include "acad.h"
#include "acap.h"
#include "cr.h"
#include "gwnd.h"
#include "user.h"
#include "desktop.h"
#include "devprim.h"
#include "fpapi.h"



#include "devprim.h"
#include "fpimage.h"

static BMPC		*bmpFP, _bmpFP;
static unsigned char FPHalfBuffer[19500];	// 130*150


void FPImageInitialize(void)
{
	bmpFP = &_bmpFP;
	bmpcInit(bmpFP, FPHalfBuffer, 130, 150);
}

void FPImageDrawBoundary(int x, int y, int Color)
{
	int		w, h;

	w = bmpFP->Width + 4; h = bmpFP->Height + 4;
	lcdFillRect(x,  y,  w, 2, Color);
	y += 2; h -= 2; w -= 4;
	lcdFillRect(x,  y,  2, h, Color);
	x += 2;
	lcdFillRect(x+w,y,  2, h, Color);
	h -= 2;
	lcdFillRect(x,  y+h,w, 2, Color);
}

void FPImageDraw(int x, int y)
{
	IMAGE	*imgS;

	imgS = fprGetImage(_GetFPReader());
	imgResizeInverseRaw((unsigned char *)bmpFP->Buffer, (int)bmpFP->Width, (int)bmpFP->Height, imgS);
	lcdPutGray(x+2, y+2, bmpFP);
}

void FPImageDrawPrompt(int x, int y)
{
	memset(bmpFP->Buffer, 0xff, 19500);
	lcdPutGray(x+2, y+2, bmpFP);
}

void FPImageGet(unsigned char *Buffer)
{
	memcpy(Buffer, bmpFP->Buffer, 19500);
}

