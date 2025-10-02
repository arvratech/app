#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef _FREETYPE_
#include <ft2build.h>
#include FT_FREETYPE_H
#else
#include "ctfont_eng_offset.h"
#include "ctfont_han_offset.h"
#endif
#include "lcdc.h"
#include "ctfont.h"

#ifdef _FREETYPE_
FT_Library	ftLibrary;
FT_Face		ftFace;
#else
static FILE		*fpEng, *fpHan;
#endif
static int		_ftSize, _ftPixelSize;


int ftInit(char *fontPath)
{
	char	path[128];
	int		rval;

#ifdef _FREETYPE_
	rval = FT_Init_FreeType(&ftLibrary);
	if(rval) {
printf("FT_Init_FreeType: error=%d\n", rval);
		return -1;
	}
	rval = FT_New_Face(ftLibrary, fontPath, 0, &ftFace);
	if(rval) {
		if(rval == FT_Err_Unknown_File_Format) {
			printf("FT_New_face: font format is unsupported\n");
		} else {
			printf("FT_New_face: error=%d\n", rval);
		}
		return -1;
	}
printf("FT_New_face: %s is loaded: fixed_sizes=%d\n", ftFace->family_name, ftFace->num_fixed_sizes);
#else
	sprintf(path, "%s/ctfont_eng", fontPath);
	fpEng = fopen(path, "r");
	if(!fpEng) printf("ftInit: ctfont_eng error: %s\n",  strerror(errno));
	sprintf(path, "%s/ctfont_han", fontPath);
	fpHan = fopen(path, "r");
	if(!fpHan) printf("ftInit: ctfont_han error: %s\n",  strerror(errno));
	if(fpEng && fpHan) rval = 0;
	else	rval = 0;
#endif
	return rval;
}

void ftExit(void)
{
#ifdef _FREETYPE_
	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLibrary);
#else
	if(fpEng) {
		fclose(fpEng); fpEng = NULL;
	}
	if(fpHan) {
		fclose(fpHan); fpHan = NULL;
	}
#endif
}

// fixed point multiply: (a x b) >> 8
//             divide  : (a >> 8) / b 
void ftSetSize(int point)
{
#ifdef _FREETYPE_
	FT_Size_Metrics	*mtx;
	int		pt, rval;
#endif

	_ftSize = point;
#ifdef _FREETYPE_
	pt = ((point * _lcdNumerator) << 6) / _lcdDenominator;
	rval = FT_Set_Char_Size(ftFace, pt, 0, 72, 72);		// pt: 26.6 fixed point
	mtx = &ftFace->size->metrics;
//printf("ftSetSize: size=%d %d,%d asc=%d desc=%d h=%d\n", point, (int)mtx->x_ppem, (int)mtx->y_ppem,
//									(int)mtx->ascender>>6, (int)mtx->descender>>6, (int)mtx->height>>6);
#else
	_ftPixelSize = point * _lcdNumerator / _lcdDenominator;
#endif
}

int ftSize(void)
{
	return _ftSize;
}

int ftPixelSize(void)
{
#ifdef _FREETYPE_
	return (int)ftFace->size->metrics.x_ppem;
#else
	return _ftPixelSize;
#endif
}

// ftfont_eng, ftfont_han extracted from NanumBarunGothic.ttf
// FT_Set_Char_Size(ftFace, 64 << 6, 0, 72, 72);
// ftFace->size->metrics.xppem = 64
// ftFace->size->metrics.yppem = 64
// ftFace->size->metrics.ascendent = 55
// ftFace->size->metrics.descendent = -20
// ftFace->size->metrics.height = 74
//
// ftFace->glyph
// maximum bitmap_top bitmap.rows-bitmap_top  bitmap.rows
//   eng:      52                19                67    
//   han:      52                11                62    
// advance.x 
//   eng: 16..67
//   han: 57

int ftPixelHeight(void)
{
#ifdef _FREETYPE_
	return (int)(ftFace->size->metrics.height >> 6);
#else
	return (74 * _ftPixelSize + 63) >> 6;
#endif
}

int ftGetPixelHeight(int size)
{
	return (74 * size + 63) >> 6;
}

int ftHeight(void)
{
	int		h;

#ifdef _FREETYPE_
	h = ftFace->size->metrics.height >> 6;
	h = _reverseScaleCeil(h);
#else
	h = (74 * _ftPixelSize + 63) >> 6;
	h = _reverseScaleCeil(h);
#endif
	return h;
}

#ifdef _FREETYPE_
int wan2unicode(int c);
#endif

int ftTextPixelWidth(char *string)
{
	char	*s;
	int		c, w, rval;

	w = 0;
	s = string;
#ifdef _FREETYPE_
	while(*s) {
		c = *s++;
		if(c == '\n') break;
		if(c & 0x80) {
			c <<= 8; c |= *s++;
			c = wan2unicode(c);
		}
		rval = FT_Load_Char(ftFace, c, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);
		w += ftFace->glyph->advance.x;
	}
	return (w+63)>>6;
#else
	while(*s) {
		c = *s++;
		if(c == '\n') break;
		if(c & 0x80) {
			c <<= 8; c |= *s++;
			w += 57;
		} else {
			if(c == ' ') w += 14;
			else	w += ctfont_eng_pitch[c-0x21];
		}
	}
	return (w * _ftPixelSize + 63) >> 6;
#endif
}

