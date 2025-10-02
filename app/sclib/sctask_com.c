#include <stdio.h>
#include <string.h>
#include "rtc.h"
#include "log.h"
#include "sctask_com.h"
#include "sccfg.h"
#include "sctask.h"

unsigned char  scOpenMode, scState, scSubState, scCount, scReaderFormat, scCardType, scCardApp;
static void (*_ScOnDataDone)(unsigned char *, int);

static unsigned char  cds_pend[MAX_CARD_SIZE][20];
static unsigned char  uids[8][8];


void scInitData(void)
{
	scClearData();
}	

void scClearData(void)
{
	int		i;

	for(i = 0;i < MAX_CARD_SIZE;i++) cds_pend[i][1] = 0;
	for(i = 0;i < 8;i++) uids[i][0] = 0;
}

void scAddUid(unsigned char *uid)
{
	int		i;

	for(i = 0;i < 8;i++) {
		if(!uids[i][0]) break;
	}
	if(i < 8) memcpy(uids[i], uid, uid[0] + 1); 
}

int scFindUid(unsigned char *uid)
{
	int		i, found;

	found = 0;
	for(i = 0;i < 8;i++) {
		if(!uids[i][0]) break;
		if(!memcmp(uids[i], uid, uid[0] + 1)) {
			found = 1;
			break;
		}
	}
	return found;
}

void scSetOnDataDone(void (*onDataDone)(unsigned char *, int))
{
	_ScOnDataDone = onDataDone;
}

void scAddData(int format, unsigned char *data, int len)
{
	unsigned char	*p;
	int		i, found;

	found = 0;
	for(i = 0;i < MAX_CARD_SIZE;i++) {
		p = &cds_pend[i][0];
		if(!p[1]) break;
		if(p[0] == format && p[1] == len && !memcmp(p+2, data, len)) {
			found = 1;
printf("Founded: [%d] %02x %d [%02x-%02x-%02x-%02x...\n", i, (int)p[0], (int)p[1], (int)p[2], (int)p[3], (int)p[4], (int)p[5]);
			break;
		}
	}
	if(!found) {
		if(i < MAX_CARD_SIZE) {
			p[0] = format; p[1] = len; memcpy(p+2, data, len);
printf("Added: [%d] %02x %d [%02x-%02x-%02x-%02x...\n", i, (int)p[0], (int)p[1], (int)p[2], (int)p[3], (int)p[4], (int)p[5]);
		} else {
printf("Overflow: %02x %d [%02x-%02x-%02x-%02x...\n", (int)format, len, (int)data[0], (int)data[1], (int)data[2], (int)data[3]);
		}
	}
}

void scDoneData(void)
{
	unsigned char	*s, *d, buf[256];
	int		i, len, format;

	if(scReaderFormat == 1 && scCardApp == 1) {
		format = 0;
		for(i = 0;i < MAX_CARD_SIZE;i++) {
			s = &cds_pend[i][0];
			if(!s[1]) break;
			if(s[0] > format) format = s[0];
		}
	} else {
		format = cds_pend[0][0];
	}
	d = buf + 5;
	for(i = 0;i < MAX_CARD_SIZE;i++) {
		s = &cds_pend[i][0];
		len = s[1];
		if(!len) break;
		if(s[0] == format) {
			memcpy(d, s+1, len+1); d += len+1;
		}
	}
	if(i) {
		buf[4] = format;
		len = d - buf - 4;
		if(_ScOnDataDone) (*_ScOnDataDone)(buf, len);
	}
	scClearData();
}

int scExistData(void)
{
	return (int)cds_pend[0][1];
}

int scDataIsFull(void)
{
	int		i;

	for(i = 0;i < MAX_CARD_SIZE;i++)
		if(!cds_pend[i][1]) break;
	if(i < MAX_CARD_SIZE) i = 0;
	else	i = 1;
	return i;
}

int scCardPresence(void)
{
	return (int)scCount;
}

int scExtractTransportCardNumber(unsigned char *buf, int len, unsigned char *data)
{
	unsigned char	*p, c;
	int		i, n;

    n = buf[1];
    if(buf[0] != 0x6f || len < n+2) return 0;
	p = buf + 2; i = 0;
	while(i < n) {
		c = p[i];
		if(c == 0xa5) break;
		else if((c & 0x1f) == 0x1f) i += p[i+2] + 3;
		else	i += p[i+1] + 2;
	}
	if(i >= n) return 0;
	p += i;
	n = p[1];
	p += 2; i = 0;
	while(i < n) {
		c = p[i];
		if(c == 0x12) break;
		else if((c & 0x1f) == 0x1f) i += p[i+2] + 3;
		else	i += p[i+1] + 2;
	}
	if(i < n) {
		n = p[i+1]; memcpy(data, p+i+2, n);
	} else	n = 0;
	return n;
}

int scExtractSpecialTransportCardNumber(unsigned char *buf, int len, unsigned char *data)
{
	unsigned char	*p, c;
	int		i, n;

    n = buf[1];
   if(buf[0] != 0x6f || len < n+2) return 0;
	p = buf + 2; i = 0;
	while(i < n) {
		c = p[i];
		if(c == 0xb0) break;
		else if((c & 0x1f) == 0x1f) i += p[i+2] + 3;
		else	i += p[i+1] + 2;
	}
	if(i >= n) return 0;
	p += i;
	n = p[1];
	p += 2;
    if(n >= 12) {
        memcpy(data, p+4, 8); n = 8;
    } else  n = 0;
	return n;
}

