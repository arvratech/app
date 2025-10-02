#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "msg.h"
#include "syscfg.h"
#include "sche.h"
#include "evt.h"
#include "func.h"
#include "dev.h"
#include "bio.h"
#include "ad.h"


void _AdSetDefault(ACCESS_DOOR_CFG *cfg)
{
    cfg->doorPulseTime			= 5;
    cfg->doorExPulseTime		= 20;
	cfg->lockType				= 0;
	cfg->eventDetectionEnable	= 0;
	cfg->timeDelay				= 0;
	cfg->timeDelayNormal		= 0;
	cfg->doorOpenTooLongEnable	= 0;
	cfg->doorOpenTooLongOC		= 0xff;
	cfg->doorOpenTooLongTime	= 10;
	cfg->forcedOpenEnable		= 0;
	cfg->forcedOpenOC			= 0xff;
	cfg->unlockScheID			= 0;
	cfg->lockScheID				= 0;
}

int _AdEncode(ACCESS_DOOR_CFG *cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	*p++ = cfg->doorPulseTime;
	*p++ = cfg->doorExPulseTime;
	*p++ = cfg->lockType;
	if(cfg->eventDetectionEnable) val = 0x01; else val = 0x00;
	*p++ = val;
	*p++ = cfg->timeDelay;
	*p++ = cfg->timeDelayNormal;
	if(cfg->doorOpenTooLongEnable) val = 0x01; else val = 0x00;
	if(cfg->forcedOpenEnable) val |= 0x02;
	*p++ = val;
	*p++ = cfg->doorOpenTooLongOC;
	*p++ = cfg->doorOpenTooLongTime;
	*p++ = cfg->forcedOpenOC;
	*p++ = cfg->unlockScheID;
	*p++ = cfg->lockScheID;
 	return p - (unsigned char *)buf;
}

int _AdDecode(ACCESS_DOOR_CFG *cfg, void *buf)
{
	unsigned char	*p, val;
	
	p = (unsigned char *)buf;
	cfg->doorPulseTime			= *p++;
	cfg->doorExPulseTime		= *p++;
	cfg->lockType				= *p++;
	val							= *p++;
	if(val & 0x01) cfg->eventDetectionEnable = 1; else cfg->eventDetectionEnable = 0;
	cfg->timeDelay				= *p++;
	cfg->timeDelayNormal		= *p++;
	val							= *p++;
	if(val & 0x01) cfg->doorOpenTooLongEnable = 1; else cfg->doorOpenTooLongEnable = 0;
	if(val & 0x02) cfg->forcedOpenEnable = 1; else cfg->forcedOpenEnable = 0;
	cfg->doorOpenTooLongOC		= *p++;
	cfg->doorOpenTooLongTime	= *p++;
	cfg->forcedOpenOC			= *p++;
	cfg->unlockScheID			= *p++;
	cfg->lockScheID				= *p++;
 	return p - (unsigned char *)buf;
}

int _AdValidate(ACCESS_DOOR_CFG *ad_cfg)
{
	return 1;
}

int adRequestToExitEnable(void *self)
{
	ACCESS_DOOR	*ad = self;

	return biType(0);
}

void adSetRequestToExitEnable(void *self, int enable)
{
	ACCESS_DOOR	*ad = self;
	int		type;

	if(enable) type = BI_TYPE_REQUEST_TO_EXIT;
	else	   type = BI_TYPE_GENERAL;
	biSetType(0, type);
}

int adDoorStatusEnable(void *self)
{
	ACCESS_DOOR	*ad = self;
	BINARY_INPUT	*bi;

	bi = bisGet(1);
	return biType(bi);
}

void adSetDoorStatusEnable(void *self, int enable)
{
	ACCESS_DOOR	*ad = self;
	BINARY_INPUT	*bi;
	int		type;

	bi = bisGet(1);
	if(enable) type = BI_TYPE_DOOR_STATUS;
	else	   type = BI_TYPE_GENERAL;
	biSetType(bi, type);
}

int adDoorPulseTime(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->doorPulseTime;
}

void adSetDoorPulseTime(void *self, int doorPulseTime)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->doorPulseTime = doorPulseTime;
}

int adDoorExPulseTime(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->doorExPulseTime;
}

void adSetDoorExPulseTime(void *self, int doorExPulseTime)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->doorExPulseTime = doorExPulseTime;
}

int adLockType(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->lockType;
}

void adSetLockType(void *self, int lockType)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->lockType = lockType;
}

int adEventDetectionEnable(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->eventDetectionEnable;
}

