int lcdDirection(void)
{
	return (int)_lcdLandscape;
}

void lcdSetDirection(int landscape)
{
	_lcdLandscape = landscape;
	lcdSetDefaultClipRect();
}

int lcdWidth(void)
{
	int		val;

	if(_lcdLandscape) val = _lcdHeight;
	else	val = _lcdWidth;
	return val;
}

int lcdHeight(void)
{
	int		val;

	if(_lcdLandscape) val = _lcdWidth;
	else	val = _lcdHeight;
	return val;
}

int lcdPixelWidth(void)
{
	if(_lcdLandscape) return g2dLcd->height;
	else	return g2dLcd->width;
}

int lcdPixelHeight(void)
{
	if(_lcdLandscape) return g2dLcd->width;
	else	return g2dLcd->height;
}

// 24.8 fixed point format
int lcdScale(void)
{
	int		scale;

	//   model     resolution  pitch(mm/pixel) resolution(point)
	// ILI9488r     320x480      0.153			  320x480
	// ILI9806E-2C	480x800      0.108            336x560
	if(g2dLcd->height > 500) scale = 0x016d;	// (30<<8) / 21;
	else	scale = 0x0100;						// 1.0
	return scale;
}

void lcdClipToRect(CGRect *rect)
{
	_lcdClipRect.x = rect->x; _lcdClipRect.y = rect->y;
	_lcdClipRect.width = rect->width; _lcdClipRect.height = rect->height;
}

//void lcdClipToMask(CGRRect *rect, BMPC *mask)

void lcdSetDefaultClipRect(void)
{
	_lcdClipRect.x = 0; _lcdClipRect.y = 0;
	_lcdClipRect.width = lcdPixelWidth();
	_lcdClipRect.height = lcdPixelHeight();
}

void lcdPointToPixelRect(CGRect *rect)
{
	int		x, y;

	x = rect->x; y = rect->y;
	rect->x = _scale(x); rect->y = _scale(y);
	rect->width = _scale(x+rect->width) - rect->x; rect->height = _scale(y+rect->height) - rect->y;
}

void lcdPixelClipToRect(CGRect *rect, CGRect *clipRect)
{
	int		x, y;

	x = rect->x + clipRect->x; y = rect->y + clipRect->y;
	_lcdClipRect.x = _scale(x);
	_lcdClipRect.y = _scale(y);
	_lcdClipRect.width  = _scale(x+clipRect->width) - _lcdClipRect.x;
	_lcdClipRect.height = _scale(y+clipRect->height) - _lcdClipRect.y;
	x = rect->x; y = rect->y;
	rect->x = _scale(x); rect->y = _scale(y);
	rect->width = _scale(x+rect->width) - rect->x; rect->height = _scale(y+rect->height) - rect->y;
}

void lcdClear(UIColor color)
{
	int		x, y, w, h;

	x = _lcdClipRect.x; y = _lcdClipRect.y; w = _lcdClipRect.width; h = _lcdClipRect.height;
	g2dFill(g2dLcd, x, y, w, h, color);
}

void lcdFillRect(int x, int y, int width, int height, UIColor color)
{
	g2dFill(g2dLcd, x, y, width, height, color);
}

void lcdDimm(int x, int y, int width, int height, int value)
{
	G2D_BUFFER	*g2d;

	g2d = g2dMemoryBuffer(0);
	g2dSetSize(g2d, width, height);
	g2dFill(g2d, 0, 0, width, height, 0x80000000);
	g2dBlit(g2dLcd, x, y, width, height, g2d, 255);
//	g2dBlit(g2dLcd, x, y, width, height, g2dLcd, 128);
}

void lcdRect(int x, int y, int width, int height, UIColor color)
{
	g2dFill(g2dLcd, x, y, width, 1, color);
	g2dFill(g2dLcd, x, y+1, 1, height-2, color);
	g2dFill(g2dLcd, x+width-1, y+1, 1, height-2, color);
	g2dFill(g2dLcd, x, y+height-1, width, 1, color);
}

