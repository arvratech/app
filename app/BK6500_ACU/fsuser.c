#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "prim.h"
#include "fpapi_new.h"
#include "sysdep.h"
#include "fsuser.h"

typedef struct _BUCKET {
	unsigned short	Next;
	unsigned short	Index;
} BUCKET;

#define	MAX_HASH_SZ			16384
#define MAX_USER_SZ			65000
#define CARDDATA_OFFSET		21	

static unsigned short	p_hashtbl[MAX_HASH_SZ];		// User Primary Hash Index Table
static unsigned short	s_hashtbl[MAX_HASH_SZ];		// User Secondary Hash Index Table
static unsigned short	a_hashtbl[MAX_HASH_SZ];		// UserAccessRights Primary Hash Index Table
static BUCKET			p_buckets[MAX_USER_SZ];		// User Primary Hash Buckets
static BUCKET			s_buckets[MAX_USER_SZ];		// User Secondary Hash Buckets
static BUCKET			a_buckets[MAX_USER_SZ];		// UserAccessRights Primary Hash Buckets

static FS_FILE	*f1, *f2, *f3, *f4, *f5, *f6, *f7, *f8;

extern unsigned char  crc16tab_h[], crc16tab_l[];

#define _HashUserID(nID)	(nID & 0x3fff)

static int _HashCardData(unsigned char *CardData)
{
	unsigned char	*p;
	unsigned char	c, c_h, c_l;
	int		i;
	
	c_h = 0xff; c_l = 0xff;		// CRC High - CRC Low
	p = CardData + 1;
	for(i = 0;i < 8;i++) {
		c = c_h ^ p[i]; c_h = c_l ^ crc16tab_h[c]; c_l = crc16tab_l[c];
	}
	i = (c_h << 8) +  c_l;
	return (i & 0x3fff);
}

static void _HashIndexAdd(int Index, long nID, unsigned char *CardData)
{
	BUCKET	*bkt;

	bkt = &p_buckets[Index];
	bkt->Next = 0; bkt->Index = Index;
	bkt = (BUCKET *)&p_hashtbl[_HashUserID(nID)];
	while(bkt->Next) bkt = &p_buckets[bkt->Next-1];
	bkt->Next = Index + 1;
	if(CardData[0]) {
		bkt = &s_buckets[Index];
		bkt->Next = 0; bkt->Index = Index;
		bkt = (BUCKET *)&s_hashtbl[_HashCardData(CardData)];
		while(bkt->Next) bkt = &s_buckets[bkt->Next-1];
		bkt->Next = Index + 1;
	}
}

static void _HashIndexRemove(int Index, long nID, unsigned char *CardData)
{
	BUCKET	*bkt, *p_bkt;

	bkt = (BUCKET *)&p_hashtbl[_HashUserID(nID)];
	while(bkt->Next) {
		p_bkt = bkt;
		bkt = &p_buckets[bkt->Next-1];
		if(bkt->Index == Index) {
			p_bkt->Next = bkt->Next;
			break;
		}
	}
	if(CardData[0]) {
		bkt = (BUCKET *)&s_hashtbl[_HashCardData(CardData)];
		while(bkt->Next) {
			p_bkt = bkt;
			bkt = &s_buckets[bkt->Next-1];
			if(bkt->Index == Index) {
				p_bkt->Next = bkt->Next;
				break;
			}
		}
	}
}

static void _HashIndexAddAR(int Index, long nID)
{
	BUCKET	*bkt;
	
	bkt = &a_buckets[Index];
	bkt->Next = 0; bkt->Index = Index;
	bkt = (BUCKET *)&a_hashtbl[_HashUserID(nID)];
	while(bkt->Next) bkt = &a_buckets[bkt->Next-1];
	bkt->Next = Index + 1;
}

static void _HashIndexRemoveAR(int Index, long nID)
{
	BUCKET	*bkt, *p_bkt;

	bkt = (BUCKET *)&a_hashtbl[_HashUserID(nID)];
	while(bkt->Next) {
		p_bkt = bkt;
		bkt = &a_buckets[bkt->Next-1];
		if(bkt->Index == Index) {
			p_bkt->Next = bkt->Next;
			break;
		}
	}
}

