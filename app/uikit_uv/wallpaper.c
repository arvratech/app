#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "rtc.h"
#include "lcdc.h"
#include "UIEnum.h"
#include "cgrect.h"
#include "wallpaper.h"


static void _WallpaperLoadBmpFile(G2D_BUFFER *g2d, char *fileName)
{
	BMPC	*bmp, _bmp;
	char	resFileName[128];
	int		rval;

	bmp = &_bmp;
	bmpcInit(bmp, (void *)g2d->addr, 0, 0);
	strcpy(resFileName, "wallpaper/"); strcat(resFileName, fileName);
	rval = bmpcReadBmp(bmp, resFileName);
//printf("lcdInitBackgroundScreen=%d [%s]\n", rval, resFileName);
	if(rval < 0) {
		g2dSetSize(g2d, lcdPixelWidth(), lcdPixelHeight());
		g2dFill(g2d, 0, 0, g2d->width, g2d->height, darkGrayColor);
	} else {
		g2dSetSize(g2d, (int)bmp->width, (int)bmp->height);
	}
}

static void _WallpaperDarken(G2D_BUFFER *g2d)
{
	BMPC	*bmp, _bmp;
	CGRect	rt;

	bmp = &_bmp;
	bmpcInit(bmp, (void *)g2d->addr, g2d->width, g2d->height);
	rt.x = rt.y = 0; rt.width = 0; rt.height = 32;
	lcdPointToPixelRect(&rt);
	bmpcDarkenRect(bmp, 0, 0, bmpcWidth(bmp), rt.height);
}

static void _WallpaperLoad(G2D_BUFFER *g2d, int id)
{
	char	fileName[64];

	if(id > 0) sprintf(fileName, "bgscreen%d.bmp", id);
	else	strcpy(fileName, "bgscreen.bmp");
	_WallpaperLoadBmpFile(g2d, fileName);
}

void wallpaperInit(int id)
{
	G2D_BUFFER	*g2d;

	g2d = g2dMemoryBuffer(1);
	_WallpaperLoad(g2d, id);
	_WallpaperDarken(g2d);
}

void wallpaperDarken(void)
{
	G2D_BUFFER	*g2d;

	g2d = g2dMemoryBuffer(1);
	_WallpaperDarken(g2d);
}

int wallpaperNext(int id)
{
	char	fileName[64];
	struct stat		buf;
	int		rval;

	sprintf(fileName, "wallpaper/bgscreen%d.bmp", id+1);
	rval = stat(fileName, &buf);
	if(rval >= 0) rval = 1;
	return rval;
}

int wallpaperChange(int wallpaperId)
{
	G2D_BUFFER	*g2d;
	int		id;

	id = wallpaperId;
	g2d = g2dMemoryBuffer(0);
	_WallpaperLoad(g2d, id);
	if(wallpaperNext(id) > 0) id++;
	else	id = 0; 
	g2d = g2dMemoryBuffer(1);
	_WallpaperLoad(g2d, id);
	return id;
}

int wallpaperAnimate(int wallpaperId, int alpha)
{
#ifdef _A20
	G2D_BUFFER	*g2dLcd, *g2d, *g2d2;
	CGRect	rt;
//unsigned long tt;

//tt = MS_TIMER;
	g2d  = g2dMemoryBuffer(1);
	g2d2 = g2dMemoryBuffer(0);
	lcdChangePage();
	rt.x = rt.y = 0; rt.width = lcdPixelWidth(); rt.height = lcdPixelHeight();
	lcdClipToRect(&rt);
	g2dLcd = g2dLcdBuffer();
	g2dBlit(g2dLcd, 0, 0, rt.width, rt.height, g2d, 255);
	g2dBlit(g2dLcd, 0, 0, rt.width, rt.height, g2d2, 255-alpha);
	lcdSwapPage();
//printf("%d\n", MS_TIMER-tt);	// 6. 11. 566. 12. 5, 12, 6, 11
#else
	G2D_BUFFER	*g2dLcd, *g2d, *g2d2;
	CGRect	rt;
	unsigned long	c, a, *s;
	int		i, j, width, height;

	g2d  = g2dMemoryBuffer(1);
	g2d2 = g2dMemoryBuffer(0);
	width = g2d->width; height = g2d->height;
	a = alpha; a = (255-alpha) << 24;
	s = (unsigned long *)g2d2->addr;
	for(i = 0;i < height;i++) {
		for(j = 0;j < width;j++) {
			c = *s;
			c = (c & 0x00ffffff) + a;
			*s++ = c;
		}
	}
	lcdChangePage();
	rt.x = rt.y = 0; rt.width = lcdPixelWidth(); rt.height = lcdPixelHeight();
	lcdClipToRect(&rt);
	g2dLcd = g2dLcdBuffer();
	g2dBlit(g2dLcd, 0, 0, rt.width, rt.height, g2d, 255);
	g2dBlit(g2dLcd, 0, 0, rt.width, rt.height, g2d2, 255);
	lcdSwapPage();
#endif
}

