#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "prim.h"
#include "rtc.h"
#include "msg.h"
#include "syscfg.h"
#include "unit.h"
#include "bio.h"
#include "sche.h"
#include "evt.h"
#include "event.h"
#include "iodev.h"
#include "cr.h"
#include "alm.h"
#include "acad.h"

#include "acadcfg.c"

extern unsigned char ServerNetState;

unsigned long	adPulseTimer[2], adTimer[2];
int				adPulseTimeout[2], adTimeout[2];
unsigned char	adState[2];

static ACCESS_DOOR	*ads;
static void (*DoorStatusCallBack)(int adID);
static void (*DoorPVCallBack)(int adID);


void adsInit(ACCESS_DOOR *g_ads, int size)
{
	ACCESS_DOOR		*ad;	
	int		i;

	DoorStatusCallBack = DoorPVCallBack = NULL;
	adsSet(g_ads);
	for(i = 0, ad = g_ads;i < size;i++, ad++) {
		ad->ID		= i;
		ad->cfg		= &sys_cfg->AccessDoors[i];
		adReset(i);
	}
}

void adsSet(ACCESS_DOOR *g_ads)
{
	ads = g_ads;
}

ACCESS_DOOR *adsGetAt(int adID)
{
	return &ads[adID];
}

void adsSetCallBack(void (*StatusCallBack)(), void (*PVCallBack)())
{
	DoorStatusCallBack	= StatusCallBack;
	DoorPVCallBack		= PVCallBack;
}

void adsInitPresentValue(void)
{
	ACCESS_DOOR		*ad;
	int		i, boID;

	for(i = 0, ad = ads;i < MAX_AD_SZ;i++, ad++) {
		if(adIsEnable(i)) {
			boID = adGetLockOutput(i);
			boSetPresentValue(boID, 0);
			if(i > 1 && ad->cfg->AuxLockEnable) boSetPresentValue(boID+1, 0);
		}
		if(i < 2) {
			adState[i] = 0;
			adTimeout[i] = 0;
		}
	}
}

void adReset(int adID)
{
	ACCESS_DOOR		*ad;	

	ad = &ads[adID];
	ad->PresentValue = S_NORMAL_LOCKED;
	rtcGetDateTime(ad->ChangeOfStateTime);
	ad->alarmState = 0;
	memcpy(ad->DoorAlarmStateTime, ad->ChangeOfStateTime, 6);
}

int adGetLockType(int adID)
{
	ACCESS_DOOR		*ad;	

	ad = &ads[adID];
	return (int)ad->cfg->LockType;
}

void adSetLockType(int adID, int Type)
{
	ACCESS_DOOR		*ad;	

	ad = &ads[adID];
	if(Type) ad->cfg->LockType = 1;
	else	 ad->cfg->LockType = 0;
}

int adGetOpenTooLong(int adID)
{
	ACCESS_DOOR		*ad;

	ad = &ads[adID];
	return (int)ad->cfg->DoorOpenTooLongEnable;
}

void adSetOpenTooLong(int adID, int Enable)
{
	ACCESS_DOOR		*ad;	

	ad = &ads[adID];
	if(Enable) ad->cfg->DoorOpenTooLongEnable = 1;
	else	   ad->cfg->DoorOpenTooLongEnable = 0;
}

int adGetForcedOpen(int adID)
{
	ACCESS_DOOR		*ad;	

	ad = &ads[adID];
	return (int)ad->cfg->DoorForcedOpenEnable;
}

void adSetForcedOpen(int adID, int Enable)
{
	ACCESS_DOOR		*ad;	

	ad = &ads[adID];
	if(Enable) ad->cfg->DoorForcedOpenEnable = 1;
	else	   ad->cfg->DoorForcedOpenEnable = 0;
}

int adGetPresentValue(int adID)
{
	ACCESS_DOOR		*ad;	

	ad = &ads[adID];
	return (int)ad->PresentValue;
}

