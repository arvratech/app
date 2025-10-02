#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "user.h"
#include "gactivity.h"
#include "gapp.h"
#include "cr.h"
#include "aa.h"
#include "wglib.h"
#include "cf.h"
#include "cfprim.h"
#include "slvmlib.h"
#include "slvm.h"
#include "appact.h"
#include "authsvr.h"

extern unsigned char gFuncState, gFuncKey;

static unsigned char	funcState, funcKey;

#include "authprim.c"


void _AuthResult(CREDENTIAL_READER *cr)
{
	unsigned char	msg[12];

	msg[0] = GM_AUTH_RESULT; memset(msg+1, 0, 9);
cr->status = 0;
	appPostMessage(msg);
}

void AuthIdentifyCredential(CREDENTIAL_READER *cr, FP_USER *user)
{
	int		rval, mode, eventID;

printf("AuthIdentifyCredentials...credType=%d\n", cr->credType);
	cr->result = cr->accessEvent = cr->captureMode = cr->retryCount = cr->authMode = cr->authDone = 0;
	rtcGetDateTime(cr->accessTime);
	funcState = gFuncState; funcKey = gFuncKey;
	switch(cr->credType) {
	case CREDENTIAL_ID:
		cr->credMode = 0;
		user->id = userDecodeID(cr->data);
		rval = userfsGet(user);
		if(rval < 0) cr->result = 2;
		else if(!rval) cr->result = 1;
		break;
	case CREDENTIAL_CARD: cr->credMode = USER_CARD; break;
	default:	cr->result = 3;
	}
	if(cr->result) {
		_AuthResult(cr);
		return;
	}
	mode = crAuthorizationMode(cr);
	if(cr->credMode == USER_CARD && mode == 3) {
		cr->accessEvent = E_ACCESS_GRANTED_ALL;
		_AccessGranted(cr, user);
		_AuthResult(cr);
		return;
	}
	if(cr->credMode == USER_CARD) {
		rval = crIdentifyMatchCard(cr, user);
printf("crIdentifyMatchCard=%d\n", rval);
	} else {
		rval = 1;
	}
	if(rval < 0) cr->result = 2;		
	else if(!rval) {
		cr->accessEvent = E_ACCESS_DENIED_UNREGISTERED_CARD;
		_AccessUnregistered(cr, 1);
	} else if(mode == 2) {
		cr->accessEvent = E_ACCESS_DENIED_ALL;
		_AccessDenied(cr, user);
		rval = 0;
	}
	if(rval < 1) {
		_AuthResult(cr);
		return;
	}
	cr->authMode = _GetAuthMode(0, (int)user->accessMode);
	if(!cr->authMode) cr->accessEvent = E_ACCESS_DENIED_ACCESS_MODE;	
	else {
		cr->accessEvent = _CheckAccess(cr, user);		
		if(!cr->accessEvent) cr->accessEvent = _CheckLockState(0);
	}
	if(cr->accessEvent) {
		_AccessDenied(cr, user);
	} else {
		cr->authDone = cr->credMode;
		cr->captureMode = _GetCaptureMode((int)cr->authMode, (int)cr->authDone);
		if(!cr->captureMode) {
			cr->accessEvent = _GetAccessGranted((int)cr->authDone);
			_AccessGranted(cr, user);
		}
	}
	_AuthResult(cr);
}

void AuthVerifyCredential(CREDENTIAL_READER *cr, FP_USER *user)
{
	int		rval, authMode;

printf("AuthVerifyfyCredentials: credType=%d authMode=%02x authDone=%02x\n", (int)cr->credType, (int)cr->authMode, (int)cr->authDone);
	rtcGetDateTime(cr->accessTime);
	switch(cr->credType) {
	case CREDENTIAL_CARD:	cr->credMode = USER_CARD; break;
	case CREDENTIAL_PIN:	cr->credMode = USER_PIN; break;
	default:	cr->result = 3;
	}
	if(cr->result) {
		_AuthResult(cr);
		return;
	}
	authMode = cr->authMode;
	if(cr->captureMode & cr->credMode) {
		if(cr->credMode == USER_CARD) rval = crVerifyMatchCard(cr, user);
		else	rval = crVerifyMatchPin(cr, user);
		if(rval < 0) cr->result = 2;
	} else	cr->result = 3;
	if(cr->result) {
		_AuthResult(cr);
		return;
	}
	if(rval > 0) {
		cr->authDone |= cr->credMode;
		cr->captureMode = _GetCaptureMode((int)cr->authMode, (int)cr->authDone);
		if(!cr->captureMode) {
			cr->accessEvent = _GetAccessGranted((int)cr->authDone);
			_AccessGranted(cr, user);
		} else	cr->retryCount = 0;
	} else if(cr->retryCount) {
		cr->authDone |= cr->credMode;
		cr->accessEvent = _GetAccessDenied((int)cr->authMode, (int)cr->authDone);
		_AccessDenied(cr, user);
	} else {
		cr->retryCount++;
	}
	_AuthResult(cr);
}

