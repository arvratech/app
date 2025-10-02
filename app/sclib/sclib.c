#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtc.h"
#include "sio.h"
#include "mfrc.h"
#include "syscfg.h"
#include "sclib.h"

extern unsigned long	WdtResetTimer;

#define MIFARE_KEY		MIFARE_AUTHENT_A

int		picc_status;


int scAuthKey(int block, unsigned char *uid, unsigned char *Key)
{
	int		rval;

	rval = PiccAuth(MIFARE_KEY, Key, uid, block);
	return rval;
}

static void _MakeSectorTrail(unsigned char *buf, unsigned char *keys)
{
	unsigned char	*p;
	int		i;
	
	p = buf;
	memcpy(p, keys, 6); p += 6; *p++ = 0xff; *p++ = 0x07; *p++ = 0x80; *p++ = 0x69;
	for(i = 0;i < 6;i++) *p++ = 0xff;
}

int scReadData(int block, unsigned char *buf, int size, unsigned char *uid, unsigned char *SecondaryKey, unsigned char *NewKey)
{
	unsigned char	*p, tmpbuf[20], tmpbuf2[20];
	int		rval, blk, blk_max, cnt, key_sector;
	
	blk = block;
	if(SecondaryKey && NewKey) _MakeSectorTrail(tmpbuf, NewKey);
	p = buf; cnt = size;
	while(cnt > 0) {
		rval = PiccAuth(MIFARE_KEY, NewKey, uid, blk);
printf("Auth(PrimaryKey=%02x-%02x-%02x-%02x-%02x-%02x) = %d(0x%x)\n", (int)NewKey[0], (int)NewKey[1], (int)NewKey[2], (int)NewKey[3], (int)NewKey[4], (int)NewKey[5], rval, rval);
		if(!rval) key_sector = 0;
		else if(SecondaryKey) {
			while(1) {
				rval = PiccRequest(ISO14443_3_REQIDL, tmpbuf2);
				if(rval == 0 || rval == 1) break;
				//if(DS_TIMER > WdtResetTimer+50L) {
				//	wdtReset(); WdtResetTimer = DS_TIMER; taskYield();
				//}
			}
			if(!rval) {
				rval = PiccSelect(uid, tmpbuf2);
//printf("PiccSelect = %d\n", rval);
				if(!rval) {
					rval = PiccAuth(MIFARE_KEY, SecondaryKey, uid, blk);
//printf("Auth(SecondaryKey) = %d\n", rval);
					if(!rval) key_sector = 1;
				}
			}
		}
		if(rval) break;
		blk_max = (blk & 0xfc) + 0x03;
		for( ;blk < blk_max;blk++) {
			rval = PiccRead(blk, p);
//printf("Read(%d) = %d\n", blk, rval);
			if(rval) break;
			p += 16; cnt -= 16;
			if(cnt <= 0) break;
		}
		if(rval) break;
		if(key_sector && NewKey) {
			rval = PiccWrite(blk_max, tmpbuf);
//printf("WriteKey(%d) = %d\n", blk_max, rval);
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
//printf("Auth(PrimaryKey=%02x-%02x-%02x-%02x-%02x-%02x) = %d\n", (int)NewKey[0], (int)NewKey[1], (int)NewKey[2], (int)NewKey[3], (int)NewKey[4], (int)NewKey[5], rval);
		if(!rval) key_sector = 0;
		else if(SecondaryKey) {
			while(1) {
				rval = PiccRequest(ISO14443_3_REQIDL, tmpbuf2);
				if(rval == 0 || rval == 1) break;
				//if(DS_TIMER > WdtResetTimer+50L) {
				//	wdtReset(); WdtResetTimer = DS_TIMER; taskYield();
				//}
			}
			if(!rval) {
				rval = PiccSelect(uid, tmpbuf2);
//printf("PiccSelect = %d\n", rval);
				if(!rval) {
					rval = PiccAuth(MIFARE_KEY, SecondaryKey, uid, blk);
//printf("Auth(SecondaryKey) = %d\n", rval);
					if(!rval) key_sector = 1;
				}
			}
		}
		if(rval) break;
		blk_max = (blk & 0xfc) + 0x03;
		for( ;blk < blk_max;blk++) {
			if(cnt < 16) memset(p+cnt, 0xff, 16-cnt); 
			rval = PiccWrite(blk, p);
//printf("Write(%d) = %d\n", blk, rval);
			if(rval) break;
			p += 16; cnt -= 16;
			if(cnt <= 0) break;
		}
		if(rval) break;
		if(key_sector && NewKey) {
			rval = PiccWrite(blk_max, tmpbuf);
//printf("WriteKey(%d) = %d\n", blk_max, rval);
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

void scSetKeyDefault(void)
{
	memset(sys_cfg->SCPrimaryKey, 0xff, 6);
	memset(sys_cfg->SCSecondaryKey, 0xff, 6);
	sys_cfg->SCKeyOption = 0;
}

int scValidateKey(unsigned char *key, int len)
{
	unsigned char	dfltkey[8], nullkey[8];
	
	memset(dfltkey, 0xff, len);
	memset(nullkey, 0x00, len);
	if(!memcmp(key, dfltkey, len) || !memcmp(key, nullkey, len)) return 0;
	else	return 1;
}

int scGetKey(SYS_CFG *sys_cfg, unsigned char *data)
{
	unsigned char	*p;
	
	p = data;	
	memcpy(p, sys_cfg->SCPrimaryKey, 6); p += 6;
	memcpy(p, sys_cfg->SCSecondaryKey, 6); p += 6;
	*p++ = sys_cfg->SCKeyOption;
	return p - data;
}

int scSetKey(SYS_CFG *sys_cfg, unsigned char *data)
{
	unsigned char	*p;
	
	p = data;	
	memcpy(sys_cfg->SCPrimaryKey, p, 6); p += 6;
	memcpy(sys_cfg->SCSecondaryKey, p, 6); p += 6;
	sys_cfg->SCKeyOption = *p++;
	return p - data;
}

void scSetValidateKey(unsigned char *data, int len)
{
	unsigned char	*p;
	
	p = data;	
	if(scValidateKey(p, len)) memcpy(sys_cfg->SCPrimaryKey, p, len);
	p += 6;
	if(scValidateKey(p, len)) memcpy(sys_cfg->SCSecondaryKey, p, len);
	p += 6;
	sys_cfg->SCKeyOption = *p;
}

int scCompareKey(unsigned char *data)
{
	if(memcmp(sys_cfg->SCPrimaryKey, data, 6) || memcmp(sys_cfg->SCSecondaryKey, data+6, 6) || sys_cfg->SCKeyOption != data[12]) return 1;
	else return 0;
}