void lcdHorizontalLine(int x, int y, int width, UIColor color)
{
	g2dFill(g2dLcd, x, y, width, 1, color);
}

void lcdVerticalLine(int x, int y, int height, UIColor color)
{
	g2dFill(g2dLcd, x, y, 1, height, color);
}

void lcdPixel(int x, int y, UIColor color)
{
	g2dFill(g2dLcd, x, y, 1, 1, color);
}

void lcdFadeoutBackgroundScreen(int alpha)
{
/*
	BMPC	*bmpS, *bmp, _bmp;
	unsigned long	*d, color;
	int		i, j, width, height;

	bmpS = &_bmpWallpaper;
	width = bmpcWidth(bmpS); height = bmpcHeight(bmpS);
	bmp = &_bmp;
	bmpcInit(bmp, _bmpBuffer[0].addr, width, height);
	color = 0xff000000;
	_bmpFill(bmp, 0, 0, width, height, color);
	_bmpBlitAlpha(bmp, 0, 0, width, height, bmpS, alpha);
	g2dBlit(g2dLcd, 0, 0, width, height, bmp, 255);
*/
}

void lcdPutBackgroundScreen(int x, int y, G2D_BUFFER *g2d)
{
	g2dBlit(g2dLcd, x, y, lcdPixelWidth()-x, lcdPixelHeight()-y, g2d, 255);
}

void lcdPutBmpFile(int x, int y, int width, int height, char *fileName)
{
	G2D_BUFFER	*g2d;
	BMPC	*bmp, _bmp;
	char	resFileName[128];
	int		rval;

	g2d = g2dMemoryBuffer(0);
	bmp = &_bmp;
	bmpcInit(bmp, g2d->addr, 0, 0);
	strcpy(resFileName, "res/"); strcat(resFileName, fileName);
	rval = bmpcReadBmp(bmp, resFileName);
	if(!rval) {
		g2dSetSize(g2d, (int)bmp->width, (int)bmp->height);
		if(width <= 0) width = bmp->width;
		if(height <= 0) height = bmp->height;
		g2dBlit(g2dLcd, x, y, width, height, g2d, 255);
	}
}

void lcdPutAlphaMaskFile(int x, int y, int width, int height, UIColor color, char *fileName)
{
	G2D_BUFFER	*g2d;
	BMPC	*bmp, _bmp;
	char	resFileName[128];
	int		rval, w, h, alpha;
	
	g2d = g2dMemoryBuffer(0);
	bmp = &_bmp;
	bmpcInit(bmp, g2d->addr, 0, 0);
	strcpy(resFileName, "res/"); strcat(resFileName, fileName);
	rval = bmpcReadAlphaMask(bmp, resFileName, color);
	if(!rval) {
		g2dSetSize(g2d, (int)bmp->width, (int)bmp->height);
		if(width <= 0) width = bmp->width;
		if(height <= 0) height = bmp->height;
		alpha = color >> 24;
		g2dBlit(g2dLcd, x, y, width, height, g2d, alpha);
	}
}

/*
void lcdPutAlphaMaskFileEx(int x, int y, int width, int height, UIColor color, char *fileName)
{
	G2D_BUFFER	*g2d;
	BMPC	*bmp, _bmp;
	char	resFileName[128];
	int		rval, w, h, alpha;
	
	g2d = g2dMemoryBuffer(0);
	bmp = &_bmp;
	bmpcInit(bmp, g2d->addr, 0, 0);
	strcpy(resFileName, "res/"); strcat(resFileName, fileName);
	rval = bmpcReadAlphaMaskEx(bmp, resFileName, color);
	if(!rval) {
		g2dSetSize(g2d, (int)bmp->width, (int)bmp->height);
		if(width <= 0) width = bmp->width;
		if(height <= 0) height = bmp->height;
		alpha = color >> 24;
		g2dBlit(g2dLcd, x, y, width, height, g2d, alpha);
	}
}
*/

