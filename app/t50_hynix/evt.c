#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_pkt.h"
#include "prim.h"
#include "syscfg.h"
#include "fsuser.h"
#include "cam.h"
#include "slvnet.h"
#include "evt.h"

static int		evtReadSize;


void evtInit(void)
{
	evtReadSize = 0;
}

int evtIsGranted(int evtId)
{
	if(evtId >= 130 && evtId <= 134 || evtId == 150 || evtId == 155 
			|| evtId == 171 || evtId == 177) return 1;
	else	return 0;
}

int evtIsAccess(int evtId)
{
	if(evtId >= 130 && evtId < 160) return 1;
	else	return 0;
}

void evtMake(unsigned char *buf, int objType, int objId, int evtId, unsigned char *ctm, unsigned char *evtData)
{
	unsigned long	lVal;

	buf[0] = objType; buf[1] = objId; buf[2] = evtId;
	datetime2longtime(ctm, &lVal); LONGtoBYTE(lVal, buf+3);
	if(evtData && evtData[0]) memcpy(buf+7, evtData, 9);
	else	memset(buf+7, 0x00, 9);
}

int evtAddRaw(unsigned char *buf, int length)
{
	int		rval;

	rval = fsAddEvent(buf);
	if(psuBatPower(NULL)) {
		fsFlushEvent();
		sync();
	}
	return rval;
}

int evtRead(unsigned char *buf)
{
	int		size;

	size = fsReadEvent(buf);
	if(size < 0) size = 0;
	evtReadSize = size;
//printf("===evtRead...%d\n", evtReadSize);
	return size;
}

void evtClear(void)
{
	int		rval;

	if(evtReadSize > 0) {
printf("===evtClear...%d\n", evtReadSize);
		rval = fsShiftEvent(evtReadSize);
		if(psuBatPower(NULL)) {
			fsCloseEvent();
			sync();
		}
		evtReadSize = 0;
	}
}

void evtClearError(void)
{
	evtReadSize = 0;
}

void evtAdd(int objType, int objId, int evtId, unsigned char *ctm, unsigned char *evtData)
{
	unsigned char	buf[32];

	evtMake(buf, objType, objId, evtId, ctm, evtData);
	fsAddEvent(buf);
	if(psuBatPower(NULL)) {
		fsCloseEvent();
		sync();
	}
}

void EventAdd(int objType, int objId, int evtId, unsigned char *ctm, unsigned char *evtData)
{
	unsigned char	buf[32];

	evtMake(buf, objType, objId, evtId, ctm, evtData);
	fsAddEvent(buf);
	if(psuBatPower(NULL)) {
		fsCloseEvent();
		sync();
	}
}

int evtcamAdd(unsigned char *evtBuf)
{
	unsigned char	*buf;
	int		size, i;

	buf = camjpgBuffer(&size);
	if(size > 0) {
		fsAddCamEvt(evtBuf, buf, size);
		if(psuBatPower(NULL)) {
			fsCloseCamEvt();
			sync();
		}
	}
	return size;
}

int evtcamRead(unsigned char *buf)
{
	int		rval;

	rval = fsReadCamEvt(buf);
	return rval;
}

void evtcamClear(void)
{
	int		rval;

	rval = fsShiftCamEvt();
	if(psuBatPower(NULL)) {
		fsCloseCamEvt();
		sync();
	}
}

void evtcamFlush(void)
{
	fsFlushCamEvt();
}

