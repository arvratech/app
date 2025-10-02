#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "prim.h"
#include "pno.h"


void pnoSetDefault(PHONE_NO *pno)
{
	pno->ObjectID = -1;
	pno->PhoneNo[0] = 0;
	memset(pno->IPAddress, 0, 4);
}

int pnoEncode(PHONE_NO *pno, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	string2bcd(pno->PhoneNo, 8, (unsigned char *)buf); p += 4;
	memcpy(p, pno->IPAddress, 4); p += 4;
	memset(p, 0xff, 4); p += 4;
	return p - (unsigned char *)buf;
}

int pnoDecode(PHONE_NO *pno, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	bcd2string((unsigned char *)buf, 8, pno->PhoneNo); p += 4;
	memcpy(pno->IPAddress, p, 4); p += 4;
	p += 4;
	return p - (unsigned char *)buf;
}

int pnoValidate(PHONE_NO *pno)
{
	int		cnt, rval;

	rval = 0;
	cnt = isdigits(pno->PhoneNo);
	if(cnt >= 1) {
		rval = CheckIPAddr(pno->IPAddress, 0);
		if(rval < 0) rval = 0; else rval = 1;
	}
	return rval;
}

void pnoEncodeID(long nID, void *buf)
{
	IDtoPACK3(nID, (unsigned char *)buf);	
}

void pnoDecodeID(long *pID, void *buf)
{
	PACK3toID((unsigned char *)buf, pID);
}

int pnoValidateID(long nID)
{
	if(nID > 0 && nID < 0xffffff) return 1;
	else	return 0;
}

int pnoEncodePhoneNo(char *PhoneNo, void *buf)
{
	string2bcd(PhoneNo, 8, (unsigned char *)buf);
	return 4;
}

int pnoDecodePhoneNo(char *PhoneNo, void *buf)
{
	bcd2string((unsigned char *)buf, 8, PhoneNo);
	return 4;
}

int pnoValidatePhoneNo(char *PhoneNo)
{
	int		cnt, rval;

	rval = 0;
	cnt = isdigits(PhoneNo);
	if(cnt >= 1) rval = 1;
	return rval;
}
