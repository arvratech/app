#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs_obj.h"
#include "prim.h"
#include "pno.h"
#include "fsuser.h"


int pnofsSeek(long position)
{
	return fsSeek(f6, position, SEEK_SET);
}

long pnofsTell(void)
{
	return fsTell(f6);
}

int pnofsEof(void)
{
	return fsEof(f6);
}

int pnofsRead(PHONE_NO *pno)
{
	unsigned char	buf[PHONE_NO_RECORD_SZ];
	int		rval;

	while((rval = fsRead(f6, buf, PHONE_NO_RECORD_SZ)) == PHONE_NO_RECORD_SZ)
		if(buf[0]) break;
	if(rval < 0) return -1;
	else if(rval < PHONE_NO_RECORD_SZ) return 0;
	if(pno) {
		pnoDecodeID(&pno->ObjectID, buf + 1);
		pnoDecode(pno, buf+4);
	}
	return 1;
}

int pnofsGet(PHONE_NO *pno)
{
	unsigned char	dID[8], buf[PHONE_NO_RECORD_SZ];
	int		rval;

	pnoEncodeID(pno->ObjectID, dID);
	fsSeek(f6, 0L, SEEK_SET);
	rval = fsFindRecord(f6, buf, PHONE_NO_RECORD_SZ, dID, 1, 3);
	if(rval <= 0) return rval;
	pnoDecode(pno, buf+4);
//cprintf("Get [%s] %d.%d.%d.%d\n", pno->PhoneNo, (int)pno->IPAddress[0], (int)pno->IPAddress[1], (int)pno->IPAddress[2], (int)pno->IPAddress[3]);
	return 1;
}

int pnofsGetPhoneNo(PHONE_NO *pno)
{
	unsigned char	dPhoneNo[16], buf[PHONE_NO_RECORD_SZ];
	int		rval;

	pnoEncodePhoneNo(pno->PhoneNo, dPhoneNo);
	fsSeek(f6, 0L, SEEK_SET);
	rval = fsFindRecord(f6, buf, PHONE_NO_RECORD_SZ, dPhoneNo, 4, MAX_PHONENO_HALF_SZ);
	if(rval <= 0) return rval;
	pnoDecodeID(&pno->ObjectID, buf+1);
	pnoDecode(pno, buf+4);
//cprintf("Get [%s] %d.%d.%d.%d\n", pno->PhoneNo, (int)pno->IPAddress[0], (int)pno->IPAddress[1], (int)pno->IPAddress[2], (int)pno->IPAddress[3]);
	return 1;
}

int pnofsGetEncoded(long nID, unsigned char *data)
{
	unsigned char	dID[4];
	int		rval;

	pnoEncodeID(nID, dID);
	rval = fsGetRecords(f6, data, 1, PHONE_NO_RECORD_SZ, dID, 3);
	if(rval > 0) rval = PHONE_NO_RECORD_SZ - 4;
	return rval;
}

// Return -1:System error 0:Full 1:New PhoneNo 2:Exist PhoneNo(Overwrite)
int pnofsAddEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[PHONE_NO_RECORD_SZ];
	int		rval;

	buf[0] = OT_PHONE_NO; IDtoPACK3(nID, buf+1); memcpy(buf+4, data, PHONE_NO_RECORD_SZ-4);
	flashWriteEnable();
	rval = fsAddRecord(f6, buf, PHONE_NO_RECORD_SZ, 3);
	flashWriteDisable();
	return rval;
/*
	fsSeek(f6, 0L, SEEK_SET);
	found = fsFindRecord(f6, tmpbuf, PHONE_NO_RECORD_SZ, buf+1, 1, 3);
	if(found < 0) return -1;
	else if(found == 0) {
		fsSeek(f6, 0L, SEEK_SET);
		rval = fsFindNullRecord(f6, PHONE_NO_RECORD_SZ);
		if(rval < 0) return rval;
		else if(rval == 0) {
			offset = fsTell(f6) + PHONE_NO_RECORD_SZ;
			if(offset > fsGetLimit(f6)) return 0;
		}
	} else if(n_memcmp(buf+4, tmpbuf+4, PHONE_NO_RECORD_SZ-4)) found = 1;
	else	found = 2;
	if(found < 2) {
		buf[0] = OT_PHONE_NO;
		flashWriteEnable();
		rval = fsWrite(f6, buf, PHONE_NO_RECORD_SZ);
		flashWriteDisable();
		if(rval != PHONE_NO_RECORD_SZ) return -1;
	}
	return found+1;
*/
}

int pnofsReadBulk(unsigned char *Data, int Size)
{
	int		rval;

	rval = fsReadRecord(f6, Data, PHONE_NO_RECORD_SZ, Size / PHONE_NO_RECORD_SZ);
	if(rval > 0) rval *= PHONE_NO_RECORD_SZ;
	return rval;
}

int pnofsAddBulk(unsigned char *Data, int Size)
{
	int		rval;

	fsSeek(f6, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f6, Data, Size);	
	flashWriteDisable();
	if(rval != Size) return -1;
	else	return 0;
}

int pnofsRemove(PHONE_NO *pno)
{
	unsigned char	dID[4], tmpbuf[PHONE_NO_RECORD_SZ];
	int		rval;

	pnoEncodeID(pno->ObjectID, dID);
	fsSeek(f6, 0L, SEEK_SET);
	rval = fsFindRecord(f6, NULL, PHONE_NO_RECORD_SZ, dID, 1, 3);
	if(rval <= 0) return rval;
	tmpbuf[0] = 0x00;
	flashWriteEnable();
	rval = fsWrite(f6, tmpbuf, 1);
	flashWriteDisable();
	if(rval != 1) return -1; 
	else	return 1;
}

int pnofsRemovePhoneNo(PHONE_NO *pno)
{
	unsigned char	dPhoneNo[16], tmpbuf[PHONE_NO_RECORD_SZ];
	int		rval;

	pnoEncodePhoneNo(pno->PhoneNo, dPhoneNo);
	fsSeek(f6, 0L, SEEK_SET);
	rval = fsFindRecord(f6, NULL, PHONE_NO_RECORD_SZ, dPhoneNo, 4, MAX_PHONENO_HALF_SZ);
	if(rval <= 0) return rval;
	tmpbuf[0] = 0x00;
	flashWriteEnable();
	rval = fsWrite(f6, tmpbuf, 1);
	flashWriteDisable();
	if(rval != 1) return -1; 
	else	return 1;
}

int pnofsRemoveAll(void)
{
	int		rval;

	fsSeek(f6, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f6, NULL, -1);
	flashWriteDisable();	
	if(rval < 0) return -1;
	else	return 1;
}

int pnofsExistPhoneNo(char *PhoneNo)
{
	unsigned char	dPhoneNo[16];
	int		rval;

	pnoEncodePhoneNo(PhoneNo, dPhoneNo);
	fsSeek(f6, 0L, SEEK_SET);
	rval = fsFindRecord(f6, NULL, PHONE_NO_RECORD_SZ, dPhoneNo, 4, MAX_PHONENO_HALF_SZ);
	if(rval > 0) rval = 1;
	return rval;
}

int pnofsGetCount(void)
{
	return fsGetRecordCount(f6, PHONE_NO_RECORD_SZ);
}
