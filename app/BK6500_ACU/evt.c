#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_pkt.h"
#include "prim.h"
#include "syscfg.h"
#include "fsuser.h"
#include "evt.h"

#define _DEBUG		1

static int				_evtSize;
static unsigned char	_evtSync;


void evtInit(void)
{
	_evtSize = 0;
	_evtSync = 1;
}

int evtIsGranted(int evtId)
{
	if(evtId >= 130 && evtId <= 134 || evtId == 150 || evtId == 155 || evtId == 171 || evtId == 177) return 1;
	else	return 0;
}

int _ACU(int evtId)
{
	if(evtId >= 100 && evtId < 130) return 1;
	else	return 0;
}	

int evtIsLock(int evtId)
{
	if(evtId >= 100 && evtId < 130) return 1;
	else	return 0;
}

int evtIsAccess(int evtId)
{
	if(evtId >= 130 && evtId < 160) return 1;
	else	return 0;
}

int evtIsOther(int evtId)
{
	if(evtId < 100 || evtId >= 160) return 1;
	else	return 0;
}

int evtCheckMask(int evtId)
{
	if(evtIsLock(evtId) && (sys_cfg->Device.EventOption & MASK_LOCK_EVENT)
		|| evtIsAccess(evtId) && (sys_cfg->Device.EventOption & MASK_ACCESS_EVENT)
		|| evtIsOther(evtId) && (sys_cfg->Device.EventOption & MASK_OTHER_EVENT)) return -1;
	else	return 0;
}

static void _EvtMake(unsigned char *buf, int objType, int objId, int evtId, unsigned char *ctm, unsigned char *evtData)
{
	unsigned long	lVal;

	buf[0] = objType; buf[1] = objId; buf[2] = evtId;
	datetime2longtime(ctm, &lVal); LONGtoBYTE(lVal, buf+3);
	if(evtData && evtData[0]) memcpy(buf+7, evtData, 9);
	else	memset(buf+7, 0x00, 9);
}

extern unsigned char	ServerNetState;

void evtAdd(int objType, int objId, int evtId, unsigned char *ctm, unsigned char *evtData)
{
	unsigned char	buf[32];

#ifdef _DEBUG
cprintf("evtAdd: %d ot=0x%02x id=%d\n", evtId, objType, objId);
#endif
	_EvtMake(buf, objType, objId, evtId, ctm, evtData);
	fsAddEvent(buf);
	if(_evtSync) fsFlushEvent();
}

void evtAddRaw(unsigned char *buf, int length)
{
	unsigned char	*p;
	int		len;

#ifdef _DEBUG
cprintf("evtAddRaw: %d\n", length);
#endif
	p = buf; len = length;
	while(len >= 16) {
		fsAddEvent(p);
		p += 16; len -= 16;
	}
	if(_evtSync) fsFlushEvent();
}	

int evtRead(unsigned char *data)
{
	_evtSize = fsReadEvent(data);
#ifdef _DEBUG
cprintf("evtRead=%d\n", _evtSize);
#endif
	return _evtSize;
}

void evtClear(void)
{
	fsShiftEvent(_evtSize);
#ifdef _DEBUG
cprintf("evtClear=%d\n", _evtSize);
#endif
	_evtSize = 0;
}

void evtEnableSync(void)
{
	fsFlushEvent();
	_evtSync = 1;
}

void evtDisableSync(void)
{
	_evtSync = 0;
}

void evtEnableNet(void)
{
	_evtSize = 0;
}

void evtDisableNet(void)
{
	_evtSize = 0;
}

int evtIsNetPending(void)
{
	if(_evtSize > 0) return 1;
	else	return 0;
}

void evtClearNetPending(void)
{
	_evtSize - 0;
}

