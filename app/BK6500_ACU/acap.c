#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "msg.h"
#include "syscfg.h"
#include "unit.h"
#include "sche.h"
#include "evt.h"
#include "cr.h"
#include "acad.h"
#include "acap.h"


#include "acapcfg.c"

static ACCESS_POINT	*aps;


void apsInit(ACCESS_POINT *g_aps, int size)
{
	ACCESS_POINT	*ap;	
	int		i;

	apsSet(g_aps);
	for(i = 0, ap = g_aps;i < size;i++, ap++) {
		ap->ID		= i;
		ap->cfg		= &sys_cfg->AccessPoints[i];
		apReset(i);
		ap->cr		= crsGetAt(i);
	}
}

void apsSet(ACCESS_POINT *g_aps)
{
	aps = g_aps;
}

ACCESS_POINT *apsGetAt(int apID)
{
	return &aps[apID];
}

void apReset(int apID)
{
	ACCESS_POINT	*ap;

	ap = &aps[apID];
	ap->AccessEvent		= 0;
	ap->Output			= 0;
	ap->State			= 0;
	ap->AccessEvent		= 0;
	ap->cr				= NULL;
	ap->MsgTimer		= 0L;
}

int apGetAuthorizationMode(int apID)
{
	ACCESS_POINT	*ap;

	ap = &aps[apID];
	return (int)ap->cfg->AuthorizationMode;
}

int apGetAccessEvent(int apID)
{
	ACCESS_POINT	*ap;

	ap = &aps[apID];
	return (int)ap->AccessEvent;
}

void apSetAccessEvent(int apID, int AccessEvent, unsigned char *ctm)
{
	ACCESS_POINT	*ap;

	ap = &aps[apID];
	ap->AccessEvent = (unsigned char)AccessEvent;
	memcpy(ap->AccessTime, ctm, 8);
}

int apGetState(int apID)
{
	ACCESS_POINT	*ap;

	ap = &aps[apID];
	return (int)ap->State;
}

int apEncodeStatus(int apID, void *buf)
{
	ACCESS_POINT	*ap;
	unsigned char	*p;
	unsigned char	c;

	ap = &aps[apID];
	p = (unsigned char *)buf;
//	if(apIsEnable((int)ap->ID)) {
//		c = 0x01;
//	} else {
		c = 0x00;
//	}
	*p++ = c;	
	return p - (unsigned char *)buf;
}
