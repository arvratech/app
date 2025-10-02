#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "defs.h"
#include "defs_obj.h"
#include "NSEnum.h"
#include "rtc.h"
#include "cbuf.h"
#include "user.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "ad.h"
#include "cr.h"


int _IsGrantedEvent(int eventID)
{
	if(eventID >= E_ACCESS_GRANTED_FP && eventID <= E_ACCESS_GRANTED_CARD_PIN || eventID == E_ACCESS_GRANTED_CARD_FP_PIN || eventID == E_ACCESS_GRANTED_ALL) return 1;
	else	return 0;
}

int crAuthPolicy(CREDENTIAL_READER *cr, int authPolicy)
{
	int		mode;

	mode = authPolicy;
	if(!crIsEnableCardReader(cr)) mode &= ~(USER_CARD | USER_CARD_FP | USER_CARD_PIN | USER_CARD_FP_PIN);
	if(!crIsEnableFpReader(cr)) mode &= ~(USER_FP | USER_FP_PIN | USER_CARD_FP | USER_CARD_FP_PIN);
	if(!crIsEnablePinReader(cr)) mode &= ~(USER_PIN | USER_CARD_PIN | USER_CARD_FP_PIN | USER_FP_PIN);
	if(mode & USER_CARD_FP_PIN) mode &= ~(USER_CARD | USER_CARD_FP | USER_CARD_PIN);
	else if(mode & (USER_CARD_FP | USER_CARD_PIN)) mode &= ~USER_CARD;
	return mode;
}

int _MaskAuthPolicy(int authPolicy, int authDone, int maskPolicy)
{
	int		ap;

	ap = authPolicy;
	if(!authDone) {
		if(maskPolicy == USER_CARD) ap &= USER_CARD | USER_CARD_FP | USER_CARD_PIN | USER_CARD_FP_PIN;
		else if(maskPolicy == USER_FP) ap &= USER_FP | USER_FP_PIN;
		else if(maskPolicy == USER_PIN) ap &= USER_PIN;
	} else if(authDone == USER_CARD) {
		if(maskPolicy == USER_FP) ap &= USER_CARD_FP | USER_CARD_FP_PIN;
		else if(maskPolicy == USER_PIN) ap &= USER_CARD_PIN;
		else	ap = 0;
	} else if(authDone == USER_FP) {
		if(maskPolicy == USER_PIN) ap &= USER_FP_PIN;
		else	ap = 0;
	} else if(authDone == (USER_CARD | USER_FP)) {
		if(maskPolicy == USER_PIN) ap &= USER_CARD_FP_PIN;
		else	ap = 0;
	} else	ap = 0;
	return ap;
}

int _GetCaptureMode(int authMode, int authDone)
{
	int		mode;

	mode = 0;
	if(!authDone) {
		if(authMode & (USER_CARD | USER_CARD_FP | USER_CARD_PIN | USER_CARD_FP_PIN)) mode |= USER_CARD;
		if(authMode & (USER_FP | USER_FP_PIN)) mode |= USER_FP;
		if(authMode & USER_PIN) mode |= USER_PIN;
	} else if(authDone == USER_CARD) {
		if(authMode & (USER_CARD_FP | USER_CARD_FP_PIN)) mode |= USER_FP;
		if(authMode & USER_CARD_PIN) mode |= USER_PIN;
	} else if(authDone == USER_FP) {
		if(authMode & USER_FP_PIN) mode |= USER_PIN;
	} else if(authDone == (USER_CARD | USER_FP)) {
		if(authMode & USER_CARD_FP_PIN) mode |= USER_PIN;
	}
	return mode;
}

int _CheckCardStatus(FP_USER *user)
{
	int		EventID;
	
	if(user->cardStatus != 1) EventID = E_ACCESS_DENIED_LOST_CARD;
	else	EventID = 0;
	return EventID;
}

// Return	0=OK  >0=Event ID  
int _CheckAccess(CREDENTIAL_READER *cr, FP_USER *user)
{
	int		EventID, AuthMode;
	
	AuthMode = crAuthorizationMode(cr);
	if(date_validate(user->activateDate) && date_comp(cr->accessTime, user->activateDate) < 0) EventID = E_ACCESS_DENIED_USER_NOT_ACTIVATED; 
	else if(date_validate(user->expireDate) && date_comp(cr->accessTime, user->expireDate) > 0) EventID = E_ACCESS_DENIED_USER_EXPIRED;
	else if(AuthMode != 1 && userValidateAccessRights(user) && !userEvaluateAccessRights(user, 0, cr->accessTime)) EventID = E_ACCESS_DENIED_ACCESS_RIGHTS;
	else	EventID = 0;
	return EventID;
}

// Return	0=OK  E_ACCESS_DENIED_ACCESS_CONTROL=Fail 
int _CheckLockState(CREDENTIAL_READER *cr)
{
	void	*ad;
	int		priority, EventID;

	ad = adsGet(crId(cr));
	priority = adPriority(ad);
	if(priority < 8) EventID = E_ACCESS_DENIED_ACCESS_CONTROL;	// modified 2013.3.19
	else	EventID = 0;
	return EventID;
}

