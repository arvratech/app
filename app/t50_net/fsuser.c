#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "NSEnum.h"
#include "rtc.h"
#include "prim.h"
#include "mycrc.h"
#include "sysdep.h"
#include "fsprim.h"
#include "fsuser.h"

#define USR_PHOTO	"data/photo"
#define FN_USR		"data/usr.dat"
#define FN_USREX	"data/usrex.dat"
#define FN_EVT		"data/evt.dat"
#define FN_ACCT		"data/acct.dat"
#define FN_CAMIMG	"data/camevt.dat"

static FILE	*fp1, *fp2, *fp5, *fp6, *fp7;


static FILE *_OpenFile(char *fileName)
{
	FILE	*fp;
	struct stat	_stat;

	fp = fopen(fileName, "r+b");	
	if(!fp) fp = fopen(fileName, "w+");
	if(!fp) {
		printf("fopen(%s) error\n", fileName);
		return (FILE *)0;
	}
	stat(fileName, &_stat);
printf("file=%s is opened: size=%ld\n", fileName, _stat.st_size);
	return fp;
}

FILE *_RemoveFile(FILE *fp, char *fileName)
{
	fclose(fp);
	remove(fileName);
	return _OpenFile(fileName);
}

int _FsEof(FILE *fp)
{
    int     c;

	c = fgetc(fp);
	if(c < 0) c = 1;
	else {
		ungetc(c, fp);
		c = 0;
	}
	return c;
}

int fsOpenFiles(int MaxUserFileSize)
{
FILE	*fp;
int		rval;

	crc8Init();
	crc16Init();
	dirCreate(USR_PHOTO);
	fp1 = _OpenFile(FN_USR);		// User file
//printf("User : %d\n", fsFileLength(fp1));
	fp2 = _OpenFile(FN_USREX);		// User Extention file
	fp5 = _OpenFile(FN_EVT);		// Event file
	if(fp5) {
//		setvbuf(fp5, NULL, _IONBF, 0);	// default:4096
		fsOpenEvent();
	}
	fp6 = _OpenFile(FN_ACCT);		// Account file
	fp7 = _OpenFile(FN_CAMIMG);		// Cam image file
	if(fp7) {
		//setvbuf(fp7, camimgBuffer, _IOFBF, CAMIMG_BUF_SZ);
//printf("cam file bufSize=%d %d %d\n", fp7->__bufend - fp7->__bufstart, fp7->__bufpos, fp7->__bufread);
//		setvbuf(fp7, NULL, _IONBF, 0);	// default:4096
		fsOpenCamEvt();
	}
/*
fp = fopen("temp", "w+");
fwrite("1234567890", 1, 10, fp);
printf("pos=%d %d\n", fp->__bufpos-fp->__bufstart, fp->__bufread-fp->__bufstart);
fseek(fp, 0L, SEEK_SET);
printf("pos=%d %d\n", fp->__bufpos-fp->__bufstart, fp->__bufread-fp->__bufstart);
fflush(fp);
printf("pos=%d %d %d\n", fp->__bufpos-fp->__bufstart, fp->__bufread-fp->__bufstart);
fclose(fp);
*/
	if(fp1 && fp2 && fp5 && fp6 && fp7) return 0;
	else	return -1;
}

void fsCloseFiles(void)
{
	if(fp1) {
		fclose(fp1); fp1 = NULL;
	}
	if(fp2) {
		fclose(fp2); fp2 = NULL;
	}
	if(fp5) {
		fsCloseEvent();
		fclose(fp5); fp5 = NULL;
	}
	if(fp6) {
		fclose(fp6); fp6 = NULL;
	}
	if(fp7) {
		fsCloseCamEvt();
		fclose(fp7); fp7 = NULL;
	}
	sync();
printf("fsCloseFiles...\n");
}

unsigned char	TmpBuf[USER_PHOTO_RECORD_SZ];

int userfsSeek(long position)
{
	return fseek(fp1, position, SEEK_SET);
}

long userfsTell(void)
{
	return ftell(fp1);
}

int userfsEof(void)
{
	return _FsEof(fp1);
}

int userfsSeekAtIndex(int index)
{
	unsigned char   buf[USER_RECORD_SZ];
	int     rval, count;

	fseek(fp1, 0L, SEEK_SET);
	count = rval = 0;
	while(1) {
		if(count >= index) break;
		rval = fread(buf, 1, USER_RECORD_SZ, fp1);
		if(rval < USER_RECORD_SZ) break;
		if(buf[0]) count++;
	}
	if(rval >= 0) rval = count;
	return  rval;
}

int userfsRead(FP_USER *user)
{
	unsigned char	buf[USER_RECORD_SZ];
	long	id;
	int		rval;

	while((rval = fread(buf, 1, USER_RECORD_SZ, fp1)) == USER_RECORD_SZ)
		if(buf[0]) break;
	if(rval < 0) return -1;
	else if(rval < USER_RECORD_SZ) return 0;
	if(user) {
		userDecode(user, buf+4);
		userDecodeId(&id, buf+1); userSetId(user, id);
	}
	return 1;
}

int userfsReadAtIndex(FP_USER *user, int index)
{
	unsigned char	buf[USER_RECORD_SZ];
	long	id;
	int		rval, count;

	fseek(fp1, 0L, SEEK_SET);
	count = 0;
	while((rval = fread(buf, 1, USER_RECORD_SZ, fp1)) == USER_RECORD_SZ)
		if(buf[0]) {
			if(count >= index) break;
			count++;
		}
	if(rval < 0) return -1;
	else if(rval < USER_RECORD_SZ) return 0;
	if(user) {
		userDecode(user, buf+4);
		userDecodeId(&id, buf+1); userSetId(user, id);
	}
	return 1;
}