void adRequestToExit(int adID)
{
	ACCESS_DOOR		*ad;
unsigned char	ctm[8];
	int		PV;

	ad = &ads[adID];	
//	if(ad->PV > PV_UNLOCK || !ad->Priority) adSetPV(ad, PV_PULSE_UNLOCK, 0);
	PV = ad->PresentValue;
	if(PV == S_NORMAL_LOCKED || PV == S_NORMAL_UNLOCKED) {
		adSetPresentValue(adID, S_NORMAL_UNLOCKED);
//rtcGetDateTime(ctm);
//EventAdd(OT_ACCESS_DOOR, adID, E_NORMAL_UNLOCKED_REQUEST_TO_EXIT, ctm, NULL);
	}
}

#define AUTO_LOCK_TIMER		3
#define adSetTimer(adID, SecValue) { adTimer[adID] = TICK_TIMER; adTimeout[adID] = rtcSecond2Tick(SecValue); }
#define adZeroTimer(adID) { adTimeout[adID] = 0; }

void adSetPresentValue(int adID, int PresentValue)
{
	ACCESS_DOOR		*ad;	
	BINARY_INPUT	*bi;
	BINARY_OUTPUT	*bo;
	int		ID, DoorStatus;
	
	ad = &ads[adID];
	ad->PresentValue = PresentValue;
	rtcGetDateTime(ad->ChangeOfStateTime);
#ifndef STAND_ALONE
	if(unitGetType(adID) == 2 || adID > 1) {
		if(PresentValue & 1) DoorStatus = 1; else DoorStatus = 0;
		ID = adGetLockOutput(adID);
		bo = bosGetAt(ID); bo->PresentValue = DoorStatus;
		if(ad->cfg->AuxLockEnable) {
			ID = adGetAuxLockOutput(adID);		
			if(boIsEnable(ID)) {
				bo = bosGetAt(ID); bo->PresentValue = DoorStatus;
			}
		}
		if(unitGetType(adID) == 2) unitcrSetDoorPV(adID, PresentValue);
		else	unitSetDoorPV(adID, PresentValue);
	} else {
#endif
		if(PresentValue & 1) {
cprintf("%ld AD-%d: PV=%d\n", DS_TIMER, adID, PresentValue); 
			if(PresentValue == S_NORMAL_UNLOCKED) {
				adPulseTimer[adID] = TICK_TIMER; 
				if(ad->cfg->DoorPulseTime) adPulseTimeout[adID] = rtcSecond2Tick((int)ad->cfg->DoorPulseTime);
				else	adPulseTimeout[adID] = rtcDS2Tick(5);	// 0.5 sec
			}
			boSetPresentValue(adGetLockOutput(adID), 1);
			if(ad->cfg->AuxLockEnable) {
				ID = adGetAuxLockOutput(adID);		
				if(boIsEnable(ID)) boSetPresentValue(ID, 1);
			}
			adState[adID] = 0; adZeroTimer(adID);
//cprintf("%ld AD-%d: S=%d Unlock...\n", DS_TIMER, adID, (int)adState[adID]); 
			adAlarmChanged(adID);
		} else {
cprintf("%ld AD-%d: PV=%d\n", DS_TIMER, adID, PresentValue); 
			ID = adGetDoorStatusInput(adID);
			bi = bisGetAt(ID);
			if(ad->cfg->DoorStatusEnable) DoorStatus = biPV[ID] ^ bi->cfg->Polarity;
			else	DoorStatus = 0;
			if(DoorStatus) {
				if(!ad->cfg->LockType) {
					boSetPresentValue(adGetLockOutput(adID), 0);
					if(ad->cfg->AuxLockEnable) {
						ID = adGetAuxLockOutput(adID);		
						if(boIsEnable(ID)) boSetPresentValue(ID, 0);
					}
				}
				if(ad->cfg->DoorOpenTooLongEnable) {
					adState[adID] = 2; adSetTimer(adID, ad->cfg->DoorOpenTooLongTime);
//if(!ad->cfg->LockType) cprintf("%ld AD-%d: S=%d Lock...\n", DS_TIMER, adID, (int)adState[adID]); 
//cprintf("%ld AD-%d: S=%d OpenTooLongTime...\n", DS_TIMER, adID, (int)adState[adID]); 
				} else {
					if(ad->cfg->LockType) adState[adID] = 1;
					else	adState[adID] = 0;
					adZeroTimer(adID);
//if(!ad->cfg->LockType) cprintf("%ld AD-%d: S=%d Lock...\n", DS_TIMER, adID, (int)adState[adID]); 
//cprintf("%ld AD-%d: S=%d Timer=0...\n", DS_TIMER, adID, (int)adState[adID]); 
				}
			} else if(adTimeout[adID]) {
				adState[adID] = 1;
//cprintf("%ld AD-%d: S=%d...\n", DS_TIMER, adID, (int)adState[adID]); 
			} else {
				boSetPresentValue(adGetLockOutput(adID), 0);
				if(ad->cfg->AuxLockEnable) {
					ID = adGetAuxLockOutput(adID);		
					if(boIsEnable(ID)) boSetPresentValue(ID, 0);
				}
				adState[adID] = 0;
//cprintf("%ld AD-%d: S=%d Lock...\n", DS_TIMER, adID, (int)adState[adID]); 
			}
		}
#ifndef STAND_ALONE
	}
#endif
	if(DoorPVCallBack) DoorPVCallBack(adID);
	if(DoorStatusCallBack) DoorStatusCallBack(adID);
#ifndef STAND_ALONE
	if(unitGetType(adID) != 2 && crGetType(adID)) unitSetCruDoor(adID);
#endif
	if(ServerNetState > 2) adNotifyStatus(adID);
}