void adSetEventDetectionEnable(void *self, int enable)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->eventDetectionEnable = enable;
}

int adTimeDelay(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->timeDelay;
}

void adSetTimeDelay(void *self, int timeDelay)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->timeDelay = timeDelay;
}

int adTimeDelayNormal(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->timeDelayNormal;
}

void adSetTimeDelayNormal(void *self, int timeDelayNormal)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->timeDelayNormal = timeDelayNormal;
}

int adDoorOpenTooLongEnable(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->doorOpenTooLongEnable;
}

void adSetDoorOpenTooLongEnable(void *self, int enable)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->doorOpenTooLongEnable = enable;
}

int adDoorOpenTooLong(void *self)
{
	ACCESS_DOOR	*ad = self;

	if(adDoorStatusEnable(ad) && adDoorOpenTooLongEnable(ad)) return 1;
	else	return 0;
}

int adDoorOpenTooLongOC(void *self)
{
	ACCESS_DOOR	*ad = self;
	int		ocID;

	ocID = ad->cfg->doorOpenTooLongOC;
//	if(ocID >= MAX_OC_SZ) ocID = -1;
	return ocID;
}

void adSetDoorOpenTooLongOC(void *self, int ocID)
{
	ACCESS_DOOR	*ad = self;

//	if(ocID < 0 || ocID >= MAX_OC_SZ) ocID = 0xff;
	ad->cfg->doorOpenTooLongOC = ocID;
}

int adDoorOpenTooLongTime(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->doorOpenTooLongTime;
}

void adSetDoorOpenTooLongTime(void *self, int doorOpenTooLongTime)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->doorOpenTooLongTime = doorOpenTooLongTime;
}

int adForcedOpenEnable(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->forcedOpenEnable;
}

void adSetForcedOpenEnable(void *self, int enable)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->forcedOpenEnable = enable;
}

int adForcedOpen(void *self)
{
	ACCESS_DOOR	*ad = self;

	if(adDoorStatusEnable(ad) && adForcedOpenEnable(ad)) return 1;
	else	return 0;
}

int adForcedOpenOC(void *self)
{
	ACCESS_DOOR	*ad = self;
	int		ocID;

	ocID = ad->cfg->forcedOpenOC;
//	if(ocID >= MAX_OC_SZ) ocID = -1;
	return ocID;
}

void adSetForcedOpenOC(void *self, int ocID)
{
	ACCESS_DOOR	*ad = self;

//	if(ocID < 0 || ocID >= MAX_OC_SZ) ocID = 0xff;
	ad->cfg->forcedOpenOC = ocID;
}

int adUnlockScheID(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->unlockScheID;
}

void adSetUnlockScheID(void *self, int unlockScheID)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->unlockScheID = unlockScheID;
}

int adLockScheID(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->lockScheID;
}

void adSetLockScheID(void *self, int lockScheID)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->lockScheID = lockScheID;
}

int adOutput(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->output;
}

void adSetOutput(void *self, int ocID)
{
	ACCESS_DOOR	*ad = self;

	ad->output = ocID;
}

int adAlarm(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->alarm;
}

void adSetAlarm(void *self, int alarmID)
{
	ACCESS_DOOR	*ad = self;

	ad->alarm = alarmID;
}

int adPresentValue(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->presentValue;
}

void adSetPresentValue(void *self, int presentValue)
{
	ACCESS_DOOR	*ad = self;
	int		pv;

printf("%ld PresentValue: %d => %d\n", MS_TIMER, (int)ad->presentValue, presentValue);
	if(presentValue == S_NORMAL_UNLOCKED || presentValue != ad->presentValue) {
		ad->presentValue = presentValue;
		if(presentValue == S_NORMAL_LOCKED) pv = 0;
		else if(presentValue == S_NORMAL_UNLOCKED) pv = 5;
		else if(presentValue & 1) pv = 1;
		else	pv = 4;
		//adDoorCommand(pv);
	}
}

int adAlarmState(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->alarm;
}

// 0:Unknown 1:Closed 2:Opened
int adDoorStatus(void *self)
{
	ACCESS_DOOR	*ad = self;
	int		val;

//	if(adDoorStatusEnable(ad)) {
//		if(spislvstats[2] & 0x40) val = 2; else val = 1;
//	} else	val = 0;
val = 0;
	return val;
}

void adInitPresentValue(void *self)
{
	ACCESS_DOOR	*ad = self;

	ad->prevPV = ad->presentValue = S_NORMAL_LOCKED;
//	_Lock(ad);
	ad->alarm = 0;
}