int userfsGet(FP_USER *user)
{
	unsigned char	dId[4], buf[USER_RECORD_SZ];
	int		rval;

	userEncodeId(user->id, dId);
	fseek(fp1, 0L, SEEK_SET);
	rval = fsFindRecord(fp1, buf, USER_RECORD_SZ, dId, 1, 3);
	if(rval <= 0) return rval;
	userDecode(user, buf + 4);
	return 1;
}

int userfsGetCardData(FP_USER *user, unsigned char *CardData)
{
	unsigned char	buf[USER_RECORD_SZ];
	long	id;
	int		rval;

	rval = fsFindRecord(fp1, buf, USER_RECORD_SZ, CardData, 21, 9);
	//rval = fsFindRecord(f1, buf, USER_RECORD_SZ, data, 22, 7);	// Facility
	if(rval > 0) {
		userDecodeId(&id, buf+1); userSetId(user, id);
		rval = userDecode(user, buf+4);
		fseek(fp1, USER_RECORD_SZ, SEEK_CUR);
		rval = 1;
	}
//if(rval > 0) cprintf("userfsGetCardData: id=%ld mode=%02x ar=%d\n", user->id, (int)user->AccessMode, (int)user->AccessRights);
//else if(rval < 0) cprintf("userfsGetCardData: file error: fsErrno=%d\n", fsErrno);
//else	cprintf("userfsGetCardData: No found\n");
	return rval;
}

int userfsGetPINData(FP_USER *user, unsigned char *PINData)
{
	unsigned char	buf[USER_RECORD_SZ];
	long	id;
	int		rval;

	rval = fsFindRecord(fp1, buf, USER_RECORD_SZ, PINData, 17, 4);
	if(rval > 0) {
		userDecodeId(&id, buf+1); userSetId(user, id);
		rval = userDecode(user, buf+4);
		fseek(fp1, USER_RECORD_SZ, SEEK_CUR);
		rval = 1;
	}
	return rval;
}

/*
int userfsGetEncoded(long id, unsigned char *data)
{
	unsigned char	dId[4];
	int		rval;

	IDtoPACK3(id, dId);
	rval = fsGetRecords(fp1, data, 1, USER_RECORD_SZ, dId, 3);
	if(rval > 0) rval = USER_RECORD_SZ - 4;
	return rval;
}
*/

// Return -1:System error 0:Full 1:New user 2:Exist user(Overwrite)
int userfsAddEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[USER_RECORD_SZ];
	int		rval;
	
	buf[0] = OT_USER; IDtoPACK3(nID, buf+1); memcpy(buf+4, data, USER_RECORD_SZ-4);
	rval = fsAddRecord(fp1, buf, USER_RECORD_SZ, 3);
	return rval;
}

// Return -1:System error 0:Full 1:New user 2:Exist user(Overwrite)
int userfsAdd(FP_USER *user)
{
	unsigned char	buf[USER_RECORD_SZ];

	userEncode(user, buf);
	return userfsAddEncoded(user->id, buf);
}

int userfsReadBulk(unsigned char *UserData, int Size)
{
	int		rval;
	
	rval = fsReadRecord(fp1, UserData, USER_RECORD_SZ, Size / USER_RECORD_SZ);
	if(rval > 0) rval *= USER_RECORD_SZ;
	return rval;
}

int userfsAddBulk(unsigned char *UserData, int Size)
{
	int		rval;

	fseek(fp1, 0L, SEEK_END);
	rval = fwrite(UserData, 1, Size, fp1);	
	if(rval != Size) return -1;
	else	return 0;
}

int userfsRemove(long id)
{
	unsigned char	dId[4];
	unsigned long	fpId;
	int		rval;

//cprintf("userfsRemove: %ld\n", id);
	userEncodeId(id, dId);
//	rval = fsDeleteRecord(fp8, USER_AR_RECORD_SZ, dId, 3);		// UserAccessRights
//	rval = fsDeleteRecord(fp4, USER_TXN_RECORD_SZ, dId, 3);		// UserTxn
//	rval = fsDeleteRecord(fp3, USER_PHOTO_RECORD_SZ, dId, 3);	// UserPhoto
	rval = fsDeleteRecord(fp2, USER_EX_RECORD_SZ, dId, 3);		// UserEx
	rval = fsDeleteRecord(fp1, USER_RECORD_SZ, dId, 3);			// User
	if(rval > 0) {
		userId2Fpid(id, &fpId);
//		sfpRemoveTemplate(fpId);
//		sfpRemoveTemplate(fpId+1);
	}
	return rval;
}

int userfsRemoveAll(void)
{
	int		rval[5];

//	sfpRemoveAllTemplate();
	fp1 = _RemoveFile(fp1, FN_USR);
	fp2 = _RemoveFile(fp2, FN_USREX);
//	fp5 = _RemoveFile(fp5, FN_EVT);
//	fp7 = _RemoveFile(fp7, FN_CAMIMG);
	return 0;
}

int userfsExist(long id)
{
	unsigned char	dId[4];
	int		rval;

	userEncodeId(id, dId);
	fseek(fp1, 0L, SEEK_SET);
	rval = fsFindRecord(fp1, NULL, USER_RECORD_SZ, dId, 1, 3);
	if(rval > 0) rval = 1;
	return rval;
}

int userfsGetCount(void)
{
	return fsGetRecordCount(fp1, USER_RECORD_SZ);
}

//#include "fsuser_fp.c"
#include "fsuser_ex.c"
//#include "fsuser_photo.c"
//#include "fsuser_txn.c"	
#include "fsevt.c"	
//#include "fsuser_ar.c"
#include "fsacct.c"
#include "fscamevt.c"