int adAlarmState(int adID)
{
	ACCESS_DOOR		*ad;

	ad = &ads[adID];
	return (int)ad->alarmState;
}

void adDoorOpened(int adID)
{
	ACCESS_DOOR		*ad;
	int		state;

cprintf("%ld AD-%d: S=%d Opened\n", DS_TIMER, adID, (int)adState[adID]); 
	ad = &ads[adID];
	state = adState[adID];
	if(state == 0) {
		if(!(ad->PresentValue & 1) && ad->cfg->DoorForcedOpenEnable) {
			if(ad->cfg->TimeDelay) {
				adSetTimer(adID, ad->cfg->TimeDelay);
//cprintf("%ld AD-%d: S=%d TimeDelay...\n", DS_TIMER, adID, (int)adState[adID]); 
			} else {
				adState[adID] = 5; adZeroTimer(adID); adAlarmChanged(adID);	// ForcedOpen
			}
		} else {
			adZeroTimer(adID);
//cprintf("%ld AD-%d: S=%d Timer=0\n", DS_TIMER, adID, (int)adState[adID]); 
		}
	} else if(state == 1) {
		if(ad->cfg->DoorOpenTooLongEnable) {
			adState[adID] = 2; adSetTimer(adID, ad->cfg->DoorOpenTooLongTime);
//cprintf("%ld AD-%d: S=%d OpenTooLongTime...\n", DS_TIMER, adID, (int)adState[adID]); 
		} else {
			adZeroTimer(adID);
//cprintf("%ld AD-%d: S=%d Timer=0\n", DS_TIMER, adID, (int)adState[adID]); 
		}			 
	} else if(state == 2) {
		adSetTimer(adID, ad->cfg->DoorOpenTooLongTime);
//cprintf("%ld AD-%d: S=%d OpenTooLongTime...\n", DS_TIMER, adID, (int)adState[adID]); 
	} else if(state == 3) {
		adSetTimer(adID, ad->cfg->TimeDelay);
//cprintf("%ld AD-%d: S=%D TimeDelay...\n", DS_TIMER, adID, (int)adState[adID]); 
	} else {
		adZeroTimer(adID);
//cprintf("%ld AD-%d: S=%D Timer=0\n", DS_TIMER, adID, (int)adState[adID]); 
	}
	if(DoorStatusCallBack) DoorStatusCallBack(adID);
#ifndef STAND_ALONE
	if(unitGetType(adID) != 2 && crGetType(adID)) unitSetCruDoor(adID);
#endif
	if(ServerNetState > 2) adNotifyStatus(adID);
}

