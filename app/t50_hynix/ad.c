#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "msg.h"
#include "syscfg.h"
#include "sche.h"
#include "evt.h"
#include "func.h"
#include "dev.h"
#include "subdev.h"
#include "bio.h"
#include "slvmlib.h"
#include "slvm.h"
#include "ad.h"

extern ACCESS_DOOR	_AccessDoors[];


void adsInit(void)
{
	ACCESS_DOOR	*ad;
	int		i, size;

	if(devLocalUnitModel(NULL)) size = MAX_CH_SZ;
	else	  size = MAX_CH_HALF_SZ;
	for(i = 0, ad = _AccessDoors;i < size;i++, ad++) {
		ad->cls			= CLS_ACCESS_DOOR;
		ad->id			= i;
		//ad->presentValue	= S_NORMAL_LOCKED;
		ad->cfg			= &sys_cfg->accessDoors[i];
		ad->alarmState	= 0;
		ad->pv			= 0;	// Relinquish default
		ad->priority	= 8;	// Relinquish default
		ad->priorityMsk	= 0;
		ad->priorityVal	= 0;
		ad->hymo = ad->auxLock = 0;
	}
}

void *_AdsGet(int id)
{
	return &_AccessDoors[id];
}

void *adsGet(int id)
{
	void	*ad, *unit;
	int		model;

	model = devLocalUnitModel(NULL);
	if(!model && id == 0) ad = &_AccessDoors[id];
	else {
		unit = unitsGet(CH2UNIT(id));
		if(unit && unitIsNonNull(unit) && unitModel(unit) < 3) ad = &_AccessDoors[id];
		else	ad = NULL;
	}
	return ad;
}

int adsMaxSize(void)
{
	int		size;

	if(devLocalUnitModel(NULL)) size = MAX_CH_SZ;
	else	  size = MAX_CH_HALF_SZ;
	return size;
}

void _AdSetDefault(ACCESS_DOOR_CFG *cfg)
{
    cfg->doorPulseTime			= 5;
    cfg->doorExPulseTime		= 20;
	cfg->lockType				= 0;
	cfg->eventDetectionEnable	= 0;
	cfg->timeDelay				= 1;
	cfg->timeDelayNormal		= 1;
	cfg->doorOpenTooLongEnable	= 0;
	cfg->doorOpenTooLongTime	= 10;
	cfg->forcedOpenEnable		= 0;
	cfg->alarmAction			= 0;
	cfg->unlockScheID			= 0;
	cfg->lockScheID				= 0;
	cfg->rtePriority			= 8;
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
	*p++ = cfg->doorOpenTooLongTime;
	*p++ = cfg->alarmAction;
	*p++ = cfg->unlockScheID;
	*p++ = cfg->lockScheID;
	*p++ = cfg->rtePriority;
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
	cfg->doorOpenTooLongTime	= *p++;
	cfg->alarmAction			= *p++;
	cfg->unlockScheID			= *p++;
	cfg->lockScheID				= *p++;
	cfg->rtePriority			= *p++;
 	return p - (unsigned char *)buf;
}

int _AdValidate(ACCESS_DOOR_CFG *ad_cfg)
{
	return 1;
}

int adId(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->id;
}

BOOL adIsNonNull(void *self)
{
	ACCESS_DOOR	*ad = self;
	void	*cr, *bo1, *bo2;
	int		id;
	BOOL	bVal;

	id = ad->id;
	cr = crsGet(id);
	bo1 = bosGet(CH2SUBDEVBO(id));
	bo2 = bosGet(CH2UNITBO(id));
	if(cr && crIsPrimary(cr) && (bo1 && boIsDoorType(bo1) || bo2 && boIsDoorType(bo2))) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;;
}

