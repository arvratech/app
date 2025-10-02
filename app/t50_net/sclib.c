#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "dev.h"
#include "sclib.h"

extern unsigned long	WdtResetTimer;

#define MIFARE_KEY		MIFARE_AUTHENT_A

int		picc_status;


static void _MakeSectorTrail(unsigned char *buf, unsigned char *keys)
{
	unsigned char	*p;
	int		i;
	
	p = buf;
	memcpy(p, keys, 6); p += 6; *p++ = 0xff; *p++ = 0x07; *p++ = 0x80; *p++ = 0x69;
	for(i = 0;i < 6;i++) *p++ = 0xff;
}

/*
int scReadData(int block, unsigned char *buf, int size, unsigned char *uid, unsigned char *SecondaryKey, unsigned char *NewKey)
{
	unsigned char	*p, tmpbuf[20], tmpbuf2[20];
	int		rval, blk, blk_max, cnt, key_sector;
	
	blk = block;
	if(SecondaryKey && NewKey) _MakeSectorTrail(tmpbuf, NewKey);
	p = buf; cnt = size;
	while(cnt > 0) {
		rval = PiccAuth(MIFARE_KEY, NewKey, uid, blk);
//cprintf("Auth(PrimaryKey=%02x-%02x-%02x-%02x-%02x-%02x) = %d\n", (int)NewKey[0], (int)NewKey[1], (int)NewKey[2], (int)NewKey[3], (int)NewKey[4], (int)NewKey[5], rval);
		if(!rval) key_sector = 0;
		else if(SecondaryKey) {
			while(1) {
				rval = PiccRequest(ISO14443_3_REQIDL, tmpbuf2);
				if(rval == 0 || rval == 1) break;
				//if(DS_TIMER > WdtResetTimer+50L) {
				//	wdtReset(); taskYield();
				//}
			}
			if(!rval) {
				rval = PiccSelect(uid, tmpbuf2);
//cprintf("PiccSelect = %d\n", rval);
				if(!rval) {
					rval = PiccAuth(MIFARE_KEY, SecondaryKey, uid, blk);
//cprintf("Auth(SecondaryKey) = %d\n", rval);
					if(!rval) key_sector = 1;
				}
			}
		}
		if(rval) break;
		blk_max = (blk & 0xfc) + 0x03;
		for( ;blk < blk_max;blk++) {
			rval = PiccRead(blk, p);
//cprintf("Read(%d) = %d\n", blk, rval);
			if(rval) break;
			p += 16; cnt -= 16;
			if(cnt <= 0) break;
		}
		if(rval) break;
		if(key_sector && NewKey) {
			rval = PiccWrite(blk_max, tmpbuf);
//cprintf("WriteKey(%d) = %d\n", blk_max, rval);
			if(rval) break;
		}
		blk++;
	}
	picc_status = rval;
	if(rval) rval = -1;
	else	 rval = p - buf;
	return rval;
}

int scWriteData(int block, unsigned char *buf, int size, unsigned char *uid, unsigned char *SecondaryKey, unsigned char *NewKey)
{
	unsigned char	*p, tmpbuf[20], tmpbuf2[20];
	int		rval, blk, blk_max, cnt, key_sector;
	
	blk = block;
	if(SecondaryKey && NewKey) _MakeSectorTrail(tmpbuf, NewKey);
	p = buf; cnt = size;
	while(cnt > 0) {
		rval = PiccAuth(MIFARE_KEY, NewKey, uid, blk);
//cprintf("Auth(PrimaryKey=%02x-%02x-%02x-%02x-%02x-%02x) = %d\n", (int)NewKey[0], (int)NewKey[1], (int)NewKey[2], (int)NewKey[3], (int)NewKey[4], (int)NewKey[5], rval);
		if(!rval) key_sector = 0;
		else if(SecondaryKey) {
			while(1) {
				rval = PiccRequest(ISO14443_3_REQIDL, tmpbuf2);
				if(rval == 0 || rval == 1) break;
				//if(DS_TIMER > WdtResetTimer+50L) {
				//	wdtReset(); taskYield();
				//}
			}
			if(!rval) {
				rval = PiccSelect(uid, tmpbuf2);
//cprintf("PiccSelect = %d\n", rval);
				if(!rval) {
					rval = PiccAuth(MIFARE_KEY, SecondaryKey, uid, blk);
//cprintf("Auth(SecondaryKey) = %d\n", rval);
					if(!rval) key_sector = 1;
				}
			}
		}
		if(rval) break;
		blk_max = (blk & 0xfc) + 0x03;
		for( ;blk < blk_max;blk++) {
			if(cnt < 16) memset(p+cnt, 0xff, 16-cnt); 
			rval = PiccWrite(blk, p);
//cprintf("Write(%d) = %d\n", blk, rval);
			if(rval) break;
			p += 16; cnt -= 16;
			if(cnt <= 0) break;
		}
		if(rval) break;
		if(key_sector && NewKey) {
			rval = PiccWrite(blk_max, tmpbuf);
//cprintf("WriteKey(%d) = %d\n", blk_max, rval);
			if(rval) break;
		}
		blk++;
	}
	picc_status = rval;
	if(rval) rval = -1;
	else	 rval = p - buf;
	return rval;
}

int scFormatCard(unsigned char *uid, unsigned char *SecondaryKey, unsigned char *NewKey)
{
	unsigned char	buf[20], tmpbuf[16];
	int		i, rval, blk, blk_max, key_sector, auth;
	
	memset(buf, 0xff, 16);
	blk = 2;
	rval = PiccAuth(MIFARE_KEY, NewKey, uid, blk);
	if(!rval) key_sector = 0;
	else if(SecondaryKey) {
		rval = PiccAuth(MIFARE_AUTHENT_B, SecondaryKey, uid, blk);
		if(!rval) key_sector = 1;
	}
	if(rval) return -1;
	auth = 0;	
	if(key_sector && NewKey) _MakeSectorTrail(tmpbuf, NewKey);
	for(i = 0;i < 16;i++) { 
		if(auth) {
			if(key_sector) rval = PiccAuth(MIFARE_AUTHENT_B, SecondaryKey, uid, blk);
			else	rval = PiccAuth(MIFARE_KEY, NewKey, uid, blk);
			if(rval) break;
		}
		blk_max = (blk & 0xfc) + 0x03;
		for( ;blk < blk_max;blk++) { 
			rval = PiccWrite(blk, buf);
			if(rval) break;
		}
		if(rval) break;
		if(key_sector && NewKey) {
			rval = PiccWrite(blk_max, tmpbuf);
			if(rval) break;
		}
		blk++;
		auth = 1;
	}
	picc_status = rval;
	if(rval) rval = -1;
	else	rval = 1;
	return rval;
}
*/

