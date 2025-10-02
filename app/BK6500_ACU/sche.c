#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "syscfg.h"
#include "fsar.h"


#include "cal.c"
#include "schecfg.c"


int scheEncode(SCHEDULE *sche, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	memcpy(p, sche->EffectivePeriod, 6); p += 6;
	for(i = 0;i < 7;i++) {
		memcpy(p, sche->WeeklySchedule[i], 16); p += 16;
	}
	for(i = 0;i < sche->Count;i++) {
		memcpy(p, sche->ExceptionSchedule[i], 25); p += 25;
	}
	i = p - (unsigned char *)buf;
	if(i < SCHEDULE_BODY_SZ) memset(p, 0xff, SCHEDULE_BODY_SZ-i);
	return SCHEDULE_BODY_SZ;
}

int scheDecode(SCHEDULE *sche, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;	
	memcpy(sche->EffectivePeriod, p, 6); p += 6;
	for(i = 0;i < 7;i++) {
		memcpy(sche->WeeklySchedule[i], p, 16); p += 16;
	}
	for(i = 0;i < 5;i++) {
		if((*p) == 0xff) break;
		memcpy(sche->ExceptionSchedule[i], p, 25); p += 25;
	}
	sche->Count = i;
	return SCHEDULE_BODY_SZ;
}

int scheValidate(SCHEDULE *sche)
{
	unsigned char	*p;
	int		i, j, rval;

	rval = dtrValidate(sche->EffectivePeriod);
	if(!rval) return 0;
	for(i = 0;i < 7;i++) {
		p = sche->WeeklySchedule[i];
		for(j = 0;j < 4;j++, p += 4) {
			if(tmrIsNull(p)) rval = 1;	// count++;
			else {
				rval = tmrValidate(p);
				if(!rval) break;
			}
 		}
		if(!rval) break;
	}
	if(!rval) return 0;		
	for(i = 0;i < sche->Count;i++) {
		rval = seValidate(sche->ExceptionSchedule[i]); break;
		if(!rval) break;
	}
	return rval;
}

// Return	-1:Null  0:False  1:True
int schePresentValue(long scheID, unsigned char *ctm)
{
	SCHEDULE	*sche, _sche;
	unsigned char	*p, ses[5][2], tmp[2];
	int		i, j, rval, count;

//cprintf("ScheID=%ld %04d/%02d/%02d %02d:%02d\n", scheID, 2000+ctm[0], (int)ctm[1], (int)ctm[2], (int)ctm[3], (int)ctm[4]);
	if(scheID < 2) return (int)scheID;
	sche = &_sche;
	sche->ID = scheID;
	rval = schefsGet(sche);
//cprintf("schefsGet(%d)=%d\n", scheID, rval);
	if(rval <= 0) return 0;
	rval = dtrCompare(sche->EffectivePeriod, ctm);
//p = sche->EffectivePeriod;
//cprintf("dtrCompare: rval=%d EffetivePeriod=%02d/%02d/%02d..%02d/%02d/%02d\n", rval, (int)p[0], (int)p[1], (int)p[2], (int)p[3], (int)p[4], (int)p[5]);
	if(rval) return 0;
	count = sche->Count;
	for(i = 0;i < count;i++) {
		ses[i][0] = i;
		ses[i][1] = (sche->ExceptionSchedule[i][8] << 3) + i;
	}
//cprintf("count=%d\n", count);
	if(count > 1) {
		for(i = 0;i < count - 1;i++) {
			for(j = 0;j < count - 1;j++) {
				if(ses[j][1] > ses[j+1][1]) {
					tmp[0] = ses[j][0]; tmp[1] = ses[j][0];
					ses[j][0] = ses[j+1][0]; ses[j][1] = ses[j+1][1];
					ses[j+1][0] = tmp[0]; ses[j+1][1] = tmp[1];
				}
			}
		}
	}
	rval = 0;
	for(i = 0;i < count;i++) {
		j = ses[i][0];
		rval = sePeriodPresentValue(sche->ExceptionSchedule[j], ctm);
		if(rval) break;
	}
	if(rval) {
		p = &sche->ExceptionSchedule[j][9];
		rval = 1;
		for(i = 0;i < 4;i++, p += 4) {
			if(!tmrIsNull(p)) {
				rval = tmrCompare(p, ctm+3);
				if(!rval) break;
			}
 		}
//cprintf("ExceptionSchedule[%d] PV=%d\n", j, !rval);
		return !rval;
	}
//for(i = 0;i < 7;i++) {
//	p = sche->WeeklySchedule[i];	
//cprintf("Week=%d %02d:%02d..%02d:%02d %02d:%02d..%02d:%02d\n", i, (int)p[0], (int)p[1], (int)p[2], (int)p[3], (int)p[4], (int)p[5], (int)p[6], (int)p[7]);
//}
	j = get_week_day2((int)ctm[0], (int)ctm[1], (int)ctm[2]);
	p = sche->WeeklySchedule[j];
//cprintf("Week=%d ", j);
	for(i = 0;i < 4;i++, p += 4) {
		if(!tmrIsNull(p)) {
//cprintf("[%d] %02d:%02d..%02d:%02d", i, (int)p[0], (int)p[1], (int)p[2], (int)p[3]);
			rval = tmrCompare(p, ctm+3);
			if(!rval) break;
		}
	}
//cprintf("WeeklySchedule[%d] PV=%d\n", j, !rval);
	return !rval;
}

