#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "cr.h"
#include "dev.h"
#include "user.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "prefspec.h"
#include "viewprim.h"
#include "dlg.h"
#include "slvnet.h"
#include "topprim.h"
#include "admprim.h"
#include "actprim.h"
#include "svcwork.h"
#include "authmst.h"
#include "appact.h"

void AcuRequestOnCreate(GACTIVITY *act);
void AcuRequestOnStart(GACTIVITY *act);
void AcuRequestOnDestroy(GACTIVITY *act);
void AcuRequestOnTimer(GACTIVITY *act, int timerID);
static void	*lbl;

void (*_AcuRequestOnResult)(GACTIVITY *act, int status, unsigned char *buf, int len);
void (*_AcuRequestOnSuccess)(GACTIVITY *act);


void AcuRequestStart(char *intent, void *ps, void (*AcuRequestOnResult)(GACTIVITY *, int, unsigned char *, int))
{
	GACTIVITY	*act;

	act = CreateActivity(AcuRequestOnCreate);
	activitySetPsContext(act, ps);
	_AcuRequestOnResult = AcuRequestOnResult;
	_AcuRequestOnSuccess = NULL;
	appStartActivityForResult(act, ACT_AUTHREQUEST, intent, 0);
}

void AcuRequestRestart(char *intent, void (*AcuRequestOnResult)(GACTIVITY *, int, unsigned char *, int))
{
	GACTIVITY	*act;
	void	*ps;

	act = appCurrentActivity();
	ps = activityPsContext(act);
	act = CreateActivity(AcuRequestOnCreate);
	activitySetPsContext(act, ps);
	_AcuRequestOnResult = AcuRequestOnResult;
	_AcuRequestOnSuccess = NULL;
	appStartActivityForResult(act, ACT_AUTHREQUEST, intent, 1);
}

void AcuRequestSetOnSuccess(void (*AcuRequestOnSuccess)(GACTIVITY *))
{
	_AcuRequestOnSuccess = AcuRequestOnSuccess;
}

void _PSobjectRestoreValue(void *ps)
{
	switch(PStype(ps)) {
	case PS_TYPE_CHECK:			PScheckRestoreValue(ps); break;
	case PS_TYPE_RADIO_GROUP:	PSradioGroupRestoreValue(ps); break;
	case PS_TYPE_SWITCH:		PSswitchRestoreValue(ps); break;
	case PS_TYPE_STEPPER:		PSstepperRestoreValue(ps); break;
	case PS_TYPE_SLIDER:		PSsliderRestoreValue(ps); break;
	case PS_TYPE_SPINNER:		PSspinnerRestoreValue(ps); break;
	}
}

void AcuRequestSetResult(GACTIVITY *act, int result);

void AcuRequestCallOnResult(int status, unsigned char *buf, int len)
{
	GACTIVITY *act;
	void	*ps;

	act = appCurrentActivity();
	if(status) {
		AcuRequestSetResult(act, R_NETWORK_NOT_OBTAINABLE);
	} else if(_AcuRequestOnResult) {
		(*_AcuRequestOnResult)(act, 0, buf, len);
	} else {
		if(buf[0]) {
			AcuRequestSetResult(act, R_FAILED);
		} else {
			ps = activityPsContext(act);
			_PSobjectRestoreValue(ps);
			AcuRequestDestroy(act);
			if(_AcuRequestOnSuccess) {
				act = appCurrentActivity();
				(*_AcuRequestOnSuccess)(act);
			}
		}
	}
}

void AcuRequestOnCreate(GACTIVITY *act)
{
	void	*wnd, *v;
	CGRect	rect;
	CREDENTIAL_READER	*cr;
	unsigned char	msg[12];
	char	temp[32], iconFile[32];
	int		x, y, w, h;

printf("AcuRequestOnCreate...\n");
	activitySetOnDestroy(act, AcuRequestOnDestroy);
	activitySetOnTimer(act, AcuRequestOnTimer);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIframe(wnd, &rect);
	w = rect.width - 40;  h = 128;
	x = (rect.width - w) >> 1; y = (rect.height - h) >> 1;
	rect.x = x; rect.y = y; rect.width = w; rect.height = h;
	UIsetFrame(wnd, &rect);
	UIsetBackgroundColor(wnd, whiteColor);
	UIsetOpaque(wnd, TRUE);
//	UIsetTintColor(wnd, blackColor);
	strcpy(iconFile, "waiting.bmp");
	x = 8; y = 0;
	v = ViewAddLabel(wnd, 0, "ACU master", 8, y, w-x-x, 48, UITextAlignmentLeft);
	UIsetTextColor(v, blackColor);
	y += 48;
	//x = 4; //v = ViewAddImageViewAlphaMask(wnd, 0, iconFile, x, y+16, 48, 48); x += 48;
	v = ViewAddLabel(wnd, 0, GetResultMsg(R_PROCESSING), x, y, w-x-x, 80, UITextAlignmentCenter);
	UIsetTextColor(v, blackColor);
	lbl = v;
	w = sys_cfg->authWaitTime * 100; if(w <= 0) w = 100;
	UItimerStart(1, w, 0);
}

