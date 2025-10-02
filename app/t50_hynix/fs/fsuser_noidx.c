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

static FS_FILE	*f1, *f2, *f3, *f4, *f5, *f6, *f7, *f8;


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

int fsOpenFiles(int MaxUserFileSize, int MaxTmplFileSize, void *Buffer)
{
	int		rval, StartBlock;

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
	flashWriteEnable();
	f1 = _OpenFile(3);		// User file
	if(f1) fsSetLimit(f1, MaxUserFileSize);
	f2 = _OpenFile(4);		// UserEx file
	if(f2) fsSetLimit(f2, MaxUserFileSize);
	f3 = _OpenFile(5);		// UserPhoto file
	if(f3) fsSetLimit(f3, MaxUserFileSize / USER_RECORD_SZ * USER_PHOTO_RECORD_SZ);
	f4 = _OpenFile(6);		// UserTransaction file
	if(f4) fsSetLimit(f4, MaxUserFileSize / USER_RECORD_SZ * 16);
	f5 = _OpenFile(7);		// Event file
	f6 = _OpenFile(8);		// PhoneNo file
	if(f6) fsSetLimit(f6, GetPhoneNoFileSize());
	f7 = _OpenFile(9);		// CamImage file
	if(f7) fsSetLimit(f7, GetCamImageFileSize());
	f8 = _OpenFile(13);		// UserAccessRights file
	if(f8) fsSetLimit(f8, MaxUserFileSize / USER_RECORD_SZ * USER_AR_RECORD_SZ);
	flashWriteDisable();
	if(f1 && f2 && f3 && f4 && f5 && f6 && f7 && f8) return 0;
	else	return -1;
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
	unsigned char	dID[4], buf[USER_RECORD_SZ];
	int		rval;

	userEncodeID(user->ID, dID);
	fsSeek(f1, 0L, SEEK_SET);
	rval = fsFindRecord(f1, buf, USER_RECORD_SZ, dID, 1, 3);
	if(rval <= 0) return rval;
	userDecode(user, buf + 4);
	return 1;
}

int userfsGetCardData(FP_USER *user, unsigned char *CardData)
{
	unsigned char	buf[USER_RECORD_SZ];
	int		rval;

	rval = fsFindRecord(f1, buf, USER_RECORD_SZ, CardData, 21, 9);
	//rval = fsFindRecord(f1, buf, USER_RECORD_SZ, data, 22, 7);	// Facility
	if(rval > 0) {
		userDecodeID(&user->ID, buf+1);
		rval = userDecode(user, buf+4);
		fsSeek(f1, USER_RECORD_SZ, SEEK_CUR);
		rval = 1;
	}
//if(rval > 0) cprintf("userfsGetCardData: ID=%ld Mode=%02x AccessRights=%d\n", user->ID, (int)user->AccessMode, (int)user->AccessRights);
//else if(rval < 0) cprintf("userfsGetCardData: file error: fsErrno=%d\n", fsErrno);
//else	cprintf("userfsGetCardData: No found\n");
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
	unsigned char	dID[4];
	int		rval;

	IDtoPACK3(nID, dID);
	rval = fsGetRecords(f1, data, 1, USER_RECORD_SZ, dID, 3);
	if(rval > 0) rval = USER_RECORD_SZ - 4;
	return rval;
}

// Return -1:System error 0:Full 1:New user 2:Exist user(Overwrite)
int userfsAddEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[USER_RECORD_SZ];
	int		rval;
	
	buf[0] = OT_USER; IDtoPACK3(nID, buf+1); memcpy(buf+4, data, USER_RECORD_SZ-4);
	flashWriteEnable();
	rval = fsAddRecord(f1, buf, USER_RECORD_SZ, 3);
	flashWriteDisable();
	return rval;
}

int userfsReadBulk(unsigned char *UserData, int Size)
{
	int		rval;
	
	rval = fsReadRecord(f1, UserData, USER_RECORD_SZ, Size / USER_RECORD_SZ);
	if(rval > 0) rval *= USER_RECORD_SZ;
	return rval;
}

int userfsAddBulk(unsigned char *UserData, int Size)
{
	int		rval;

	fsSeek(f1, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f1, UserData, Size);	
	flashWriteDisable();
	if(rval != Size) return -1;
	else	return 0;
}

int userfsRemove(long nID)
{
	unsigned char	dID[4];
	unsigned long	FPID;
	int		rval;

//cprintf("userfsRemove: %ld\n", nID);
	userEncodeID(nID, dID);
	flashWriteEnable();
	rval = fsDeleteRecord(f8, USER_AR_RECORD_SZ, dID, 3);		// UserAccessRights
	rval = fsDeleteRecord(f4, USER_TXN_RECORD_SZ, dID, 3);		// UserTxn
	rval = fsDeleteRecord(f3, USER_PHOTO_RECORD_SZ, dID, 3);	// UserPhoto
	rval = fsDeleteRecord(f2, USER_EX_RECORD_SZ, dID, 3);		// UserEx
	rval = fsDeleteRecord(f1, USER_RECORD_SZ, dID, 3);			// User
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
	if(rval[0] < 0 || rval[1] < 0 || rval[2] < 0 || rval[3] < 0 || rval[4] < 0) return -1;
	else	return 1;
}

int userfsExist(long nID)
{
	unsigned char	dID[4];
	int		rval;

	userEncodeID(nID, dID);
	fsSeek(f1, 0L, SEEK_SET);
	rval = fsFindRecord(f1, NULL, USER_RECORD_SZ, dID, 1, 3);
	if(rval > 0) rval = 1;
	return rval;
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
