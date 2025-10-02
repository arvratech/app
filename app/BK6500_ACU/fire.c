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
#include "bio.h"
#include "user.h"
#include "cr.h"
#include "alm.h"
#include "fire.h"

#include "firecfg.c"


static FIRE_POINT	*fps;
static FIRE_ZONE	*fzs;


void fpsInit(FIRE_POINT *g_fps, int size)
{
	FIRE_POINT	*fp;
	int		i;

	fpsSet(g_fps);
	for(i = 0, fp = g_fps;i < size;i++, fp++) {
		fp->ID	= i;
		fpReset(i);
	}
}

void fpsSet(FIRE_POINT *g_fps)
{
	fps = g_fps;
}

FIRE_POINT *fpsGetAt(int ID)
{
	return &fps[ID];
}

void fpReset(int ID)
{
	FIRE_POINT	*fp;

	fp = &fps[ID];
	fp->PresentValue = fp->TrackingValue = 0;
	rtcGetDateTime(fp->ChangeOfStateTime);
}

int fpIsEnable(int ID)
{
	FIRE_ZONE_CFG	*cfg;
	int		i;

	if(fpIsUsable(ID)) {
		for(i = 0, cfg = sys_cfg->FireZones;i < MAX_UNIT_HALF_SZ;i++, cfg++)
			if(cfg->Enable && BitMaskGet(cfg->FirePointBits, ID)) break;
		if(i < MAX_UNIT_HALF_SZ) i = 1; else i = 0;
	} else	i = 0;
	return i;
}

