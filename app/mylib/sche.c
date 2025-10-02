#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "syscfg.h"
#include "sche.h"
		

int holidayEncode(void *buf, unsigned char *HolDate, int HolTab)
{
	unsigned char	*p;
	
	p = (unsigned char *)buf;
	*p++ = 0x21;
	char2bcd(HolDate[0], p); p++;
	char2bcd(HolDate[1], p); p++;
	char2bcd(HolDate[2], p); p++;
	*p++ = HolTab;
	return p - (unsigned char *)buf;
}

int holidayDecode(void *buf, unsigned char *HolDate, int *HolTab)
{
	unsigned char	*p;
	
	p = (unsigned char *)buf;
	p++;
	bcd2char(p, HolDate); p++;
	bcd2char(p, HolDate+1); p++;
	bcd2char(p, HolDate+2); p++;
	*HolTab = *p++;
	return p - (unsigned char *)buf;
}

int holidayValidate(unsigned char *HolDate, int HolTab, unsigned char *CurDate)
{
	if(HolDate[0] != CurDate[0] && HolDate[0] != CurDate[0]+1 || !date_validate(HolDate) || (HolTab & 0xf0)) return 0;
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
int tmsBound(unsigned char *tms, unsigned char *CurTime)
{
//cprintf("%02d:%02d-%02d%02d %02d:%02d\n", (int)tms[0], (int)tms[1], (int)tms[2], (int)tms[3], (int)CurTime[0], (int)CurTime[1]);
	if(CurTime[0] < tms[0] || CurTime[0] > tms[2]) return 0;
	else if(CurTime[0] == tms[0] && CurTime[1] < tms[1]) return 0;
	else if(CurTime[0] == tms[2] && CurTime[1] >= tms[3]) return 0;
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

int tmzEncode(void *buf, int TmzID, unsigned char *TmzTab)
{
	unsigned char	*p;
	int		i;
	
	p = (unsigned char *)buf;
	*p++ = 0x22;
	*p++ = TmzID;
 	for(i = 0;i < 16;i++, p++) char2bcd(TmzTab[i], p);
	return p - (unsigned char *)buf;	
}

int tmzDecode(void *buf, int *TmzID, unsigned char *TmzTab)
{
	unsigned char	*p;
	int		i;
	
	p = (unsigned char *)buf;
	p++;
	*TmzID = *p++;
	for(i = 0;i < 16;i++, p++) bcd2char(p, TmzTab+i);
	return p - (unsigned char *)buf;
}

// Input	TmzID: 0=Never 1-32=TimeZoneID 99=Always
// Return	0=Invalid 1=Valid
int tmzValidateID(int TmzID)
{
	if(TmzID < 0 || TmzID > 32 && TmzID != ALWAYS_VAL) return 0;
	else	return 1;
}

// Input	TmxID: 1-32=TimeZoneID  TmzTab: 16Bytes
// Return	0=Invalid 1=Valid
int tmzValidateData(int TmzID, unsigned char *TmzTab)
{
	unsigned char	*p;
	int		i;
	
	if(TmzID <= 0 || TmzID > 32) return 0;
	if(TmzTab) {
		for(p = TmzTab, i = 0;i < 4;i++, p += 4)
			if(!tmsIsNull(p) && !tmsValidate(p)) return 0;
	}
	return 1;
}

// Input	TmzID: 0=Never 1-32=TimeZoneID 99=Always
// Return	0=Invalid 1=Valid
int tmzValidate(int TmzID)
{
	int		rval;

	if(TmzID < 0 || TmzID > 32 && TmzID != ALWAYS_VAL) rval = 0;
	else if(TmzID == 0 || TmzID == ALWAYS_VAL) rval = 1;	// Bug fix: 2012.7.10 if(TmzID == 0 || TmzID == 0) => if(TmzID == 0 || TmzID == ALWAYS_VAL)
	else	rval = tmzValidateData(TmzID, sys_cfg->TimeZones[TmzID-1]);
	return rval;
}

// Input	TmzID: 0=Never 1-32=TimeZone ID 99=Always
// Return	0=No 1=Yes
int tmzBound(int TmzID, unsigned char *CurTime)
{
	unsigned char	*tms;
	int		i, rval;
	
	if(TmzID == 0) return 0;
	else if(TmzID == ALWAYS_VAL) return 1;
	tms = sys_cfg->TimeZones[TmzID-1];
	rval = 0;
	for(i = 0;i < 4;i++) {
		if(tmsBound(tms, CurTime)) {
			rval = 1;
			break;
		}
		tms += 4;
	}
	return rval;
}

int scheEncode(void *buf, int ScheID, unsigned char *ScheTab)
{
	unsigned char	*p;
	int		i;
	
	p = (unsigned char *)buf;
	*p++ = 0x23;
	*p++ = ScheID;
 	for(i = 0;i < 11;i++) *p++ = ScheTab[i];
	return p - (unsigned char *)buf;	
}

int scheDecode(void *buf, int *ScheID, unsigned char *ScheTab)
{
	unsigned char	*p;
	int		i;
	
	p = (unsigned char *)buf;
	p++;
	*ScheID = *p++;
	for(i = 0;i < 11;i++) ScheTab[i] = *p++;
	return p - (unsigned char *)buf;
}

// Input	ScheID: 0=Never 1-32=ScheduleID 99=Always
// Return	0=Invalid 1=Valid
int scheValidateID(int ScheID)
{
	if(ScheID < 0 || ScheID > 32 && ScheID != ALWAYS_VAL) return 0;
	else	return 1;
}

// Input	ScheID: 1-32=ScheduleID  ScheTab: 11Bytes
// Return	0=Invalid 1=Valid
int scheValidateData(int ScheID, unsigned char *ScheTab)
{
	int		i, TmzID;
	
	if(ScheID <= 0 || ScheID > 32) return 0;
	if(ScheTab) {
		for(i = 0;i < 7;i++) {
			TmzID = ScheTab[i];
			if(!tmzValidateID(TmzID)) return 0;
		}
		TmzID = ScheTab[7];		// AltTmzID
		if(ScheTab[9] & 0x1f) {	// AltWeek
			if(!tmzValidateID(TmzID) || !(ScheTab[9] & 0xe0)) return 0;
		} else {
			if(!tmzValidateID(TmzID)) ScheTab[7] = 0;
		}
		if(ScheTab[10] > 4) return 0;
		TmzID = ScheTab[8];	// HolTmzID
		if(ScheTab[10]) {	// ScheHolTable
			if(!tmzValidateID(TmzID)) return 0;
		} else {
			if(!tmzValidateID(TmzID)) ScheTab[8] = 0;
		}
	}
	return 1;
}

// Input	ScheID: 0=Never 1-32=ScheduleID 99=Always
// Return	0=Invalid 1=Valid
int scheValidate(int ScheID)
{
	int		rval;

	if(ScheID < 0 || ScheID > 32 && ScheID != ALWAYS_VAL) rval = 0;
	else if(ScheID == 0 || ScheID == ALWAYS_VAL) rval = 1;
	else	rval = scheValidateData(ScheID, sys_cfg->Schedules[ScheID-1]);
	return rval;	
}

// Input	ScheID: 0=Never 1-32=ScheduleID 99=Always
// Return	0=No 1=Yes
int scheBound(int ScheID, unsigned char *CurDateTime)
{
	unsigned char	*ScheTab;
	int		rval, wday, alt_wday, alt_week, week_cycle, TmzID;

	if(ScheID == 0) return 0;
	else if(ScheID == ALWAYS_VAL) return 1;
	ScheTab = sys_cfg->Schedules[ScheID-1];
	rval = 0;
	if(IsHoliday(CurDateTime, (int)ScheTab[10])) {
		TmzID = ScheTab[8];
		if(tmzBound(TmzID, CurDateTime+3)) rval = 1;
	} else {
		wday = get_week_day((int)CurDateTime[0], (int)CurDateTime[1], (int)CurDateTime[2]);
		alt_wday = ScheTab[9] >> 5; alt_week = ScheTab[9] & 0x1f;
		week_cycle = 0x01 << ((CurDateTime[2] - 1) / 7);
		if(alt_wday == wday+1 && (alt_week & week_cycle)) {
			TmzID = ScheTab[7];
			if(tmzBound(TmzID, CurDateTime+3)) rval = 1;
		} else {
			TmzID = ScheTab[wday];
			if(tmzBound(TmzID, CurDateTime+3)) rval = 1;
		}
	}
	return rval;
}

int IsHoliday(unsigned char *CurDate, int HolTable)
{
	HOLIDAY	*h;
	int		rval;
	
	if(CurDate[0] == sys_cfg->Holidays[0].Year) h = &sys_cfg->Holidays[0];
	else if(CurDate[0] == sys_cfg->Holidays[1].Year) h = &sys_cfg->Holidays[1];
	else	h = (HOLIDAY *)0;
	if(h && HolTable >= 1 && HolTable <= 4) rval = is_holiday(h->HolTabs[HolTable-1], (int)CurDate[0], (int)CurDate[1], (int)CurDate[2]);
	else	rval = 0;
	return rval;
}
