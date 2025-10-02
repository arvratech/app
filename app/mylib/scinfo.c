#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prim.h"
#include "scinfo.h"


void scInfoSetDefault(SMART_CARD_INFO *scInfo)
{
	memset(scInfo->mifareKey, 0xff, 6);		// Mifare key
	scInfo->mifareBlockNo		= 0;		// Mifare Sector# x 4 + Block#
	scInfo->mifareBlockCount	= 1;		// Mifare block count
	scInfo->aidLength			= 0;		// ISO7816-4 AID length
	memset(scInfo->aid, 0xff, 10);			// ISO7816-4 AID
	scInfo->apduMode			= 0;		// reserve
	memset(scInfo->apdu, 0x00, 5);			// ISO7816-4 APDU: INS, P1, P2, Lc, Le
	scInfo->startPosition		= 0;		// Start position of ISO7816-4 file
	scInfo->dataLength			= 0;		// Data length of ISO7816-4 file
}

int scInfoEncode(SMART_CARD_INFO *scInfo, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	memcpy(p, scInfo->mifareKey, 6); p += 6;
	*p++ = scInfo->mifareBlockNo;
	*p++ = scInfo->mifareBlockCount;
	*p++ = scInfo->aidLength;
	memcpy(p, scInfo->aid, 10); p += 10;
	*p++ = scInfo->apduMode;
	memcpy(p, scInfo->apdu, 5); p += 5;
	*p++ = scInfo->startPosition;
	*p++ = scInfo->dataLength;
 	return p - (unsigned char *)buf;
}

int scInfoDecode(SMART_CARD_INFO *scInfo, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	memcpy(scInfo->mifareKey, p, 6); p += 6;
	scInfo->mifareBlockNo		= *p++;
	scInfo->mifareBlockCount	= *p++;
	scInfo->aidLength			= *p++;
	memcpy(scInfo->aid, p, 10); p += 10;
	scInfo->apduMode			= *p++;
	memcpy(scInfo->apdu, p, 5); p += 5;
	scInfo->startPosition		= *p++;
	scInfo->dataLength			= *p++;
 	return p - (unsigned char *)buf;
}