static void _HashIndexRemoveAll(void)
{	
	memset(p_hashtbl, 0, MAX_HASH_SZ<<1);
	memset(s_hashtbl, 0, MAX_HASH_SZ<<1);
	memset(a_hashtbl, 0, MAX_HASH_SZ<<1);
}

void InitCRC8(void);
void InitCRC16_CCITT(void);

static void _HashIndexInit(void)
{
	unsigned char	buf[USER_RECORD_SZ];
	long	nID;
	int		rval, idx;

	idx = 0;
	fsSeek(f1, 0L, SEEK_SET);
	while(1) {
		rval = fsRead(f1, buf, USER_RECORD_SZ);
		if(rval < USER_RECORD_SZ) break;
		else if(buf[0]) {		
			PACK3toID(buf+1, &nID);
//cprintf("%02x Idx=%d ID=%d\n", (int)buf[0], idx, nID);
			_HashIndexAdd(idx, nID, buf+CARDDATA_OFFSET);
		}
		idx++;
	}
}

static void _HashIndexInitAR(void)
{
	unsigned char	buf[USER_AR_RECORD_SZ];
	long	nID;
	int		rval, idx;
	BUCKET	*bkt;

	bkt = &a_buckets[2288];
	idx = 0;
	fsSeek(f8, 0L, SEEK_SET);
	while(1) {
		rval = fsRead(f8, buf, USER_AR_RECORD_SZ);
		if(rval < USER_AR_RECORD_SZ) break;
		else if(buf[0]) {
			PACK3toID(buf+1, &nID);
			_HashIndexAddAR(idx, nID);
		}
		idx++;
	}
}

//  -1:Error  0:None  >=1:Index+1
static int _HashIndexGet(long nID, unsigned char *buf)
{
	BUCKET	*bkt;
	unsigned char	dID[4];
	long	position;
	int		rval;

	userEncodeID(nID, dID);
	bkt = (BUCKET *)&p_hashtbl[_HashUserID(nID)];
	rval = 0;
	while(bkt->Next) {
		bkt = &p_buckets[bkt->Next-1];
		position = bkt->Index << USER_RECORD_SHIFT;
		fsSeek(f1, position, SEEK_SET);
		rval = fsRead(f1, buf, USER_RECORD_SZ);
		if(rval >= USER_RECORD_SZ && buf[0] && !n_memcmp(buf+1, dID, 3)) break;
		else if(rval < 0) break;
		rval = 0;
	}
	if(rval > 0) rval = bkt->Index + 1;
	return rval;
}

//  -1:Error  0:None  >=1:Index+1
static int _HashIndexGetAR(long nID, unsigned char *buf)
{
	BUCKET	*bkt;
	unsigned char	dID[4];
	long	position;
	int		rval;

	userEncodeID(nID, dID);
	bkt = (BUCKET *)&a_hashtbl[_HashUserID(nID)];
	rval = 0;
	while(bkt->Next) {
		bkt = &a_buckets[bkt->Next-1];
		position = bkt->Index << USER_AR_RECORD_SHIFT;
		fsSeek(f8, position, SEEK_SET);
		rval = fsRead(f8, buf, USER_AR_RECORD_SZ);
		if(rval >= USER_AR_RECORD_SZ && buf[0] && !n_memcmp(buf+1, dID, 3)) break;
		else if(rval < 0) break;
		rval = 0;
	}
	if(rval > 0) rval = bkt->Index + 1;
	return rval;
}

FS_FILE *_OpenFile(int file)
{
	FS_FILE		*f;
		
	if(!fsExist(file)) {
		f = fsCreate(file);
		if(!f) {
#ifdef BK_DEBUG
			cprintf("fsCreate(%d) error: %d\n", file, fsErrno);
#endif
			return (FS_FILE *)0;
		}
		fsClose(f);
	}
	f = fsOpen(file, FS_WRITE);	
	if(!f) {
#ifdef BK_DEBUG
		cprintf("fsOpen(%d) error: %d\n", file, fsErrno);
#endif
		return (FS_FILE *)0;
	}
#ifdef BK_DEBUG
	cprintf("file=%d is opened: size=%ld\n", file, f->position);
#endif
	return f;
}

unsigned char	eebuf[2048];

