#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "NSEnum.h"
#include "prim.h"
#include "sysdep.h"
#include "acct.h"


int acctfsSeek(long position)
{
	return fseek(fp6, position, SEEK_SET);
}

long acctfsTell(void)
{
	return ftell(fp6);
}

int acctfsEof(void)
{
	int		c;

	c = fgetc(fp6);
	if(c < 0) c = 1;
	else {
		ungetc(c, fp6);
		c = 0;
	}
}

int acctfsRead(ACCT *acct)
{
	unsigned char	buf[ACCT_RECORD_SZ];
	int		rval;

	while((rval = fread(buf, 1, ACCT_RECORD_SZ, fp6)) == ACCT_RECORD_SZ)
		if(buf[0]) break;
	if(rval < 0) return -1;
	else if(rval < ACCT_RECORD_SZ) return 0;
	if(acct) {
		acctDecodeID(&acct->id, buf+1);
		acctDecode(acct, buf+2);
	}
	return 1;
}

int acctfsReadAtIndex(ACCT *acct, int index)
{
	unsigned char	buf[ACCT_RECORD_SZ];
	int		rval, count;

	fseek(fp6, 0L, SEEK_SET);
	count = 0;
	while((rval = fread(buf, 1, ACCT_RECORD_SZ, fp6)) == ACCT_RECORD_SZ)
		if(buf[0]) {
			if(count >= index) break;
			count++;
		}
	if(rval < 0) return -1;
	else if(rval < ACCT_RECORD_SZ) return 0;
	if(acct) {
		acctDecodeID(&acct->id, buf+1);
		acctDecode(acct, buf+2);
	}
	return 1;
}

int acctfsGet(ACCT *acct)
{
	unsigned char	dID[4], buf[ACCT_RECORD_SZ];
	int		rval;

	acctEncodeID(acct->id, dID);
	fseek(fp6, 0L, SEEK_SET);
	rval = fsFindRecord(fp6, buf, ACCT_RECORD_SZ, dID, 1, 1);
	if(rval <= 0) return rval;
	acctDecode(acct, buf + 2);
	return 1;
}

int acctfsGetWithUrl(ACCT *acct, char *url)
{
	unsigned char	buf[ACCT_RECORD_SZ], key[64];
	int		rval;

	memcpy_pad(key, url, 64);
	rval = fsFindRecord(fp6, buf, ACCT_RECORD_SZ, key, 2, 64);
	if(rval > 0) {
		if(acct) {
			acctDecodeID(&acct->id, buf+1);
			acctDecode(acct, buf+2);
		}
		fseek(fp6, ACCT_RECORD_SZ, SEEK_CUR);
		rval = 1;
	}
	return rval;
}

// Return -1:System error 0:Full 1:New acct 2:Exist acct(Overwrite)
int acctfsAddEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[ACCT_RECORD_SZ];
	int		rval;
	
	buf[0] = OT_PHONE_NO; acctEncodeID(nID, buf+1); memcpy(buf+2, data, ACCT_RECORD_SZ-2);
	rval = fsAddRecord(fp6, buf, ACCT_RECORD_SZ, 1);
	return rval;
}

// Return -1:System error 0:Full 1:New acct 2:Exist acct(Overwrite)
int acctfsAdd(ACCT *acct)
{
	unsigned char	buf[ACCT_RECORD_SZ], idtbl[256];
	long	id;
	int		rval;

	if(acct->id < 0) {
		memset(idtbl, 0, 255);
		fseek(fp6, 0L, SEEK_SET);
		while((rval = fread(buf, 1, ACCT_RECORD_SZ, fp6)) == ACCT_RECORD_SZ)
			if(buf[0]) {
				acctDecodeID(&id, buf+1);
				idtbl[id] = 1;
			}
		for(id = 0;id < 255;id++)
			if(!idtbl[id]) break;
		if(id >= 255) return 0; 
		acct->id = id;
	}
	acctEncode(acct, buf);
	rval = acctfsAddEncoded(acct->id, buf);
	return rval;
}

int acctfsRemove(long nID)
{
	unsigned char	dID[4];
	int		rval;

	acctEncodeID(nID, dID);
	rval = fsDeleteRecord(fp6, ACCT_RECORD_SZ, dID, 1);
	return rval;
}

int acctfsExist(long nID)
{
	unsigned char	dID[4];
	int		rval;

	acctEncodeID(nID, dID);
	fseek(fp6, 0L, SEEK_SET);
	rval = fsFindRecord(fp6, NULL, ACCT_RECORD_SZ, dID, 1, 1);
	if(rval > 0) rval = 1;
	return rval;
}

int acctfsGetCount(void)
{
	return fsGetRecordCount(fp6, ACCT_RECORD_SZ);
}