// PrimaryKey(P) + SecondaryKey(S) + KeyOption(O)
// src_data:PPPPPPSSSSSSO => buf:XXXXPPPPPPXXXXSSSSSSXXXO => encode => dst_data
int scEncodeKey(unsigned char *src_data, unsigned char *dst_data)
{
	unsigned char	*s, *d, buf[36];
	unsigned long	key;
	int		len;
	
	s = src_data; d = buf;	
	simRand(d, 4); d += 4;
	memcpy(d, s, 6); s += 6; d += 6;
	simRand(d, 4); d += 4;
	memcpy(d, s, 6); s += 6; d += 6;
	simRand(d, 3); d += 3;
	*d++ = *s;
	len = d - buf;		
	key = devId(NULL);
	key = (key + (key << 16)) * key + key;
	sim32Encrypt(buf, len, dst_data, 0xa26e95c6, key);
	return len;
}

// CurrentPrimaryKey(C) + NewPrimaryKey(P) + NewSecondaryKey(S) + KeyLegth(L) + KeyOption(O)
// src_data => decode => buf:XXXXCCCCCCXXXXPPPPPPXXXXSSSSSSXLXO => dst_data:CCCCCCPPPPPPSSSSSSO
int scDecodeKey(unsigned char *src_data, unsigned char *dst_data)
{
	unsigned char	*s, *d, buf[36];
	unsigned long	key;
	int		len, c;
	
	key = devId(NULL);
	key = (key + (key << 16)) * key + key;
	sim32Decrypt(src_data, 34, buf, 0xa26e95c6, key);
	s = buf; d = dst_data;
	s += 4;
	memcpy(d, s, 6); s += 10; d += 6;
	memcpy(d, s, 6); s += 10; d += 6;
	memcpy(d, s, 6); s += 7;  d += 6;
	len = *s; s += 2;
	*d = c = *s;
	if(len < 1 || len > 6 || c > 0x03) len = -1; 
	return len;
}

void scPrintKey(unsigned char *key, char *msg)
{
	printf("%s=[%02x %02x %02x %02x %02x %02x]\n", msg, (int)key[0], (int)key[1], (int)key[2], (int)key[3], (int)key[4], (int)key[5]);
}

int scValidateKey(unsigned char *key)
{
	unsigned char	dfltkey[8], nullkey[8];
	
	memset(dfltkey, 0xff, 6);
	memset(nullkey, 0x00, 6);
	if(!memcmp(key, dfltkey, 6) || !memcmp(key, nullkey, 6)) return 0;
	else	return 1;
}
