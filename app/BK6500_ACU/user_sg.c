#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "cotask.h"
#include "prim.h"
#include "sche.h"
#include "acar.h"
#include "acap.h"
#ifndef STAND_ALONE
#include "acaz.h"
#endif
#include "syscfg.h"
#include "user.h"


#include "usercfg.c"


void userEncodeTransaction(FP_USER *user, void *buf)
{
	unsigned char	*p;
	unsigned long	lVal;

	p = (unsigned char *)buf;
	datetime2longtime(user->AccessTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	*p++ = user->AccessPoint;
	*p++ = user->ZoneTo;
	*p++ = user->Passback;
	*p++ = user->FuncCode;
	memset(p, 0xff, 4); p += 4;
}
	
void userDecodeTransaction(FP_USER *user, void *buf)
{
	unsigned char	*p;
	unsigned long	lVal;

	p = (unsigned char *)buf;
	BYTEtoLONG(p, &lVal); p += 4; longtime2datetime(lVal, user->AccessTime);
	user->AccessPoint	= *p++;
	user->ZoneTo		= *p++;
	user->Passback		= *p++;
	user->FuncCode		= *p++;
}

void userID2FPID(long nID, unsigned long *pFPID)
{
	unsigned long	FPID;

	FPID = nID;
	FPID <<= 1;
	*pFPID = FPID;
}

void userFPID2ID(unsigned long FPID, long *pID)
{
	*pID = FPID >> 1;
}

int userFPData1IsValid(FP_USER *user)
{
	if(user->FPFlag & FP_DATA_1_EXIST) return 1;
	else	return 0;
}

int userFPData2IsValid(FP_USER *user)
{
	if(user->FPFlag & FP_DATA_2_EXIST) return 1;
	else	return 0;
}

int userGetFPDataCount(FP_USER *user)
{
	int		count;
	
	count = 0;
	if(user->FPFlag & FP_DATA_1_EXIST) count++;
	if(user->FPFlag & FP_DATA_2_EXIST) count++;
	return count;		
}

void userSetFPData1(FP_USER *user)
{
	user->FPFlag |= FP_DATA_1_EXIST;
}

void userSetFPData2(FP_USER *user)
{
	user->FPFlag |= FP_DATA_2_EXIST;
}

// Return	0:False  1:True
int userEvaluateAccessRights(FP_USER *user, int apID, unsigned char *ctm)
{
	ACCESS_POINT	*ap;
	unsigned char	*p, c;
	long	scheID;
	int		i, rval, ID;

	if(user->AccessRights < 0) {
		ap = apsGetAt(apID);
		rval = 0;
		for(i = 0;i < user->Count;i++) {
			p = user->AccessRules[i];
			c = *p; ID = c & 0x7f;
			if(c & 0x80) {
				if(ap->cfg->ZoneTo == ID) rval = 1;
			} else if(ID == apID) rval = 1;
			if(rval) break;
		}
		if(rval) PACK1toID(p+1, &scheID);
		else	 PACK1toID(&user->DefaultTimeRange, &scheID);
//if(i < user->Count) cprintf("User=%ld AccessRules[%d]=%02x-%02x-%02x ScheID=%d\n", user->ID, i, (int)p[0], (int)p[1], (int)p[2], scheID);
//else	cprintf("User=%ld DefaultTimeRange=%02x SchID=%d\n", user->ID, (int)user->DefaultTimeRange, scheID);
		if(scheID >= 0) rval = schePresentValue(scheID, ctm);
		else	rval = 0;
	} else	rval = arEvaluate(user->AccessRights, apID, ctm);
	return rval;
}

#ifndef STAND_ALONE

#include "fsuser.h"
#include "unitcfg.h"

int userDelayPassback(FP_USER *user, int apID, unsigned char *ctm)
{
	int		rval, val;

	rval = 0;
	if(apID == user->AccessPoint) {
		val = sys_cfg->RepeatAccessDelay;
		if(val > 0 && val > datetime_diff(user->AccessTime, ctm)) rval = 1;
	}
	return rval;
}	

// Passback settings in Access Zone 
// Return	0:False  1:HardPassback  2:SoftPassback  
int userEvaluatePassback(FP_USER *user, int apID, unsigned char *ctm)
{
	ACCESS_POINT	*ap;
	ACCESS_ZONE_CFG	*az_cfg;
	long	scheHardID, scheSoftID;
	int		val, rval, adID, zoneTo, zoneFrom, PassbackMode;

	adID = apGetAssignedDoor(apID);
	if(adID < 0) return 0;
	if(adID == apID) {
		ap = apsGetAt(apID);
		zoneTo = ap->cfg->ZoneTo;		
	} else {
		ap = apsGetAt(adGetPairAccessPoint(adID));
		zoneTo = ap->cfg->ZoneFrom;
	}
	if(!zoneTo || !apValidateZone(ap->cfg)) return 0;	// outside
	rval = 0;
	az_cfg = &sys_cfg->AccessZones[zoneTo-1];
	PACK1toID(&az_cfg->HardPassbackSchedule, &scheHardID);
	PACK1toID(&az_cfg->SoftPassbackSchedule, &scheSoftID);
	if(scheHardID < 0 && scheSoftID < 0) PassbackMode = az_cfg->PassbackMode;
	else {
		PassbackMode = 0;
		if(scheHardID >= 0) {
			val = schePresentValue(scheHardID, ctm);
			if(val) PassbackMode = 1;
		}
		if(scheSoftID >= 0 && !PassbackMode) {
			val = schePresentValue(scheSoftID, ctm);
			if(val) PassbackMode = 2;
		}
	}
	if(PassbackMode) {
		if(zoneTo == user->ZoneTo) {
			val = az_cfg->PassbackTimeout;
			if(!val || val*60 > datetime_diff(user->AccessTime, ctm)) rval = PassbackMode;
		} else {
/*			
			apID = user->AccessPoint;
			adID = apGetAssignedDoor(apID);
			if(adID < 0) return 0;
			if(adID == apID) {
				ap = apsGetAt(apID);
				zoneFrom = ap->cfg->ZoneFrom;
			} else {
				ap = apsGetAt(adGetPairAccessPoint(adID));
				zoneFrom = ap->cfg->ZoneTo;
			}
			if(zoneTo == zoneFrom) {
				val = sys_cfg->RepeatAccessDelay;
				if(val > 0 && val > datetime_diff(user->AccessTime, ctm)) rval = PassbackMode;
			}
*/
		}
	}
	return rval;
}

int userEvaluatePassbackOnExit(FP_USER *user, int apID, unsigned char *ctm)
{
	ACCESS_POINT	*ap;
	ACCESS_ZONE_CFG	*az_cfg;
	long	scheHardID, scheSoftID;
	int		val, rval, adID, zoneFrom, PassbackMode;

	adID = apGetAssignedDoor(apID);
	if(adID < 0) return 0;
	if(adID == apID) {
		ap = apsGetAt(apID);
		zoneFrom = ap->cfg->ZoneFrom;		
	} else {
		ap = apsGetAt(adGetPairAccessPoint(adID));
		zoneFrom = ap->cfg->ZoneTo;
	}
	if(!zoneFrom || !apValidateZone(ap->cfg)) return 0;		// outside
	rval = 0;
	az_cfg = &sys_cfg->AccessZones[zoneFrom-1];
	if(!(az_cfg->PassbackOption & 0x01)) return 0;
	PACK1toID(&az_cfg->HardPassbackSchedule, &scheHardID);
	PACK1toID(&az_cfg->SoftPassbackSchedule, &scheSoftID);
	if(scheHardID < 0 && scheSoftID < 0) PassbackMode = az_cfg->PassbackMode;
	else {
		PassbackMode = 0;
		if(scheHardID >= 0) {
			val = schePresentValue(scheHardID, ctm);
			if(val) PassbackMode = 1;
		}
		if(scheSoftID >= 0 && !PassbackMode) {
			val = schePresentValue(scheSoftID, ctm);
			if(val) PassbackMode = 2;
		}
	}
	if(PassbackMode && zoneFrom != user->ZoneTo) {
		val = az_cfg->PassbackTimeout;
		if(!val || val*60 > datetime_diff(user->AccessTime, ctm)) rval = PassbackMode;
	}
	return rval;
}

/*
// Passback settings in Access Zone 
// Return	0:False  1:HardPassback  2:SoftPassback  
int userEvaluatePassback(FP_USER *user, int apID, unsigned char *ctm)
{
	ACCESS_POINT	*apNow, *apPre;
	ACCESS_ZONE		*azNow, *azPre;
	long	scheHardID, scheSoftID;
	int		val, rval, PassbackMode;
	
	apNow = apsGetAt(apID);
	if(apNow->cfg->ZoneFrom == 0xff || apNow->cfg->ZoneTo == 0xff) return 0;
	val = userfsReadTransaction(user);
	if(val <= 0) return 0;
	apPre = apsGetAt((int)user->AccessPoint);
	if(apPre->cfg->ZoneFrom == 0xff || apPre->cfg->ZoneTo == 0xff) return 0;
	azPre = azsGetAt((int)apPre->cfg->ZoneTo);
	rval = 0;
	if(apNow->cfg->ZoneTo) {
		azNow = azsGetAt((int)apNow->cfg->ZoneTo);
		PACK1toID(&azNow->cfg->EntryHardPassbackSchedule, &scheHardID);
		PACK1toID(&azNow->cfg->EntrySoftPassbackSchedule, &scheSoftID);
		if(scheHardID < 0 && scheSoftID < 0) PassbackMode = azNow->cfg->EntryPassbackMode;
		else {
			PassbackMode = 0;
			if(scheHardID >= 0) {
				val = schePresentValue(scheHardID, ctm);
				if(val) PassbackMode = 1;
			}
			if(scheSoftID >= 0 && !PassbackMode) {
				val = schePresentValue(scheSoftID, ctm);
				if(val) PassbackMode = 2;
			}
		}
		if(PassbackMode && azPre->ID == azNow->ID) {
			val = azNow->cfg->EntryPassbackTimeout;
			if(!val || val*60 > datetime_diff(user->AccessTime, ctm)) rval = PassbackMode;
		}
	}
	rval = 0;
	if(!rval && apNow->cfg->ZoneFrom) {
		azNow = azsGetAt((int)apNow->cfg->ZoneFrom);
		PACK1toID(&azNow->cfg->ExitHardPassbackSchedule, &scheHardID);
		PACK1toID(&azNow->cfg->ExitSoftPassbackSchedule, &scheSoftID);
		if(scheHardID < 0 && scheSoftID < 0) PassbackMode = azNow->cfg->ExitPassbackMode;
		else {
			PassbackMode = 0;
			if(scheHardID >= 0) {
				val = schePresentValue(scheHardID, ctm);
				if(val) PassbackMode = 1;
			}
			if(scheSoftID >= 0 && !PassbackMode) {
				val = schePresentValue(scheSoftID, ctm);
				if(val) PassbackMode = 2;
			}
		}
		if(PassbackMode && azPre->ID != azNow->ID) {
			val = azNow->cfg->ExitPassbackTimeout;
			if(!val || val*60 > datetime_diff(user->AccessTime, ctm)) rval = PassbackMode;
		}
	}
	return rval;
}

// Passback settings in Access Point
// Return	0:False  1:HardPassback  2:SoftPassback  
int userEvaluatePassback(FP_USER *user, int apID, unsigned char *ctm)
{
	ACCESS_POINT	*apNow, *apPre;
	long	scheHardID, scheSoftID;
	int		val, rval, PassbackMode;
	
	apNow = apsGetAt(apID);
	if(apNow->cfg->ZoneFrom == 0xff || apNow->cfg->ZoneTo == 0xff) return 0;
	val = userfsReadTransaction(user);
	if(val <= 0) return 0;
	apPre = apsGetAt((int)user->AccessPoint);
	if(apPre->cfg->ZoneFrom == 0xff || apPre->cfg->ZoneTo == 0xff) return 0;
	rval = 0;
	if(apNow->cfg->ZoneTo) {
		PACK1toID(&apNow->cfg->EntryHardPassbackSchedule, &scheHardID);
		PACK1toID(&apNow->cfg->EntrySoftPassbackSchedule, &scheSoftID);
		if(scheHardID < 0 && scheSoftID < 0) PassbackMode = apNow->cfg->EntryPassbackMode;
		else {
			PassbackMode = 0;
			if(scheHardID >= 0) {
				val = schePresentValue(scheHardID, ctm);
				if(val) PassbackMode = 1;
			}
			if(scheSoftID >= 0 && !PassbackMode) {
				val = schePresentValue(scheSoftID, ctm);
				if(val) PassbackMode = 2;
			}
		}
		if(PassbackMode && user->AccessPoint == apNow->cfg->ZoneTo) {
			val = apNow->cfg->EntryPassbackTimeout;
			if(!val || val*60 > datetime_diff(user->AccessTime, ctm)) rval = PassbackMode;
		}
	}
	rval = 0;
	if(!rval && apNow->cfg->ZoneFrom) {
		PACK1toID(&apNow->cfg->ExitHardPassbackSchedule, &scheHardID);
		PACK1toID(&apNow->cfg->ExitSoftPassbackSchedule, &scheSoftID);
		if(scheHardID < 0 && scheSoftID < 0) PassbackMode = apNow->cfg->ExitPassbackMode;
		else {
			PassbackMode = 0;
			if(scheHardID >= 0) {
				val = schePresentValue(scheHardID, ctm);
				if(val) PassbackMode = 1;
			}
			if(scheSoftID >= 0 && !PassbackMode) {
				val = schePresentValue(scheSoftID, ctm);
				if(val) PassbackMode = 2;
			}
		}
		if(PassbackMode && user->AccessPoint == apNow->cfg->ZoneFrom) {
			val = apNow->cfg->ExitPassbackTimeout;
			if(!val || val*60 > datetime_diff(user->AccessTime, ctm)) rval = PassbackMode;
		}
	}
	return rval;
}
*/

#endif

long userGetNewID(long StartID, long MinID, long MaxID)
{
	FP_USER	*user, _user;
	long	nID;

	user = &_user;
	if(StartID <= MinID || StartID > MaxID) StartID = MinID;
	nID = StartID;
	while(1) {
		user->ID = nID;
//cprintf("NewUserID [%s] %d\n", user->ID, userfsGet(user));	 
		if(userfsGet(user) <= 0) return nID;
		nID++;
		if(nID > MaxID) nID = MinID;
		if(!(nID & 0xf)) taskYield();
	} while(nID != StartID) ;
	return 0L;
}

int userIsFP(FP_USER *user)
{
	int		mode;

	mode = user->AccessMode;
	if((mode & USER_FP) || (mode & USER_FP_PIN) || (mode & USER_CARD_FP) || (mode & USER_CARD_FP_PIN) ) return 1;
	else	return 0;
}

int userIsCard(FP_USER *user)
{
	int		mode;

	mode = user->AccessMode;
	if((mode & USER_CARD) || (mode & USER_CARD_FP) || (mode & USER_CARD_PIN) || (mode & USER_CARD_FP_PIN)) return 1;
	else	return 0;
}

int userIsPIN(FP_USER *user)
{
	int		mode;

	mode = user->AccessMode;
	if((mode & USER_PIN) || (mode & USER_CARD_PIN) || (mode & USER_CARD_FP_PIN) || (mode & USER_FP_PIN)) return 1;
	else	return 0;
}

void userCopy(FP_USER *userD, FP_USER *userS)
{
	memcpy(userD, userS, sizeof(FP_USER));	
}

int userCompare(FP_USER *user1, FP_USER *user2)
{
	unsigned char	buf[128];

	userEncodeID(user1->ID, buf);
	userEncode(user1, buf+3);
	userEncodeID(user2->ID, buf+64);
	userEncode(user2, buf+67);
	if(!n_memcmp(buf, buf+64, 31)) return 0;
	else	return -1;
}

void userCopyHead(FP_USER *userD, FP_USER *userS)
{
	memcpy(userD, userS, FP_USER_HEAD_SZ);
}

void userPrint(FP_USER *user)
{
#ifdef BK_DEBUG	
	cprintf("UserID=[%s]\n", user->UserID);
	cprintf("AccessMode=0x%02x AcessRights=%d Date=%02d%02d%02d-%02d%02d%02d\n", (int)user->AccessMode, (int)user->AccessRights, (int)user->ActivateDate[0], (int)user->ActivateDate[1], (int)user->ActivateDate[2],
												(int)user->ExpireDate[0], (int)user->ExpireDate[1], (int)user->ExpireDate[2]);
	cprintf("PIN=[%s] CardData=[%s] CardStatus=%d SL=%d Idx[0]=%d Idx[1]=%d\n",
						 user->PIN, user->CardData, (int)user->CardStatus, (int)user->FPSecurityLevel, (int)user->FPIndex[0], (int)user->FPIndex[1]);
#endif
}