int fsOpenFiles(int MaxUserFileSize, int MaxTmplFileSize, void *Buffer)
{
	int		rval, StartBlock;
FS_FILE		*f;
int		size;

#ifdef BK_DEBUG
	cprintf("Initializing file system. Please wait...\n");
#endif
	StartBlock = GetUserStartBlock();
	rval = fsInit(StartBlock, -1, Buffer);
	if(rval < 0) {
#ifdef BK_DEBUG
		cprintf("Could not initialize filesystem: %d\n", fsErrno);
#endif
		return -1;
	}
	InitCRC8();
	InitCRC16_CCITT();
	_HashIndexRemoveAll();
	flashWriteEnable();
	f1 = _OpenFile(3);		// User file
	if(f1) {
		fsSetLimit(f1, MaxUserFileSize);
		_HashIndexInit();
	}
	f2 = _OpenFile(4);		// UserEx file
	if(f2) fsSetLimit(f2, MaxUserFileSize);
	f3 = _OpenFile(5);		// UserPhoto file
	if(f3) fsSetLimit(f3, MaxUserFileSize / USER_RECORD_SZ * USER_PHOTO_RECORD_SZ);
	f4 = _OpenFile(6);		// UserTransaction file
	if(f4) fsSetLimit(f4, MaxUserFileSize / USER_RECORD_SZ * 16);
	f5 = _OpenFile(7);		// Event file
	fsOpenEvent();
	f6 = _OpenFile(8);		// PhoneNo file
	if(f6) fsSetLimit(f6, GetPhoneNoFileSize());
	f7 = _OpenFile(9);		// CamImage file
	if(f7) fsSetLimit(f7, GetCamImageFileSize());
	f8 = _OpenFile(13);		// UserAccessRights file
	if(f8) {
		fsSetLimit(f8, MaxUserFileSize / USER_RECORD_SZ * USER_AR_RECORD_SZ);
		_HashIndexInitAR();
	}
/*
f = _OpenFile(19);
cprintf("evt=%d evtBackup=%d\n", fsFileLength(f5), fsFileLength(f)); 
fsSeek(f5, 0L, SEEK_SET);
while(1) {
	size = fsRead(f5, eebuf, 2048);
	if(size > 0) {
		rval = fsWrite(f, eebuf, size);
printf("size=%d rval=%d\n", size, rval);
	} else {
printf("rval=%d break\n", rval);
		break;
	}	
}
fsClose(f);
*/	
	flashWriteDisable();
	if(f1 && f2 && f3 && f4 && f5 && f6 && f7 && f8) return 0;
	else	return -1;
}

void fsCloseFiles(void)
{
	fsClose(f1);
	fsClose(f2);
	fsClose(f3);
	fsClose(f4);
	fsCloseEvent();
	fsClose(f5);
	fsClose(f6);
	fsClose(f7);
	fsClose(f8);
}

unsigned char	TmpBuf[USER_PHOTO_RECORD_SZ];

int userfsSeek(long position)
{
	return fsSeek(f1, position, SEEK_SET);
}

long userfsTell(void)
{
	return fsTell(f1);
}

int userfsEof(void)
{
	return fsEof(f1);
}

int userfsRead(FP_USER *user)
{
	unsigned char	buf[USER_RECORD_SZ];
	int		rval;

	while((rval = fsRead(f1, buf, USER_RECORD_SZ)) == USER_RECORD_SZ)
		if(buf[0]) break;
	if(rval < 0) return -1;
	else if(rval < USER_RECORD_SZ) return 0;
	if(user) {
		userDecode(user, buf+4);
		userDecodeID(&user->ID, buf+1);
	}
	return 1;
}

int userfsGet(FP_USER *user)
{
	unsigned char	buf[USER_RECORD_SZ];
	int		rval;

	rval = _HashIndexGet(user->ID, buf);
	if(rval > 0) userDecode(user, buf + 4);
	return rval;
}

