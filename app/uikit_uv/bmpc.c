#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "ctfont.h"
#include "bmpc.h"


void bmpcInit(BMPC *bmp, void *buffer, int width, int height)
{
	bmp->type		= TYPE_COLOR_BITMAP;
	bmp->bitCount	= 0;
	bmp->status		= 0;
	bmp->buffer		= (unsigned long *)buffer;
	bmp->width		= width;
	bmp->height		= height;
}

void bmpcSetSize(BMPC *bmp, int width, int height)
{
	bmp->width = width; bmp->height = height;
}

int bmpcWidth(BMPC *bmp)
{
	return (int)bmp->width;
}

int bmpcHeight(BMPC *bmp)
{
	return (int)bmp->height;
}

unsigned long *bmpcBuffer(BMPC *bmp)
{
	return bmp->buffer;
}

unsigned long *bmpcStartBuffer(BMPC *bmp, int x, int y)
{
	return bmp->buffer + y * bmp->width + x;
}

void bmpcClipRect(BMPC *bmp, CGRect *rect)
{
	CGRect	rt;

	rt.x = rt.y = 0; rt.width = bmp->width; rt.height = bmp->height;
	rectIntersection(&rt, rect, rect);
}

int bmpcPut(BMPC *bmp, int x, int y, BMPC *bmpS)
{
	return bmpcPutEx(bmp, x, y, (int)bmpS->width, (int)bmpS->height, bmpS);
}

int bmpcPutEx(BMPC *bmp, int x, int y, int width, int height, BMPC *bmpS)
{
	unsigned long	*s, *d;
	CGRect	rt;
	int		i, j, w, h, sw, dw;

	rt.x = x; rt.y = y; rt.width = width; rt.height = height;
	bmpcClipRect(bmp, &rt);
	w = rt.width; h = rt.height;
	if(w <= 0 || h <= 0) return -1;
	dw = bmp->width; d = bmp->buffer + rt.y * dw + rt.x;
	x = rt.x - x; y = rt.y - y;
	sw = bmpS->width; s = bmpS->buffer + y * sw + x;
//printf("%d,%d %d %d sw=%d d=%08x\n", x, y, w, h, sw, d);
	sw -= w; dw -= w;
	for(i = 0;i < h;i++) {
		for(j = 0;j < w;j++) *d++ = *s++;
		s += sw; d += dw;
	}
	return 0;
}

void bmpcFillRect(BMPC *bmp, int x, int y, int width, int height, unsigned long color)
{
	CGRect	rt;
	unsigned long	*d;
	int		i, j, w, h, dw;	

	rt.x = x; rt.y = y; rt.width = width; rt.height = height;
	bmpcClipRect(bmp, &rt);
	w = rt.width; h = rt.height;
	if(w <= 0 || h <= 0) return;
	dw = bmp->width; d = bmp->buffer + rt.y * dw + rt.x;
	dw -= w;
	for(i = 0;i < h;i++) {
		for(j = 0;j < w;j++) *d++ = color;
		d += dw;
	}
}

void bmpcDarkenRect(BMPC *bmp, int x, int y, int width, int height)
{
	CGRect	rt;
	unsigned long	*d, color, a;
	int		c, i, j, w, h, dw, alpha;

	rt.x = x; rt.y = y; rt.width = width; rt.height = height;
	bmpcClipRect(bmp, &rt);
	w = rt.width; h = rt.height;
	if(w <= 0 || h <= 0) return;
	dw = bmp->width; d = bmp->buffer + rt.y * dw + rt.x;
	dw -= w; alpha = 256 - h * 4;
	for(i = 0;i < h;i++) {
		for(j = 0;j < w;j++) {
			color = *d;
			a = 0xff000000;
			c = color & 0xff; c = (c * alpha) >> 8; a |= c;
			c = (color >> 8) & 0xff; c = (c * alpha) >> 8; a |= c << 8;
			c = (color >> 16) & 0xff; c = (c * alpha) >> 8; a |= c << 16;
			*d++ = a;
		}
		d += dw; alpha += 4;
	}
}

