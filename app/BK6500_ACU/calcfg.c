#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "prim.h"
#include "pno.h"
#include "cal.h"


int calEncodeID(long nID, void *buf)
{
	IDtoPACK1(nID, buf);
	return 1;
}

int calDecodeID(long *pID, void *buf)
{
	PACK1toID(buf, pID);
	return 1;
}

int calValidateID(long nID)
{
	if(nID >= 0 && nID < 0xff) return 1;
	else	return 0;
}

int ceValidate(unsigned char *ce)
{
	int		rval;

	switch(ce[0]) {
	case 0:		rval = dateValidate(ce+1); break;
	case 1:		rval = dtrValidate(ce+1); break;
	case 2:		rval = weekndayValidate(ce+1); break;
	default:	rval = 0;
	}
	return rval;
}

int cePresentValue(unsigned char *ce, unsigned char *cdate)
{
	int		rval;

	switch(ce[0]) {
	case 0:		rval = dateCompare(ce+1, cdate); break;
	case 1:		rval = dtrCompare(ce+1, cdate); break;
	case 2:		rval = weekndayCompare(ce+1, cdate); break;
	default:	rval = 1;
	}
	if(rval) rval = 0; else rval = 1;
	return rval;
}

//#include "D:/BioTele/BKTELECOM-ACU/log.h"
int dateValidate(unsigned char *date)
{
	int		mm, mday;

//LOG(0, "Date: %d-%d-%d", (int)date[0], (int)date[1], (int)date[2]); 
	mm = date[1];
	if(date[0] == 0xff && mm == 2) mday = 29;
	else if(mm == 0xff || mm == 13 || mm == 14) mday = 31;
	else {
		mday = get_month_days(2000+date[0], mm);
		if(mday < 0) return 0;
	}
	if(date[2] != 32 && (!date[2] || date[2] > mday && date[2] < 0xff)) return 0;	
	else	return 1;
}

// yymmdd : yymmdd
int dateCompare(unsigned char *date, unsigned char *cdate)
{
	if((date[0] == 0xff || cdate[0] == date[0])
		&& (date[1] == 0xff || date[1] == 13 && (cdate[1] & 1) || date[1] == 14 && !(cdate[1] & 1) || cdate[1] == date[1])
		&& (date[2] == 0xff || cdate[2] == date[2])) return 0;
	else	return 1;
}

// Input	dtr:yymmdd-yymmdd
int dtrValidate(unsigned char *dtr)
{
	int		mm1, dd1, mm2, dd2, mday;

//LOG(0, "DateRange: %d-%d-%d : %d-%d-%d", (int)dtr[0], (int)dtr[1], (int)dtr[2], (int)dtr[3], (int)dtr[4], (int)dtr[5]); 
	mm1 = dtr[1];
	if(dtr[0] == 0xff && mm1 == 2) mday = 29;
	else if(mm1 == 0xff || mm1 == 13 || mm1 == 14) mday = 31;
	else {
		mday = get_month_days(2000+dtr[0], mm1);
		if(mday < 0) return 0;
	}
	if(dtr[2] == 32) dd1 = mday;
	else {
		dd1 = dtr[2];
		if(dd1 <= 0 || dd1 > mday && dd1 < 0xff) return 0;	
	}
	mm2 = dtr[4]; dd2 = dtr[5];
	if(dtr[3] == 0xff && mm2 == 2) mday = 29;
	else if(mm2 == 0xff) mday = 31;
	else if(mm2 == 13 || mm2 == 14) {
		if(mm2 != mm1) return 0;	
		mday = 31;
	} else {
		mday = get_month_days(2000+dtr[3], mm2);
		if(mday < 0) return 0;
	}
	if(dtr[5] == 32) dd2 = mday;
	else {
		dd2 = dtr[5];
		if(dd2 <= 0 || dd2 > mday && dd2 < 0xff) return 0;	
	}
	mday = 0;
	if(dtr[0] == 0xff || dtr[3] == 0xff || dtr[0] == dtr[3]) {
		if(mm1 == 0xff || mm2 == 0xff || mm1 == mm2) {
			if(dd1 == 0xff || dd2 == 0xff || dd1 <= dd2) mday = 1; 
		} else if(mm1 < mm2) mday = 1; 
	} else if(dtr[0] < dtr[3]) mday = 1; 
	return mday;
}

// Input	dtr:yymmdd-yymmdd  dtr1=dtr[0..2] dtr2=dtr[3..5] date:yymmdd
// Return	-1:date < dtr1 0:dtr1 <= date <= dtr2 1:date > dtr2 
int dtrCompare(unsigned char *dtr, unsigned char *date)
{
//cprintf("%02d/%02d/%02d-%02d/%02d/%02d %02d/%02d/%02d\n", (int)dtr[0], (int)dtr[1], (int)dtr[2], (int)dtr[3], (int)dtr[4], (int)dtr[5], (int)date[0], (int)date[1], (int)date[2]);
	int		rval;

	rval = -1;
	if(dtr[0] == 0xff || date[0] == dtr[0]) {
		if(dtr[1] == 0xff || date[1] == dtr[1]) {
			if(dtr[2] == 0xff || date[2] >= dtr[2]) rval = 0; 
		} else if(date[1] > dtr[1]) rval = 0; 
	} else if(date[0] > dtr[0]) rval = 0; 
	if(!rval) {
		if(dtr[3] == 0xff || date[0] == dtr[3]) {
			if(dtr[4] == 0xff || date[1] == dtr[4]) {
				if(dtr[5] != 0xff && date[2] > dtr[5]) rval = 1; 
			} else if(date[1] > dtr[4]) rval = 1; 
		} else if(date[0] > dtr[3]) rval = 1; 
	}
	return rval;
}

int weekndayValidate(unsigned char *wnd)
{
	if(!wnd[0] || wnd[0] > 14 && wnd[0] < 0xff
		|| !wnd[1] || wnd[1] > 6 && wnd[1] < 0xff
		|| !wnd[2] || wnd[2] > 7 && wnd[2] < 0xff) return 0;
	else	return 1;
}
//#include "../../BioTele/BKTELECOM-ACU/log.h"
// Month+WeekOfMonth+DayOfWeek : yymmdd
int weekndayCompare(unsigned char *wnd, unsigned char *cdate)
{
	int		s_day, e_day;

	if(wnd[1] == 6) {
		e_day = get_month_days(2000+cdate[0], (int)cdate[1]);
		s_day = e_day - 6;
	} else if(wnd[1] < 6) {
		s_day = 7 * (wnd[1]-1) + 1; e_day = s_day + 6;
	}
	if((wnd[0] == 0xff || wnd[0] == 13 && (cdate[1] & 1) || wnd[0] == 14 && !(cdate[1] & 1) || cdate[1] == wnd[0])
		&& (wnd[1] == 0xff || cdate[2] >= s_day && cdate[2] <= e_day)
		&& (wnd[2] == 0xff || wnd[2] == get_week_day2(cdate[0], (int)cdate[1], (int)cdate[2])+1)) return 0;
	else	return 1;
}
