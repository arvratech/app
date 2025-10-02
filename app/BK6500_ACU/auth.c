#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "iodev.h"
#include "key.h"
#include "lcdc.h"
#include "syscfg.h"
#include "acad.h"
#include "acap.h"
#include "unit.h"
#include "fsuser.h"
#include "cr.h"
#include "sinport.h"
#include "slavenet.h"
#include "sclib.h"
#include "msg.h"
#include "evt.h"
#include "sche.h"
#include "menuprim.h"
#include "wnds.h"
#include "desktop.h"
#include "devprim.h"
#include "topprim.h"
#include "auth.h"

extern unsigned long WdtResetTimer;

int  _IsGrantedEvent(int EventID);
int  _GetAuthMode(CREDENTIAL_READER *cr, int AuthMode, int MaskMode);
int  _GetCaptureMode(int AuthMode, int AuthDone);
int  _CheckCardStatus(CREDENTIAL_READER *cr, FP_USER *user);
int  _CheckAccess(CREDENTIAL_READER *cr, FP_USER *user, unsigned char *ctm);
int  _CheckLockState(CREDENTIAL_READER *cr);
int  _CheckPassback(CREDENTIAL_READER *cr, FP_USER *user, unsigned char *ctm);
void _AccessUnregistered(ACCESS_POINT *ap, CREDENTIAL_READER *cr, unsigned char *ctm);
void _AccessDenied(ACCESS_POINT *ap, CREDENTIAL_READER *cr, FP_USER *user, unsigned char *ctm);
void _AccessGranted(ACCESS_POINT *ap, CREDENTIAL_READER *cr, FP_USER *user, unsigned char *ctm);
int  crIdentifyMatchCard(CREDENTIAL_READER *cr, FP_USER *user);

/*
// PIN-Only
void ApIdentifyCredential(ACCESS_POINT *ap, CREDENTIAL_READER *cr)
{
	FP_USER			*user, _user;
	unsigned char	ctm[8];
	int		rval, mode;

	user = &_user;
	cr->AuthType = 0;
	cr->Result = cr->AccessEvent = cr->CardIndex = cr->CaptureMode = 0;
	ap->SessionNo++;
	ap->AuthMode = ap->AuthDone = 0;
	if(cr->CredentialType != CREDENTIAL_PIN) {
		cr->Result = 3;
		ProcessAuthResult(ap, cr, NULL);
		return;
	}
	user->UserExternalID[0] = user->UserName[0] = 0;
	rtcGetDateTime(ctm);
	mode = apGetAuthorizationMode((int)ap->ID);
	userfsSeek(0L);
	rval = userfsGetPINData(user, cr->Data);
cprintf("rval=%d [%02x-%02x-%02x-%02x]\n", rval, (int)cr->Data[0], (int)cr->Data[1], (int)cr->Data[2], (int)cr->Data[3]);
	if(rval < 0) {
		cr->Result = 2;		
		ProcessAuthResult(ap, cr, NULL);
	} else if(!rval) {
		cr->AccessEvent = E_ACCESS_DENIED_UNREGISTERED_FP;
		cr->Data[0] = 0;
		_AccessUnregistered(ap, cr, ctm);
		ProcessAuthResult(ap, cr, NULL);
	} else {
		ap->UserID = user->ID;
		userfsGetEx(user);
		cr->AccessEvent = E_ACCESS_GRANTED_FP;
		_AccessGranted(ap, cr, user, ctm);
		ProcessAuthResult(ap, cr, user);
	}
}
*/