void bmpcInverse(BMPC *bmp, int x, int y, int width, int height)
{
	unsigned long	*d, c;
	CGRect	rt;
	int		i, j, w, h, dw;	

	rt.x = x; rt.y = y; rt.width = width; rt.height = height;
	bmpcClipRect(bmp, &rt);
	w = rt.width; h = rt.height;
	if(w <= 0 || h <= 0) return;
	dw = bmp->width; d = bmp->buffer + rt.y * dw + rt.x;
	dw -= w;
	for(i = 0;i < h;i++) {
		for(j = 0;j < w;j++) {
			c = (*d) & 0x00ffffff;
			*d++ = ~c;
		}
		d += dw;
	}
}

void bmpcDarken(BMPC *bmp, int x, int y, int width, int height)
{
	unsigned long	*d;
	CGRect	rt;
	int		i, j, w, h, dw;	

	rt.x = x; rt.y = y; rt.width = width; rt.height = height;
	bmpcClipRect(bmp, &rt);
	w = rt.width; h = rt.height;
	if(w <= 0 || h <= 0) return;
	dw = bmp->width; d = bmp->buffer + rt.y * dw + rt.x;
	dw -= w;
	for(i = 0;i < h;i++) {
		for(j = 0;j < w;j++) {
			if(!(i&1) && (j&1) || (i&1) && !(j&1)) *d = 0xff000000;
			d++;
		}
		d += dw;
	}
}

void bmpcRect(BMPC *bmp, int x, int y, int width, int height, unsigned long color)
{
	unsigned long	*d;
	CGRect	rt;
	int		i, w, h, dw;	

	rt.x = x; rt.y = y; rt.width = width; rt.height = height;
	bmpcClipRect(bmp, &rt);
	w = rt.width; h = rt.height;
	if(w <= 0 || h <= 0) return;
	color |= 0xff000000;
	dw = bmp->width; d = bmp->buffer + rt.y * dw + rt.x;
	for(i = 1;i < w;i++) *d++ = color;
	for(i = 1;i < h;i++) { *d = color; d += dw; }
	for(i = 1;i < w;i++) *d-- = color;
	for(i = 1;i < h;i++) { *d = color; d -= dw; }
}

void bmpcHorizontalLine(BMPC *bmp, int x, int y, int width, unsigned long color)
{
	bmpcFillRect(bmp, x, y, width, 1, color);
}

void bmpcVerticalLine(BMPC *bmp, int x, int y, int height, unsigned long color)
{
	bmpcFillRect(bmp, x, y, 1, height, color);
}

static unsigned char _LineBuffer[8096];

