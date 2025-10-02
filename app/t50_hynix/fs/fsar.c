#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "prim.h"
#include "sysdep.h"
#include "fsar.h"

FS_FILE *_OpenFile(int file);


static FS_FILE	*f1, *f2, *f3;


int fsOpenScheFiles(void)
{
	flashWriteEnable();
	f1 = _OpenFile(10);		// Calendar file
	f2 = _OpenFile(11);		// Schedule file
	f3 = _OpenFile(12);		// AccessRights file
	if(f1 && f2 && f3) return 0;
	else	return -1;
}

int arfsSeek(long position)
{
	return fsSeek(f3, position, SEEK_SET);
}

long arfsTell(void)
{
	return fsTell(f3);
}

int arfsEof(void)
{
	return fsEof(f3);
}

int arfsGet(ACCESS_RIGHTS *ar)
{
	unsigned char	dID[4], buf[ACCESS_RIGHTS_RECORD_SZ];
	int		rval;

	arEncodeID(ar->ID, dID);
	fsSeek(f3, 0L, SEEK_SET);
	rval = fsFindRecord(f3, buf, ACCESS_RIGHTS_RECORD_SZ, dID, 1, 3);
	if(rval <= 0) return rval;
	arDecode(ar, buf + 4);
	return 1;
}

int arfsGetEncoded(long nID, unsigned char *data)
{
	unsigned char	dID[4];
	int		rval;

	arEncodeID(nID, dID);
	rval = fsGetRecords(f3, data, 1, ACCESS_RIGHTS_RECORD_SZ, dID, 3);
	if(rval > 0) rval *= ACCESS_RIGHTS_BODY_SZ;
	return rval;
}

// Return -1:System error 0:Full 1:New 2:Exist(Overwrite) 3:Skip(Same)
int arfsAddEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[ACCESS_RIGHTS_RECORD_SZ];
	int		rval;

	buf[0] = OT_ACCESS_RIGHTS; IDtoPACK3(nID, buf+1); memcpy(buf+4, data, ACCESS_RIGHTS_BODY_SZ);
	flashWriteEnable();
	rval = fsAddRecord(f3, buf, ACCESS_RIGHTS_RECORD_SZ, 3);
	flashWriteDisable();
	return rval;
}

int arfsReadBulk(unsigned char *Data, int Size)
{
	int		rval;

	rval = fsReadRecord(f3, Data, ACCESS_RIGHTS_RECORD_SZ, Size / ACCESS_RIGHTS_RECORD_SZ);
	if(rval > 0) rval *= ACCESS_RIGHTS_RECORD_SZ;
	return rval;
}

int arfsAddBulk(unsigned char *Data, int Size)
{
	int		rval;

	fsSeek(f3, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f3, Data, Size);	
	flashWriteDisable();
	if(rval != Size) return -1;
	else	return 0;
}

int arfsRemove(long nID)
{
	unsigned char	dID[4];
	int		rval;

	arEncodeID(nID, dID);
	flashWriteEnable();
	rval = fsDeleteRecord(f3, ACCESS_RIGHTS_RECORD_SZ, dID, 3);
	flashWriteDisable();
	return rval;
}

int arfsRemoveAll(void)
{
	int		rval;

	fsSeek(f3, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f3, NULL, -1);
	flashWriteDisable();	
	if(rval < 0) return -1;
	else	return 1;
}

int arfsGetCount(void)
{
	return fsGetRecordCount(f3, ACCESS_RIGHTS_RECORD_SZ);
}