BOOL adIsCreatable(void *self)
{
	ACCESS_DOOR	*ad = self;
	void	*cr, *bo1, *bo2;
	int		id;
	BOOL	bVal;

	id = ad->id;
	cr = crsGet(id);
	bo1 = bosGet(CH2SUBDEVBO(id));
	bo2 = bosGet(CH2UNITBO(id));
	if(cr && crIsCreatablePrimary(cr) && (bo1 && boIsCreatableDoor(bo1) || bo2 && boIsCreatableDoor(bo2))) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void *_BisGet(int id);
void *_BosGet(int id);

void _AdDelete(int id)
{
	ACCESS_DOOR	*ad;
	void	*bi, *bo, *cr;
	int		cid;

	ad = _AdsGet(id);
	cr = adPrimaryReader(ad);
	if(cr) crSetType(cr, -1);
	cr = adSecondaryReader(ad);
	if(cr && crIsSecondary(cr)) crSetType(cr, -1);
	cid = CH2SUBDEVBO(id);
	bo = bosGet(cid);
//printf("_AdDelete: id=%d bo=%x\n", cid, bo);
	if(bo) {
		if(boIsDoorType(bo)) boDelete(bo);
		bo = _BosGet(cid+1);
		if(boIsDoorType(bo)) boDelete(bo);
		cid = CH2SUBDEVBI(id);
		bi = _BisGet(cid);
		if(biIsDoorType(bi)) biDelete(bi);
		bi = _BisGet(cid+1);
		if(biIsDoorType(bi)) biDelete(bi);
	} 
	cid = CH2UNITBO(id);
	bo = bosGet(cid);
//printf("_AdDelete: id=%d bo=%x\n", cid, bo);
	if(bo) {
		if(boIsDoorType(bo)) boDelete(bo);
		bo = _BosGet(cid+1);
		if(boIsDoorType(bo)) boDelete(bo);
		cid = CH2UNITBI(id);
		bi = _BisGet(cid);
		if(biIsDoorType(bi)) biDelete(bi);
		bi = _BisGet(cid+1);
		if(biIsDoorType(bi)) biDelete(bi);
	}
}

void adCreate(void *self)
{
	ACCESS_DOOR	*ad = self;
	void	*cr, *bo, *bi;
	int		val, id, ids[2];

	id = ad->id;
//printf("adCreate...%d\n", id);
	_AdDelete(id);
	_AdSetDefault(ad->cfg);
	cr = adPrimaryReader(ad);
	crSetType(cr, CR_TYPE_PRIMARY);
	val = adGetCreatableIoChs(ad, ids);
	if(val > 1) id = ids[1];
	else	id = ids[0];
	bo = bosGet(IO2BO(id));
	boCreate(bo, BO_TYPE_DOOR);
	bi = _BisGet(IO2BI(id));
	if(biIsCreatableDoor(bi)) biCreate(bi, BI_TYPE_DOOR);
}

void adDelete(void *self)
{
	ACCESS_DOOR	*ad = self;

//printf("adDelete...%d\n", (int)ad->id);
	_AdDelete((int)ad->id);
}

void *adPrimaryReader(void *self)
{
	ACCESS_DOOR	*ad = self;
	void	*subdev, *cr;
	int		id;

	id = ad->id;
	if(!devLocalUnitModel(NULL) && id == 0) cr = crsGet(id);
	else {
		subdev = subdevsGet(id);
		if(subdev && subdevIsNonNull(subdev)) cr = crsGet(id);
		else	cr = NULL;
	}
	return cr;
}

void *adSecondaryReader(void *self)
{
	ACCESS_DOOR	*ad = self;
	void	*subdev, *cr;
	int		id;

	id = ad->id;
	if(id & 1) id--;
	else	   id++;
	if(!devLocalUnitModel(NULL) && id == 0) cr = crsGet(id);
	else {
		subdev = subdevsGet(id);
		if(subdev && subdevIsNonNull(subdev)) cr = crsGet(id);
		else	cr = NULL;
	}
	return cr;
}

int adGetCreatableIoChs(void *self, int *ioIds)
{
	ACCESS_DOOR	*ad = self;
	void	*bo;
	int		i, id;

	id = ad->id;
	i = 0;
	bo = bosGet(CH2SUBDEVBO(id));
	if(bo && boIsCreatableDoor(bo)) {
		ioIds[i] = id; i++;
	}
	bo = bosGet(CH2UNITBO(id));
	if(bo && boIsCreatableDoor(bo)) {
		ioIds[i] = id+32; i++;
	}
	return i;
}

void adChangeIoCh(void *self, int ioId)
{
	ACCESS_DOOR	*ad = self;
	void	*bo, *bi, *obo, *obi;
	int		i, id, boid, biid, oboid, obiid;

	id = ad->id;
	bo = bosGet(IO2BO(ioId));
	boid = boId(bo);
	if(boid < MAX_BO_HALF_SZ) {
		biid  = CH2SUBDEVBI(id);
		oboid = CH2UNITBO(id); obiid = CH2UNITBI(id);
	} else {
		biid  = CH2UNITBI(id);
		oboid = CH2SUBDEVBO(id); obiid = CH2SUBDEVBI(id);
	}
	boCreate(bo, BO_TYPE_DOOR);
	obo = bosGet(oboid);
	if(obo) {
		if(boIsDoorType(obo)) boDelete(obo);
		bo = _BosGet(boid+1);
		obo = _BosGet(oboid+1);
		if(!boIsNonNull(bo) && boIsDoorType(obo)) {
			boCreate(bo, BO_TYPE_DOOR); boDelete(obo);
		}
		bi = _BisGet(biid);
		obi = _BisGet(obiid);
		if(!biIsNonNull(bi) && biIsDoorType(obi)) {
			biCreate(bi, BI_TYPE_DOOR); biDelete(obi);
		}
		bi = _BisGet(biid+1);
		obi = _BisGet(obiid+1);
		if(!biIsNonNull(bi) && biIsDoorType(obi)) {
			biCreate(bi, BI_TYPE_DOOR); biDelete(obi);
		}
	} else {
		bo = _BosGet(boid+1);
		if(boIsDoorType(bo)) boDelete(bo);
		bi = _BisGet(biid);
		if(biIsCreatableDoor(bi)) biCreate(bi, BI_TYPE_DOOR);
		bi = _BisGet(biid+1);
		if(biIsDoorType(bi)) biDelete(bi);
	}
}

int adIoSet(void *self)
{
	ACCESS_DOOR	*ad = self;
	int		id;
	void	*bo;

	id = ad->id;
	bo = bosGet(CH2SUBDEVBO(id));
	if(!bo || !boIsNonNull(bo) || boType(bo) != BO_TYPE_DOOR) {
		bo = bosGet(CH2UNITBO(id));
		if(bo && boIsNonNull(bo) && boType(bo) == BO_TYPE_DOOR) id += 32;
		else	id = -1;
	}
	return id;
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

int adDoorExtendedPulseTime(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->doorExPulseTime;
}

void adSetDoorExtendedPulseTime(void *self, int doorExtendedPulseTime)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->doorExPulseTime = doorExtendedPulseTime;
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
	void	*bi;
	int		id;

	id = IO2BI(adIoSet(ad));
	bi = bisGet(id+1);
	if(biIsDoorSwitch(bi) && adDoorOpenTooLongEnable(ad)) return 1;
	else	return 0;
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
	void	*bi;
	int		id;

	id = IO2BI(adIoSet(ad));
	bi = bisGet(id+1);
	if(biIsDoorSwitch(bi) && adForcedOpenEnable(ad)) return 1;
	else	return 0;
}

int adAlarmAction(void *self)
{
	ACCESS_DOOR	*ad = self;
	int		aaId;

	aaId = ad->cfg->alarmAction;
	if(!aasGet(aaId)) {
		aaId = 0; ad->cfg->alarmAction = aaId;
	}
	return aaId;
}

void adSetAlarmAction(void *self, int aaId)
{
	ACCESS_DOOR	*ad = self;

	if(aaId < 0 || aaId >= MAX_AA_SZ) aaId = 0;
	ad->cfg->alarmAction = aaId;
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

int adRtePriority(void *self)
{
	ACCESS_DOOR	*ad = self;

	return (int)ad->cfg->rtePriority;
}

void adSetRtePriority(void *self, int rtePriority)
{
	ACCESS_DOOR	*ad = self;

	ad->cfg->rtePriority = rtePriority;
}

void adGetPriorityArray(void *self, int *priorityArray)
{
	ACCESS_DOOR		*ad = self;
	int		i, msk, pv;

	msk = ad->priority;
	if(msk < 8) {
		for(i = 0;i < msk;i++) priorityArray[i] = -1;
		priorityArray[i] = ad->pv;
		i++;
		msk = 0x80 >> i;
		for( ;i < 8;i++, msk >>= 1) {
			if(ad->priorityMsk & msk) { 
				if(ad->priorityVal & msk) pv = 1; else pv = 0;
			} else	pv = -1;
			priorityArray[i] = pv;
		}
	} else {
		for(i = 0;i < 8;i++) priorityArray[i] = -1;
	}
}

void adSetPriorityArray(void *self, int *priorityArray)
{
	ACCESS_DOOR		*ad = self;
	int		i, msk;

	ad->priorityMsk = ad->priorityVal = 0x00;
	for(i = 0;i < 8;i++)
		if(priorityArray[i] >= 0) break;
	if(i < 8) {
		ad->priority = i;
		ad->pv = priorityArray[i];
		i++;
		msk = 0x80 >> i;
		for( ;i < 8;i++, msk >>= 1) {
			if(priorityArray[i] >= 0) {
				ad->priorityMsk |= msk;
				if(priorityArray[i] > 0) ad->priorityVal |= msk;
			}
		}
	} else {
		ad->priority = 8; ad->pv = 0;	
	}
//printf("adSetPriorityArray: pri=%d pv=%d\n", (int)ad->priority, (int)ad->pv);
}

// pv: 0=Lock  1=Unlock  2=PulseUnlock  3=ExtendedPulseUnlock
int adPresentValue(void *self)
{
	ACCESS_DOOR		*ad = self;

	return (int)ad->pv;
}

// priority: 1(highest) .. 8(lowest)
int adPriority(void *self)
{
	ACCESS_DOOR		*ad = self;
	int		priority;

	priority = ad->priority + 1;
	if(priority > 8) priority = 8;
	return priority;
}

// priority: 1(highest) .. 8(lowest)
int adPresentValueAtPriority(void *self, int priority)
{
	ACCESS_DOOR		*ad = self;
	int		pv, opriority, msk;

	priority--; opriority = ad->priority;
	if(priority < opriority) pv = -1;
	else if(priority == opriority) pv = ad->pv;
	else {
		msk = 0x80 >> priority;
		if(ad->priorityMsk & msk) { 
			if(ad->priorityVal & msk) pv = 1; else pv = 0;
		} else	pv = -1;
	}
	return pv;
}

// Return	0:No changed  1:Changed  2:Changed & Require slave command
int adSetPresentValue(void *self, int presentValue, int priority)
{
	ACCESS_DOOR		*ad = self;
	int		pv, opv, opriority, msk, val;

	priority--; pv = presentValue;
	opv = ad->pv; opriority = ad->priority;
	if(priority < opriority) {
		if(opriority < 8 && opv < 2) {
			msk = 0x80 >> opriority;
			ad->priorityMsk |= msk;
			if(opv) ad->priorityVal |= msk; else ad->priorityVal &= ~msk;
		}
		ad->pv = pv; ad->priority = priority;
		val = 2;
	} else if(priority == opriority) {
		if(pv < 2 && pv == opv) val = 0;
		else {
			ad->pv = pv; ad->priority = priority;
			val = 2;
		}
	} else {
		if(pv > 1) val = 0;
		else {
			msk = 0x80 >> priority;
			if(ad->priorityMsk & msk) {
				if(ad->priorityVal & msk) opv = 1; else opv = 0;
				if(pv == opv) val = 0;
				else {
					ad->priorityVal ^= msk; val = 1;
				}
			} else {
				ad->priorityMsk |= msk;
				if(pv) ad->priorityVal |= msk; else ad->priorityVal &= ~msk;  
				val = 1;
			}
		}
	}
	return val;
}

// Return	0:No changed  1:Changed  2:Changed & Require slave command
int adNullifyPresentValue(void *self, int priority)
{
	ACCESS_DOOR		*ad = self;
	int		pv, opv, opriority, msk, val;

	priority--;
	opv = ad->pv; opriority = ad->priority;
	if(priority < opriority) val = 0;
	else if(priority == opriority) {
		priority++; msk = 0x80 >> priority;
		for( ;priority < 8;priority++, msk >>= 1)
			if(ad->priorityMsk & msk) break;
		if(priority < 8) {
			if(ad->priorityVal & msk) pv = 1;
			else	pv = 0;
			ad->priorityMsk &= ~msk;
		} else	pv = 0;
		ad->priority = priority; ad->pv = pv;
		val = 2;
	} else {
		msk = 0x80 >> priority;
		if(ad->priorityMsk & msk) {
			ad->priorityMsk &= ~msk; val = 1;
		} else	val = 0;
	}
	return val;
}

int adAlarmState(void *self)
{
	ACCESS_DOOR	*ad = self;
/*
	int		id, bid, dlu, alarm;

	id = adIoSet(ad);
	bid = IO2BO(id) + 1;
	dlu = devLocalUnitModel(NULL);
	if(!dlu && bid < 2) {
		alarm = commStats[2] >> 2 & 0x3;
	} else if(bid < MAX_BO_HALF_SZ) {
	} else {
		alarm = unitStats[IO2UNIT(id)][2+(id&3)*3] >> 2 & 0x3;
	}
	return alarm;
*/
	return (int)ad->alarmState;
}

void adSetAlarmState(void *self, int alarm)
{
	ACCESS_DOOR	*ad = self;

	ad->alarmState = (unsigned char)alarm;
}

void adProcessAlarmCommand(void *self, int pv)
{
	void	*bo, *aa;
	unsigned char	buf[4];
	int		id;

	id = IO2BI(adIoSet(self));
	bo = bosGet(id+1);
	if(bo && boType(bo) == BO_TYPE_DOOR) {
		aa = aasGet(adAlarmAction(self));
		if(aaEnableAtIndex(aa, pv)) {
			buf[0] = aaOnTimeAtIndex(aa, pv);
			buf[1] = aaOffTimeAtIndex(aa, pv);
			buf[2] = aaRepeatCountAtIndex(aa, pv);
			slvmOutputCommand(boId(bo), buf);
		}
	}
}

void adStopAlarmCommand(void *self)
{
	void	*bo, *aa;
	unsigned char	buf[4];
	int		id;

	id = IO2BO(adIoSet(self));
	bo = bosGet(id+1);
	if(bo && boType(bo) == BO_TYPE_DOOR) {
		buf[0] = buf[1] = buf[2] = 0;
		slvmOutputCommand(boId(bo), buf);
	}
}

// 0:Unknown 1:Closed 2:Opened
int adDoorStatus(void *self)
{
	void	*bi;
	int		id, val;

	id = IO2BI(adIoSet(self));
	bi = bisGet(id+1);
	if(biIsDoorSwitch(bi)) {
		if(biPresentValue(bi)) val = 2; else val = 1;
	} else	val = 0;
	return val;
}

/* Return
 *	0 : No
 *  1 : Lock schedule
 *  2 : Unlock schedule
 *  3 : Lock+Unlock schedule
 */
int adCheckLockSchedule(void *self, unsigned char *ctm)
{
	ACCESS_DOOR	*ad = self;
	int		lock, scheID;

	lock = 0;
	scheID = adLockScheID(ad);
	if(scheValidate(scheID) && scheBound(scheID, ctm)) lock = 1;
	scheID = adUnlockScheID(ad);
	if(scheValidate(scheID) && scheBound(scheID, ctm)) lock += 2;
	return lock;
}

void adProcessLockSchedule(void *self, unsigned char *ctm)
{
	ACCESS_DOOR	*ad = self;
	char	data[8];
	int		pv, priority, code, evtID, scheID;

	code = adCheckLockSchedule(ad, ctm);
	evtID = 0;
	priority = 5;
	pv = adPresentValueAtPriority(ad, priority);
	if(pv == PV_LOCK && code != 1) {
		adNullifyPresentValue(ad, priority);
		evtID = E_NORMAL_LOCKED_SCHEDULED_LOCK_END; scheID = adLockScheID(ad);
	} else if(pv == PV_UNLOCK && code < 2) {
		adNullifyPresentValue(ad, priority);
		evtID = E_NORMAL_LOCKED_SCHEDULED_UNLOCK_END; scheID = adUnlockScheID(ad);
	}
	if(evtID > 0) {
		utoa2((unsigned int)scheID, data);
//		EventAdd(evtID, ctm, data);
	}
	evtID = 0;
	pv = adPresentValueAtPriority(ad, priority);
	if(code > 1 && pv < 0) {
		adSetPresentValue(ad, PV_UNLOCK, priority);
		evtID = E_SCHEDULED_UNLOCKED; scheID = adUnlockScheID(ad);
	} else if(code == 1 && pv < 0) {
		adSetPresentValue(ad, PV_LOCK, priority);
		evtID = E_SCHEDULED_LOCKED; scheID = adLockScheID(ad);
	}
	if(evtID > 0) {
		if(scheID > 0) utoa2((unsigned int)scheID, data);
		else	data[0] = 0;
//		EventAdd(evtID, ctm, data);
	}
}

int adEncodeStatus(void *self, void *buf)
{
	ACCESS_DOOR	*ad = self;
	unsigned char	*p;
	unsigned char	c, val;
	int		pv, priority;
	
	p = (unsigned char *)buf;
	c = 0x01;
	val = adAlarmState(ad);
	c |= val << 1;
	pv = adPresentValue(ad); priority = adPriority(ad);
	if(pv > 1) val = 1;
	else	   val = ((8-priority) << 1) + pv;
	c |= val << 3;	// c |= val;	2020.8.12
	c |= adDoorStatus(ad) << 6;		// 0:Unknown 1:Closed 2:Opened
	*p++ = c;
	return p - (unsigned char *)buf;
}

#include "gactivity.h"
#include "gapp.h"
#include "appact.h"

int AdSetPv(void *ad, int pv, int priority)
{
	unsigned char	msg[12];
	int		val, rte;

printf("### AdSetPv: pv=%d pri=%d\n", pv, priority);
	val = adSetPresentValue(ad, pv, priority);
	if(val) {
		if(val > 1) {
			if(adPriority(ad) < adRtePriority(ad)) rte = 0; else rte = 1;  
			slvmDoorCommand(pv, rte); 
		}
		msg[0] = GM_AD_LOCK_CHANGED; memset(msg+1, 9, 0);
		appPostMessage(msg);
		if(devMode(NULL)) SlvnetSendRequest();
	}
	return val;
}

int AdNullifyPv(void *ad, int priority)
{
	unsigned char	msg[12];
	int		val, pv, rte;

printf("### AdNullifyPv: pri=%d\n", priority);
	val = adNullifyPresentValue(ad, priority);
	if(val) {
		if(val > 1) {
			pv = adPresentValue(ad);	
			if(adPriority(ad) < adRtePriority(ad)) rte = 0; else rte = 1;  
			slvmDoorCommand(pv, rte); 
		}
		msg[0] = GM_AD_LOCK_CHANGED; memset(msg+1, 9, 0);
		appPostMessage(msg);
		if(devMode(NULL)) SlvnetSendRequest();
	}
	return val;
}