void wallpaperDraw(CGRect *rect)
{
	G2D_BUFFER	*g2d;
	CGRect	rt;

	if(rect) {
//		rt.x = _scale(rect->x); rt.y = _scale(rect->y); rt.width = _scale(rect->width); rt.height = _scale(rect->height);
		rt.x = rect->x; rt.y = rect->y; rt.width = rect->width; rt.height = rect->height;
	} else {
		rt.x = rt.y = 0; rt.width = lcdPixelWidth(); rt.height = lcdPixelHeight();
	}
	g2d = g2dMemoryBuffer(1);
	lcdClipToRect(&rt);
	lcdPutBackgroundScreen(0, 0, g2d);
}

void wallpaperSwap(char *fileName)
{
	G2D_BUFFER	*g2d;

	g2d = g2dMemoryBuffer(1);
	_WallpaperLoadBmpFile(g2d, fileName);
	_WallpaperDarken(g2d);
}

void _WallpaperBitmap(G2D_BUFFER *g2d, UIColor color, char *fileName, char *text, char *text2)
{
	G2D_BUFFER	*g2dS;
	BMPC	*bmpS, _bmpS;
	char	resFileName[128];
	int		y, w, h, wh, tw, th, rval, size;

	w = lcdPixelWidth(); h = lcdPixelHeight();
	g2dSetSize(g2d, w, h);
	g2dFill(g2d, 0, 0, w, h, color);
	g2dS = g2dMemoryBuffer(0);
	bmpS = &_bmpS;
	bmpcInit(bmpS, (void *)g2dS->addr, 0, 0);
	strcpy(resFileName, "res/"); strcat(resFileName, fileName);
	rval = bmpcReadAlphaMask(bmpS, resFileName, RGB(0,0,255));
	if(rval) return;
	g2dSetSize(g2dS, (int)bmpS->width, (int)bmpS->height);
	wh = w >> 1;
	y = (lcdHeight() >> 3) + 60 + 24;	// clock;
	y = _scale(y);
	y += 30;
	g2dBlit(g2d, (w-wh)>>1, y, wh, wh, g2dS, 255);
	if(text2) y += wh + 40;
	else	  y += wh + 60;
	ftSetSize(26);
	size = ftPixelSize();
	bmpcInit(bmpS, (void *)g2dS->addr, 0, 0);
	bmpcPutText(bmpS, text, RGB(0,0,0));
	g2dSetSize(g2dS, (int)bmpS->width, (int)bmpS->height);
	tw = (bmpS->width * size + 63) >> 6;
	th = (bmpS->height * size + 63) >> 6;
	g2dBlit(g2d, (w-tw)>>1, y, tw, th, g2dS, 255);
	if(text2) {
		y += th + 20;
		bmpcInit(bmpS, (void *)g2dS->addr, 0, 0);
		bmpcPutText(bmpS, text2, RGB(0,0,0));
		g2dSetSize(g2dS, (int)bmpS->width, (int)bmpS->height);
		tw = (bmpS->width * size + 63) >> 6;
		th = (bmpS->height * size + 63) >> 6;
		g2dBlit(g2d, (w-tw)>>1, y, tw, th, g2dS, 255);
	}
}

void wallpaperSwapBitmap(UIColor color, char *fileName, char *text, char *text2)
{
	G2D_BUFFER	*g2d;

	g2d = g2dMemoryBuffer(1);
	_WallpaperBitmap(g2d, color, fileName, text, text2);
}

void wallpaperPut(void)
{
	G2D_BUFFER	*g2d;

	g2d = g2dMemoryBuffer(1);
	lcdPutBackgroundScreen(0, 0, g2d);
}

