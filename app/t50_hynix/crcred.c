#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "asciidefs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
//#include "wglib.h"
#include "user.h"
#include "msg.h"
#include "cf.h"
#include "cfprim.h"
#include "dev.h"
#include "subdev.h"
#include "ad.h"
#include "cr.h"
#include "crcred.h"


void crPrintCred(CREDENTIAL_READER *cr)
{
	int		i;

	printf("[%02x", (int)cr->data[0]);
	for(i = 1;i < 9;i++) printf("-%02x", (int)cr->data[i]);
	printf("]\n");
}

void bin2card(unsigned char *data, int len, unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x10 + len;
	memcpy(p, data, len); p += len;
	len = buf + 9 - p;
	if(len > 0) memset(p, 0x00, len);
}

// len: nibble length
void bcd2card(unsigned char *data, int len, unsigned char *buf)
{
	unsigned char	*p;
	int		c, val;

	p = buf;
	*p++ = 0x20;
	val = len >> 1;
	memcpy(p, data, val); p += val;
	if(len & 1) {
		c = data[val]; c |= 0x0f;
		*p++ = c; val++;
	}
	val = buf + 9 - p;
	if(val > 0) memset(p, 0xff, val);
}

void digits2card(char *data, unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x20;
	string2bcd(data, 16, p);
}

void digitslen2card(char *data, int len, unsigned char *buf)
{
	unsigned char	*p,  tmpbuf[20];
	
	memcpy(tmpbuf, data, len); tmpbuf[len]  = 0;
	p = buf;
	*p++ = 0x20;
	string2bcd(tmpbuf, 16, p);
}

void str2card(char *data, unsigned char *buf)
{
	unsigned char	*p;
	int		len;

	len = strlen(data);
	p = buf;
	*p++ = 0x30;	
	memcpy(p, data, len); p += len;
	len = buf + 9 - p;
	if(len > 0) memset(p, ' ', len);
}

void strlen2card(char *data, int len, unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x30;	
	memcpy(p, data, len); p += len;
	len = buf + 9 - p;
	if(len > 0) memset(p, ' ', len);
}

int cardisdigits(unsigned char *data)
{
	unsigned char	*p;
	int		i, c, n, cnt;
	
	p = data + 1;
	for(i = cnt = 0;i < 8;i++) {
		c = p[i];
		n = c >> 4;
		if(n >= 0 && n <= 9) cnt++;
		else if(n == 0xf) break;
		else	return -1;
		n = c & 0x0f;
		if(n >= 0 && n <= 9) cnt++;
		else if(n == 0xf) break;
		else	return -1;
	} 
	return cnt;
}

void crCapturedUserId(CREDENTIAL_READER *cr, long userId)
{
	cr->credType = CREDENTIAL_ID;
	if(devMode(NULL)) {
		IDtoPACK3(userId, cr->data);
		cr->dataLength = 3;
	} else {
		userEncodeId(userId, cr->data);
		cr->dataLength = 4;
	}
}

void crCapturedPin(CREDENTIAL_READER *cr, char *pin)
{
	int		len;

	cr->credType = CREDENTIAL_PIN;
	len = strlen(pin);
	string2bcd(pin, 8, cr->data);
}