int fpEncodeStatus(int ID, void *buf)
{
	FIRE_POINT		*fp;
	unsigned char	*p;
	unsigned long	lVal;
	int		UnitID;

	fp = &fps[ID];
	p = (unsigned char *)buf;
	UnitID = ID << 1;
	if(unitGetType(UnitID) == 2) *p++ = crGetReliability(UnitID);
	else	*p++ = unitGetReliability(UnitID);
	*p++ = fp->PresentValue;
	*p++ = fp->TrackingValue;
	datetime2longtime(fp->ChangeOfStateTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	return p - (unsigned char *)buf;
}

int fpGetPresentValue(int ID)
{
	return (int)fps[ID].PresentValue;
}

void fpSetPresentValue(int ID, int PresentValue, unsigned char *ctm)
{
	FIRE_POINT	*fp;

	fp = &fps[ID];
	fp->PresentValue = (unsigned char)PresentValue;
	memcpy(fp->ChangeOfStateTime, ctm, 6);
}

int fpGetTrackingValue(int ID)
{
	return (int)fps[ID].TrackingValue;
}

void fpSetTrackingValue(int ID, int TrackingValue)
{
	FIRE_POINT	*fp;

	fp = &fps[ID];
	fp->TrackingValue = (unsigned char)TrackingValue;
}

void fzsInit(FIRE_ZONE *g_fzs, int size)
{
	FIRE_ZONE	*fz;
	int		i;

	fzsSet(g_fzs);
	for(i = 0, fz = g_fzs;i < size;i++, fz++) {
		fz->ID	= i;
		fz->cfg	= &sys_cfg->FireZones[i];
		fzReset(i);
	}
}

void fzsSet(FIRE_ZONE *g_fzs)
{
	fzs = g_fzs;
}

FIRE_ZONE *fzsGetAt(int ID)
{
	return &fzs[ID];
}

void fzReset(int ID)
{
	FIRE_ZONE	*fz;

	fz = &fzs[ID];
	fz->PresentValue = 0;
	rtcGetDateTime(fz->ChangeOfStateTime);
	fz->Silenced	 = 1;
}

int fzGetEnable(int fzID)
{
	int		enable;

	if(fzID >= 0 && fzID < MAX_UNIT_HALF_SZ) enable = sys_cfg->FireZones[fzID].Enable;
	else	enable = 0;
	return enable;
}

void fzSetEnable(int fzID, int Enable)
{
	if(fzID >= 0 && fzID < MAX_UNIT_HALF_SZ) sys_cfg->FireZones[fzID].Enable = (unsigned char)Enable;
}

int fzGetFirePoint(int ID, int fpID)
{
	FIRE_ZONE_CFG	*cfg;
	int		rval;

	cfg = &sys_cfg->FireZones[ID];
	if(cfg->Enable && BitMaskGet(cfg->FirePointBits, fpID)) rval = 1;
	else	rval = 0;
	return rval;
}

void fzSetFirePoint(int ID, int fpID, int Enable)
{
	FIRE_ZONE_CFG	*cfg;

	cfg = &sys_cfg->FireZones[ID];
	if(cfg->Enable)  {
		if(Enable) BitMaskSet(cfg->FirePointBits, fpID);
		else	   BitMaskClear(cfg->FirePointBits, fpID);
	}
}

int fzGetAccessDoor(int ID, int adID)
{
	FIRE_ZONE_CFG	*cfg;
	int		rval;
	
	cfg = &sys_cfg->FireZones[ID];
	if(cfg->Enable && BitMaskGet(cfg->AccessDoorBits, adID)) rval = 1;
	else	rval = 0;
	return rval;
}

void fzSetAccessDoor(int ID, int adID, int Enable)
{
	FIRE_ZONE_CFG	*cfg;
	
	cfg = &sys_cfg->FireZones[ID];
	if(cfg->Enable)  {
		if(Enable) BitMaskSet(cfg->AccessDoorBits, adID);
		else	   BitMaskClear(cfg->AccessDoorBits, adID);
	}
}

int fzGetAlarmAction(int ID, int actID)
{
	FIRE_ZONE_CFG	*cfg;
	int		rval;
	
	cfg = &sys_cfg->FireZones[ID];
	if(cfg->Enable && BitMaskGet(cfg->AlarmActionBits, actID)) rval = 1;
	else	rval = 0;
	return rval;
}

void fzSetAlarmAction(int ID, int actID, int Enable)
{
	FIRE_ZONE_CFG	*cfg;
	
	cfg = &sys_cfg->FireZones[ID];
	if(cfg->Enable)  {
		if(Enable) BitMaskSet(cfg->AlarmActionBits, actID);
		else	   BitMaskClear(cfg->AlarmActionBits, actID);
	}
}

int fzGetPresentValue(int ID)
{
	return (int)fzs[ID].PresentValue;
}

void fzSetPresentValue(int ID, int PresentValue, unsigned char *ctm)
{
	FIRE_ZONE	*fz;
	int		i;

	fz = &fzs[ID];
	fz->PresentValue = (unsigned char)PresentValue;
	memcpy(fz->ChangeOfStateTime, ctm, 6);
	if(!PresentValue) {
		for(i = 0;i < MAX_UNIT_HALF_SZ;i++)
			if(fzGetFirePoint(ID, i) && fpGetPresentValue(i) && !fpGetFireZonePresentValue(i)) fpSetPresentValue(i, 0, ctm);
	}
}

int fzGetTrackingValue(int ID)
{
	int		i;

	for(i = 0;i < MAX_UNIT_HALF_SZ;i++)
		if(fzGetFirePoint(ID, i) && fpGetTrackingValue(i)) break;
	if(i < MAX_UNIT_HALF_SZ) i = 1; else i = 0;
	return 0;
}

int fzGetSilenced(int ID)
{
	return (int)fzs[ID].Silenced;	
}

void fzSetSilenced(int ID, int Silenced, unsigned char *ctm)
{
	FIRE_ZONE	*fz;
	int		i, val, PV, NewPV, EventID;

	fz = &fzs[ID];
	fz->Silenced = (unsigned char)Silenced;
	if(Silenced) {
		for(i = 0;i < MAX_AD_SZ;i++)
			if(adIsEnable(i) && fzGetAccessDoor(ID, i) && !adGetFireZoneUnsilenced(i)) { 
				PV = adGetPresentValue(i);
				if(PV == S_LOCK_OPENED) {
cprintf("LockOpenRelease: adID=%d\n", i);
					val = adCheckLockSchedule(i, ctm);
					if(val > 1) {
						NewPV   = S_SCHEDULED_UNLOCKED;
						EventID = E_SCHEDULED_UNLOCKED_LOCK_DOWN_RELEASE;
					} else if(val == 1) {
						NewPV   = S_SCHEDULED_LOCKED;
						EventID = E_SCHEDULED_LOCKED_LOCK_DOWN_RELEASE;
					} else if(PV == S_FORCED_LOCKED) {
						NewPV   = S_FORCED_LOCKED;
						EventID = E_FORCED_LOCKED_LOCK_OPEN_RELEASE;
					} else if(PV == S_FORCED_UNLOCKED) {
						NewPV   = S_FORCED_UNLOCKED;
						EventID = E_FORCED_UNLOCKED_LOCK_OPEN_RELEASE;
					} else { 
						NewPV   = S_NORMAL_LOCKED;
						EventID = E_NORMAL_LOCKED_LOCK_OPEN_RELEASE;
					}
					adSetPresentValue(i, NewPV);
					//EventAdd(OT_ACCESS_DOOR, adID, EventID, ctm, data);
				}
			}
		for(i = 0;i < MAX_UNIT_SZ;i++)
			if(actIsEnable(i) && fzGetAlarmAction(ID, i) && ocExist(actGetOutputCommand(i, 4)) && !actGetFireZoneUnsilenced(i))
				actSetPresentValue(i, 0xff);
	} else {
		for(i = 0;i < MAX_AD_SZ;i++)
			if(adIsEnable(i) && fzGetAccessDoor(ID, i)) { 
				PV = adGetPresentValue(i);
				if(PV != S_LOCK_OPENED) {
					adSetPresentValue(i, S_LOCK_OPENED);
					//EventAdd(OT_ACCESS_DOOR, adID, EventID, ctm, data);
cprintf("LockOpened: adID=%d\n", i);
				}
			}		
		for(i = 0;i < MAX_UNIT_SZ;i++)
			if(actIsEnable(i) && fzGetAccessDoor(ID, i) && ocExist(actGetOutputCommand(i, 4)))
				actSetPresentValue(i, 4);
	}
}

int fzEncodeStatus(int ID, void *buf)
{
	FIRE_ZONE		*fz;
	unsigned char	*p;
	unsigned long	lVal;
	int		i, val;

	fz = &fzs[ID];
	p = (unsigned char *)buf;
	for(i = val = 0;i < MAX_UNIT_HALF_SZ;i++)
		if(fzGetFirePoint(ID, i)) {
			if(unitGetType(i<<1) == 2) val = crGetReliability(i<<1);
			else	val = unitGetReliability(i<<1);
			if(val) break;
		}
	*p++ = (unsigned char)val;
	*p++ = fzGetPresentValue(ID);
	*p++ = fzGetTrackingValue(ID);
	*p++ = fzGetSilenced(ID);
	datetime2longtime(fz->ChangeOfStateTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	return p - (unsigned char *)buf;
}

#include "defs_pkt.h"
#include "net.h"

void _SendNet(NET *net);

void fzNotifyStatus(int ID)
{
	NET		*net;
	unsigned char	*p, Buffer[64];
	
	net = (NET *)Buffer;
	p = net->Data;
	*p++ = OT_FIRE_ZONE;
	*p++ = ID;
	p += fzEncodeStatus(ID, p);
	net->DataLength = p - net->Data;	
	netCodeUnconfRequest(net, P_STATUS_NOTIFICATION);
	_SendNet(net);
}

int fzsEncodeStatus(unsigned char *Buffer)
{
	unsigned char*p;
	int		i;

	p = Buffer;
	*p++ = OT_FIRE_ZONE;
	for(i = 0;i < MAX_UNIT_HALF_SZ;i++)
		if(fzGetEnable(i)) {
			*p++ = i; p += fzEncodeStatus(i, p);
		}
	return p - Buffer;
}

int fpGetFireZonePresentValue(int fpID)
{
	int		i;

	for(i = 0;i < MAX_UNIT_HALF_SZ;i++)
		if(fzGetFirePoint(i, fpID) && fzGetPresentValue(i)) break;
	if(i < MAX_UNIT_HALF_SZ) i = 1; else i = 0;
	return i;
}

int adGetFireZoneUnsilenced(int adID)
{
	int		i;

	for(i = 0;i < MAX_UNIT_HALF_SZ;i++)
		if(fzGetAccessDoor(i, adID) && !fzGetSilenced(i)) break;
	if(i < MAX_UNIT_HALF_SZ) i = 1; else i = 0;
	return i;
}

int actGetFireZoneUnsilenced(int actID)
{
	int		i;

	for(i = 0;i < MAX_UNIT_HALF_SZ;i++)
		if(fzGetAlarmAction(i, actID) && !fzGetSilenced(i)) break;
	if(i < MAX_UNIT_HALF_SZ) i = 1; else i = 0;
	return i;
}