int bmpcReadBmp(BMPC *bmp, char *fileName)
{
	FILE	*fp;
	BITMAPFILEHEADER	*bmfh, _bmfh;
	BITMAPINFOHEADER	*bmih, _bmih;
	RGBQUAD		rgbqs[256];
	unsigned long	*d, argb;
	unsigned char	*s;
	long	offset;
	int		i, j, c, rval, w, h, sw, dw, size;

	fp = fopen(fileName, "r");
	if(!fp) {
printf("bmpFile=[%s] open error: %s\n", fileName, strerror(errno));
		return -1;
	}
	// BITMAPFILEHEADER(14Bytes) :  | bfType |     bfSize     | bfRes.1|| bfRes.2|    bfOffBits   |
	// 		ARM 에서는 bfType <-> bfSize 의 hole 때문에 sizeof(BITMAPFILEHEADER)=16
	bmfh = &_bmfh;
	rval = fread(bmfh, 1, 2, fp);
	if(rval < 0) { fclose(fp); return -2; }
	if(bmfh->bfType != 0x4d42) { fclose(fp); return -3; }	// 'BM'
	rval = fread(&bmfh->bfSize, 1, 12, fp);
	if(rval < 0) { fclose(fp); return -2; }
	bmih = &_bmih;
	rval = fread(bmih, 1, sizeof(BITMAPINFOHEADER), fp);
	if(rval < 0) { fclose(fp); return -2; }
//printf("size=%d width=%d height=%d planes=%d bitCount=%d\n", (int)bmih->biSize, (int)bmih->biWidth, (int)bmih->biHeight, (int)bmih->biPlanes, (int)bmih->biBitCount);
	if(bmih->biPlanes != 1 || (bmih->biBitCount != 8 && bmih->biBitCount != 24 && bmih->biBitCount != 32)) { fclose(fp); return -4; }
//	if(bmih->biSizeImage == 0) {
//printf("biSizeImage is recalculated\n");
	w = bmih->biWidth; h = bmih->biHeight;
	if(bmih->biBitCount == 8) sw = w;
	else if(bmih->biBitCount == 24) sw = w * 3;
	else	sw = w << 2;
	if(sw & 0x03) sw = ((sw >> 2) + 1) << 2;	// 4's multiply
	bmih->biSizeImage = sw * h;
	offset = bmfh->bfOffBits + bmih->biSizeImage;
	bmpcSetSize(bmp, w, h);
	dw = w; d = bmp->buffer;
//printf("bfSize=%d offBits=%d sizeImage=%d offset=%d\n", (int)bmfh->bfSize, (int)bmfh->bfOffBits, (int)bmih->biSizeImage, offset);
	if(bmih->biBitCount == 8) {
		dw -= w;
		size = sizeof(RGBQUAD) * 256;
		rval = fread(rgbqs, 1, size, fp);
		if(rval != size) { fclose(fp); return -3; }
		for(i = 0;i < h;i++) {
			offset -= sw;
			fseek(fp, offset, SEEK_SET);
			rval = fread(_LineBuffer, 1, w, fp);
			if(rval != w) { fclose(fp); return -2; }
			for(j = 0, s = _LineBuffer;j < w;j++) {
				c = *s++;
				argb = (rgbqs[c].rgbRed << 16) | (rgbqs[c].rgbGreen << 8) || rgbqs[c].rgbBlue;
				*d++ = ~argb;
			}
			d += dw;
		}
	} else if(bmih->biBitCount == 24) {
//printf("bitCount=24\n");
		dw -= w;
//printf("offset=%d sw=%d dw=%d\n", offset, sw, dw);
		size = w * 3;
		for(i = 0;i < h;i++) {
			offset -= sw;
			fseek(fp, offset, SEEK_SET);
			rval = fread(_LineBuffer, 1, size, fp);
			if(rval != size) { printf("error:i=%d/%d %d %d\n", i, h, rval, size); fclose(fp); return -2; }
			for(j = 0, s = _LineBuffer;j < w;j++) {
				argb = 0xff000000 | (s[2] << 16) | (s[1] << 8) | s[0];
				*d++ = argb; s += 3;
			}
			d += dw;
		}
	} else {
//printf("%d,%d %d %d offset=%d sw=%d\n", x, y, w, h, offset, sw);
//printf("bitCount=24\n");
		size = w << 2;
		for(i = 0;i < h;i++) {
			offset -= sw;
			fseek(fp, offset, SEEK_SET);
			rval = fread(d, 1, size, fp);
//if(File == 150 && rval != size) printf("5 %d/%d rval=%d size=%d fsErrno=%d\n", i, h, rval, size, fsErrno);	
			if(rval != size) { fclose(fp); return -2; }
			d += dw;
		}
	}
	fclose(fp);
	return 0;
}