void adDoorClosed(int adID)
{
	ACCESS_DOOR		*ad;
	int		state;

	ad = &ads[adID];
	state = adState[adID];
cprintf("%ld AD-%d: S=%d Closed\n", DS_TIMER, adID, (int)adState[adID]); 
	if(state == 0) {
		if((ad->PresentValue & 1) && ad->cfg->LockType) {
			adSetTimer(adID, AUTO_LOCK_TIMER);
//cprintf("%ld AD-%d: S=%d AutoLockTimer...\n", DS_TIMER, adID, (int)adState[adID]); 
		} else {
			adZeroTimer(adID);		
//cprintf("%ld AD-%d: S=%d Timer=0...\n", DS_TIMER, adID, (int)adState[adID]); 
		}
	} else if(state == 1) {
		adSetTimer(adID, AUTO_LOCK_TIMER);
//cprintf("%ld AD-%d: S=%d AutoLockTimer...\n", DS_TIMER, adID, (int)adState[adID]); 
	} else if(state < 5) {
		if(ad->cfg->TimeDelayNormal) {
			adSetTimer(adID, ad->cfg->TimeDelayNormal);
//cprintf("%ld AD-%d: S=%d TimeDelayNormal...\n", DS_TIMER, adID, (int)adState[adID]); 
		} else {
			if(ad->cfg->LockType) {
				adState[adID] = 1; adSetTimer(adID, AUTO_LOCK_TIMER);
//cprintf("%ld AD-%d: S=%d AutoLockTimer...\n", DS_TIMER, adID, (int)adState[adID]); 
			} else {
				adState[adID] = 0; adZeroTimer(adID);
//cprintf("%ld AD-%d: S=%d Timer=0...\n", DS_TIMER, adID, (int)adState[adID]); 
			}
			adAlarmChanged(adID);
		}
	}
	if(DoorStatusCallBack) DoorStatusCallBack(adID);
#ifndef STAND_ALONE
	if(unitGetType(adID) != 2 && crGetType(adID)) unitSetCruDoor(adID);
#endif
	if(ServerNetState > 2) adNotifyStatus(adID);
}

