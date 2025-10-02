#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "schecfg.h"
#include "sche.h"
		

int holidayEncode(void *buf, unsigned char *holDate, int holTab)
{
	unsigned char	*p;
	
	p = (unsigned char *)buf;
	*p++ = 0x21;
	char2bcd(holDate[0], p); p++;
	char2bcd(holDate[1], p); p++;
	char2bcd(holDate[2], p); p++;
	*p++ = holTab;
	return p - (unsigned char *)buf;
}

int holidayDecode(void *buf, unsigned char *holDate, int *holTab)
{
	unsigned char	*p;
	
	p = (unsigned char *)buf;
	p++;
	bcd2char(p, holDate); p++;
	bcd2char(p, holDate+1); p++;
	bcd2char(p, holDate+2); p++;
	*holTab = *p++;
	return p - (unsigned char *)buf;
}

int holidayValidate(unsigned char *holDate, int holTab, unsigned char *curDate)
{
	if(holDate[0] != curDate[0] && holDate[0] != curDate[0]+1 || !date_validate(holDate) || (holTab & 0xf0)) return 0;
	else	return 1;
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
// Return	0= tms1 > tms2, invalid(except tms2=2400)
//          1= tms1 <= tms2
int tmsValidate(unsigned char *tms)
{
	if(tms[0] > 23 || tms[1] > 59 || tms[3] > 59) return 0; 
	if(tms[2] > 23 && (tms[2] != 24 || tms[3] != 0)) return 0; 
	if(tms[0] > tms[2]) return 0;		// hh1>hh2
	else if(tms[0] < tms[2]) return 1;	// hh1<hh2
	if(tms[1] < tms[3]) return 1;		// hh1=hh2 mm1<mm2
	else	return 0;					// hh1=hh2 mm1>=mm2
}

// Input	tms:hhmm-hhmm  tms1=tms[0:1] tms2=tms[2:3]  CurTime:hhmm
// Return	0=No 1=Yes(tm1 <= CurTime < tms2)
int tmsBound(unsigned char *tms, unsigned char *curTime)
{
//cprintf("%02d:%02d-%02d%02d %02d:%02d\n", (int)tms[0], (int)tms[1], (int)tms[2], (int)tms[3], (int)CurTime[0], (int)CurTime[1]);
	if(curTime[0] < tms[0] || curTime[0] > tms[2]) return 0;
	else if(curTime[0] == tms[0] && curTime[1] < tms[1]) return 0;
	else if(curTime[0] == tms[2] && curTime[1] >= tms[3]) return 0;
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

// Input	tms:hhmm-hhmm  tms1=tms[0:1] tms2=tms[2:3]  curTime:hhmm
// Return	0=No 1=Yes(tm1 <= curTime < tms2)
int tmsBoundEx(unsigned char *tms, unsigned char *curTime)
{
	unsigned char	tms2[4];
	int		rval;

	rval = tmsBound(tms, curTime);
	if(!rval && (tms[2] > 24 || tms[2] == 24 && tms[3] > 0)) {
		tms2[0] = tms2[1] = 0; tms2[2] = tms[2] - 24; tms2[3] = tms[3];
		rval = tmsBound(tms2, curTime);
	}
//cprintf("RVAL=%d %02d:%02d-%02d%02d %02d:%02d\n", rval, (int)tms[0], (int)tms[1], (int)tms[2], (int)tms[3], (int)curTime[0], (int)curTime[1]);
	return rval;
} 

int tmzEncode(void *buf, int tmzID, unsigned char *tmzTab)
{
	unsigned char	*p;
	int		i;
	
	p = (unsigned char *)buf;
	*p++ = 0x22;
	*p++ = tmzID;
 	for(i = 0;i < 16;i++, p++) char2bcd(tmzTab[i], p);
	return p - (unsigned char *)buf;
}

int tmzDecode(void *buf, int *tmzID, unsigned char *tmzTab)
{
	unsigned char	*p;
	int		i;
	
	p = (unsigned char *)buf;
	p++;
	*tmzID = *p++;
	for(i = 0;i < 16;i++, p++) bcd2char(p, tmzTab+i);
	return p - (unsigned char *)buf;
}

// Input	tmzID: 0=Never 1-32=TimeZoneID 99=Always
// Return	0=Invalid 1=Valid
int tmzValidateID(int tmzID)
{
	if(tmzID < 0 || tmzID > 32 && tmzID != ALWAYS_VAL) return 0;
	else	return 1;
}

// Input	TmxID: 1-32=TimeZoneID  TmzTab: 16Bytes
// Return	0=Invalid 1=Valid
int tmzValidateData(int tmzID, unsigned char *tmzTab)
{
	unsigned char	*p;
	int		i;
	
	if(tmzID <= 0 || tmzID > 32) return 0;
	if(tmzTab) {
		for(p = tmzTab, i = 0;i < 4;i++, p += 4)
			if(!tmsIsNull(p) && !tmsValidate(p)) return 0;
	}
	return 1;
}

// Input	tmzID: 0=Never 1-32=TimeZoneID 99=Always
// Return	0=Invalid 1=Valid
int tmzValidate(int tmzID)
{
	int		rval;

	if(tmzID < 0 || tmzID > 32 && tmzID != ALWAYS_VAL) rval = 0;
	else if(tmzID == 0 || tmzID == ALWAYS_VAL) rval = 1;	// Bug fix: 2012.7.10 if(tmzID == 0 || tmzID == 0) => if(tmzID == 0 || tmzID == ALWAYS_VAL)
	else	rval = tmzValidateData(tmzID, sche_cfg->timeZones[tmzID-1]);
	return rval;
}

// Input	tmzID: 0=Never 1-32=TimeZone ID 99=Always
// Return	0=No 1=Yes
int tmzBound(int tmzID, unsigned char *curTime)
{
	unsigned char	*tms;
	int		i, rval;
	
	if(tmzID == 0) return 0;
	else if(tmzID == ALWAYS_VAL) return 1;
	tms = sche_cfg->timeZones[tmzID-1];
	rval = 0;
	for(i = 0;i < 4;i++) {
		if(tmsBound(tms, curTime)) {
			rval = 1;
			break;
		}
		tms += 4;
	}
	return rval;
}

int scheEncode(void *buf, int scheID, unsigned char *scheTab)
{
	unsigned char	*p;
	int		i;
	
	p = (unsigned char *)buf;
	*p++ = 0x23;
	*p++ = scheID;
 	for(i = 0;i < 11;i++) *p++ = scheTab[i];
	return p - (unsigned char *)buf;	
}

int scheDecode(void *buf, int *scheID, unsigned char *scheTab)
{
	unsigned char	*p;
	int		i;
	
	p = (unsigned char *)buf;
	p++;
	*scheID = *p++;
	for(i = 0;i < 11;i++) scheTab[i] = *p++;
	return p - (unsigned char *)buf;
}

// Input	scheID: 0=Never 1-32=ScheduleID 99=Always
// Return	0=Invalid 1=Valid
int scheValidateID(int scheID)
{
	if(scheID < 0 || scheID > 32 && scheID != ALWAYS_VAL) return 0;
	else	return 1;
}

// Input	scheID: 1-32=ScheduleID  scheTab: 11Bytes
// Return	0=Invalid 1=Valid
int scheValidateData(int scheID, unsigned char *scheTab)
{
	int		i, tmzID;
	
	if(scheID <= 0 || scheID > 32) return 0;
	if(scheTab) {
		for(i = 0;i < 7;i++) {
			tmzID = scheTab[i];
			if(!tmzValidateID(tmzID)) return 0;
		}
		tmzID = scheTab[7];		// AlttmzID
		if(scheTab[9] & 0x1f) {	// AltWeek
			if(!tmzValidateID(tmzID) || !(scheTab[9] & 0xe0)) return 0;
		} else {
			if(!tmzValidateID(tmzID)) scheTab[7] = 0;
		}
		if(scheTab[10] > 4) return 0;
		tmzID = scheTab[8];	// HoltmzID
		if(scheTab[10]) {	// ScheholTable
			if(!tmzValidateID(tmzID)) return 0;
		} else {
			if(!tmzValidateID(tmzID)) scheTab[8] = 0;
		}
	}
	return 1;
}

// Input	scheID: 0=Never 1-32=ScheduleID 99=Always
// Return	0=Invalid 1=Valid
int scheValidate(int scheID)
{
	int		rval;

	if(scheID < 0 || scheID > 32 && scheID != ALWAYS_VAL) rval = 0;
	else if(scheID == 0 || scheID == ALWAYS_VAL) rval = 1;
	else	rval = scheValidateData(scheID, sche_cfg->schedules[scheID-1]);
	return rval;	
}

// Input	scheID: 0=Never 1-32=ScheduleID 99=Always
// Return	0=No 1=Yes
int scheBound(int scheID, unsigned char *curDateTime)
{
	unsigned char	*scheTab;
	int		rval, wday, alt_wday, alt_week, week_cycle, tmzID;

	if(scheID == 0) return 0;
	else if(scheID == ALWAYS_VAL) return 1;
	scheTab = sche_cfg->schedules[scheID-1];
	rval = 0;
	if(IsHoliday(curDateTime, (int)scheTab[10])) {
		tmzID = scheTab[8];
		if(tmzBound(tmzID, curDateTime+3)) rval = 1;
	} else {
		wday = get_week_day((int)curDateTime[0], (int)curDateTime[1], (int)curDateTime[2]);
		alt_wday = scheTab[9] >> 5; alt_week = scheTab[9] & 0x1f;
		week_cycle = 0x01 << ((curDateTime[2] - 1) / 7);
		if(alt_wday == wday+1 && (alt_week & week_cycle)) {
			tmzID = scheTab[7];
			if(tmzBound(tmzID, curDateTime+3)) rval = 1;
		} else {
			tmzID = scheTab[wday];
			if(tmzBound(tmzID, curDateTime+3)) rval = 1;
		}
	}
	return rval;
}

int IsHoliday(unsigned char *curDate, int holTable)
{
	HOLIDAY	*h;
	int		rval;
	
	if(curDate[0] == sche_cfg->holidays[0].year) h = &sche_cfg->holidays[0];
	else if(curDate[0] == sche_cfg->holidays[1].year) h = &sche_cfg->holidays[1];
	else	h = (HOLIDAY *)0;
	if(h && holTable >= 1 && holTable <= 4) rval = is_holiday(h->holTabs[holTable-1], (int)curDate[0], (int)curDate[1], (int)curDate[2]);
	else	rval = 0;
	return rval;
}