void AcuRequestSetResult(GACTIVITY *act, int result)
{
	void	*wnd;

	UItimerStop(1);
	wnd = activityWindow(act);
	UIsetBackgroundColor(wnd, magentaColor);
	UIsetText(lbl, GetResultMsg(result));
	UItimerStart(2, 2200, 0);
}

void AcuRequestDestroy(GACTIVITY *act)
{
	UItimerStop(1);
	DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
}

void AcuRequestOnDestroy(GACTIVITY *act)
{
//printf("AcuRequestOnDestroy...\n");
	AdmActExit(act);
}

void PrefReloadRowAtIndex(void *act, int index);

void AcuRequestOnTimer(GACTIVITY *act, int timerId)
{
	void	*ps, *pss;
	int		index;

	switch(timerId) {
	case 1:
		AcuRequestSetResult(act, R_ACU_TIMEOUT);
		break;
	case 2: 
		if(_AcuRequestOnResult) {
			(*_AcuRequestOnResult)(act, 1, NULL, 0);
		} else {
			ps = activityPsContext(act);
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
			act = appCurrentActivity();
			pss = activityPsContext(act);
			index = PSindexForObject(pss, ps);
			PrefReloadRowAtIndex(act, index);
		}
		break;
	}
}

/*
void acuProcessResponse(void *buf)
{
	FP_USER		*user;
	unsigned char	*p, msg;
	char	temp[32];
	long	lnum;
	int		result, size, count;

	p = SlvnetConfRspBuf(); size = SlvnetConfRspRxSize();
printf("Rx Rsp: size=%d\n", size);
	result = 0;
	switch(reqCmd) {
	case 0x30:
		buf[0] = GM_CONF_CNF;
		appPostMessage(buf);
		break;
	case 0x31:
		buf[0] = GM_CONF_CNF;
		appPostMessage(buf);
		break;
	case P_READ_OBJECT:
		switch(reqOt) {
		case OT_USER:
			user = &_user;
			result = acuCnfReadUser(user);
			break;
		}
		break;
	case P_WRITE_OBJECT:
		switch(reqOt) {
		case OT_USER:
			result = acuCnfWriteUser();
			break;
		}
		break;
	case P_DELETE_OBJECT:
		switch(reqOt) {
		case OT_USER:
			result = acuCnfDeleteUser();
			break;
		}
		break;
	case P_FIND_OBJECT:
		switch(reqOt) {
		case OT_USER:
			result = acuCnfFindUser();
			break;
		}
		break;	
	case P_READ_PROPERTY:
		switch(reqOt) {
		case OT_DEVICE:
			switch(reqOp) {
			case OP_DEVICE_PASSWORD:
				result = acuCnfReadPassword(temp);
				break;
			}
			break;
		}
		break;	
	case P_WRITE_PROPERTY:
		switch(reqOt) {
		case OT_USER:
			switch(reqOp) {
			case OP_FP_AUTHENTICATION_FACTOR:
				result = acuCnfWriteUserFPTemplate();
				break;
			}
			break;
		}
		break;
	case P_DELETE_PROPERTY:
		switch(reqOt) {
		case OT_USER:
			switch(reqOp) {
			case OP_FP_AUTHENTICATION_FACTOR:
				result = acuCnfDeleteUserFPTemplate();
				break;
			}
			break;
		}
		break;
	case P_GET_PROPERTY:
		switch(reqOt) {
		case OT_USER:
			switch(reqOp) {
			case 0x30:
				result = acuCnfGetNewUserID(&lnum);
				break;
			case 0x31:
				result = acuCnfGetUserCount(&count);
				break;
			}
			break;
		}
		break;	
	case P_FIND_PROPERTY:
		switch(reqOt) {
		case OT_USER:
			switch(reqOp) {
			case OP_FP_AUTHENTICATION_FACTOR:
				user = &_user;
				result = acuCnfFindUserFPTemplate(user);
				break;
			case OP_SECOND_CARD_AUTHENTICATION_FACTOR:
				result = acuCnfFindUserCardData(buf);
				break;
			}
			break;
		}
		break;	
	case P_EVENT_NOTIFICATION:
		result = acuCnfEventNotification();
		break;
	}	
}
*/