void ApIdentifyCredential(ACCESS_POINT *ap, CREDENTIAL_READER *cr)
{
	FP_USER			*user, _user;
	unsigned char	ctm[8];
	int		rval, mode, CredMode;

	user = &_user;
	cr->AuthType = 0;
	cr->Result = cr->AccessEvent = cr->CardIndex = cr->CaptureMode = 0;
	ap->SessionNo++;
	ap->AuthMode = ap->AuthDone = 0;
	switch(cr->CredentialType) {
	case CREDENTIAL_ID:
		CredMode = 0;
		userDecodeID((long *)&user->ID, cr->Data);
		rval = userfsGet(user);
		if(rval < 0) cr->Result = 2;
		else if(!rval) cr->Result = 1;
		break;
	case CREDENTIAL_CARD:	 CredMode = USER_CARD; break;
	case CREDENTIAL_FP:		 CredMode = USER_FP; break;
	default:	cr->Result = 3;
	}
	if(cr->Result) {
		ProcessAuthResult(ap, cr, NULL);
		return;
	}
	user->UserExternalID[0] = user->UserName[0] = 0;
	rtcGetDateTime(ctm);
	mode = apGetAuthorizationMode((int)ap->ID);
	if(CredMode == USER_CARD && mode == 4) {
		if(cr->readerFormat[0] == 0x02 || cr->readerFormat[0] == 0x30)
			cr->AccessEvent = E_ACCESS_GRANTED_ALL_EMV;
		else
			cr->AccessEvent = E_ACCESS_GRANTED_ALL;		
		_AccessGranted(ap, cr, NULL, ctm);
		ProcessAuthResult(ap, cr, NULL);
		return;
	}
	if(CredMode == USER_CARD) {
		rval = crIdentifyMatchCard(cr, user);
//cprintf("crIdentifyMatchCard=%d\n", rval);
	} else if(CredMode == USER_FP) {
		if(DS_TIMER > WdtResetTimer+25L) {
			wdtReset(); taskYield();
		}
		rval = userIdentifyMatchFP(user, cr->Data);
	} else {
		rval = 1;
	}
	if(rval < 0) {
		cr->Result = 2;		
		ProcessAuthResult(ap, cr, NULL);
//EventAdd(OT_DEVICE, 0, E_MAKE_CALL, ctm, NULL);
//wdtReset(); taskDelay(15); wdtResetSystem();
	} else if(!rval) {
		if(CredMode == USER_FP) {
			cr->AccessEvent = E_ACCESS_DENIED_UNREGISTERED_FP;	
			cr->Data[0] = 0;
		} else {
			if(cr->readerFormat[0] == 0x02 || cr->readerFormat[0] == 0x30)
				cr->AccessEvent = E_ACCESS_DENIED_UNREGISTERED_EMV_CARD;
			else
				cr->AccessEvent = E_ACCESS_DENIED_UNREGISTERED_CARD;
		}
		_AccessUnregistered(ap, cr, ctm);
		ProcessAuthResult(ap, cr, NULL);
	} else if(mode == 3) {
		cr->AccessEvent = E_ACCESS_DENIED_ALL;
		_AccessDenied(ap, cr, user, ctm);
		ProcessAuthResult(ap, cr, user);
		rval = 0;
	}
	if(rval < 1) return;
	ap->UserID = user->ID;
	userfsGetEx(user);
	ap->AuthMode = _GetAuthMode(cr, (int)user->AccessMode, CredMode);
//cprintf("UserID=%ld AR=%d AM=%02x=>%02x Reader=0x%02x\n", user->ID, user->AccessRights, (int)user->AccessMode, (int)ap->AuthMode, (int)cr->cfg->Reader); 
	if(!ap->AuthMode) cr->AccessEvent = E_ACCESS_DENIED_ACCESS_MODE;	
	else {
		if(user->AccessRights < 0) userfsGetAccessRights(user);
		if(CredMode == USER_CARD) cr->AccessEvent = _CheckCardStatus(cr, user);
		else	cr->AccessEvent = 0;
/*
if(!cr->AccessEvent) {
	biID = unitGetBinaryInput((int)ap->ID);
	if(!biGetPresentValue(biID+2) || !biGetPresentValue(biID+3)) cr->AccessEvent = E_ACCESS_DENIED_ACCESS_CONTROL;
}
if(!cr->AccessEvent) {
	if(ap->ID == 1) {
		if(ctm[3] < 12 || ctm[0] == 12 && ctm[4] < 30) rval = -1;
		else if(ctm[3] > 14 || ctm[3] == 14 && ctm[4] >= 0) rval = 1;
		else	rval = 0;
	} else	rval = 0;
	if(rval) cr->AccessEvent = E_ACCESS_DENIED_ACCESS_RIGHTS;	
}
*/
		if(!cr->AccessEvent) cr->AccessEvent = _CheckAccess(cr, user, ctm);
		if(!cr->AccessEvent) cr->AccessEvent = _CheckLockState(cr);
		if(!cr->AccessEvent) cr->AccessEvent = _CheckPassback(cr, user, ctm);
	}
	if(!cr->AccessEvent && CredMode == ap->AuthMode) {
		if(CredMode == USER_CARD) {
			if(cr->readerFormat[0] == 0x02 || cr->readerFormat[0] == 0x30)
				cr->AccessEvent = E_ACCESS_GRANTED_EMV_CARD;
 			else
				cr->AccessEvent = E_ACCESS_GRANTED_CARD;
		} else	cr->AccessEvent = E_ACCESS_GRANTED_FP;
	}
	if(cr->AccessEvent) {
		if(_IsGrantedEvent((int)cr->AccessEvent)) _AccessGranted(ap, cr, user, ctm);
		else	_AccessDenied(ap, cr, user, ctm);	
	} else {
		ap->AuthDone = CredMode;
		cr->CaptureMode = _GetCaptureMode((int)ap->AuthMode, (int)ap->AuthDone);
		cr->RetryCount = 0;
	}
	ProcessAuthResult(ap, cr, user);
}

