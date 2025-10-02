long userfsTellFPTemplate(void)
{
	FS_FILE	*f;

	f = fsOpenAlreadyOpened(1);
	if(!f) return -1;
	return fsTell(f);
}

int userfsEofFPTemplate(void)
{
	FS_FILE	*f;

	f = fsOpenAlreadyOpened(1);
	if(!f) return -1;
	return fsEof(f);
}

int userfsGetFPTemplate(FP_USER *user, unsigned char *FPTemplate)
{
	unsigned long	FPID;
	unsigned char	*p;
	int		rval;

	user->FPFlag = 0;
	userID2FPID(user->ID, &FPID);
	p = FPTemplate;
	rval = sfpGetTemplate(FPID, p);
	if(rval > 0) {
		p += SG_STORED_TMPL_SZ;
		userSetFPData1(user);
	}
	FPID++;
	rval = sfpGetTemplate(FPID, p);
	if(rval > 0) userSetFPData2(user);
	return rval;
}

int userfsGetEncodedFPTemplate(unsigned long FPID, unsigned char *buf)
{
	int		rval;

//	rval = sfpGetRawTemplate(FPID, buf);
// modified because byte alignment error
	rval = sfpGetTemplate(FPID, buf+4);
	if(rval > 0) memcpy(buf, &FPID, 4);
	return rval;
}

int userfsAddEncodedFPTemplate(unsigned char *buf)
{
	unsigned long	FPID;
	int		rval;

	memcpy(&FPID, buf, 4);
	rval = sfpAddTemplate(FPID, buf+4);
	return rval;
}

int userfsAddFPTemplate(unsigned long FPID, unsigned char *buf)
{
	int		rval;

	rval = sfpAddTemplate(FPID, buf);
	return rval;
}

#include "rtc.h"
#include "fpmgr.h"

int userfsReadBulkFPTemplate(unsigned char *FPTemplate, int Size, long *NextPosition)
{
	FS_FILE		*f;
	unsigned char	*p;
	int		rval, count;

	f = fsOpenAlreadyOpened(1);
//cprintf("%ld Max=%d Position=%ld fsTell=%ld...\n", DS_TIMER, Size, *NextPosition, fsTell(f));
	count = Size; p = FPTemplate;
	fsSeek(f, *NextPosition, SEEK_SET);
	while(count >= 1024) {
		// 삭제된 지문은 3006(FPMGR_ERROR_INVALID_RECORD), 마지막에 3008(FPMGR_ERROR_END_FILE) 리턴
//cprintf("DB_GetRecord... Position=%d\n", fsTell(f));
		rval = DB_GetRecord((FPRECORD *)p, -1L);
//cprintf("DB_GetRecord=%d Position=%d\n", rval, fsTell(f));
		if(rval) {
			if(rval != FPMGR_ERROR_RECORD_CHECKSUM && rval != FPMGR_ERROR_INVALID_RECORD) break;
		} else {
			HFP_EncryptTamplate(p+4, 400, 2); HFP_EncryptTamplate(p+404, 400, 2);
			p += 804; count -= 804;
		}
	}
//cprintf("%ld gcnt=%d Size=%d fsTell=%d\n", DS_TIMER, gcnt, p - FPTemplate, fsTell(f));
	if(rval || userfsEofFPTemplate()) *NextPosition = 0L; else *NextPosition = fsTell(f);	
	return p - FPTemplate;
}

int userfsAddBulkFPTemplate(unsigned char *FPTemplate, int Size)
{
	FS_FILE	*f;
	FPRECORD	*FPRecord;
	unsigned char	*s, *se, *d, *d0, buf[2048];
	int		rval, count, size;

//cprintf("%ld fsAddBulkFPTemplate(Size=%d Num=%d)...\n", DS_TIMER, Size, Size/804);
	f = fsOpenAlreadyOpened(1);
	fsSeek(f, 0L, SEEK_END);
	flashWriteEnable();
	s = FPTemplate;	se = s + Size;
	d0 = buf; size = 2048;
	while(1) {
		count = 0; d = d0;
		while(s < se && count <= size-1024) {
			memcpy(d, s, 804);
			FPRecord = (FPRECORD *)d;
			HFP_DecryptTamplate(FPRecord->min1);
			HFP_DecryptTamplate(FPRecord->min2);
//memcpy(xxx2, FPRecord->min1, 400);
//HFP_EncryptTamplate(xxx2, 400, 2);
//if(n_memcmp(xxx1, xxx2, 400)) cprintf("Diff...\n"); else cprintf("Same...\n");
			memset(FPRecord->reserved, 0xff, 218);
			FPRecord->checksum = FPCheckSum(FPRecord, 1022);
			s += 804; d += 1024; count += 1024;
			//if(DS_TIMER > WdtResetTimer+7L) taskYield();
		}
		if(count <= 0) {
			rval = 0;
			break;
		}
		rval = fsWrite(f, d0, count);	
		if(rval != count) {
			rval = -1;
			break;
		}
		d0 = FPTemplate; size = s - d0;
	}
	flashWriteDisable();
//cprintf("%ld fsAddBulkFPTemplate rval=%d Hamster: %d/%d\n", DS_TIMER, rval, xxcnt, Size/804);	
	return rval;
}

int userfsRemoveFPTemplate(FP_USER *user)
{	
	unsigned long	FPID;
	int		rval, cnt;

	cnt = 0;
	userID2FPID(user->ID, &FPID);
	rval = sfpRemoveTemplate(FPID);
	if(rval > 0) cnt++;
	rval = sfpRemoveTemplate(FPID+1);
	if(rval > 0) cnt++;
//cprintf("userfsRemoveFPTemplate...%d\n", cnt);
	return cnt;
}

int userfsGetFPTemplateCount(void)
{
	int		rval;
/*
FS_FILE	*f;
FPRECORD	*FPRecord, _FPRecord;

f = fsOpenAlreadyOpened(1);
rval = fsSeek(f, 0L, SEEK_SET);
FPRecord = &_FPRecord;
while(1) {
	rval = fsRead(f, FPRecord, 1024);
	if(rval <= 0) break;
	if(FPRecord->id == 0xffffffff) cprintf("ID=Null\n");
	else	cprintf("ID=[%ld-%ld]\n", FPRecord->id>>1, FPRecord->id&1);
}
*/
	rval = DB_GetFPRecordNum();
//cprintf("DB_GetFPrecordNum=%d\n", rval);
	return rval;
}