#ifndef _FREETYPE_

int _FtTextPixelWidth64(char *string)
{
	char	*s;
	int		c, w, rval;

	w = 0;
	s = string;
	while(*s) {
		c = *s++;
		if(c == '\n') break;
		if(c & 0x80) {
			c <<= 8; c |= *s++;
			w += 57;
		} else {
			if(c == ' ') w += 14;
			else	w += ctfont_eng_pitch[c-0x21];
		}
	}
	return w;
}

int ftGetTextPixelWidth(int size, char *string)
{
	int		w;

	w = _FtTextPixelWidth64(string);
	return (w * size + 63) >> 6;
}

#endif

int ftTextWidth(char *string)
{
	int		w;

	w = ftTextPixelWidth(string);
	w = _reverseScaleCeil(w);
	return w;
}

int ftTextPixelPosition(char *string, int textPosition)
{
	char	*s;
	int		c, w, len, rval;

	w = len = 0;
	s = string;
#ifdef _FREETYPE_
	while((*s) && len < textPosition) {
		c = *s++; len++;
		if(c & 0x80) {
			c <<= 8; c |= *s++; len++;
			c = wan2unicode(c);
		}
		rval = FT_Load_Char(ftFace, c, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);
		w += ftFace->glyph->advance.x;
	}
	return w >> 6;
#else
	while((*s) && len < textPosition) {
		c = *s++; len++;
		if(c & 0x80) {
			c <<= 8; c |= *s++; len++;
			w += 57;
		} else {
			if(c == ' ') w += 14;
			else	w += ctfont_eng_pitch[c-0x21];
		}
	}
	return (w * _ftPixelSize + 63) >> 6;
#endif
}

// return textPosition from touched position 
int ftTextPositionFromTouched(char *string, int pixelPosition, int touchPosition)
{
	char	*s, *s0;
	int		c, x, w, dw, val, pixeldx;

#ifdef _FREETYPE_
	pixeldx = _scale24_6(touchPosition) - (pixelPosition << 6);
	w = 0; s = s0 = string;
	while(*s) {
		c = *s++;
		if(c & 0x80) {
			c <<= 8; c |= *s++;
			c = wan2unicode(c);
		}
		val = FT_Load_Char(ftFace, c, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);
		dw = ftFace->glyph->advance.x;
		if(pixeldx < w+dw) break;
		s0 = s;
		w += dw;
	}
	return pixelPosition + (w >> 6);
#else
	pixeldx = _scale(touchPosition) - pixelPosition;
	w = 0; s = s0 = string;
	while(*s) {
		c = *s++;
		if(c & 0x80) {
			c <<= 8; c |= *s++;
			dw = 57;
		} else {
			if(c == ' ') dw = 14;
			else	dw = ctfont_eng_pitch[c-0x21];
		}
		val = ((w+dw) * _ftPixelSize + 63) >> 6;
		if(pixeldx < val) break;
		s0 = s;
		w += dw;
	}
#endif
	w = s0 - string;
//printf("Focused: [%s] touched=%d position=%d\n", string, touchPosition, w);
	return w;
}

#ifndef _FREETYPE_

int _FtGetEngFont(int c, CT_FONT *font, unsigned char *buf)
{
	int		idx, size;

	if(c == ' ') { 
		font->pitch = 14; 
		font->width = font->height = 0;
		size = 0;
	} else {
		idx = c - 0x21;
		fseek(fpEng, ctfont_eng_offset[idx], SEEK_SET);
		fread(font, 1, 4, fpEng);
		size = font->width * font->height;
		if(size >= 4000) {
			printf("##### font file error: code=%02x #####\n", c);
			printf("%d,%d %d %d\n", (int)font->x, (int)font->y, (int)font->width, (int)font->height);
			size = 0;
		} else {
			fread(buf, 1, size, fpEng);
		}
		font->pitch = ctfont_eng_pitch[idx]; 
	}
	return size;
}

int _FtGetHanFont(int c, CT_FONT *font, unsigned char *buf)
{
	int		c1, c2, size;

	c1 = (c >> 8) - 0xb0; c2 = (c & 0xff) - 0xa1;
	fseek(fpHan, ctfont_han_offset[c1][c2], SEEK_SET);
	fread(font, 1, 4, fpHan);
	size = font->width * font->height;
	if(size >= 4000) {
		printf("##### font file error: code=%04x #####\n", c);
		printf("%d,%d %d %d\n", (int)font->x, (int)font->y, (int)font->width, (int)font->height);
		size = 0;
	} else {
		fread(buf, 1, size, fpHan);
	}
	font->pitch = 57;
	return size;
}

#endif