int bmpcReadAlphaMask(BMPC *bmp, char *fileName, int color)
{
	FILE	*fp;
	BITMAPFILEHEADER	*bmfh, _bmfh;
	BITMAPINFOHEADER	*bmih, _bmih;
	RGBQUAD		rgbqs[256];
	unsigned long	*d, a;
	unsigned char	*s;
	long	offset;
	int		i, j, c, rval, w, h, sw, size;

	fp = fopen(fileName, "r");
	if(!fp) {
printf("bmpFile=[%s] open error: %s\n", fileName, strerror(errno));
		return -1;
	}
	// BITMAPFILEHEADER(14Bytes) :  | bfType |     bfSize     | bfRes.1|| bfRes.2|    bfOffBits   |
	// 		ARM 에서는 bfType <-> bfSize 의 hole 때문에 sizeof(BITMAPFILEHEADER)=16
	bmfh = &_bmfh;
	rval = fread(bmfh, 1, 2, fp);
	if(rval < 0) { fclose(fp); return -2; }
	if(bmfh->bfType != 0x4d42) { printf("bmp bmp file\n"); fclose(fp); return -3; }	// 'BM'
	rval = fread(&bmfh->bfSize, 1, 12, fp);
	if(rval < 0) { fclose(fp); return -2; }
	bmih = &_bmih;
	rval = fread(bmih, 1, sizeof(BITMAPINFOHEADER), fp);
	if(rval < 0) { fclose(fp); return -2; }
	if(bmih->biPlanes != 1 || bmih->biBitCount != 8) { fclose(fp); return -4; }
	w = bmih->biWidth; h = bmih->biHeight;
	sw = w;
	if(sw & 0x03) sw = ((sw >> 2) + 1) << 2;	// 4's multiply
	bmih->biSizeImage = sw * h;
	offset = bmfh->bfOffBits + bmih->biSizeImage;
	bmpcSetSize(bmp, w, h);
	d = bmp->buffer;
	size = sizeof(RGBQUAD) * 256;
	rval = fread(rgbqs, 1, size, fp);
	if(rval != size) { fclose(fp); printf("quad read error=%d %d\n", rval, size); return -3; }
	// palette
	//   0     1      32       64         92          128         160         192      193   194  ...  255
	// 0-0-0 1-1-1 37-37-37 79-79-79  113-113-113 170-170-170 222-222-222 255-255-255 0-0-0 0-0-0 ... 0-0-0
	c = color & 0x00ffffff;
	for(i = 0;i < h;i++) {
		offset -= sw;
		fseek(fp, offset, SEEK_SET);
		rval = fread(_LineBuffer, 1, w, fp);
		if(rval != w) { fclose(fp); return -2; }
		for(j = 0, s = _LineBuffer;j < w;j++) {
			a = *s++;
			a = rgbqs[a].rgbGreen ^ 0xff;
			*d++ = (a << 24) | c;
		}
	}
	fclose(fp);
	return 0;
}

int bmpcReadAlphaMaskEx(BMPC *bmp, char *fileName, int color)
{
	FILE	*fp;
	BITMAPFILEHEADER	*bmfh, _bmfh;
	BITMAPINFOHEADER	*bmih, _bmih;
	RGBQUAD		rgbqs[256];
	unsigned long	*d, a;
	unsigned char	*s;
	long	offset;
	int		i, j, c, rval, w, h, sw, size, alpha;

	fp = fopen(fileName, "r");
	if(!fp) {
printf("bmpFile=[%s] open error: %s\n", fileName, strerror(errno));
		return -1;
	}
	// BITMAPFILEHEADER(14Bytes) :  | bfType |     bfSize     | bfRes.1|| bfRes.2|    bfOffBits   |
	// 		ARM 에서는 bfType <-> bfSize 의 hole 때문에 sizeof(BITMAPFILEHEADER)=16
	bmfh = &_bmfh;
	rval = fread(bmfh, 1, 2, fp);
	if(rval < 0) { fclose(fp); return -2; }
	if(bmfh->bfType != 0x4d42) { fclose(fp); return -3; }	// 'BM'
	rval = fread(&bmfh->bfSize, 1, 12, fp);
	if(rval < 0) { fclose(fp); return -2; }
	bmih = &_bmih;
	rval = fread(bmih, 1, sizeof(BITMAPINFOHEADER), fp);
	if(rval < 0) { fclose(fp); return -2; }
	if(bmih->biPlanes != 1 || bmih->biBitCount != 8) { fclose(fp); return -4; }
	w = bmih->biWidth; h = bmih->biHeight;
	sw = w;
	if(sw & 0x03) sw = ((sw >> 2) + 1) << 2;	// 4's multiply
	bmih->biSizeImage = sw * h;
	offset = bmfh->bfOffBits + bmih->biSizeImage;
	bmpcSetSize(bmp, w, h);
	d = bmp->buffer;
	size = sizeof(RGBQUAD) * 256;
	rval = fread(rgbqs, 1, size, fp);
	if(rval != size) { fclose(fp); return -3; }
	// palette
	//   0     1      32       64         92          128         160         192      193   194  ...  255
	// 0-0-0 1-1-1 37-37-37 79-79-79  113-113-113 170-170-170 222-222-222 255-255-255 0-0-0 0-0-0 ... 0-0-0
	c = color & 0x00ffffff;
	alpha = color >> 24;
	for(i = 0;i < h;i++) {
		offset -= sw;
		fseek(fp, offset, SEEK_SET);
		rval = fread(_LineBuffer, 1, w, fp);
		if(rval != w) { fclose(fp); return -2; }
		for(j = 0, s = _LineBuffer;j < w;j++) {
			a = *s++;
			a = rgbqs[a].rgbGreen ^ 0xff;
			*d++ = ((a * alpha) << 24) | c;
		}
	}
	fclose(fp);
	return 0;
}

