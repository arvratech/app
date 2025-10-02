#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prim.h"
#include "cf.h"


// sys_cfg->cardFormats[MAX_CF_SZ];
//		0: SC reader - UID
//		1: SC reader - Mifare block
//		2: SC reader - ISO7816-4 file
//		3: EM reader
//		4: Wiegand reader
//		5: Serial reader
//		6: IP reader

void cfSetDefault(CARD_FORMAT *cf, int index)
{
	cf->formatType		= 0;	// 0:Binary  1:BCD  2:ASCII
	// cf->reader		0:SC reader 1:EM reader 2:Wiegand reader 3:Serial reader 4:IP reader
	// cf->scReaderMode	0:UID 1:Mifare block 2:ISO7816-4 file
	if(index < 3) {
		cf->reader = 0; cf->scReaderMode = index;
	} else {
		cf->reader = index - 2; cf->scReaderMode = 0;
	}
	cf->startPosition	= 0;
	cf->length			= 1;
	cf->encoding		= 0;	// 0:Binary  1:BCD  2:ASCII
	cf->reverseBits		= 0;
}

int cfEncode(CARD_FORMAT *cf, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cf->formatType;
	*p++ = cf->reader;
	*p++ = cf->scReaderMode;
	*p++ = cf->startPosition;
	*p++ = cf->length;
	*p++ = cf->encoding;
	*p++ = cf->reverseBits;
 	return p - (unsigned char *)buf;
}

int cfDecode(CARD_FORMAT *cf, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cf->formatType		= *p++;
	cf->reader			= *p++;
	cf->scReaderMode	= *p++;
	cf->startPosition	= *p++;
	cf->length			= *p++;
	cf->encoding		= *p++;
	cf->reverseBits		= *p++;
 	return p - (unsigned char *)buf;
}

int cfGetFormatType(CARD_FORMAT *cf)
{
	return (int)cf->formatType;
}

void cfSetFormatType(CARD_FORMAT *cf, int formatType)
{
	cf->formatType = formatType;
}

int  cfGetStartPosition(CARD_FORMAT *cf)
{
	return (int)cf->startPosition;
}

void cfSetStartPosition(CARD_FORMAT *cf, int startPosition)
{
	cf->startPosition = startPosition;
}

int  cfGetLength(CARD_FORMAT *cf)
{
	return (int)cf->length;
}

void cfSetLength(CARD_FORMAT *cf, int length)
{
	cf->length = length;
}

int  cfGetEncoding(CARD_FORMAT *cf)
{
	return (int)cf->encoding;
}

void cfSetEncoding(CARD_FORMAT *cf, int encoding)
{
	cf->encoding = encoding;
}
