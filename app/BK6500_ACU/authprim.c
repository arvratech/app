#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "iodev.h"
#include "sio.h"
#include "sin.h"
#include "wiegand.h"
#include "key.h"
#include "lang.h"
#include "gfont.h"
#include "msg.h"
#include "lcdc.h"
#include "syscfg.h"
#include "unit.h"
#include "acad.h"
#include "acap.h"
#include "alm.h"
#include "fsuser.h"
#include "acar.h"
#include "sclib.h"
#include "cr.h"
#include "msg.h"
#include "evt.h"
#include "event.h"
#include "sche.h"
#include "sinport.h"
#include "slavenet.h"
#include "menuprim.h"
#include "wnds.h"
#include "desktop.h"
#include "devprim.h"
#include "topprim.h"
#include "auth.h"


int _IsGrantedEvent(int evtId)
{
	if(evtId >= 130 && evtId <= 134 || evtId == 150 || evtId == 155 || evtId == 171 || evtId == 177) return 1;
	else	return 0;
}

static int _EnableFPReader(CREDENTIAL_READER *cr)
{
	int		rval, ID;

	ID = cr->ID;
	rval = 0;
	if(unitGetType(ID) == 2 || crGetType(ID)) {
		if(crGetFPReaderEnable(cr->cfg)) rval = 1;
	}
	return rval;
}

static int _EnableCardReader(CREDENTIAL_READER *cr)
{	
	int		rval, ID;

	ID = cr->ID;
	rval = 0;
	if(unitGetType(ID) == 2 || crGetType((int)cr->ID)) {
		if(crGetCardReaderEnable(cr->cfg)) rval = 1;
	} else {
		if(crGetWiegandReaderEnable(cr->cfg)) rval = 1;
	}
	return rval;
}

static int _EnablePINReader(CREDENTIAL_READER *cr)
{
	int		rval, ID;

	ID = cr->ID;
	rval = 0;
	if(unitGetType(ID) == 2 || crGetType((int)cr->ID)) {
		if(crGetPINReaderEnable(cr->cfg)) rval = 1;
	} else {
		//if(crGetPINReaderEnable(cr->cfg)) rval = 1;
	}
	return rval;
}

int _GetAuthMode(CREDENTIAL_READER *cr, int AuthMode, int MaskMode)
{
	int		mode;

	if(MaskMode == USER_CARD) {
		if(cr && !_EnableCardReader(cr)) mode = 0;
		else {
			mode = AuthMode & (USER_CARD | USER_CARD_FP | USER_CARD_PIN | USER_CARD_FP_PIN);
			if(cr) {
				if(!_EnableFPReader(cr)) mode &= ~(USER_CARD_FP | USER_CARD_FP_PIN);
				if(!_EnablePINReader(cr)) mode &= ~(USER_CARD_PIN | USER_CARD_FP_PIN);
			}
			if(mode & USER_CARD_FP_PIN) mode &= ~(USER_CARD | USER_CARD_FP | USER_CARD_PIN);
			else if(mode & (USER_CARD_FP | USER_CARD_PIN)) mode &= ~USER_CARD;
		}
	} else if(MaskMode == USER_FP) {
		if(cr && !_EnableFPReader(cr)) mode = 0;
		else {
			mode = AuthMode & (USER_FP | USER_FP_PIN);
			if(cr) {
				if(!_EnablePINReader(cr)) mode &= ~(USER_FP_PIN);
			}
			if(mode & USER_FP_PIN) mode &= ~USER_FP;
		}
	} else if(MaskMode == USER_PIN) {
		if(cr && !_EnablePINReader(cr)) mode = 0;
		else	mode = AuthMode & USER_PIN;
	} else {
		mode = AuthMode;
		if(!_EnableFPReader(cr)) mode &= ~(USER_FP | USER_FP_PIN | USER_CARD_FP | USER_CARD_FP_PIN);
		if(!_EnableCardReader(cr)) mode &= ~(USER_CARD | USER_CARD_FP | USER_CARD_PIN | USER_CARD_FP_PIN);
		if(!_EnablePINReader(cr)) mode &= ~(USER_PIN | USER_CARD_PIN | USER_CARD_FP_PIN | USER_FP_PIN);
		if(mode & USER_CARD_FP_PIN) mode &= ~(USER_CARD | USER_CARD_FP | USER_CARD_PIN);
		else if(mode & (USER_CARD_FP | USER_CARD_PIN)) mode &= ~USER_CARD;
		if(mode & USER_FP_PIN) mode &= ~USER_FP;
	}
	return mode;
}