// Return	-1=Others  0=No  1=Access denied  2=Access granted
int LocalIdentifyCard(CREDENTIAL_READER *cr, FP_USER *user)
{
	int		rval, mode, EventID, AuthMode;

	cr->result = 0;
	rtcGetDateTime(cr->accessTime);
	AuthMode = crAuthorizationMode(cr);
printf("LocalIdentifycard...authMode=%d\n", AuthMode);
	if(AuthMode == 3) {
		cr->accessEvent = E_ACCESS_GRANTED_ALL;
		_AccessGranted(cr, user);
		return 2;		
	}
	rval = crIdentifyMatchCard(cr, user);
printf("IdentifyMatchCard...rval=%d\n", rval);
	if(rval < 1) {
		//memcpy(ap->data, cr->data, 9);	
		if(mode == 1) {
			_AccessUnregisteredEvent(cr);
			rval = 3;	
		} else {
			_AccessUnregistered(cr, 1);
			rval = 1;
		}
		return rval;
	}
	//memcpy(ap->data, cr->data+(cr->cardIndex-1)*9, 9);	
	memcpy(cr->data, cr->data+(cr->cardIndex-1)*9, 9);	
	if(AuthMode == 2) {
		cr->accessEvent = E_ACCESS_DENIED_ALL;
		_AccessDenied(cr, user);
		return 2;		
	}
printf("IdentifyMatchCard...\n");
	mode = _GetAuthModeCardOnly(0, user);
	if(!mode) EventID = E_ACCESS_DENIED_ACCESS_MODE;	
	else {
		EventID = _CheckCardStatus(user);
		if(!EventID) EventID = _CheckAccess(cr, user);
		if(!EventID) EventID = _CheckLockState(cr);
	}
printf("IdentifyMatchCard...EventID=%d\n", EventID);
	if(!EventID) EventID = E_ACCESS_GRANTED_CARD;
	cr->accessEvent = EventID;
	if(evtIsGranted(EventID)) {
		_AccessGranted(cr, user);
		rval = 2;
	} else {
		_AccessDenied(cr, user);
		rval = 1;
	}
	return rval;
}


void AuthWiegand(CREDENTIAL_READER *cr)
{
	void	*wf, *pf, *pwf;
	unsigned char	data[100], wgdata[100];
	int		len, ow;

	if(cr->data[0] == CRED_PIN) {
		pf = cfPinGet();
		if(len > 0) {
			len = cfPinEncodeData(pf, cr->data, data);
			wgdata[0] = 0x20;
			ow = cfPinOutputWiegand(pf);
			pwf = cfWgPinsGet(ow);
			if(cfWgPinParity(pwf)) len = wgEncode(data, len, wgdata+2);
			else	memcpy(wgdata+2, data, (len+7)>>3);
			wgdata[1] = len;
		}
	} else if(cr->data[0] == CRED_WIEGAND) {
		wgdata[0] = 0x20;
		len = cr->data[1];
		memcpy(wgdata+2, data, (len+7)>>3);
	} else {
		wf = cfWgCardsGet((int)cr->outputWiegand);
printf("credData: %d %02x-%02x-%02x-%02x\n", (int)cr->data[0], (int)cr->data[1], (int)cr->data[2], (int)cr->data[3], (int)cr->data[4]);
		len = cfWgCardEncodeData(wf, cr->data, data);
printf("wgData: %d %02x-%02x-%02x-%02x\n", len, (int)data[0], (int)data[1], (int)data[2], (int)data[3]);
		wgdata[0] = 0x20;
		if(cfWgCardParity(wf)) len = wgEncode(data, len, wgdata+2);
		else	memcpy(wgdata+2, data, (len+7)>>3);
		wgdata[1] = len;
printf("wgEncData: %02x %d %02x-%02x-%02x-%02x-%02x\n", (int)wgdata[0], (int)wgdata[1], (int)wgdata[2], (int)wgdata[3], (int)wgdata[4], wgdata[5], (int)wgdata[6]);
	}
	if(len) {
		slvmWriteCredBuffer(wgdata, 2 + ((len+7)>>3));
	}
}

void PinCharWiegand(int ch)
{
	void	*pf;
	unsigned char	val, data[4], wgdata[4];
	int		ow, len;

	pf = cfPinGet();
	ow = cfPinOutputWiegand(pf);
	if(ch == '*') data[0] = 10;
	else if(ch == '#') data[0] = 11;
	else	data[0] = ch - '0';
	if(ow == 0) {
		wgdata[0] = 4;
		wgdata[1] = len = 4; 
		memcpy(wgdata+2, data, (len+7)>>3);
	} else if(ow == 1) {
		wgdata[0] = 4;
		wgdata[1] = len = 8; 
		val = data[0];
		data[0] |= ~(val << 4 | 0x0f);
		memcpy(wgdata+2, data, (len+7)>>3);
	} else	len = 0;
	if(len) slvmWriteCredBuffer(wgdata, 2 + ((len+7)>>3));
}

void PinStrWiegand(char *str)
{
	void	*pf, *pwf;
	unsigned char	data[4], wgdata[4];
	int		ow, val, len;

	pf = cfPinGet();
	ow = cfPinOutputWiegand(pf);
	pwf = cfWgPinsGet(ow);
	if(ow == 2) { 
		data[0] = cfWgPinFacilityCode(pf);
		val = n_atoi(str);
		IDtoPACK3(val, data+1);
		wgdata[0] = 4;
		len = wgEncode(data, 24, wgdata+2);
		wgdata[1] = len;
	} else if(ow == 3) {
		len = strlen(str);
		string2bcd(str, len, data);
		len <<= 2;
		if(cfWgPinParity(pwf)) len = wgEncode(data, len, wgdata+2);
		else	memcpy(wgdata+2, data, (len+7)>>3);
		wgdata[0] = 4;
		wgdata[1] = len;
	} else	len = 0;
	if(len) slvmWriteCredBuffer(wgdata, 2 + ((len+7)>>3));
}

int		AuthEvent;	
int		AuthResult;	
int		AuthMentID;
int		AuthMealsCount;
char	AuthMsg[100];