int userfsGetCardData(FP_USER *user, unsigned char *CardData)
{
	BUCKET	*bkt;
	unsigned char	buf[USER_RECORD_SZ];
	long	position;
	int		rval, i;

	bkt = (BUCKET *)&s_hashtbl[_HashCardData(CardData)];
	rval = 0;
	while(bkt->Next) {
		bkt = &s_buckets[bkt->Next-1];
		position = bkt->Index << USER_RECORD_SHIFT;
		fsSeek(f1, position, SEEK_SET);
		rval = fsRead(f1, buf, USER_RECORD_SZ);
		if(rval >= USER_RECORD_SZ && buf[0] && !n_memcmp(buf+CARDDATA_OFFSET, CardData, 9)) break;
		else if(rval < 0) break;
		rval = 0;
	}
	if(rval > 0) {
		userDecodeID(&user->ID, buf+1);
		rval = userDecode(user, buf+4);
		rval = 1;
	}
//if(rval > 0) cprintf("userfsGetCardData: ID=%ld Mode=%02x AccessRights=%d\n", user->ID, (int)user->AccessMode, (int)user->AccessRights);
//else if(rval < 0) cprintf("userfsGetCardData: file error: fsErrno=%d\n", fsErrno);
//else	cprintf("userfsGetCardData: No found\n");
	return rval;
}


int userfsGetCountCardData(unsigned char *CardData)
{
	BUCKET	*bkt;
	unsigned char	buf[USER_RECORD_SZ];
	long	position;
	int		rval, cnt;

	bkt = (BUCKET *)&s_hashtbl[_HashCardData(CardData)];
	cnt = rval = 0;
	while(bkt->Next) {
		bkt = &s_buckets[bkt->Next-1];
		position = bkt->Index << USER_RECORD_SHIFT;
		fsSeek(f1, position, SEEK_SET);
		rval = fsRead(f1, buf, USER_RECORD_SZ);
		if(rval >= USER_RECORD_SZ && buf[0] && !n_memcmp(buf+CARDDATA_OFFSET, CardData, 9)) cnt++;
		else if(rval < 0) break;
		rval = 0;
	}
	if(rval >= 0) rval = cnt;
	return rval;
}

int userfsGetPINData(FP_USER *user, unsigned char *PINData)
{
	unsigned char	buf[USER_RECORD_SZ];
	int		rval;

	rval = fsFindRecord(f1, buf, USER_RECORD_SZ, PINData, 17, 4);
	if(rval > 0) {
		userDecodeID(&user->ID, buf+1);
		rval = userDecode(user, buf+4);
		fsSeek(f1, USER_RECORD_SZ, SEEK_CUR);
		rval = 1;
	}
	return rval;
}

int userfsGetEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[USER_RECORD_SZ];
	int		rval;

	rval = _HashIndexGet(nID, buf);
	if(rval > 0) {
		rval = USER_RECORD_SZ - 4;
		memcpy(data, buf+4, rval);
	}
	return rval;
}

// Return -1:System error 0:Full 1:New user 2:Exist user(Overwrite)
int userfsAddEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[USER_RECORD_SZ];
	long	position;
	int		rval, idx;
	
	rval = _HashIndexGet(nID, buf);
	if(rval > 0) {
		if(n_memcmp(buf+4, data, USER_RECORD_SZ-4)) {
			idx = rval - 1;
			_HashIndexRemove(idx, nID, buf+CARDDATA_OFFSET);
			memcpy(buf+4, data, USER_RECORD_SZ-4);
			position = idx << USER_RECORD_SHIFT;
			fsSeek(f1, position, SEEK_SET);
			flashWriteEnable();
			rval = fsWrite(f1, buf, USER_RECORD_SZ);
			flashWriteDisable();
			if(rval != USER_RECORD_SZ) rval = -1;
			else {
				_HashIndexAdd(idx, nID, buf+CARDDATA_OFFSET);
				rval = 2;
//cprintf("Overwrite Added(%d) Index=%d\n", nID, idx);
			}
		} else {
			rval = 2;
		}
	} else if(!rval) {
		buf[0] = OT_USER; IDtoPACK3(nID, buf+1); memcpy(buf+4, data, USER_RECORD_SZ-4);
		flashWriteEnable();
		rval = fsAddRecordNull(f1, buf, USER_RECORD_SZ);
		flashWriteDisable();
		if(rval > 0) {
			idx = (fsTell(f1) - USER_RECORD_SZ) >> USER_RECORD_SHIFT;
			_HashIndexAdd(idx, nID, buf+CARDDATA_OFFSET);
			rval = 1;
//cprintf("New Added(%d) Index=%d\n", nID, idx);	
		}
	}
	return rval;
}