void bmpcReadBitMask(BMPC *bmp, BMPC *bmpMask, int stride, int color)
{
	unsigned char	*s, *s0;
	unsigned long	*d;
	int		i, j, w, h, c, m, sw;

	w = bmpMask->width; h = bmpMask->height;
	bmpcSetSize(bmp, w, h);
	d = bmp->buffer;
	sw = (stride + 7) >> 3;
	s0 = (unsigned char *)bmpMask->buffer;
	color |= 0xff000000;
	for(i = 0;i < h;i++) {
		s = s0; m = 0;
		for(j = 0;j < w;j++) {
			if(!m) {
				m = 0x80; c = *s++;
			}
			if(c & m) *d = color;
			else	*d = 0;
			d++; m >>= 1;
		}
		s0 += sw;
	}
}

#ifdef _FREETYPE_

#include <ft2build.h>
#include FT_FREETYPE_H
extern FT_Face		ftFace;

int bmpcPutText(BMPC *bmp, char *string, unsigned long color)
{
	char	*s;
	unsigned char	*p, *p0;
	unsigned long	*d, *d0;
	int		c, i, j, x, y, w, h, dw;

	if(!string || !string[0]) {
		bmpcSetSize(bmp, 0, 0);
		x = 0;
	} else {
		w = ftTextPixelWidth(string);
		h = ftFace->size->metrics.height >> 6;
		bmpcSetSize(bmp, w, h);
		bmpcFillRect(bmp, 0, 0, w, h, 0);
		color &= 0xffffff;
		x = 0;
		y = ftFace->size->metrics.ascender >> 6;
		d0 = (unsigned long *)bmp->buffer;
		s = string;
		while(*s) {
			c = *s++;
			if(c & 0x80) {
				c <<= 8; c |= *s++;
				c = wan2unicode(c);
			}
			c = FT_Load_Char(ftFace, c, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);
			w = ftFace->glyph->bitmap.width; h = ftFace->glyph->bitmap.rows;
//printf("x=%d %d,%d w=%d h=%d adv=%d\n", x >> 6, (int)ftFace->glyph->bitmap_left, (int)ftFace->glyph->bitmap_top, w, h, ftFace->glyph->advance.x>>6);
			d = d0 + (x >> 6);
			d += ftFace->glyph->bitmap_left;
			c = y - ftFace->glyph->bitmap_top;
			d += c * bmp->width;
			p0 = ftFace->glyph->bitmap.buffer; 
			dw = bmp->width - w;
			for(i = 0;i < h;i++) {
				p = p0;
				for(j = 0;j < w;j++) {
					c = *p++;
					*d++ = (c << 24) | color;
				}
				p0 += ftFace->glyph->bitmap.pitch;
				d += dw;

			}
			x += ftFace->glyph->advance.x;
		}
	}
	return (x+63)>>6;
}

#else

int  _FtTextPixelWidth64(char *string);
int  _FtGetEngFont(int c, CT_FONT *font, unsigned char *buf);
int  _FtGetHanFont(int c, CT_FONT *font, unsigned char *buf);
unsigned char	fontbuf[4000];