int sePeriodPresentValue(unsigned char *se, unsigned char *cdate)
{
	int		rval;

	if(se[0] == 0) rval = cePresentValue(se+1, cdate);
	else if(se[0] == 1) rval = calPresentValue((long)se[2], cdate);
	else	rval = 0;
	return rval;
}

// Input	tms:hhmm-hhmm  tms1=tms[0:1] tms2=tms[2:3]
// Return	1= if tms[0],tms[1],tms[2],tms[3] is zero
//			0= else        
int tmsIsNull(unsigned char *tms)
{
	if(tms[0] || tms[1] || tms[2] || tms[3]) return 0;
	else	return 1;
}

// Input	tms:hhmm-hhmm  tms1=tms[0:1] tms2=tms[2:3]
// Return	0=tms1 > tms2, invalid(except tms2=2400-2800)
//          1=tms1 <= tms2
int tmsValidateEx(unsigned char *tms)
{
	if(tms[0] > 23 || tms[1] > 59 || tms[3] > 59) return 0; 
	if(tms[2] > 28 || (tms[2] == 28 && tms[3] > 0)) return 0; 
	if(tms[0] > tms[2]) return 0;		// hh1>hh2
	else if(tms[0] < tms[2]) return 1;	// hh1<hh2
	if(tms[1] < tms[3]) return 1;		// hh1=hh2 mm1<mm2
	else	return 0;					// hh1=hh2 mm1>=mm2
}

// Input	tms:hhmm-hhmm  tms1=tms[0:1] tms2=tms[2:3]  CurTime:hhmm
// Return	0=No 1=Yes(tm1 <= CurTime < tms2)
int tmsBound(unsigned char *tms, unsigned char *CurTime)
{
//cprintf("%02d:%02d-%02d%02d %02d:%02d\n", (int)tms[0], (int)tms[1], (int)tms[2], (int)tms[3], (int)CurTime[0], (int)CurTime[1]);
	if(CurTime[0] < tms[0] || CurTime[0] > tms[2]) return 0;
	else if(CurTime[0] == tms[0] && CurTime[1] < tms[1]) return 0;
	else if(CurTime[0] == tms[2] && CurTime[1] >= tms[3]) return 0;
	else	return 1;
} 

// Input	tms:hhmm-hhmm  tms1=tms[0:1] tms2=tms[2:3]  CurTime:hhmm
// Return	0=No 1=Yes(tm1 <= CurTime < tms2)
int tmsBoundEx(unsigned char *tms, unsigned char *CurTime)
{
	unsigned char	tms2[4];
	int		rval;

	rval = tmsBound(tms, CurTime);
	if(!rval && (tms[2] > 24 || tms[2] == 24 && tms[3] > 0)) {
		tms2[0] = tms2[1] = 0; tms2[2] = tms[2] - 24; tms2[3] = tms[3];
		rval = tmsBound(tms2, CurTime);
	}
//cprintf("RVAL=%d %02d:%02d-%02d%02d %02d:%02d\n", rval, (int)tms[0], (int)tms[1], (int)tms[2], (int)tms[3], (int)CurTime[0], (int)CurTime[1]);
	return rval;
} 