int _GetCaptureMode(int AuthMode, int AuthDone)
{
	int		mode;

	mode = 0;
	if(!AuthDone) {
		if(AuthMode & (USER_FP | USER_FP_PIN)) mode |= USER_FP;
		if(AuthMode & (USER_CARD | USER_CARD_FP | USER_CARD_PIN | USER_CARD_FP_PIN)) mode |= USER_CARD;
		if(AuthMode & USER_PIN) mode |= USER_PIN;
	} else if(AuthDone == USER_CARD) {
		if(AuthMode == USER_CARD_FP || AuthMode == USER_CARD_FP_PIN) mode = USER_FP;
		else if(AuthMode == USER_CARD_PIN) mode = USER_PIN;
		else if(AuthMode == (USER_CARD_FP | USER_CARD_PIN)) mode = USER_FP | USER_PIN;
	} else if(AuthDone == USER_FP) {
		if(AuthMode == USER_FP_PIN) mode = USER_PIN;
	} else if(AuthMode == (USER_CARD | USER_FP)) {
		if(AuthMode == USER_CARD_FP_PIN) mode = USER_PIN;
	}
	return mode;
}

int _CheckCardStatus(CREDENTIAL_READER *cr, FP_USER *user)
{
	int		evtId;

	evtId = 0;
	if(user->CardStatus == 4) {
		if(cr->readerFormat[0] == 0x02 || cr->readerFormat[0] == 0x30) evtId = E_ACCESS_DENIED_ACCESS_MODE;
	} else if(user->CardStatus == 3) {
		if(cr->readerFormat[0] != 0x02 && cr->readerFormat[0] != 0x30) evtId = E_ACCESS_DENIED_ACCESS_MODE;
	} else if(user->CardStatus != 1) evtId = E_ACCESS_DENIED_LOST_CARD;
	return evtId;
}

// Return	0=OK  >0=Event ID  
int _CheckAccess(CREDENTIAL_READER *cr, FP_USER *user, unsigned char *ctm)
{
	int		EventID, AuthMode;
	
	AuthMode = apGetAuthorizationMode((int)cr->ID);
	if(date_validate(user->ActivateDate) && date_comp(ctm, user->ActivateDate) < 0) EventID = E_ACCESS_DENIED_USER_NOT_ACTIVATED;
	else if(date_validate(user->ExpireDate) && date_comp(ctm, user->ExpireDate) > 0) EventID = E_ACCESS_DENIED_USER_EXPIRED;
	else if(AuthMode != 2 && !userEvaluateAccessRights(user, (int)cr->ID, ctm)) EventID = E_ACCESS_DENIED_ACCESS_RIGHTS;
	else	EventID = 0;
	return EventID;
}

// Return	0=OK  E_ACCESS_DENIED_ACCESS_CONTROL=Fail 
int _CheckLockState(CREDENTIAL_READER *cr)
{
	int		adID, PV, EventID;

	adID = apGetAssignedDoor((int)cr->ID);
	EventID = 0;
	if(adID >= 0) {
		PV = adGetPresentValue(adID);
		if(PV != S_NORMAL_LOCKED && PV != S_NORMAL_UNLOCKED) EventID = E_ACCESS_DENIED_ACCESS_CONTROL;
	}
	return EventID;
}

int _CheckPassback(CREDENTIAL_READER *cr, FP_USER *user, unsigned char *ctm)
{
	int		rval, EventID;
	
	if(sys_cfg->Passback & 0x01) {
		rval = userfsGetTransaction(user);
		if(rval <= 0) {
			user->AccessPoint = 99; user->ZoneTo = 0;
		}
		rval = userDelayPassback(user, (int)cr->ID, ctm);
		if(rval) {
			user->Passback = 0;
			EventID = 0;
		} else {		
			rval = userEvaluatePassbackOnExit(user, (int)cr->ID, ctm);
//cprintf("userEvaluatePassbackOnExit=%d\n", rval);
			if(!rval) rval = userEvaluatePassback(user, (int)cr->ID, ctm);
			user->Passback = (unsigned char)rval;
			if(rval == 1) EventID = E_ACCESS_DENIED_PASSBACK;
			else	EventID = 0;
		}
	} else {
		user->Passback = 0;
		EventID = 0;
	}
//cprintf("CheckPassback: PassbackMode=%d EventID=%d\n", (int)user->Passback, EventID);
	return EventID;
}

void _AccessUnregistered(ACCESS_POINT *ap, CREDENTIAL_READER *cr, unsigned char *ctm)
{
	int		ID;

	ID = ap->ID;
	ap->UserID = 0;
	ap->AccessEvent = cr->AccessEvent; memcpy(ap->AccessTime, ctm, 8);
//userfsGetCount();
//cprintf("ID=%d AccessUnregistered\n", ID);
	if(actIsEnable(ID) && ocExist(actGetOutputCommand(ID, 7))) actSetPresentValue(ID, 7);
	EventAdd(OT_ACCESS_POINT, ID, (int)ap->AccessEvent, ap->AccessTime, cr->Data);
//userfsGetCount();
}

