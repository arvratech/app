#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "unit.h"
#include "sche.h"
#include "evt.h"
#include "event.h"
#include "acad.h"
#include "iodev.h"
#include "user.h"
#include "cr.h"
#include "bio.h"

#include "biocfg.c"

extern unsigned char ServerNetState;

#ifdef STAND_ALONE
volatile unsigned char	biPV[5];
unsigned long	biTimer[5];
int				biTimeout[5];
#else
volatile unsigned char	biPV[10];
unsigned long	biTimer[10];
int				biTimeout[10];
#endif

static BINARY_INPUT		*bis;
static BINARY_OUTPUT	*bos;
static void (*PutInputCallBack)(int biID);
static void (*PutOutputCallBack)(int boID);


void bisInit(BINARY_INPUT *g_bis, int size)
{
	BINARY_INPUT	*bi;
	int		i, max;

	bisSet(g_bis);
	for(i = 0, bi = g_bis;i < size;i++, bi++) {
		bi->ID		= i;
		bi->cfg		= &sys_cfg->BinaryInputs[i];
		biReset(i);
	}
#ifdef STAND_ALONE
	max = 4;
#else
	max = 10;
#endif
	for(i = 0;i < max;i++) {
		biPV[i]		 = 0;
		biTimeout[i] = 0;
	}	
}

void bisSet(BINARY_INPUT *g_bis)
{
	bis = g_bis;
}

BINARY_INPUT *bisGetAt(int biID)
{
	return &bis[biID];
}

void bisSetPutCallBack(void (*PutCallBack)())
{
	PutInputCallBack = PutCallBack;
}

void biReset(int biID)
{
	BINARY_INPUT	*bi;

	bi = &bis[biID];
	rtcGetDateTime(bi->ChangeOfStateTime);
	memcpy(bi->AlarmStateTime, bi->ChangeOfStateTime, 6);
}

int biGetAlarmInhibit(int biID)
{
	BINARY_INPUT	*bi;

	bi = &bis[biID];
	return (int)bi->cfg->AlarmInhibit;
}

void biLocalPVChanged(int biID, int PV)
{
	BINARY_INPUT	*bi;
	ACCESS_DOOR		*ad;
	int		ID, rval;

	ID = biID;
	bi = bisGetAt(ID);
	PV ^= bi->cfg->Polarity;
//if(PV) cprintf("%ld BI-%d: On\n", DS_TIMER, ID);
//else   cprintf("%ld BI-%d: Off\n", DS_TIMER, ID);
	rtcGetDateTime(bi->ChangeOfStateTime);
	if(ID < 4) {
		if(adIsEnable(0)) {
			ad = adsGetAt(0);
			if(PV) {
				if(ID == 0 && ad->cfg->RequestToExitEnable) adRequestToExit(0);
				else if(ID == 1 && ad->cfg->DoorStatusEnable) adDoorOpened(0);
				else	ad = NULL;
			} else {		
				if(ID == 0 && ad->cfg->RequestToExitEnable) ;
				else if(ID == 1 && ad->cfg->DoorStatusEnable) adDoorClosed(0);
				else	ad = NULL;
			}
		} else	ad = NULL;
	} else {	
		if(adIsEnable(1)) {
			ad = adsGetAt(1);
			if(PV) {
				if(ID == 4 && ad->cfg->RequestToExitEnable) adRequestToExit(1);
				else if(ID == 5 && ad->cfg->DoorStatusEnable) adDoorOpened(1);
				else	ad = NULL;
			} else {		
				if(ID == 4 && ad->cfg->RequestToExitEnable) ;
				else if(ID == 5 && ad->cfg->DoorStatusEnable) adDoorClosed(1);
				else	ad = NULL;
			}
		} else	ad = NULL;
	}
	if(!ad) {
		rval = 0;
		if(biTimeout[ID]) biTimeout[ID] = 0;
		else if(PV && bi->cfg->TimeDelay) {
			biTimer[ID] = TICK_TIMER; biTimeout[ID] = rtcSecond2Tick((int)bi->cfg->TimeDelay);
		} else if(!PV && bi->cfg->TimeDelayNormal) {
			biTimer[ID] = TICK_TIMER; biTimeout[ID] = rtcSecond2Tick((int)bi->cfg->TimeDelayNormal);
		} else {
			rval = biAlarmChanged(biID);
		}
		if(rval == 0 && biGetAssignedFirePoint(biID) < 0)
			if(ServerNetState > 2) biNotifyStatus(biID);
	}
	if(PutInputCallBack) PutInputCallBack(ID);
}

void biProcessLocal(int biID)
{
	if(biTimeout[biID] && (TICK_TIMER-biTimer[biID]) > biTimeout[biID]) {
		biTimeout[biID] = 0;
		biAlarmChanged(biID);
	}
}

#ifndef STAND_ALONE