int crCapturedCard(CREDENTIAL_READER *cr, unsigned char *data, int size)
{
	CF_CARD		*cf;
	CF_WIEGAND	*wf;
	unsigned char	*s, *d, temp[80];
	int		rval, len, count, cfId, type, fmt;

	s = data; d = cr->data;
printf("[%02x", (int)s[0]); for(len=1;len<size;len++) printf("-%02x", (int)s[len]); printf("]\n");
	fmt = *s++; size--;
    cr->readerFormat = fmt;
	type = fmt >> 4; fmt &= 0x0f;
	cr->cardType = type;
	cr->credType = CREDENTIAL_CARD;
	count = 0;
	switch(type) {
	case 0:			// SC Reader
		cfId = crScFormat(NULL);
		cf = cfCardsGet(cfId);
//printf("fmt=%d cfId=%d size=%d\n", fmt, cfId, size);
		if(fmt == cfId) {
			while(size > 0 && size > s[0]) {
				len = *s++;
				rval = cfCardDecodeCard(cf, s, len, d);
				if(rval) {
					d += 9; count++;
				}
				s += len; size -= len + 1;
			}
		} else if(cfId == 1 && fmt == 2) {  // cfCardCardApp(cf);
			while(size > 0 && size > s[0]) {
				len = *s++;
				rval = cfCardDecodeCard(cf, s, 8, d);
				if(rval) {
					d += 9; count++;
				}
				s += len; size -= len + 1;
			}
		}
		break;
	case 1:			// EM Reader		
		cf = cfCardsGet(CFCARD_EM);
		while(size > 0 && size > s[0]) {
			len = *s++;
			rval = cfCardDecodeCard(cf, s, len, d);
			if(rval) {
				d += 9; count++;
			}
			s += len; size -= len + 1;
		}
		break;
	case 2:			// Serial
		cf = cfCardsGet(CFCARD_SERIAL);
		len = *s++;
		rval = cfCardDecodeCard(cf, s, len, d);
		if(rval) {
			d += 9; count++;
		}
		break;
	case 4:			// Wiegand
#ifdef CARD_DEBUG
printf("wiegand: %d [%02x", (int)s[0], (int)s[1]); for(len = 1;len < size-1;len++) printf("-%02x", (int)s[len]); printf("]\n");
#endif
		len = *s++;
		wf = crMatchWiegandFormat(cr, len);
		if(wf) {
			if(cfWiegandId(wf) == 1) rval = wgDecode35Bits(s, len, temp);
			else if(cfWiegandParity(wf)) rval = wgDecode(s, len, temp);
			else {
				memcpy(temp, s, (len+7)>>3); rval = len; 
			}
			if(rval) {
				rval = cfWiegandDecodeCard(wf, data, rval, d);
				if(rval) {
					d += 9; count++;
				}
			}
		}
		break;
	}
if(count) {
printf("[%02x", (int)cr->data[0]); for(len=1;len<9;len++) printf("-%02x", (int)cr->data[len]); printf("]\n");
} else	printf("cardCapture: fail\n");
	cr->dataLength  = count * 9;;
	return count;
}

void cardClear(CREDENTIAL_READER *cr)
{
	cr->dataLength = 0;
}

int cardPeek(CREDENTIAL_READER *cr)
{
	return (int)cr->dataLength;
}

void crClearCred(void *self)
{
	CREDENTIAL_READER	*cr = self;

	cr->credType = 0xff;
}

int crVerifyMatchCard(CREDENTIAL_READER *cr, unsigned char *cardData)
{
	unsigned char	*p;
	int		index, size;

	size = cr->dataLength;
	p = cr->data;
	index = 0;
	while(size >= 9) {
		if(!memcmp(cardData, p, 9)) break;
		index++; p += 9; size -= 9;
	}
	if(size < 9) index = 0;
	else {
		if(index) memcpy(cr->data, cr->data+9*index, 9);	
		index = 1;
	} 
	return index;
}

int crVerifyMatchPin(CREDENTIAL_READER *cr, unsigned char *pin)
{
	int		rval;
	
//printf("id=%u pin=%02x%02x%02x%02x\n", user->id, (int)user->pin[0], (int)user->pin[1], (int)user->pin[2], (int)user->pin[3]);
//printf("cr->data=%02x%02x%02x%02x\n", (int)cr->data[0], (int)cr->data[1], (int)cr->data[2], (int)cr->data[3]);
	if(!memcmp(cr->data, pin, 4)) rval = 1;
	else	rval = 0;
	return rval;
}

#include "user.h"
#include "fsuser.h"