int bmpcPutText(BMPC *bmp, char *string, unsigned long color)
{
	CT_FONT		*font, _font;
	char	*s;
	unsigned char	*p;
	unsigned long	*d, *d0;
	int		c, i, j, x, y, w, h, dw, rval;

	font = &_font;
	if(!string || !string[0]) {
		bmpcSetSize(bmp, 0, 0);
		x = 0;
	} else {
		w = _FtTextPixelWidth64(string);
		h = 74;
		bmpcSetSize(bmp, w, h);
		bmpcFillRect(bmp, 0, 0, w, h, 0);
		color &= 0xffffff;
		x = 0;
		y = 55;		// ascedent
		d0 = (unsigned long *)bmp->buffer;
		s = string;
		while(*s) {
			c = *s++;
			if(c & 0x80) {
				c <<= 8; c |= *s++;
				rval = _FtGetHanFont(c, font, fontbuf);
			} else {
				rval = _FtGetEngFont(c, font, fontbuf);
			}
			if(rval > 0) {
				w = font->width; h = font->height;
				d = d0 + font->x;
				c = y - font->y;
				d += c * bmp->width;
				p = fontbuf;
				dw = bmp->width - w;
				for(i = 0;i < h;i++) {
					for(j = 0;j < w;j++) {
						c = *p++;
						*d++ = (c << 24) | color;
					}
					d += dw;
				}
			}
			x += font->pitch;
			d0 += font->pitch;
		}
	}
	return x;
}

#endif

//#define	YCbCr2R(Y,Cb,Cr)	(int)(Y + 1.402*(Cr-128) + 0.5)
//#define YCbCr2G(Y,Cb,Cr)	(int)(Y - 0.34414*(Cb-128) - 0.71414*(Cr-128) + 0.5)
//#define	YCbCr2B(Y,Cb,Cr)	(int)(Y + 1.772*(Cb-128) + 0.5)

// 1.402*(Cr-128) + 0.5
short R_Cr_Tbl[256] = {
-178,-177,-176,-174,-173,-171,-170,-169,-167,-166,-164,-163,-162,-160,-159,-157,
-156,-155,-153,-152,-150,-149,-148,-146,-145,-143,-142,-141,-139,-138,-136,-135,
-134,-132,-131,-129,-128,-127,-125,-124,-122,-121,-120,-118,-117,-115,-114,-113,
-111,-110,-108,-107,-106,-104,-103,-101,-100, -99, -97, -96, -94, -93, -92, -90,
 -89, -87, -86, -85, -83, -82, -80, -79, -78, -76, -75, -73, -72, -71, -69, -68,
 -66, -65, -63, -62, -61, -59, -58, -56, -55, -54, -52, -51, -49, -48, -47, -45,
 -44, -42, -41, -40, -38, -37, -35, -34, -33, -31, -30, -28, -27, -26, -24, -23,
 -21, -20, -19, -17, -16, -14, -13, -12, -10,  -9,  -7,  -6,  -5,  -3,  -2,   0,
   0,   1,   3,   4,   6,   7,   8,  10,  11,  13,  14,  15,  17,  18,  20,  21,
  22,  24,  25,  27,  28,  29,  31,  32,  34,  35,  36,  38,  39,  41,  42,  43,
  45,  46,  48,  49,  50,  52,  53,  55,  56,  57,  59,  60,  62,  63,  64,  66,
  67,  69,  70,  72,  73,  74,  76,  77,  79,  80,  81,  83,  84,  86,  87,  88,
  90,  91,  93,  94,  95,  97,  98, 100, 101, 102, 104, 105, 107, 108, 109, 111,
 112, 114, 115, 116, 118, 119, 121, 122, 123, 125, 126, 128, 129, 130, 132, 133,
 135, 136, 137, 139, 140, 142, 143, 144, 146, 147, 149, 150, 151, 153, 154, 156,
 157, 158, 160, 161, 163, 164, 165, 167, 168, 170, 171, 172, 174, 175, 177, 178
};