void adProcessLocal(int adID)
{
	ACCESS_DOOR		*ad;
	BINARY_INPUT	*bi;
	int		state, ID, DoorStatus;
	
	ad = &ads[adID];
	state = adState[adID];
	if(ad->PresentValue == S_NORMAL_UNLOCKED & (TICK_TIMER-adPulseTimer[adID]) >= adPulseTimeout[adID]) {
		adSetPresentValue(adID, S_NORMAL_LOCKED);
	}
	if(adTimeout[adID] && (TICK_TIMER-adTimer[adID]) >= adTimeout[adID]) {
//cprintf("%ld AD-%d: S=%d Timeout\n", DS_TIMER, adID, (int)adState[adID]); 
		ID = adGetDoorStatusInput(adID);
		bi = bisGetAt(ID);
		if(ad->cfg->DoorStatusEnable) DoorStatus = biPV[ID] ^ bi->cfg->Polarity;
		else	DoorStatus = 0;
		if(state == 0) {
			if(DoorStatus) {
				adState[adID] = 5; adZeroTimer(adID); adAlarmChanged(adID);	// ForcedOpen
			} else {
				adZeroTimer(adID);
			}
		} else if(state == 1) {
			boSetPresentValue(adGetLockOutput(adID), 0);	
#ifdef STAND_ALONE
			if(ad->cfg->AuxLockEnable) {
				ID = adGetAuxLockOutput(adID);		
				if(boIsEnable(ID)) boSetPresentValue(ID, 0);
			}
#endif
			adState[adID] = 0; adZeroTimer(adID);
//cprintf("%ld AD-%d: S=%d Lock...\n", DS_TIMER, adID, (int)adState[adID]); 
//cprintf("%ld AD-%d: S=%d Timer=0...\n", DS_TIMER, adID, (int)adState[adID]); 
		} else if(state < 5 && !DoorStatus) {
			if(ad->cfg->LockType && ad->cfg->TimeDelayNormal < AUTO_LOCK_TIMER) {
				adState[adID] = 1; adSetTimer(adID, AUTO_LOCK_TIMER-ad->cfg->TimeDelayNormal);
//cprintf("%ld AD-%d: S=%d AutoLockTimer-TimeDelayNormal...\n", DS_TIMER, adID, (int)adState[adID]); 
			} else {
				if(ad->cfg->LockType) {
					boSetPresentValue(adGetLockOutput(adID), 0);
#ifdef STAND_ALONE
					if(ad->cfg->AuxLockEnable) {
						ID = adGetAuxLockOutput(adID);		
						if(boIsEnable(ID)) boSetPresentValue(ID, 0);
					}
#endif
				}
				adState[adID] = 0; adZeroTimer(adID);
//if(ad->cfg->LockType) cprintf("%ld AD-%d: S=%d Lock...\n", DS_TIMER, adID, (int)adState[adID]); 
//cprintf("%ld AD-%d: S=%d Timer=0...\n", DS_TIMER, adID, (int)adState[adID]); 
			}
			adAlarmChanged(adID);
		} else if(state == 2 && ad->cfg->TimeDelay) {
			adState[adID] = 3; adSetTimer(adID, ad->cfg->TimeDelay);
//cprintf("%ld AD-%d: S=%d TimeDelay...\n", DS_TIMER, adID, (int)adState[adID]); 
		} else if(state == 2 || state == 3) {
			adState[adID] = 4; adZeroTimer(adID); adAlarmChanged(adID);	// OpenTooLong
		} else {
			adZeroTimer(adID);
		}
		if(DoorStatusCallBack) DoorStatusCallBack(adID);
#ifndef STAND_ALONE
		if(unitGetType(adID) != 2 && crGetType(adID)) unitSetCruDoor(adID);
#endif
		if(ServerNetState > 2) adNotifyStatus(adID);
	}
}

#ifndef STAND_ALONE

void adRemoteDoorStatusChanged(int adID)
{
	if(DoorStatusCallBack) DoorStatusCallBack(adID);
	if(unitGetType(adID) != 2 && crGetType(adID)) unitSetCruDoor(adID);
	if(ServerNetState > 2) adNotifyStatus(adID);
}

void adRemotePVChanged(int adID)
{
	ACCESS_DOOR		*ad;

	ad = &ads[adID];
	rtcGetDateTime(ad->ChangeOfStateTime);
	if(adGetDoorPV(adID)) {
//cprintf("%ld AD-%d: PV=1...\n", DS_TIMER, adID); 
		if(ad->PresentValue == S_NORMAL_LOCKED) {
			ad->PresentValue = S_NORMAL_UNLOCKED;
			if(DoorPVCallBack) DoorPVCallBack(adID);
		}
	} else {
//cprintf("%ld AD-%d: PV=0...\n", DS_TIMER, adID); 
		if(ad->PresentValue == S_NORMAL_UNLOCKED) {
			ad->PresentValue = S_NORMAL_LOCKED;
			if(DoorPVCallBack) DoorPVCallBack(adID);
		}
	}
	if(unitGetType(adID) != 2 && crGetType(adID)) unitSetCruDoor(adID);
	if(ServerNetState > 2) adNotifyStatus(adID);
}

#endif