void _AccessUnregistered(CREDENTIAL_READER *cr, int cardauth)
{
	int		EventID, output;

	if(cardauth) EventID = E_ACCESS_DENIED_UNREGISTERED_CARD;
	else		 EventID = E_ACCESS_DENIED_UNREGISTERED_FP;	
	cr->accessEvent = EventID;
//	EventAddRaw(EventID, cr->accessTime, cr->data);
//	output = apGetUnregisteredAlarm(0);
//	if(output) modAlarmCommand(2);
}

void _AccessDenied(CREDENTIAL_READER *cr)
{
	char	data[32];
	int		item, output;

//printf("AccessDenied: %d\n", (int)cr->accessEvent);
	if(cr->accessEvent == E_ACCESS_DENIED_UNREGISTERED_CARD) {
			|| cr->accessEvent == E_ACCESS_DENIED_UNREGISTERED_EMV_CARD) {

//		EventAddRaw((int)cr->accessEvent, cr->accessTime, cr->data);
	} else {
		item = cr->funcKey - 1; if(item < 0) item = 0;	
		if(taGetItemSize() > 10) utoa02(item, data);
		else {
			data[0] = '5';		// ID=1 => '5'
			data[1] = item + '0';
		}
		sprintf(data+2, "%u", cr->userId);
//	output = apGetExpiredAlarm(0);
//	if(output) modAlarmCommand(2);
		EventAdd(OT_ACCESS_POINT, 0, (int)cr->accessEvent, cr->accessTime, data);
	}
}

void _AccessGranted(CREDENTIAL_READER *cr)
{
	void	*ad;
	char	*p, data[32];
	int		item, val, idx;
	unsigned long	lval;

//printf("AccessGranted: %d\n", (int)cr->accessEvent);
	ad = adsGet(0);
	AdSetPv(ad, PV_PULSE_UNLOCK, 8);
/*
	if(ap->duress) {
		if(user) EventAdd(E_ACCESS_DURESS, cr->accessTime, user->id);
		else	 EventAddRaw(E_ACCESS_DURESS, cr->accessTime, ap->data);
		val = sys_cfg->accessPoints[0].duressAlarm;
		if(val) modAlarmCommand(2);
//cprintf("DURESS, output=%d\n", output);
	}
*/
	if(cr->accessEvent == E_ACCESS_GRANTED_ALL || cr->accessEvent == E_ACCESS_GRANTED_ALL_EMV) {
//		EventAddRaw((int)cr->accessEvent, cr->accessTime, cr->data);
	} else {
		item = cr->funcKey - 1; if(item < 0) item = 0;	
		if(taGetItemSize() > 10) utoa02(item, data);
		else {
			data[0] = '5';		// ID=1 => '5'
			data[1] = item + '0';
		}
		sprintf(data+2, "%u", cr->userId);
	}
	if(cr->accessEvent != E_ACCESS_GRANTED_ALL && cr->accessEvent != E_ACCESS_GRANTED_ALL_EMV)
		EventAdd(OT_ACCESS_POINT, 0, (int)cr->accessEvent, cr->accessTime, data);
}

void authIdentifyRequest(CREDENTIAL_READER *cr)
{
	FP_USER	*user, _user;
	int		rval, mode, credMode, biID;

	user = &_user;
	cr->sessionNo++;
	cr->result = cr->accessEvent = cr->cardIndex = cr->authMode = cr->authDone = cr->captureMode = 0;
	cr->retryCount = 0;
	switch(cr->credType) {
	case CREDENTIAL_ID:
		credMode = 0;
		userDecodeId(&user->id, cr->data);
		rval = userfsGet(user);
		if(rval < 0) cr->result = 2;
		else if(!rval) cr->result = 1;
		break;
	case CREDENTIAL_CARD:	 credMode = USER_CARD; break;
	default:	cr->result = 3;
	}
	if(cr->result) return;
	user->userExternalId[0] = user->userName[0] = 0;
	rtcGetDateTime(cr->accessTime);
	mode = crAuthorizationMode(cr);
	if(credMode == USER_CARD && mode == 3) {
		cr->accessEvent = E_ACCESS_GRANTED_ALL;
		return;
	}
	if(credMode == USER_CARD) {
		rval = crIdentifyMatchCard(cr, user);
//cprintf("crIdentifyMatchCard=%d\n", rval);
	} else {
		rval = 1;
	}
	if(rval < 0) cr->result = 2;		
	else if(!rval) cr->accessEvent = E_ACCESS_DENIED_UNREGISTERED_CARD;
	else if(mode == 2) cr->accessEvent = E_ACCESS_DENIED_ALL;
	if(cr->result || cr->accessEvent) return;
	userfsGetEx(user);
	cr->userId = user->id;
	strcpy(cr->userExternalId, user->userExternalId);
	strcpy(cr->userName, user->userName);
	memcpy(cr->pin, user->pin, 4);
	memcpy(cr->cardData, user->cardData, 9);
	mode = crAuthPolicy(cr, (int)user->accessMode);
	if(mode && credMode) mode = _MaskAuthPolicy(mode, (int)cr->authDone, credMode);
	cr->authMode = mode;
	if(!cr->authMode) {
		cr->accessEvent = E_ACCESS_DENIED_ACCESS_MODE;	
	} else {
		cr->authDone = credMode;
		cr->captureMode = _GetCaptureMode((int)cr->authMode, (int)cr->authDone);
		if(credMode == USER_CARD) cr->accessEvent = _CheckCardStatus(user);
		if(!cr->accessEvent) cr->accessEvent = _CheckAccess(cr, user);
		if(!cr->accessEvent) cr->accessEvent = _CheckLockState(cr);
		if(!cr->accessEvent && !cr->captureMode) {
			if(credMode == USER_CARD) cr->accessEvent = E_ACCESS_GRANTED_CARD;
			else	cr->accessEvent = E_ACCESS_GRANTED_FP;
		}
	}
}