// -0.34414*(Cb-128) + 0.5
short G_Cb_Tbl[256] = {
 44, 44, 43, 43, 43, 42, 42, 42, 41, 41, 41, 40, 40, 40, 39, 39,
 39, 38, 38, 38, 37, 37, 36, 36, 36, 35, 35, 35, 34, 34, 34, 33,
 33, 33, 32, 32, 32, 31, 31, 31, 30, 30, 30, 29, 29, 29, 28, 28,
 28, 27, 27, 26, 26, 26, 25, 25, 25, 24, 24, 24, 23, 23, 23, 22,
 22, 22, 21, 21, 21, 20, 20, 20, 19, 19, 19, 18, 18, 18, 17, 17,
 17, 16, 16, 15, 15, 15, 14, 14, 14, 13, 13, 13, 12, 12, 12, 11,
 11, 11, 10, 10, 10,  9,  9,  9,  8,  8,  8,  7,  7,  7,  6,  6,
  6,  5,  5,  4,  4,  4,  3,  3,  3,  2,  2,  2,  1,  1,  1,  0,
  0,  0,  0,  0,  0, -1, -1, -1, -2, -2, -2, -3, -3, -3, -4, -4,
 -5, -5, -5, -6, -6, -6, -7, -7, -7, -8, -8, -8, -9, -9, -9,-10,
-10,-10,-11,-11,-11,-12,-12,-12,-13,-13,-13,-14,-14,-14,-15,-15,
-16,-16,-16,-17,-17,-17,-18,-18,-18,-19,-19,-19,-20,-20,-20,-21,
-21,-21,-22,-22,-22,-23,-23,-23,-24,-24,-24,-25,-25,-25,-26,-26,
-27,-27,-27,-28,-28,-28,-29,-29,-29,-30,-30,-30,-31,-31,-31,-32,
-32,-32,-33,-33,-33,-34,-34,-34,-35,-35,-35,-36,-36,-37,-37,-37,
-38,-38,-38,-39,-39,-39,-40,-40,-40,-41,-41,-41,-42,-42,-42,-43
};

// -0.71414*(Cr-128) + 0.5
short G_Cr_Tbl[256] = {
 91, 91, 90, 89, 89, 88, 87, 86, 86, 85, 84, 84, 83, 82, 81, 81,
 80, 79, 79, 78, 77, 76, 76, 75, 74, 74, 73, 72, 71, 71, 70, 69,
 69, 68, 67, 66, 66, 65, 64, 64, 63, 62, 61, 61, 60, 59, 59, 58,
 57, 56, 56, 55, 54, 54, 53, 52, 51, 51, 50, 49, 49, 48, 47, 46,
 46, 45, 44, 44, 43, 42, 41, 41, 40, 39, 39, 38, 37, 36, 36, 35,
 34, 34, 33, 32, 31, 31, 30, 29, 29, 28, 27, 26, 26, 25, 24, 24,
 23, 22, 21, 21, 20, 19, 19, 18, 17, 16, 16, 15, 14, 14, 13, 12,
 11, 11, 10,  9,  9,  8,  7,  6,  6,  5,  4,  4,  3,  2,  1,  1,
  0,  0,  0, -1, -2, -3, -3, -4, -5, -5, -6, -7, -8, -8, -9,-10,
-10,-11,-12,-13,-13,-14,-15,-15,-16,-17,-18,-18,-19,-20,-20,-21,
-22,-23,-23,-24,-25,-25,-26,-27,-28,-28,-29,-30,-30,-31,-32,-33,
-33,-34,-35,-35,-36,-37,-38,-38,-39,-40,-40,-41,-42,-43,-43,-44,
-45,-45,-46,-47,-48,-48,-49,-50,-50,-51,-52,-53,-53,-54,-55,-55,
-56,-57,-58,-58,-59,-60,-60,-61,-62,-63,-63,-64,-65,-65,-66,-67,
-68,-68,-69,-70,-70,-71,-72,-73,-73,-74,-75,-75,-76,-77,-78,-78,
-79,-80,-80,-81,-82,-83,-83,-84,-85,-85,-86,-87,-88,-88,-89,-90
};