int userfsReadBulk(unsigned char *UserData, int Size)
{
	int		rval;
	
	rval = fsReadRecord(f1, UserData, USER_RECORD_SZ, Size / USER_RECORD_SZ);
	if(rval > 0) rval *= USER_RECORD_SZ;
	return rval;
}

int userfsAddBulk(unsigned char *Data, int Size)
{
	unsigned char	*p;
	long	nID;
	int		rval, i, idx, count;

	fsSeek(f1, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f1, Data, Size);	
	flashWriteDisable();
	if(rval != Size) return -1;
	idx = (fsTell(f1) - Size) >> USER_RECORD_SHIFT;
	count = Size >> USER_RECORD_SHIFT; 
	for(i = 0, p = Data;i < count;i++, idx++, p += USER_RECORD_SZ) {
		userDecodeID(&nID, p+1);
		_HashIndexAdd(idx, nID, p+CARDDATA_OFFSET);
	}
	return 1;
}

int userfsRemove(long nID)
{
	unsigned char	dID[4], buf[USER_AR_RECORD_SZ];
	unsigned long	FPID;
	int		rval;

	userEncodeID(nID, dID);
	flashWriteEnable();
	rval = _HashIndexGetAR(nID, buf);
	if(rval > 0) {
		_HashIndexRemoveAR(rval-1, nID);
		buf[0] = 0x00;
		fsSeek(f8, -USER_AR_RECORD_SZ, SEEK_CUR);
		rval = fsWrite(f8, buf, 1);
//cprintf("Deleted(%d) = %d\n", nID, rval);
		if(rval != 1) rval = -1;
	}
	rval = fsDeleteRecord(f4, USER_TXN_RECORD_SZ, dID, 3);		// UserTxn
	rval = fsDeleteRecord(f3, USER_PHOTO_RECORD_SZ, dID, 3);	// UserPhoto
	rval = fsDeleteRecord(f2, USER_EX_RECORD_SZ, dID, 3);		// UserEx
	rval = _HashIndexGet(nID, buf);
	if(rval > 0) {
		_HashIndexRemove(rval-1, nID, buf+CARDDATA_OFFSET);
		buf[0] = 0x00;
		fsSeek(f1, -USER_RECORD_SZ, SEEK_CUR);
		rval = fsWrite(f1, buf, 1);
//cprintf("Deleted(%d) = %d\n", nID, rval);
		if(rval != 1) rval = -1;
	}
	flashWriteDisable();
	if(rval > 0) {
		userID2FPID(nID, &FPID);
		sfpRemoveTemplate(FPID);
		sfpRemoveTemplate(FPID+1);
	}
	return rval;
}

int userfsRemoveAll(void)
{
	int		rval[5];

	sfpRemoveAllTemplate();
	flashWriteEnable();
	fsSeek(f1, 0L, SEEK_SET);
	rval[0] = fsShift(f1, NULL, -1);
	fsSeek(f2, 0L, SEEK_SET);
	rval[1] = fsShift(f2, NULL, -1);
	fsSeek(f3, 0L, SEEK_SET);
	rval[2] = fsShift(f3, NULL, -1);
	fsSeek(f4, 0L, SEEK_SET);
	rval[3] = fsShift(f4, NULL, -1);
	fsSeek(f8, 0L, SEEK_SET);
	rval[4] = fsShift(f8, NULL, -1);
	flashWriteDisable();	
	_HashIndexRemoveAll();
	if(rval[0] < 0 || rval[1] < 0 || rval[2] < 0 || rval[3] < 0 || rval[4] < 0) return -1;
	else	return 1;
}

int userfsExist(long nID)
{
	unsigned char	buf[USER_RECORD_SZ];

	return _HashIndexGet(nID, buf);
}

int userfsGetCount(void)
{
	return fsGetRecordCount(f1, USER_RECORD_SZ);
}

#include "fsuser_fp.c"
#include "fsuser_ex.c"
#include "fsuser_photo.c"
#include "fsuser_txn.c"	
#include "fsevt.c"	
#include "fsuser_ar.c"
#include "fspno.c"
#include "fscamimg.c"