void adAlarmChanged(int adID)
{
	ACCESS_DOOR		*ad;
	int		val, oldAlarmState, newAlarmState;

	ad = &ads[adID];
	oldAlarmState = ad->alarmState;
	newAlarmState = adEvaluateAlarmState(adID);
	if(oldAlarmState == newAlarmState) return;
	ad->alarmState = newAlarmState;
	rtcGetDateTime(ad->DoorAlarmStateTime);
	if(newAlarmState == 1) {
cprintf("%lu AD-%d: alarmState=OpenTooLong...\n", DS_TIMER, adID); 
		EventAdd(OT_ACCESS_DOOR, adID, E_DOOR_OPEN_TOO_LONG, ad->DoorAlarmStateTime, NULL);
		if(DoorStatusCallBack) DoorStatusCallBack(adID);
		if(actIsEnable(adID) && ocExist(actGetOutputCommand(adID, 5))) actSetPresentValue(adID, 5);
	} else if(newAlarmState == 2) {
cprintf("%lu AD-%d: alarmState=ForcedOpen...\n", DS_TIMER, adID); 
		EventAdd(OT_ACCESS_DOOR, adID, E_DOOR_FORCED_OPEN, ad->DoorAlarmStateTime, NULL);
		if(DoorStatusCallBack) DoorStatusCallBack(adID);
		if(actIsEnable(adID) && ocExist(actGetOutputCommand(adID, 6))) actSetPresentValue(adID, 6);
	} else {
cprintf("%lu AD-%d: alarmState=Normal...\n", DS_TIMER, adID);
		EventAdd(OT_ACCESS_DOOR, (int)ad->ID, E_DOOR_NORMAL, ad->DoorAlarmStateTime, NULL);
		if(DoorStatusCallBack) DoorStatusCallBack(adID);
		if(oldAlarmState == 1) val = 5; else val = 6; 
		if(actIsEnable(adID) && ocExist(actGetOutputCommand(adID, val))) actSetPresentValue(adID, 0xff);
	}
#ifndef STAND_ALONE
	if(unitGetType(adID) == 2) unitcrSetState(adID);
	else if(crGetType(adID)) unitSetCruDoor(adID);
#endif
	if(ServerNetState > 2) adNotifyStatus(adID);
}

// 0:Opened 1:Closed 3:Null(Unused)
int adGetDoorStatus(int adID)
{
	ACCESS_DOOR		*ad;
	int		val;
	
	ad = adsGetAt(adID);
	if(!adIsEnable(adID) || !ad->cfg->DoorStatusEnable) val = 3;
	else	val = biGetPresentValue(adGetDoorStatusInput(adID));
	return val;
}

/* Return
 *	0 : No
 *  1 : Lock schedule
 *  2 : Unlock schedule
 *  3 : Lock+Unlock schedule 
 */
int adCheckLockSchedule(int adID, unsigned char *ctm)
{
	ACCESS_DOOR		*ad;
	long	ScheID;
	int		lock;

	ad = adsGetAt(adID);
	lock = 0;
	PACK1toID(&ad->cfg->LockSchedule, &ScheID);
	if(ScheID >= 0 && schePresentValue(ScheID, ctm)) lock = 1;
	PACK1toID(&ad->cfg->UnlockSchedule, &ScheID);
	if(ScheID >= 0&& schePresentValue(ScheID, ctm)) lock += 2;
	return lock;
}

void adProcessLockSchedule(int adID, unsigned char *ctm)
{
	ACCESS_DOOR		*ad;
	unsigned char	EventData[12];
	char	data[8];
	int		PresentValue, val, EventID, ScheID;

	ad = &ads[adID];
	val = adCheckLockSchedule(adID, ctm);
	EventID = 0;
	PresentValue = adGetPresentValue(adID);
	if(PresentValue == S_SCHEDULED_LOCKED && val != 1) {
		adSetPresentValue(adID, S_NORMAL_LOCKED);
		EventID = E_NORMAL_LOCKED_SCHEDULED_LOCK_END; ScheID = ad->cfg->LockSchedule;
	} else if(PresentValue == S_SCHEDULED_UNLOCKED && val < 2) {
		adSetPresentValue(adID, S_NORMAL_LOCKED);
		EventID = E_NORMAL_LOCKED_SCHEDULED_UNLOCK_END; ScheID = ad->cfg->UnlockSchedule;
	}
	if(EventID > 0) {
		sprintf(data, "%d", (int)ScheID); digits2card(data, EventData);	
		EventAdd(OT_ACCESS_DOOR, adID, EventID, ctm, EventData);
	}
	EventID = 0;
	PresentValue = adGetPresentValue(adID);	
	if(val > 1 && PresentValue < S_SCHEDULED_UNLOCKED) {
		adSetPresentValue(adID, S_SCHEDULED_UNLOCKED);
		EventID = E_SCHEDULED_UNLOCKED; ScheID = ad->cfg->UnlockSchedule;
	} else if(val == 1 && PresentValue < S_SCHEDULED_LOCKED) {
		adSetPresentValue(adID, S_SCHEDULED_LOCKED);
		EventID = E_SCHEDULED_LOCKED; ScheID = ad->cfg->LockSchedule;
	}
	if(EventID > 0) {
		sprintf(data, "%d", (int)ScheID); digits2card(data, EventData);	
		EventAdd(OT_ACCESS_DOOR, adID, EventID, ctm, EventData);
	}
}