void authVerifyRequest(CREDENTIAL_READER *cr)
{
	int		rval, mode, credMode;

	cr->result = cr->accessEvent = cr->cardIndex = 0;
	if(cr->sessionNo != cr->sessionNo) {
		cr->result = 3;
		return;
	}
	switch(cr->credType) {
	case CREDENTIAL_CARD:
		credMode = USER_CARD;
		mode = _MaskAuthPolicy((int)cr->authMode, (int)cr->authDone, credMode);
		if(mode) {
			rval = crVerifyMatchCard(cr, cr->cardData);
			if(!rval) cr->result = 1;
		} else	cr->result = 3;
		break;
	case CREDENTIAL_PIN:
		credMode = USER_PIN;
		mode = _MaskAuthPolicy((int)cr->authMode, (int)cr->authDone, credMode);
		if(mode) {
			rval = crVerifyMatchPin(cr, cr->pin);
			if(!rval) cr->result = 1;
		} else	cr->result = 3;
		break;
	default:	cr->result = 3;
	}
	if(cr->result > 1) return;
	if(!cr->result) {
		cr->authMode = mode;
		cr->authDone |= credMode;
		cr->captureMode = _GetCaptureMode((int)cr->authMode, (int)cr->authDone);
		if(!cr->captureMode) {
			switch(cr->authDone) {
			case USER_CARD:	cr->accessEvent = E_ACCESS_GRANTED_CARD; break;
			case USER_FP:	cr->accessEvent = E_ACCESS_GRANTED_FP; break;
			case USER_PIN:	cr->accessEvent = E_ACCESS_GRANTED_PIN; break;
			case (USER_CARD | USER_FP):	 cr->accessEvent = E_ACCESS_GRANTED_CARD_FP; break;
			case (USER_CARD | USER_PIN): cr->accessEvent = E_ACCESS_GRANTED_CARD_PIN; break;
			case (USER_FP | USER_PIN):	 cr->accessEvent = E_ACCESS_GRANTED_FP_PIN; break;
			case (USER_CARD | USER_FP | USER_PIN): cr->accessEvent = E_ACCESS_GRANTED_CARD_FP_PIN; break;
			}
		}
		cr->retryCount = 0;
	} else if(cr->retryCount) {
		cr->authDone |= credMode;
		switch(cr->authDone) {
		case USER_CARD:	cr->accessEvent = E_ACCESS_DENIED_BAD_CARD; break;
		case USER_FP:	cr->accessEvent = E_ACCESS_DENIED_BAD_FP; break;
		case USER_PIN:	cr->accessEvent = E_ACCESS_DENIED_BAD_PIN; break;
		case (USER_CARD | USER_FP):	 cr->accessEvent = E_ACCESS_DENIED_GOOD_CARD_BAD_FP; break;
		case (USER_CARD | USER_PIN): cr->accessEvent = E_ACCESS_DENIED_GOOD_CARD_BAD_PIN; break;
		case (USER_FP | USER_PIN):	 cr->accessEvent = E_ACCESS_DENIED_BAD_PIN; break;
		case (USER_CARD | USER_FP | USER_PIN): cr->accessEvent = E_ACCESS_DENIED_GOOD_CARD_FP_BAD_PIN; break;
		}
	} else {
		cr->retryCount++;
	}
}
*/

void authIdentifyResult(CREDENTIAL_READER *cr)
{
	unsigned char	msg[12];

	if(cr->accessEvent) {
		if(_IsGrantedEvent((int)cr->accessEvent)) _AccessGranted(cr);
		else	_AccessDenied(cr);	
	}
	if(crId(cr) == 0) {
		msg[0] = GM_IDENTIFY_RESULT; msg[1] = 0;
		appPostMessage(msg);
	}
}

void authVerifyResult(CREDENTIAL_READER *cr)
{
	unsigned char	msg[12];

	if(cr->accessEvent) {
		if(_IsGrantedEvent((int)cr->accessEvent)) _AccessGranted(cr);
		else	_AccessDenied(cr);	
	}
	if(crId(cr) == 0) {
		msg[0] = GM_VERIFY_RESULT; msg[1] = 0;
		appPostMessage(msg);
	}
}

int		AuthEvent;	
int		AuthResult;	
int		AuthMentID;
int		AuthMealsCount;
char	AuthMsg[100];