#include "jpegdev.h"

void lcdPutJpeg(int x, int y, int width, int height, unsigned char *jpegBuffer, int jpegSize)
{
	G2D_BUFFER	*g2d;
	JPEG		*jpeg, _jpeg;
	BMP_INFO	*bmp, _bmp;
	int		rval;

	jpeg = &_jpeg;
	jpeg->fileName	= NULL;
	jpeg->buf		= jpegBuffer;
	jpeg->bufLength	= jpegSize;
	bmp = &_bmp;
	g2d = g2dMemoryBuffer(0);
#ifdef _A20
	bmp->buf = g2d->addr;
#endif	
	rval = jpegDevDecode(jpeg, bmp);
printf("jpegDevDecode=%d\n", rval);
	if(!rval) {
printf("jpegDevDecode: w=%d h=%d\n", bmp->width, bmp->height);
		if(width <= 0) width = bmp->width;
		if(height <= 0) height = bmp->height;
#ifndef _A20
		memcpy((void *)g2d->addr, bmp->buf, bmp->bufLength);
#endif
		g2dSetSize(g2d, bmp->width, bmp->height);
		g2dBlit(g2dLcd, x, y, width, height, g2d, 255);
	}
}

void lcdPutJpegFile(int x, int y, int width, int height, char *fileName)
{
	G2D_BUFFER	*g2d;
	JPEG		*jpeg, _jpeg;
	BMP_INFO	*bmp, _bmp;
	int		rval;

printf("%d,%d %d %d [%s]\n", x, y, width, height, fileName);
	jpeg = &_jpeg;
	jpeg->fileName	= fileName;
	jpeg->buf		= NULL;
	jpeg->bufLength	= 0;
	bmp = &_bmp;
	g2d = g2dMemoryBuffer(0);
#ifdef _A20
	bmp->buf = g2d->addr;
#endif	
	rval = jpegDevDecode(jpeg, bmp);
printf("jpegDevDecode=%d\n", rval);
	if(!rval) {
printf("jpegDevDecode: w=%d h=%d\n", bmp->width, bmp->height);
		if(width <= 0) width = bmp->width;
		if(height <= 0) height = bmp->height;
#ifndef _A20
		memcpy((void *)g2d->addr, jpeg->buf, jpeg->bufLength);
#endif
		g2dSetSize(g2d, bmp->width, bmp->height);
		g2dBlit(g2dLcd, x, y, width, height, g2d, 255);
	}
}

int lcdPutText(int x, int y, char *string, UIColor color)
{
#ifdef _FREETYPE_
	G2D_BUFFER	*g2d;
	BMPC	*bmp, _bmp;
	int		w, alpha;

	g2d = g2dMemoryBuffer(0);
	bmp = &_bmp;
	bmpcInit(bmp, g2d->addr, 0, 0);
	w = bmpcPutText(bmp, string, color);
	g2dSetSize(g2d, (int)bmp->width, (int)bmp->height);
	alpha = color >> 24;
	g2dBlit(g2dLcd, x, y, g2d->width, g2d->height, g2d, alpha);
#else
	G2D_BUFFER	*g2d;
	BMPC	*bmp, _bmp;
	int		w, h, size, alpha;

//printf("%d,%d [%s]\n", x, y, string);
	g2d = g2dMemoryBuffer(0);
	bmp = &_bmp;
	bmpcInit(bmp, g2d->addr, 0, 0);
	bmpcPutText(bmp, string, color);
	g2dSetSize(g2d, (int)bmp->width, (int)bmp->height);
	size = ftPixelSize();
	w = (g2d->width * size + 63) >> 6;
	h = (g2d->height * size + 63) >> 6;
	alpha = color >> 24;
//printf("%d,%d %d %d\n", x, y, w, h);
	g2dBlit(g2dLcd, x, y, w, h, g2d, alpha);
#endif
	return w;
}

