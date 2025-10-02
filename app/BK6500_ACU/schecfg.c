#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs_obj.h"
#include "prim.h"
#include "fsar.h"
#include "sche.h"


int scheEncodeID(long nID, void *buf)
{
	IDtoPACK1(nID, buf);
	return 1;
}

int scheDecodeID(long *pID, void *buf)
{
	PACK1toID(buf, pID);
	return 1;
}

int scheValidateID(long nID)
{
	if(nID >= 2 && nID < 0xff) return 1;
	else	return 0;
}

// SpecialEvent 25 = Period(8) + EventPriority(1) + TimeRanges(16)
int seValidate(unsigned char *se)
{
	unsigned char	*p;
	int		i, rval;

	if(se[0] == 0) rval = ceValidate(se+1);
	else if(se[0] == 1 && se[1] == OT_CALENDAR && se[2] != 0xff) rval = 1;
	else	rval = 0;
	if(!rval) return 0;
	if(se[8] < 1 || se[8] > 16) return 0;
	for(i = 0, p = se+9;i < 4;i++, p += 4) {
		if(tmrIsNull(p)) rval = 1;	// count++;
		else {
			rval = tmrValidate(p);
			if(!rval) break;
		}
 	}
//	if(count >= 4) rval = 0;
	return rval;
}

// Return	1= if tmr[0-3] is null(0xff)
//			0= else
int tmrIsNull(unsigned char *tmr)
{
	if(tmr[0] == 0xff && tmr[1] == 0xff && tmr[2] == 0xff && tmr[3] == 0xff) return 1;
	else	return 0;
}

// Input	tmr:hhmm-hhmm  tmr1=tmr[0..1] tmr2=tmr[2..3]
// Return	0= tmr1 > tmr2, invalid(except tmr2=2400)
//          1= tmr1 <= tmr2
int tmrValidate(unsigned char *tmr)
{
	if(tmr[0] > 23 || tmr[1] > 59 || tmr[3] > 59 || tmr[2] > 24 || tmr[2] == 24 && tmr[3] 
		|| tmr[0] > tmr[2] || tmr[0] == tmr[2] && tmr[1] >= tmr[3]) return 0;	
	else	return 1;
}

// Input	tmr:hhmm-hhmm  tmr1=tms[0..1] tmr2=tms[2..3]  time:hhmm
// Return	-1:time < tmr1 0:tmr1 <= time < tmr2 1:time >= tmr2 
int tmrCompare(unsigned char *tmr, unsigned char *time)
{
//cprintf("%02d:%02d-%02d:%02d %02d:%02d\n", (int)tmr[0], (int)tmr[1], (int)tmr[2], (int)tmr[3], (int)time[0], (int)time[1]);
	int		rval;

	if(time[0] < tmr[0] || time[0] == tmr[0] && time[1] < tmr[1]) rval = -1;
	else if(time[0] > tmr[2] || time[0] == tmr[2] && time[1] >= tmr[3]) rval = 1;
	else	rval = 0;
	return rval;
}
