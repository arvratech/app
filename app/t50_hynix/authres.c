#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "jpegdev.h"
#include "msg.h"
#include "fsuser.h"
#include "cr.h"
#include "ad.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "dlg.h"
#include "evt.h"
#include "topprim.h"
#include "admprim.h"
#include "admtitle.h"
#include "actprim.h"
#include "appact.h"

extern int	authCount;

AUTH_USER	*AuthUser, _AuthUser;

void AuthResultOnCreate(GACTIVITY *act);
void AuthResultOnDestroy(GACTIVITY *act);
void AuthResultOnTimer(GACTIVITY *act, int timerId);
void AuthResultOnClick(GACTIVITY *act, void *view);
void AuthResultOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AuthResultMsgMent(GACTIVITY *act, AUTH_USER *user);


void AuthResultStart(void)
{
	GACTIVITY	*act;
	CREDENTIAL_READER	*cr;

	cr = crsGet(0);
	AuthUser = &_AuthUser;
	AuthUser->id = cr->userId;
	AuthUser->status = cr->status;
	AuthUser->accessEvent = cr->accessEvent;
	memcpy(AuthUser->accessTime, cr->accessTime, 6);
	AuthUser->credType = cr->credType;
	memcpy(AuthUser->credData, cr->data, 9);
	strcpy(AuthUser->userExternalId, cr->userExternalId);
	AuthUser->jpegSize = 0;
	authCount++;
	UItimerStop(1);
	act = CreateActivity(AuthResultOnCreate);
	appStartActivityForResult(act, ACT_AUTHRESULT, NULL, 1);
}

#include <sys/stat.h> 
unsigned long	tloop;

void AuthResultOnCreate(GACTIVITY *act)
{
	void	*wnd, *lbl, *imgv, *ad;
	CGRect		rt;
	struct stat	_stat;
	unsigned char	*p, ments[2];
	char	*st, temp[80];
	int		x, y, w, h, color, size, more;

//printf("AuthResultOnCreate...\n");
	activitySetOnStart(act, NULL);
	activitySetOnDestroy(act, AuthResultOnDestroy);
	activitySetOnTimer(act, AuthResultOnTimer);
	activitySetOnClick(act, AuthResultOnClick);
	activitySetOnAppMessage(act, AuthResultOnAppMessage);
	activitySetViewBuffer(act, ViewAllocBuffer());
PrintBuffer("AuthRes");
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	more = 0;
//AuthUser->accessEvent = 146;
	if(evtIsGranted(AuthUser->accessEvent)) {
		color = greenColor;
		strcpy(temp, "user.bmp");
		ments[0] = 120; ments[1] = 100;
		if(!devMode(NULL)) {
			ad = adsGet(0);
			if(adIsNonNull(ad)) AdSetPv(ad, PV_PULSE_UNLOCK, 8);
		}
	} else {
		ments[0] = 121;
		if(AuthUser->accessEvent == E_ACCESS_DENIED_UNREGISTERED_CARD
						|| AuthUser->accessEvent == E_ACCESS_DENIED_UNREGISTERED_EMV_CARD
						|| AuthUser->accessEvent == E_ACCESS_DENIED_UNREGISTERED_FP) {
			strcpy(temp, "cross.bmp");
			ments[1] = 102;
		} else {
			strcpy(temp, "user.bmp"); more = 1;
			ments[1] = 101;
		}
		color = redColor;	
	}
	AuthResultMsgMent(act, AuthUser);
	if(sys_cfg->userSoundVolume) MentUserNormalMultiple(AuthUser->ments, 2);
	UIsetBackgroundColor(wnd, color); 
	UIsetTintColor(wnd, blackColor);
	UIframe(wnd, &rt);
//printf("Window: %d,%d %d %d\n", rt.x, rt.y, rt.width, rt.height);
//	if(AuthUser->accessEvent == E_ACCESS_DENIED_UNREGISTERED_CARD) size = 0;
//	else	size = fsGetUserPhoto(AuthUser->id, AuthUser->jpegBuffer);
size = 0;
	if(size > 0) {
		w = 240; x = (rt.width - w) >> 1;
		y = 20; h = 320;
		//imgv = ViewAddImageViewJpeg(wnd, 0, temp2, x, y, w, h);
		imgv = ViewAddImageViewMemJpeg(wnd, 0, AuthUser->jpegBuffer, size, x, y, w, h);
		y += h + 10;
	} else {
		y = 60; h = rt.width * 6 / 10;
		x = (rt.width - h) >> 1;
		imgv = ViewAddImageViewAlphaMask(wnd, 0, temp, x, y, h, h);
		UIsetTintColor(imgv, blueColor);
		if(more) {
			imgv = ViewAddImageViewAlphaMask(imgv, 0, "cancel.bmp", 128, 128, 64, 64);
			UIsetOpaque(imgv, TRUE);
			UIsetBackgroundColor(imgv, clearColor);
			UIsetTintColor(imgv, darkGrayColor);
		}
		y += h + 40;
	}
	x = 40; w = rt.width - 2 * x;
	datetime2str(AuthUser->accessTime, temp);
	h = 22;
	lbl = ViewAddLabel(wnd, 0, temp, x, y, w, h, UITextAlignmentCenter);
	UIsetTextColor(lbl, blackColor);
#ifndef _HYNIX
	if(AuthUser->accessEvent != E_ACCESS_GRANTED_ALL
				&& AuthUser->accessEvent != E_ACCESS_GRANTED_ALL_EMV
				&& AuthUser->accessEvent != E_ACCESS_DENIED_UNREGISTERED_CARD
				&& AuthUser->accessEvent != E_ACCESS_DENIED_UNREGISTERED_EMV_CARD
				&& AuthUser->accessEvent != E_ACCESS_DENIED_UNREGISTERED_FP) {
		y += h + 20; h = 28;
//printf("y=%d msg=[%s]\n", y, GetResultMsg(AuthUser->msgIndex));
		sprintf(temp, "id: %ld", AuthUser->id);
		lbl = ViewAddLabel(wnd, 0, temp, x, y, w, h, UITextAlignmentCenter);
		UIsetFont(lbl, 28);
		UIsetTextColor(lbl, blackColor);
	}
#endif
	y += h + 20; h = 28;
	if(AuthUser->authIndex) st = xauth_msgs[AuthUser->authIndex-1];
	else	st = GetResultMsg(AuthUser->msgIndex);
	strcpy_swap(temp, st, '\n', ' ');
	lbl = ViewAddLabel(wnd, 0, temp, 10, y, rt.width-20, h, UITextAlignmentCenter);
	UIsetFont(lbl, 28);
	//statSetLinePitch(stat, 8);
	UIsetTextColor(lbl, blackColor);
	w = syscfgAuthResultTime(NULL) * 100; if(w <= 0) w = 100;
	UItimerStart(1, w, 0);
}

void AuthResultOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

void AuthResultOnTimer(GACTIVITY *act, int timerId)
{
	void	*wnd;

	wnd = activityWindow(act);
	if(timerId == 1) {
		DestroyActivityForResult(act, RESULT_TIMEOUT, NULL);
	}
}

void AuthResultOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
		UItimerStop(1);
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	}
}

void AuthIdentify(GACTIVITY *act, int clearTop);

void AuthResultOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;
	ACCESS_DOOR	_ad;
	int		result;

	wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_CRED_CAPTURED:
		if(!syscfgResultInhibitAuth(NULL)) {
			UItimerStop(1);
			AuthIdentify(act, 1);
		}
		break;
	}
}

void AuthResultMsgMent(GACTIVITY *act, AUTH_USER *user)
{
	void	*ad;
	CREDENTIAL_READER	*cr;
	int		evt, authidx, pv;

	evt = user->accessEvent;
	if(evtIsGranted(evt)) {
		user->msgIndex = R_ACCESS_GRANTED;
		user->ments[0] = 120;
		if(psuBatPower(NULL) && !devEnableBatteryLock(NULL)) user->ments[1] = 160;
		else	user->ments[1] = 100;
		user->authIndex = 0;
	} else {
		if(evt == E_DENIED_FAIL_SERVER) {
			user->msgIndex = R_ACCESS_DENIED_FAIL_SERVER;
			authidx = 0;
			user->ments[0] = 121; user->ments[1] = 101;
		} else if(evt == E_ACCESS_DENIED_UNREGISTERED_FP || evt == E_ACCESS_DENIED_UNREGISTERED_CARD
						|| evt == E_ACCESS_DENIED_UNREGISTERED_EMV_CARD) {
			user->msgIndex = R_USER_NOT_FOUND;
			authidx = 0;
			user->ments[0] = 121; user->ments[1] = 102;
		} else {
			user->msgIndex = R_ACCESS_DENIED;	
			switch(evt) {
			case E_ACCESS_DENIED_USER_NOT_ACTIVATED:
			case E_ACCESS_DENIED_USER_EXPIRED:			authidx = 1; break;
			case E_ACCESS_DENIED_ACCESS_CONTROL:
				ad = adsGet(0);
				if(ad && adIsNonNull(ad)) {
					if(adPresentValue(ad)) authidx = 2; else authidx = 3;
				} else {
					cr = crsGet(0);
					pv = (cr->captureMode >> 3) & 0x03;
					if(pv & 1) authidx = 2; else authidx = 3;
				}
				break;
			case E_ACCESS_DENIED_ACCESS_RIGHTS:			authidx = 4; break;
			case E_ACCESS_DENIED_LOST_CARD:				authidx = 5; break;
			case E_ACCESS_DENIED_ACCESS_MODE:			authidx = 6; break;
			case E_ACCESS_DENIED_PASSBACK:				authidx = 7; break;
			case E_DENIED_MEAL_OVERLAP:					authidx = 8; break;
			case E_ACCESS_DENIED_ALL:					authidx = 9; break;
			default:									authidx = 0;
			}
			user->ments[0] = 121; user->ments[1] = 101;
		}
		if(authidx > 0 && authidx < 9) user->ments[1] = 150 + authidx - 1;
		user->authIndex = authidx;
//string_co2db(user->credData, 9, temp);
	}
}