int crIdentifyMatchCard(CREDENTIAL_READER *cr, void *userp)
{
	FP_USER		*user;
	unsigned char	*p;
	int		rval, index, size;

	user = userp;
	size = cr->dataLength;
	p = cr->data;
	rval = index = 0;
	while(size >= 9) {
		userfsSeek(0L);
		rval = userfsGetCardData(user, p);
		if(rval) break;
		index++; p += 9; size -= 9;
	}
	if(rval > 0 && index) memcpy(cr->data, cr->data+9*index, 9);	
	return rval;
}


/*

int _CardReadSerialRF900(CREDENTIAL_READER *cr, unsigned char *buf, int len)
{
	int		rval;
		
//#ifdef CARD_DEBUG
printf("Serial RF900: %d [", len); for(rval = 0;rval < len;rval++) printf("%02x", (int)buf[rval]); printf("]\n");
//#endif
	return 0;
}

int _CardReadSerialBarCode(CREDENTIAL_READER *cr, unsigned char *buf, int len)
{
	unsigned char	*p;
	int		rval, dataType, dataPos, dataLen;

#ifdef CARD_DEBUG
printf("Serial BarCode: %d [", len); for(rval = 0;rval < len;rval++) printf("%02x", (int)buf[rval]); printf("]\n");
#endif
	dataType = sys_cfg->extData[1];	dataPos = sys_cfg->extData[2]; dataLen = sys_cfg->extData[3]; 
	len = buf[0]; p = buf + 1;
	if(sys_cfg->extData[3] == 0 || len < dataPos+dataLen) rval = 0;
	else {
		p += dataPos;
		if(dataType == 3) {			// BCD code
			dataLen <<= 1;
			bcd2card(p, dataLen, cr->data);
		} else if(dataType == 2) {	// Binary
			bin2card(p, dataLen, cr->data);	
		} else {					// Digits
			p[dataLen] = 0;
			digits2card((char *)p, cr->data);
		}
		if(dataType != 2) {
			rval = cardisdigits(cr->data);
printf("%d %d [", dataLen, rval); for(dataPos = 0;dataPos < 9;dataPos++) printf("%02x", (int)cr->data[dataPos]); printf("]\n");
			if(rval == dataLen) rval = 1;
			else	rval = 0;
		} else	rval = 1;
	}
	return rval;
}

int _CardReadSerialEM(CREDENTIAL_READER *cr, unsigned char *buf, int len)
{
	unsigned char	checksum;
	int		i, rval, dataType;

//#ifdef CARD_DEBUG
printf("Serial EM: %d [", len); for(rval = 0;rval < len;rval++) printf("%02x", (int)buf[rval]); printf("]\n");
//#endif
	rval = 0;
	if(len == 9 && buf[0] == ASCII_STX && buf[8] == ASCII_ETX && buf[1] == 9) {
		checksum = 0;
		for(i = 0;i < 5;i++) checksum += buf[i+2];
		if(checksum == buf[7]) rval = 1;	
	}
	if(rval) {
		dataType = (cr->SCDataType >> 4) & 0x0f;
		if(dataType == 3) {
			buf[4] &= 0x0f;
			bin2card(buf+4, 3, cr->data);
		} else if(dataType == 2) {
			bin2card(buf+4, 3, cr->data);
		} else {
			bin2card(buf+3, 4, cr->data);
		}
		rval = 9;
	}
	return rval;
}

int _CardReadSerialHID(CREDENTIAL_READER *cr, unsigned char *buf, int len)
{
	int		rval;

//#ifdef CARD_DEBUG
printf("Serial HID: %d [", len); for(rval = 0;rval < len;rval++) printf("%02x", (int)buf[rval]); printf("]\n");
//#endif
	if(len == 18) {
		buf[0] = (buf[14] << 6) | (buf[15] >> 2);
		buf[1] = (buf[15] << 6) | (buf[16] >> 2);
		buf[2] = (buf[16] << 6) | (buf[17] >> 2);
		buf[3] = buf[17] << 6;
		rval = wgCheck26Data(buf);
		if(rval == 26) {
			bin2card(buf, 3, cr->data);
			rval = 9;
		} else	rval = 0;
	} else	rval = 0;
	return rval;
}
*/
