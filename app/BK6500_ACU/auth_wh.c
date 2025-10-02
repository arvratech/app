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
#include "sclib.h"
#include "msg.h"
#include "evt.h"
#include "sche.h"
#include "sinport.h"
#include "slavenet.h"
#include "menuprim.h"
#include "wnds.h"
#include "desktop.h"
#include "devprim.h"
#include "topprim.h"
#include "auth.h"

extern unsigned char ServerNetState, gFuncState, gFuncKey;
extern int		gCommand;

unsigned char svrevt_buf[EVENT_RECORD_SZ];
int		svrevt_count;
int		AuthCode;
char	AuthMsg[48];

void _AccessUnregistered(ACCESS_POINT *ap, CREDENTIAL_READER *cr, unsigned char *ctm);
void _AccessGranted(ACCESS_POINT *ap, CREDENTIAL_READER *cr, FP_USER *user, unsigned char *ctm);
void _ProcessAuthResult(ACCESS_POINT *ap, CREDENTIAL_READER *cr, FP_USER *user);
void _AccessDeniedNoEvent(ACCESS_POINT *ap, CREDENTIAL_READER *cr, unsigned char *ctm);
void _AccessGrantedNoEvent(ACCESS_POINT *ap, CREDENTIAL_READER *cr, unsigned char *ctm);


// 창고관리시스템: PIN-Only
void ApIdentifyCredential(ACCESS_POINT *ap, CREDENTIAL_READER *cr)
{
	FP_USER			*user, _user;
	unsigned long	timer;
	unsigned char	ctm[8];
	int		rval;

	user = &_user;
	cr->AuthType = 0;
	cr->Result = cr->AccessEvent = cr->CardIndex = cr->CaptureMode = 0;
	ap->SessionNo++;
	ap->AuthMode = ap->AuthDone = 0;
	if(cr->CredentialType != CREDENTIAL_PIN) {
		cr->Result = 3;
		_ProcessAuthResult(ap, cr, NULL);
		return;
	}
cprintf("PIN=[%02x-%02x-%02x-%02x]\n", (int)cr->Data[0], (int)cr->Data[1], (int)cr->Data[2], (int)cr->Data[3]);
	user->ID = 0; user->UserExternalID[0] = user->UserName[0] = 0;
	rtcGetDateTime(ctm);
	if(ServerNetState != 3 || gCommand == P_EVENT2) {
		userfsSeek(0L);
		rval = userfsGetPINData(user, cr->Data);
		if(rval < 0) {
			cr->Result = 2;		
			user = NULL;
		} else if(!rval) {
			cr->AccessEvent = E_ACCESS_DENIED_UNREGISTERED_FP;
			cr->Data[0] = 0;
			_AccessUnregistered(ap, cr, ctm);
			user = NULL;
			strcpy(AuthMsg, GetResultMsg(R_USER_NOT_FOUND));			
		} else {
			ap->UserID = user->ID;
			userfsGetEx(user);
			cr->AccessEvent = E_ACCESS_GRANTED_FP;
			_AccessGranted(ap, cr, user, ctm);
			if(user->UserName[0]) strcpy(AuthMsg, user->UserName);
			else	sprintf(AuthMsg, "%ld", user->ID);
		}
		/*
		buf[0] = 191; bin2bcd(ctm, 6, buf+1);
		buf[7] = 1;
		buf[8] = 0x20;
		memcpy(buf+9, cr->Data, 4);
		memset(buf+13, 0xff, 3);
		evtAddRaw(buf, 16);
		cr->AccessEvent = E_DENIED_FAIL_SERVER;
		strcpy(AuthMsg, "네트워크 장애입니다");
		AuthMentID = 0;
		*/
	} else {
		svrevt_buf[0] = OT_ACCESS_POINT;
		svrevt_buf[1] = ap->ID;
		svrevt_buf[2] = 190;
		datetime2longtime(ctm, &timer); LONGtoBYTE(timer, svrevt_buf+3);
		svrevt_buf[7] = 0x20;
		memcpy(svrevt_buf+8, cr->Data, 4);
		memset(svrevt_buf+12, 0xff, 4);
		svrevt_count = 1;
		AuthCode = -1;
		timer = DS_TIMER;
		while((DS_TIMER-timer) < 30) {
			if(AuthCode >= 0) break;
			taskYield();
		}
		if(AuthCode >= 0) {
			if(AuthCode == 0) {
				cr->AccessEvent = E_ACCESS_GRANTED_CARD;
				_AccessGrantedNoEvent(ap, cr, ctm);
			} else {
				cr->AccessEvent = E_ACCESS_DENIED_BAD_CARD;
				_AccessDeniedNoEvent(ap, cr, ctm);
			}
		} else {
			cr->AccessEvent = E_ACCESS_DENIED_BAD_CARD;
			strcpy(AuthMsg, "서버에서\n처리 지연");
		}
		user = NULL;
	}
	_ProcessAuthResult(ap, cr, user);
}

void ApVerifyCredential(ACCESS_POINT *ap, CREDENTIAL_READER *cr)
{
	cr->AuthType = 1;
	cr->AccessEvent = cr->CardIndex = cr->CaptureMode = 0;
	cr->Result = 3;
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

void _ProcessAuthResult(ACCESS_POINT *ap, CREDENTIAL_READER *cr, FP_USER *user)
{
	unsigned char	*p, *p0;
	int		ID, result;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxRspData(ID);
	*p++ = cr->Result;
	*p++ = cr->AccessEvent;
	*p++ = cr->CardIndex;
	*p++ = cr->CaptureMode;
	*p++ = ap->SessionNo;
	if(cr->Result == 0) {
		memcpy_pad(p, AuthMsg, 48); p += 48;
	}
cprintf("%ld CRU-%d: Identify Result=%d Event=%d Data=%d\n", DS_TIMER, ID, (int)cr->Result, (int)cr->AccessEvent, p - p0);
	SlaveNetSetTxRspDataSize(ID, p - p0, 0);
	UpdateReaderEvent(ID);
}
