#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "acct.h"


void acctSetDefault(ACCT *acct)
{
	acct->id = -1;
	acct->url[0] = acct->acctName[0] = 0;
}

int acctEncode(ACCT *acct, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	memcpy_pad(p, acct->url, 64); p += 64;
	memcpy_pad(p, acct->acctName, 32); p += 32;
	memset(p, 0xff, 30); p += 30;
	return p - (unsigned char *)buf;
}

int acctDecode(ACCT *acct, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	memcpy_chop(acct->url, p, 64); p += 64;
	memcpy_chop(acct->acctName, p, 32); p += 32;
	p += 30;
	return p - (unsigned char *)buf;
}

int acctValidate(ACCT *acct)
{
	return 1;
}

int acctEncodeID(long nID, void *buf)
{
	IDtoPACK1(nID, buf);
	return 1;
}

int acctDecodeID(long *pID, void *buf)
{
	PACK1toID(buf, pID);
	return 1;
}

int acctValidateID(long nID)
{
	if(nID >= 0 && nID < 255) return 1;
	else	return 0;
}

void acctCopy(ACCT *acctD, ACCT *acctS)
{
	memcpy(acctD, acctS, sizeof(ACCT));	
}

int acctCompare(ACCT *acct1, ACCT *acct2)
{
	unsigned char	buf[256];

	acctEncodeID(acct1->id, buf);
	acctEncode(acct1, buf+1);
	acctEncodeID(acct2->id, buf+128);
	acctEncode(acct2, buf+129);
	if(!memcmp(buf, buf+128, 127)) return 0;
	else	return -1;
}