// 1.772*(Cb-128) + 0.5
short B_Cb_Tbl[256] = {
-226,-224,-222,-221,-219,-217,-215,-213,-212,-210,-208,-206,-205,-203,-201,-199,
-197,-196,-194,-192,-190,-189,-187,-185,-183,-182,-180,-178,-176,-174,-173,-171,
-169,-167,-166,-164,-162,-160,-158,-157,-155,-153,-151,-150,-148,-146,-144,-143,
-141,-139,-137,-135,-134,-132,-130,-128,-127,-125,-123,-121,-119,-118,-116,-114,
-112,-111,-109,-107,-105,-104,-102,-100, -98, -96, -95, -93, -91, -89, -88, -86,
 -84, -82, -81, -79, -77, -75, -73, -72, -70, -68, -66, -65, -63, -61, -59, -57,
 -56, -54, -52, -50, -49, -47, -45, -43, -42, -40, -38, -36, -34, -33, -31, -29,
 -27, -26, -24, -22, -20, -18, -17, -15, -13, -11, -10,  -8,  -6,  -4,  -3,  -1,
   0,   2,   4,   5,   7,   9,  11,  12,  14,  16,  18,  19,  21,  23,  25,  27,
  28,  30,  32,  34,  35,  37,  39,  41,  43,  44,  46,  48,  50,  51,  53,  55,
  57,  58,  60,  62,  64,  66,  67,  69,  71,  73,  74,  76,  78,  80,  82,  83,
  85,  87,  89,  90,  92,  94,  96,  97,  99, 101, 103, 105, 106, 108, 110, 112,
 113, 115, 117, 119, 120, 122, 124, 126, 128, 129, 131, 133, 135, 136, 138, 140,
 142, 144, 145, 147, 149, 151, 152, 154, 156, 158, 159, 161, 163, 165, 167, 168,
 170, 172, 174, 175, 177, 179, 181, 183, 184, 186, 188, 190, 191, 193, 195, 197,
 198, 200, 202, 204, 206, 207, 209, 211, 213, 214, 216, 218, 220, 222, 223, 225
};

static unsigned long _RGB(int r, int g, int b)
{
	unsigned long	c;

	if(r > 255) r = 255; else if(r < 0) r = 0;
	if(g > 255) g = 255; else if(g < 0) g = 0;
	if(b > 255) b = 255; else if(b < 0) b = 0;
	c = 0xff000000 | (r << 16) | (g << 8) | b;
	return c;
}

void bmpcPutYUV420(BMPC *bmp, unsigned char *YUVBuffer)
{
	unsigned char	*y1, *y2, *cb, *cr;
	unsigned long	*d1, *d2;
	int		i, j, w, h, yw, cw, r, g, b, Y, CB, CR;

	w = ((bmp->width+15) >> 4) << 4; h = ((bmp->height+15) >> 4) << 4;
	i = w * h;
	y1 = YUVBuffer; y2 = y1 + w; cb = y1 + i; cr = cb + (i >> 2);
	cw = w - bmp->width;
	yw = w + cw;
	cw >>= 1;
	w = bmp->width >> 1; h = bmp->height >> 1;
	d1 = bmp->buffer; d2 = d1 + w + w;
	for(i = 0;i < h;i++) {
		for(j = 0;j < w;j++) {
			CB = *cb++; CR = *cr++;
			r = R_Cr_Tbl[CR];
			g = G_Cb_Tbl[CB] + G_Cr_Tbl[CR];
			b = B_Cb_Tbl[CB];
			Y = *y1++;
			*d1++ = _RGB(Y+r, Y+g, Y+b);
			Y = *y1++;
			*d1++ = _RGB(Y+r, Y+g, Y+b);
			Y = *y2++;
			*d2++ = _RGB(Y+r, Y+g, Y+b);
			Y = *y2++;
			*d2++ = _RGB(Y+r, Y+g, Y+b);
		}
		y1 += yw; y2 += yw; cb += cw; cr += cw;
		d1 += w + w; d2 += w + w;
	}
}