void biRemotePVChanged(int biID)
{
	BINARY_INPUT	*bi;
	int		adID, UnitID;

	bi = bisGetAt(biID);	
	rtcGetDateTime(bi->ChangeOfStateTime);
if(biGetPresentValue(biID)) cprintf("%ld BI-%d: On\n", DS_TIMER, biID);
else	cprintf("%ld BI-%d: Off\n", DS_TIMER, biID); 
	if(PutInputCallBack) PutInputCallBack(biID);
	UnitID = biGetUnit(biID);
	if(UnitID >= 0 && biGetType(biID) == 1) {
		adID = biGetAssignedDoor(biID);
		if(unitGetType(adID) == 2 || adID > 1) adRemoteDoorStatusChanged(adID);
	}
	if(biGetAssignedDoor(biID) < 0 && biGetAssignedFirePoint(biID) < 0)
		if(ServerNetState > 2) biNotifyStatus(biID);
}

#endif

#include "acad.h"
#include "alm.h"
#include "fire.h"

int biAlarmChanged(int biID)
{
	BINARY_INPUT	*bi;
	unsigned char	ctm[8];
	int		i, fpID, UnitID, PV, rval, type;

	rtcGetDateTime(ctm);
	fpID = biGetAssignedFirePoint(biID);
	UnitID = biGetUnit(biID);
	if(UnitID >= 0 && fpID >= 0) {
		PV = biGetAlarmState(biID);
		fpSetTrackingValue(fpID, PV);
		if(PV && !fpGetPresentValue(fpID)) {
			fpSetPresentValue(fpID, 1, ctm);
			for(i = 0;i < MAX_UNIT_HALF_SZ;i++)
				if(fzGetFirePoint(i, fpID) && !fzGetPresentValue(i)) {
					fzSetPresentValue(i, 1, ctm);
					if(fzGetSilenced(i)) fzSetSilenced(i, 0, ctm);
cprintf("%ld FireZone=%d: OffNormal\n", DS_TIMER, fpID);
					EventAdd(OT_FIRE_ZONE, i, E_FIRE_ALARM, ctm, NULL);
					if(ServerNetState > 2) fzNotifyStatus(i);
				}
		}
		rval = -1;
	} else if(!biGetAlarmInhibit(biID)) {
		bi = bisGetAt(biID);
		memcpy(bi->AlarmStateTime, ctm, 6);
if(biGetPresentValue(biID)) cprintf("%ld BI-%d: OffNormal\n", DS_TIMER, biID);
else cprintf("%ld BI-%d: Normal\n", DS_TIMER, biID);
		type = biGetType(biID); if(type > 3) type = 3;
		if(biGetPresentValue(biID)) {
			EventAdd(OT_BINARY_INPUT, biID, E_INPUT_ALARM, ctm, NULL);
			if(UnitID >= 0 && actIsEnable(UnitID) && ocExist(actGetOutputCommand(UnitID, type))) actSetPresentValue(UnitID, type);
		} else {
			EventAdd(OT_BINARY_INPUT, biID, E_INPUT_NORMAL, ctm, NULL);
			if(UnitID >= 0 && actIsEnable(UnitID) && ocExist(actGetOutputCommand(UnitID, type))) actSetPresentValue(UnitID, 0xff);
		}
		if(PutInputCallBack) PutInputCallBack(biID);
		if(ServerNetState > 2) biNotifyStatus(biID);
		rval = 1;
	} else {
		rval = 0;
	}
	return rval;
}