/*
int narfsSeek(long position)
{
	return fsSeek(f4, position, SEEK_SET);
}

long narfsTell(void)
{
	return fsTell(f4);
}

int narfsEof(void)
{
	return fsEof(f4);
}

int narfsGetEncoded(long nID, unsigned char *Data, int Size)
{
	unsigned char	dID[4];
	int		rval;
	
	arEncodeID(nID, dID);
	rval = fsGetRecords(f4, Data, Size / (ACCESS_RULE_RECORD_SZ-4), ACCESS_RULE_RECORD_SZ, dID, 3);
	if(rval > 0) rval *= ACCESS_RULE_RECORD_SZ - 4;
	return rval;
}

// Return -1:System error 0:Full 1:New 2:Exist(Overwrite) 3:Skip(Same)
int narfsAddEncoded(long nID, unsigned char *Data, int Size)
{
	unsigned char	dID[4], *s, *d, *NewMark, *OldData;	
	long	*lp, val;
	int		i, j, rval, NewCnt, OldCnt;
	
	arEncodeID(nID, dID);
	NewCnt = Size / ACCESS_RULE_RECORD_SZ;
	NewMark = s = Data + Size;
	for(i = 0;i < NewCnt;i++) *s++ = 0;
	i = s - Data; i = ((i + 3) >> 2) << 2;
	OldData = &Data[i];
	OldCnt = fsGetPositionRecords(f4, OldData, 10000, ACCESS_RULE_RECORD_SZ, dID, 3);
	for(i = 0, s = Data;i < NewCnt;i++, s += ACCESS_RULE_RECORD_SZ - 4) {
		for(j = 0, d = OldData;j < OldCnt;j++, d += ACCESS_RULE_RECORD_SZ + 4) {
			lp = (long *)d; val = *lp;
			if(val >= 0 && !n_memcmp(s, d+8, ACCESS_RULE_RECORD_SZ - 4)) {
				*lp = -1; NewMark[i] = 1;
			}
		}
	}
	j = 0, d = OldData;
	flashWriteEnable();
	for(i = 0, s = Data;i < NewCnt;i++, s += ACCESS_RULE_RECORD_SZ - 4) {
		if(!NewMark[i]) {
			for( ;j < OldCnt;j++, d += ACCESS_RULE_RECORD_SZ + 4) {
				lp = (long *)d; val = *lp;
				if(val >= 0) {
					memcpy(d+8, s, ACCESS_RULE_RECORD_SZ - 4);
					fsSeek(f4, val, SEEK_SET);
					fsWrite(f4, d+4, ACCESS_RULE_RECORD_SZ);
					break;
				}
			}
			if(j >= OldCnt) break;
			j++, d += ACCESS_RULE_RECORD_SZ + 4;
		}
	}
	if(i < NewCnt) {
		for( ;i < NewCnt;i++, s += ACCESS_RULE_RECORD_SZ - 4) {
			if(!NewMark[i]) {
				memcpy(d+8, s, ACCESS_RULE_RECORD_SZ - 4);
				rval = fsAddRecordNull(f4, d+4, ACCESS_RULE_RECORD_SZ);
			}
		}
	} else if(j < OldCnt) {
		dID[0] = 0;
		for( ;j < OldCnt;j++, d += ACCESS_RULE_RECORD_SZ + 4) {
			lp = (long *)d; val = *lp;
			if(val >= 0) {
				fsSeek(f4, val, SEEK_SET);
				fsWrite(f4, dID, 1);
			}
		}
	}
	flashWriteDisable();
	return 1;
}

int narfsReadBulk(unsigned char *Data, int Size)
{
	int		rval;

	rval = fsReadRecord(f4, Data, ACCESS_RULE_RECORD_SZ, Size / ACCESS_RULE_RECORD_SZ);
	if(rval > 0) rval *= ACCESS_RULE_RECORD_SZ;
	return rval;
}

int narfsAddBulk(unsigned char *Data, int Size)
{
	int		rval;

	fsSeek(f4, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f4, Data, Size);	
	flashWriteDisable();
	if(rval != Size) return -1;
	else	return 0;
}

int narfsRemoveAll(void)
{
	int		rval;

	fsSeek(f4, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f4, NULL, -1);
	flashWriteDisable();	
	if(rval < 0) return -1;
	else	return 1;
}

int narfsGetCount(void)
{
	return fsGetRecordCount(f4, ACCESS_RULE_RECORD_SZ);
}
*/

#include "fscal.c"
#include "fssche.c"