int adEncodeShortStatus(int adID, void *buf)
{
	ACCESS_DOOR		*ad;
	unsigned char	*p;
	unsigned char	c, val;

	ad = &ads[adID];
	p = (unsigned char *)buf;
	if(adIsEnable(adID)) {
		c = 0x01;
		val = adAlarmState(adID);
		c |= val << 1;
		val = adGetPresentValue(adID);
		c |= val << 3;
	} else {
		c = 0x00;
	}
	if(!adIsEnable(adID) || !ad->cfg->DoorStatusEnable) val = 0;
	else	val = biGetPresentValue(adGetDoorStatusInput(adID)) + 1;
	c |= val << 6;	// 0:Unknown 1:Closed 2:Opened
	*p++ = c;
	return p - (unsigned char *)buf;
}


int adShortStatus(int apID)
{
	ACCESS_DOOR		*ad;
	unsigned char	c, val;
	int		adID;
	
	adID = apGetAssignedDoor(apID);
	ad = &ads[adID];
	if(adIsEnable(adID)) {
		c = 0x01;
		val = adAlarmState(adID);
		c |= val << 1;
		val = adGetPresentValue(adID);
		c |= val << 3;
	} else {
		c = 0x00;
	}
	if(!adIsEnable(adID) || !ad->cfg->DoorStatusEnable) val = 0;
	else	val = biGetPresentValue(adGetDoorStatusInput(adID)) + 1;
	c |= val << 6;	// 0:Unknown 1:Closed 2:Opened
	return (int)c;
}
	
int adEncodeStatus(int adID, unsigned char *Buffer)
{
	ACCESS_DOOR		*ad;
	unsigned char	*p;
	unsigned char	c, val;
	unsigned long	lVal;

	ad = &ads[adID];
	p = Buffer;
	*p++ = adGetReliability(adID);
	val = adAlarmState(adID);
	c = val << 1;
	val = adGetPresentValue(adID);
	c |= val << 3;
	if(!ad->cfg->DoorStatusEnable) val = 0;
	else	val = biGetPresentValue(adGetDoorStatusInput(adID)) + 1;
	c |= val << 6;	// 0:Unknown 1:Closed 2:Opened
	*p++ = c;
	datetime2longtime(ad->ChangeOfStateTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	datetime2longtime(ad->DoorAlarmStateTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	return p - Buffer;
}

#include "defs_obj.h"
#include "defs_pkt.h"
#include "net.h"

void _SendNet(NET *net);

void adNotifyStatus(int adID)
{
	NET		*net;
	unsigned char	*p, Buffer[64];
	
	net = (NET *)Buffer;
	p = net->Data;
	*p++ = OT_ACCESS_DOOR;
	*p++ = adID;
	p += adEncodeStatus(adID, p);
	net->DataLength = p - net->Data;	
	netCodeUnconfRequest(net, P_STATUS_NOTIFICATION);
	_SendNet(net);
}

int adsEncodeStatus(unsigned char *Buffer)
{
	unsigned char*p;
	int		i;

	p = Buffer;
	*p++ = OT_ACCESS_DOOR;
	for(i = 0;i < MAX_AD_SZ;i++)
		if(adIsEnable(i)) {
			*p++ = i; p += adEncodeStatus(i, p);
		}
	return p - Buffer;
}