int biEncodeStatus(int biID, unsigned char *Buffer)
{
	BINARY_INPUT	*bi;
	unsigned char	*p;
	unsigned char	val;
	unsigned long	lVal;
	int		ID;

	bi = &bis[biID];
	p = Buffer;
#ifndef STAND_ALONE
	ID = biGetIOUnit(biID);
	if(ID >= 0) {
		val = iounitGetReliability(ID);
	} else {
		ID = biGetUnit(biID);
		if(unitGetType(ID) == 2) val = val = crGetReliability(ID);
		else	val = unitGetReliability(ID);
	}
#else
	val = 0;
#endif
	*p++ = val;	
	if(biGetPresentValue(biID)) val = 0x01; else val = 0x00;
	if(biGetAlarmState(biID)) val |= 0x02;
	*p++ = val;
	datetime2longtime(bi->ChangeOfStateTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	datetime2longtime(bi->AlarmStateTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	return p - Buffer;
}

#include "defs_obj.h"
#include "defs_pkt.h"
#include "net.h"

void _SendNet(NET *net);

void biNotifyStatus(int biID)
{
	NET		*net;
	unsigned char	*p, Buffer[64];
	
	net = (NET *)Buffer;
	p = net->Data;
	*p++ = OT_BINARY_INPUT;
	*p++ = biID;
	p += biEncodeStatus(biID, p);
	net->DataLength = p - net->Data;	
	netCodeUnconfRequest(net, P_STATUS_NOTIFICATION);
	_SendNet(net);
}

int bisEncodeStatus(unsigned char *Buffer)
{
	unsigned char*p;
	int		i;

	p = Buffer;
	*p++ = OT_BINARY_INPUT;
	for(i = 0;i < MAX_BI_SZ;i++)
		if(biIsEnable(i) && biGetAssignedDoor(i) < 0 && biGetAssignedFirePoint(i) < 0) {
			*p++ = i; p += biEncodeStatus(i, p);
		}
	for( ;i < MAX_BI_SZ+MAX_XBI_SZ;i++)
		if(biIsEnable(i)) {
			*p++ = i; p += biEncodeStatus(i, p);
		}
	return p - Buffer;
}

// Condition: biIsEnable=TRUE
// -1:No >=0:FirePointID
int biGetAssignedFirePoint(int biID)
{
	int		ID, type;

	ID = biID >> 3;
	type = biID & 0x07;
	if(type != 3 || !fpIsEnable(ID)) ID = -1;
	return ID;
}

void bosInit(BINARY_OUTPUT *g_bos, int size)
{
	BINARY_OUTPUT	*bo;
	int		i;

	bosSet(g_bos);
	for(i = 0, bo = g_bos;i < size;i++, bo++) {
		bo->ID		= i;
		bo->cfg		= &sys_cfg->BinaryOutputs[i];
		boReset(i);
	}
}

void bosSet(BINARY_OUTPUT *g_bos)
{
	bos = g_bos;
}

BINARY_OUTPUT *bosGetAt(int boID)
{
	return &bos[boID];
}

void bosSetPutCallBack(void (*PutCallBack)())
{
	PutOutputCallBack = PutCallBack;
}

void boReset(int boID)
{
	BINARY_OUTPUT	*bo;

	bo = &bos[boID];
	rtcGetDateTime(bo->ChangeOfStateTime);
}

int boGetPresentValue(int boID)
{
	BINARY_OUTPUT	*bo;

	bo = &bos[boID];
	return (int)bo->PresentValue;
}	

void boSetPresentValue(int boID, int PresentValue)
{
	BINARY_OUTPUT	*bo;
	int		ID;

	bo = &bos[boID];
	bo->PresentValue = PresentValue;
	rtcGetDateTime(bo->ChangeOfStateTime);
//if(PresentValue) cprintf("%ld BO-%d: On\n", DS_TIMER, boID);
//else	cprintf("%ld BO-%d: Off\n", DS_TIMER, boID);
#ifndef STAND_ALONE
	ID = boGetIOUnit(boID);
	if(ID >= 0) iounitSetPresentValue(ID, boID, PresentValue);
	else {
		ID = boGetUnit(boID);
		if(unitGetType(ID) == 2) unitcrSetOutputPV(boID, PresentValue);	
		else if(boID >= 4) 	unitSetOutputPV(boID, PresentValue);	
		else {
			if(bo->PresentValue ^ bo->cfg->Polarity) ioOnLock(boID);
			else	ioOffLock(boID);
		}		
	}
#else
	if(boID == 0) {
		if(bo->PresentValue ^ bo->cfg->Polarity) ioOnLock(boID);
		else	ioOffLock(boID);
	} else {
		if(bo->PresentValue ^ bo->cfg->Polarity) ioOnRelay(boID);
		else	ioOffRelay(boID);
	}	
#endif
	if(PutOutputCallBack) PutOutputCallBack(boID);
	if(boGetAssignedDoor(boID) < 0 && !boIsAssignedAlarmAction(boID))
		if(ServerNetState > 2) boNotifyStatus(boID);
}

int boEncodeStatus(int boID, unsigned char *Buffer)
{
	BINARY_OUTPUT	*bo;
	unsigned char	*p;
	unsigned char	val;
	unsigned long	lVal;
	int		ID;

	bo = &bos[boID];
	p = Buffer;
#ifndef STAND_ALONE
	ID = boGetUnit(boID);
	if(unitGetType(ID) == 2) val = crGetReliability(ID);
	else	val = unitGetReliability(ID);
#else
	val = 0;
#endif
	*p++ = val;		// Reliability	
	if(boGetPresentValue(boID)) val = 0x01; else val = 0x00;
	*p++ = val;
	datetime2longtime(bo->ChangeOfStateTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	return p - Buffer;
}

void boNotifyStatus(int boID)
{
	NET		*net;
	unsigned char	*p, Buffer[64];
	
	net = (NET *)Buffer;
	p = net->Data;
	*p++ = OT_BINARY_OUTPUT;
	*p++ = boID;
	p += boEncodeStatus(boID, p);
	net->DataLength = p - net->Data;	
	netCodeUnconfRequest(net, P_STATUS_NOTIFICATION);
	_SendNet(net);
}

int bosEncodeStatus(unsigned char *Buffer)
{
	unsigned char*p;
	int		i;

	p = Buffer;
	*p++ = OT_BINARY_OUTPUT;
	for(i = 0;i < MAX_BO_SZ;i++)
		if(boIsEnable(i) && boGetAssignedDoor(i) < 0 && !boIsAssignedAlarmAction(i)) {
			*p++ = i; p += boEncodeStatus(i, p);
		}
	for( ;i < MAX_BO_SZ+MAX_XBO_SZ;i++)
		if(boIsEnable(i)) {
			*p++ = i; p += boEncodeStatus(i, p);
		}
	return p - Buffer;
}
