#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "prim.h"
#include "fsar.h"

#include "calcfg.c"


int calEncode(CALENDAR *cal, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	for(i = 0;i < cal->Count;i++) {
		memcpy(p, cal->DateList[i], 7); p += 7;
	}
	i = p - (unsigned char *)buf;
	if(i < CALENDAR_BODY_SZ) memset(p, 0xff, CALENDAR_BODY_SZ-i);
	return CALENDAR_BODY_SZ;
}

int calDecode(CALENDAR *cal, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	for(i = 0;i < 18;i++) {
		if((*p) == 0xff) break;
		memcpy(cal->DateList[i], p, 7); p += 7;
	}
	cal->Count = i;
	return CALENDAR_BODY_SZ;
}

int calValidate(CALENDAR *cal)
{
	int		i, rval;

	rval = 1;
	for(i = 0;i < cal->Count;i++) {
		rval = ceValidate(cal->DateList[i]);
		if(!rval) break;
	}
	return rval;
}

int calPresentValue(long calID, unsigned char *cdate)
{
	CALENDAR	*cal, _cal;
	int		i, rval;

	cal = &_cal;
	cal->ID = calID;
	rval = calfsGet(cal);
	if(rval > 0) {
		rval = 0;
		for(i = 0;i < cal->Count;i++) {
			rval = cePresentValue(cal->DateList[i], cdate);
			if(rval) break;
		}
	} else	rval = 0;
//if(rval) cprintf("calPresentValue: ID=%ld PV=1 DateList[%d]\n", calID, i);
//else	cprintf("calPresentValue: ID=%ld PV=0\n", calID);
	return rval;
}