void _AccessDenied(ACCESS_POINT *ap, CREDENTIAL_READER *cr, FP_USER *user, unsigned char *ctm)
{
	unsigned char	EventData[12];
	char	data[32];
	int		ID;
	
	ID = ap->ID;
	if(user) ap->UserID = user->ID; else ap->UserID = 0;
	ap->AccessEvent = cr->AccessEvent; memcpy(ap->AccessTime, ctm, 8);
//userfsGetCount();
//cprintf("ID=%d AccessDenied: UserID=[%ld] Event=%d\n", ID, user->ID, ap->AccessEvent);
	if(actIsEnable(ID)) {
		if(ap->AccessEvent == E_ACCESS_DENIED_USER_EXPIRED) { 
			if(ocExist(actGetOutputCommand(ID, 8))) actSetPresentValue(ID, 8);
		} else if(ap->AccessEvent == E_ACCESS_DENIED_PASSBACK) {
			if(ocExist(actGetOutputCommand(ID, 9))) actSetPresentValue(ID, 9); 
		} else {
			if(ocExist(actGetOutputCommand(ID, 10))) actSetPresentValue(ID, 10);
		}
	}
	utoa02((int)cr->FuncKey, data); sprintf(data+2, "%ld", user->ID);
	digits2card(data, EventData);
	EventAdd(OT_ACCESS_POINT, ID, (int)ap->AccessEvent, ap->AccessTime, EventData);
//userfsGetCount();
}

void _AccessGranted(ACCESS_POINT *ap, CREDENTIAL_READER *cr, FP_USER *user, unsigned char *ctm)
{
	unsigned char	EventData[12];
	char	data[32];
	int		ID, adID, EventID;

//cprintf("Card=[%02x", (int)user->CardData[0]); for(adID = 1;adID < 9;adID++) cprintf("-%02x", (int)user->CardData[adID]); cprintf("]\n");
	ID = ap->ID;
	if(user) ap->UserID = user->ID; else ap->UserID = 0;
	ap->AccessEvent = cr->AccessEvent; memcpy(ap->AccessTime, ctm, 8);
	EventID = ap->AccessEvent;
//userfsGetCount();
//cprintf("ID=%d AccessGranted: UserID=[%ld] Event=%d\n", ID, user->ID, EventID);
	adID = apGetAssignedDoor(ID);
	if(adID >= 0) adSetPresentValue(adID, S_NORMAL_UNLOCKED);
	if(ap->Duress) {
		if(user) {
			sprintf(data, "%ld", user->ID); digits2card(data, EventData);
			EventAdd(OT_ACCESS_POINT, ID, E_ACCESS_DURESS, ap->AccessTime, EventData);
		} else {
			EventAdd(OT_ACCESS_POINT, ID, E_ACCESS_DURESS, ap->AccessTime, cr->Data);
		}
	}
	if(EventID == E_ACCESS_GRANTED_ALL || EventID == E_ACCESS_GRANTED_ALL_EMV) {
		EventAdd(OT_ACCESS_POINT, ID, EventID, ap->AccessTime, cr->Data);
	} else {
		if(user && user->Passback == 2) { 
			sprintf(data, "%ld", user->ID); digits2card(data, EventData);
			EventAdd(OT_ACCESS_POINT, ID, E_PASSBACK_DETECTED, ap->AccessTime, EventData);
			if(ocExist(actGetOutputCommand(ID, 9))) actSetPresentValue(ID, 9); 
		}
		ap->State = 2;
		if((sys_cfg->Passback & 0x01) && user && adID >= 0 && apValidateZone(ap->cfg)) {
			memcpy(user->AccessTime, ctm, 6);
			user->AccessPoint = ap->ID;
			user->ZoneTo	  = ap->cfg->ZoneTo;
			userfsAddTransaction(user);
		}
		utoa02((int)cr->FuncKey, data); sprintf(data+2, "%ld", user->ID);
		digits2card(data, EventData);
		EventAdd(OT_ACCESS_POINT, ID, EventID, ap->AccessTime, EventData);
	}
//userfsGetCount();
}

void _AccessDeniedNoEvent(ACCESS_POINT *ap, CREDENTIAL_READER *cr, unsigned char *ctm)
{
	int		ID;
	
	ID = ap->ID;
	ap->AccessEvent = cr->AccessEvent; memcpy(ap->AccessTime, ctm, 8);
	if(actIsEnable(ID)) {
		if(ap->AccessEvent == E_ACCESS_DENIED_USER_EXPIRED && ocExist(actGetOutputCommand(ID, 8))) actSetPresentValue(ID, 8);
		else if(ap->AccessEvent != E_ACCESS_DENIED_USER_EXPIRED && ocExist(actGetOutputCommand(ID, 10))) actSetPresentValue(ID, 10);
	}	
}

void _AccessGrantedNoEvent(ACCESS_POINT *ap, CREDENTIAL_READER *cr, unsigned char *ctm)
{
	int		ID, adID;

	ID = ap->ID;
	ap->AccessEvent = cr->AccessEvent; memcpy(ap->AccessTime, ctm, 8);
	adID = apGetAssignedDoor(ID);
	if(adID >= 0) adSetPresentValue(adID, S_NORMAL_UNLOCKED);
}