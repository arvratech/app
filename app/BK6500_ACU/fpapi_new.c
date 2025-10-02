#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "system_hdr.h"
#include "cotask.h"
#include "rtc.h"
#include "key.h"
#include "prim.h"
#include "fpapi_new.h"
#include "..\BootMonitor\fs.h"

#define MIN_PROP	512
#define MIN2		0x8000

extern unsigned long g_LevelTable[15];

int		sfpErrno;


int sfpInitTemplate(int fpIdentify)
{
	unsigned char	bUseSearch;
	int		rval;

	if(fpIdentify) bUseSearch = 1;
	else	 bUseSearch = 0;
	sfpErrno = DB_InitFPRecord(bUseSearch);
	if(sfpErrno) rval = -1;
	else	rval = 0;
	return rval; 
}

void fsSetNoRemove(int file);

void sfpExitTemplate(void)
{
	fsSetNoRemove(1);		// for forbid FPTemplates removal: fsShift(1, NULL, -1)
	DB_RemovAllFPRecord();	// call fsShift(1, NULL, -1), destroy 1:N DB
	fsSetNoRemove(0);
}

// -1:Error 0:Non-Matched 1:Matched
int sfpVerifyMatch(unsigned long fpId, unsigned char *tmpl, int securityLevel)
{
	FPRECORD	*FPRecord, _FPRecord;
	int		rval, score, pass_score;

	FPRecord = &_FPRecord;	
	sfpErrno = DB_GetFPRecord(FPRecord, fpId);
	if(sfpErrno == 3002) return 0;
	else if(sfpErrno) return -1;
	pass_score = g_LevelTable[securityLevel];
	rval = HFP_Match(tmpl, FPRecord->min1, &score, MIN_PROP);
//cprintf("HFP_Match=%d score=%d PassScore=%d\n", rval, score, pass_score);
	if(!rval && score < pass_score) rval = HFP_Match(tmpl, FPRecord->min2, &score, MIN_PROP);
	if(!rval) {
		sfpErrno = 0;
		if(score >= pass_score) rval = 1;
	} else {
		sfpErrno = FPMGR_ERROR_VERIFY_FAILED;		
		rval = -1;
	}
	return rval;
}

// -1:Error 0:Non-Matched 1:Matched
int sfpIdentifyMatch(unsigned char *tmpl, int securityLevel, unsigned long *fpId)
{
	FPRECORD	*fpRecord, _fpRecord;
	unsigned char	*p;
	unsigned short	candidate[7], cand_pos;
	unsigned long	id;
	int		rval, i, found, score, max_score;
  
	if(!DB_Initialized()) {
		sfpErrno = FPMGR_ERROR_DB_NOT_INIT;
		return -1;
	}
	rval = HFP_SearchFP(tmpl, candidate);
//cprintf("HFP_SearchFP=%d\n", rval);
	if(rval) {
		sfpErrno = FPMGR_ERROR_IDENTIFY_FAILED;
		return -1;
	}
	fpRecord = &_fpRecord;	
	max_score = g_LevelTable[securityLevel];
	found = 0;
	for(i = 0;i < 7;i++) {
		if(candidate[i] == 0xffff) continue;
		cand_pos = candidate[i] & ~MIN2;
		rval = DB_GetFPRecordPos(fpRecord, cand_pos);
		if(rval) break;
		if(candidate[i] & MIN2) p = fpRecord->min2; else p = fpRecord->min1;
		rval = HFP_Match(tmpl, p, &score, MIN_PROP);
//cprintf("HFP_Match=%d score=%d PassScore=%d\n", rval, score, pass_score);
		if(rval) break;
		if(score >= max_score) {
			found = 1; max_score = score; id = fpRecord->id;
		}
	}
	if(rval) {
		sfpErrno = FPMGR_ERROR_IDENTIFY_FAILED;
		rval = -1;
	} else {
		sfpErrno = 0;
		if(found) {
			*fpId = id; rval = 1;
		}
	} 
	return rval;
}

unsigned short FPCheckSum(void *buf, int size)
{
	unsigned short	*p, checksum;
	int		i;
	
	p = (unsigned short *)buf;
	size >>= 1;
	checksum = 0;
	for(i = 0;i < size;i++) checksum += p[i];
	return checksum;
}

// -1:Error 0:Not-found  1:OK
int sfpGetRawTemplate(unsigned long fpId, unsigned char *buf)
{
	FPRECORD	*fpRecord;
	int		rval;

	fpRecord = (FPRECORD *)buf;
	sfpErrno = DB_GetFPRecord(fpRecord, fpId);
//cprintf("DB_GetFPRecord=%d ID=[%ld-%ld]\n", rval, FPID>>1, FPID&1);
	if(sfpErrno == 3002) rval = 0;
	else if(sfpErrno) rval = -1;
	else	rval = 1;
	return rval;
}

// -1:Error 0:Not-found  1:OK
int sfpGetTemplate(unsigned long fpId, unsigned char *tmpl)
{
	FPRECORD	*fpRecord;
	unsigned long	buf[256];
	int		rval;

	fpRecord = (FPRECORD *)buf;
	sfpErrno = DB_GetFPRecord(fpRecord, fpId);
//if(!sfpErrno) cprintf("RAW ID=%08x CheckSum=%04x %04x\n", FPRecord->id, (int)FPRecord->checksum, (int)FPCheckSum(FPRecord, sizeof(FPRECORD)-2));
	if(sfpErrno == 3002) rval = 0;
	else if(sfpErrno) rval = -2;
	else	sfpErrno = 1;
	if(rval > 0 && tmpl) {
		memcpy(tmpl, fpRecord->min1, 400);
		memcpy(tmpl+400, fpRecord->min2, 400);		
	}
	return rval;
}

// -1:Error 1:OK
int sfpAddTemplate(unsigned long fpId, unsigned char *tmpl)
{
	FPRECORD	*fpRecord, _fpRecord;
	int		rval;

	fpRecord = &_fpRecord;	
	fpRecord->id = fpId;
	memcpy(fpRecord->min1, tmpl, 400);
	memcpy(fpRecord->min2, tmpl+400, 400);
	sfpErrno = DB_AddFPRecord(fpRecord, (unsigned char)1);
//tmplcnt++;
//cprintf("%d DB_AddFPRecord=%d ID=[%ld-%ld]\n", tmplcnt, rval, fpId>>1, fpId&1);
	if(sfpErrno) rval = -1;
	else	rval = 1;
	return rval;
}

// -1:Error 1:OK
int sfpRemoveTemplate(unsigned long fpId)
{
	int		rval;
	
	sfpErrno = DB_RemoveFPRecord(fpId);
//cprintf("DB_RemoveFPRecord=%d ID=[%ld-%ld]\n\n", rval, FPID>>1, FPID&1);
	if(sfpErrno) rval = -1;
	else	rval = 1;
	return rval;
}

// -1:Error 1:OK
int sfpRemoveAllTemplate(void)
{
	int		rval;

	sfpErrno = DB_RemovAllFPRecord();	// call flashWriteEnable(), call flashWriteDisable() in function
//cprintf("DB_RemovAllFPRecord=%d\n\n", rval);
	if(sfpErrno) rval = -1;
	else	rval = 1;
	return rval;
}

int sfpTemplateCount(void)
{
	int		rval;

	rval = DB_GetFPRecordNum();
//cprintf("DB_GetFPrecordNum=%d\n", rval);
	return rval;
}

int CheckTmplTopHeader(unsigned char *tmpl)
{
	if(tmpl[0] == 0xff) return -1;
	else	return 0;
}