void ApVerifyCredential(ACCESS_POINT *ap, CREDENTIAL_READER *cr)
{
	FP_USER		*user, _user;
	unsigned char	ctm[8];
	int		rval, mode, AuthMode, CredMode;

	user = &_user;
	cr->AuthType = 1;
	cr->Result = cr->AccessEvent = cr->CardIndex = cr->CaptureMode = 0;
	rtcGetDateTime(ctm);
	if(cr->SessionNo != ap->SessionNo) cr->Result = 1;
	else {
		switch(cr->CredentialType) {
		case CREDENTIAL_CARD:	CredMode = USER_CARD; break;
		case CREDENTIAL_FP:		CredMode = USER_FP; break;
		case CREDENTIAL_PIN:	CredMode = USER_PIN; break;
		default:	cr->Result = 3;
		}
	}
	if(!cr->Result) {
		user->ID = ap->UserID;
		rval = userfsGet(user);
		if(user->AccessRights < 0) userfsGetAccessRights(user);
		if(rval < 0) cr->Result = 2;
		else if(!rval) cr->Result = 1;
	}
	if(cr->Result) {
		ProcessAuthResult(ap, cr, NULL);
		return;
	}
	AuthMode = ap->AuthMode;
	mode = _GetCaptureMode(AuthMode, (int)ap->AuthDone);
	if(mode & CredMode) {
		if(CredMode == USER_CARD) rval = crVerifyMatchCard(cr, user->CardData);
		else if(CredMode == USER_FP) rval = userVerifyMatchFP(user, cr->Data);
		else	rval = userVerifyMatchPIN(user, cr->Data);
		if(rval < 0) cr->Result = 2;
	} else	cr->Result = 3;
	if(cr->Result) {
		ProcessAuthResult(ap, cr, NULL);
		return;
	}
	if(!ap->AuthDone) AuthMode = _GetAuthMode(NULL, AuthMode, CredMode);
	if(rval) cr->Result = 0; else cr->Result = 1;
	if(!cr->Result) {
		if(!ap->AuthDone) {
			if(AuthMode == CredMode) {
				if(CredMode == USER_CARD) {
					if(cr->readerFormat[0] == 0x02 || cr->readerFormat[0] == 0x30)
						cr->AccessEvent = E_ACCESS_GRANTED_EMV_CARD;
 					else
						cr->AccessEvent = E_ACCESS_GRANTED_CARD;
				} else if(CredMode == USER_FP) cr->AccessEvent = E_ACCESS_GRANTED_FP;
				else	cr->AccessEvent = E_ACCESS_GRANTED_PIN;
			}
		} else if(ap->AuthDone == USER_CARD) {
			if(AuthMode != USER_CARD_FP_PIN) {
				if(CredMode == USER_FP) cr->AccessEvent = E_ACCESS_GRANTED_CARD_FP;
				else	cr->AccessEvent = E_ACCESS_GRANTED_CARD_PIN;
			}
		} else if(ap->AuthDone == USER_FP) {
			cr->AccessEvent = E_ACCESS_GRANTED_FP;	// non-exist E_ACCESS_GRANTED_FP_PIN
		} else {
			cr->AccessEvent = E_ACCESS_GRANTED_CARD_FP_PIN;
		}
		ap->AuthDone |= CredMode;
		ap->AuthMode = AuthMode;
		cr->CaptureMode = _GetCaptureMode(AuthMode, (int)ap->AuthDone);
		cr->RetryCount = 0;
		if(cr->AccessEvent) _AccessGranted(ap, cr, user, ctm);
	} else if(cr->RetryCount) {
		if(!ap->AuthDone) {	
			if(CredMode == USER_CARD) cr->AccessEvent = E_ACCESS_DENIED_BAD_CARD;
			else if(CredMode == USER_FP) cr->AccessEvent = E_ACCESS_DENIED_BAD_FP;
			else	cr->AccessEvent = E_ACCESS_DENIED_BAD_PIN;
		} else if(ap->AuthDone == USER_CARD) {
			if(AuthMode == (USER_CARD_FP | USER_CARD_PIN)) 
				cr->AccessEvent = E_ACCESS_DENIED_GOOD_CARD_BAD_FP_BAD_PIN;
			else if(AuthMode == USER_CARD_FP_PIN) cr->AccessEvent = E_ACCESS_DENIED_GOOD_CARD_BAD_FP_PIN;
			else if(CredMode == USER_FP) cr->AccessEvent = E_ACCESS_DENIED_GOOD_CARD_BAD_FP;
			else	cr->AccessEvent = E_ACCESS_DENIED_GOOD_CARD_BAD_PIN;
		} else if(ap->AuthDone == USER_FP) {
			cr->AccessEvent = E_ACCESS_DENIED_BAD_FP;
		} else {
			cr->AccessEvent = E_ACCESS_DENIED_GOOD_CARD_FP_BAD_PIN;
		}
		_AccessDenied(ap, cr, user, ctm);	
	} else {
		ap->AuthMode = AuthMode;
		cr->CaptureMode = _GetCaptureMode(AuthMode, (int)ap->AuthDone);
		cr->RetryCount++;
	}
	ProcessAuthResult(ap, cr, NULL);
}

void ApCaptureCredential(CREDENTIAL_READER *cr)
{
	unsigned char	*d0, *d;
	int		ID;

	ID = cr->ID;
	d = d0 = SlaveNetGetTxRspData(ID);
	*d++ = 0;
	*d++ = 0;
	SlaveNetSetTxRspDataSize(ID, d-d0, 0);
	cr->Captured = 1;
}
