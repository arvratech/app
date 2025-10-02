#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "defs_obj.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "cr.h"
#include "dev.h"
#include "slvmlib.h"
#include "slvm.h"
#include "user.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "dlg.h"
#include "slvnet.h"
#include "topprim.h"
#include "admprim.h"
#include "admtitle.h"
#include "actprim.h"
#include "authmst.h"
#include "appact.h"
#include "uv.h"

void AuthRequestOnCreate(GACTIVITY *act);
void AuthRequestOnStart(GACTIVITY *act);
void AuthRequestOnDestroy(GACTIVITY *act);
void AuthRequestOnTimer(GACTIVITY *act, int timerID);
void AuthRequestOnAppMessage(GACTIVITY *act, unsigned char *msg);


void AuthRequestStart(void)
{
	GACTIVITY	*act;

printf("AuthRequestStart...\n");
	act = CreateActivity(AuthRequestOnCreate);
	appStartActivityForResult(act, ACT_AUTHREQUEST, NULL, 0);
}

void AuthRequestRestart(void)
{
	GACTIVITY	*act;

printf("AuthRequestRestart...\n");
	act = CreateActivity(AuthRequestOnCreate);
	appStartActivityForResult(act, ACT_AUTHREQUEST, NULL, 1);
}

void AuthRequestIdentify(GACTIVITY *act);
void AuthOnReceivedRsp(unsigned char *buf, int size);

void AuthRequestOnCreate(GACTIVITY *act)
{
	void	*wnd, *v;
	CGRect	rt;
	CREDENTIAL_READER	*cr;
	ACCESS_DOOR			*ad;
	unsigned char	msg[12], buf[4];
	char	temp[32], iconFile[32];
	int		x, y, w, h;

printf("AuthRequestOnCreate...\n");
	activitySetOnStart(act, AuthRequestOnStart);
	activitySetOnDestroy(act, AuthRequestOnDestroy);
	activitySetOnTimer(act, AuthRequestOnTimer);
	activitySetOnAppMessage(act, AuthRequestOnAppMessage);
	activitySetViewBuffer(act, ViewAllocBuffer());
PrintBuffer("AuthReq");
	wnd = activityWindow(act);
	UIframe(wnd, &rt);
	w = rt.width - 40;  h = 128;
	x = (rt.width - w) >> 1; y = (rt.height - h) >> 1;
	rt.x = x; rt.y = y; rt.width = w; rt.height = h;
	UIsetFrame(wnd, &rt);
	UIsetBackgroundColor(wnd, grayColor);
	UIsetOpaque(wnd, TRUE);
//	UIsetTintColor(wnd, blackColor);
	strcpy(iconFile, "waiting.bmp");
	x = 8; y = 0;
	v = ViewAddLabel(wnd, 0, "Authentication", 8, y, w-x, 48, UITextAlignmentLeft);
	UIsetTextColor(v, blackColor);
	y += 48;
	x = 4;
	v = ViewAddImageViewAlphaMask(wnd, 0, iconFile, x, y+16, 48, 48);
	x += 48;
	v = ViewAddLabel(wnd, 0, GetResultMsg(R_PROCESSING), x, y, w-x, 80, UITextAlignmentCenter);
	UIsetTextColor(v, blackColor);
	w = sys_cfg->authWaitTime * 100; if(w <= 0) w = 100;
	UItimerStart(1, w, 0);
	cr = crsGet(0);
	if(cr->credType == CREDENTIAL_FP) {
//		fpmRequestIdentifyMatch();
		msg[0] = GM_IDENTIFY_RESULT;
		appPostMessage(msg);
	} else {
		if(!SlvnetIsConnected()) {
			if(cr->credType == CREDENTIAL_CARD) {
				if(cr->readerFormat == 0x30 || cr->readerFormat == 0x02) cr->accessEvent = E_ACCESS_GRANTED_ALL_EMV;
				else    cr->accessEvent = E_ACCESS_GRANTED_ALL;
				cr->result = 0;
				cr->userId = 0; cr->userName[0] = cr->userExternalId[0] = 0;
				rtcGetDateTime(cr->accessTime);
				msg[0] = GM_IDENTIFY_RESULT;
				appPostMessage(msg);
				ad = adsGet(0);
				if(!psuBatPower(NULL) || devEnableBatteryLock(NULL)) {
printf("ad=%x adisNonNull=%d\n", ad, adIsNonNull(ad));
					if(ad && adIsNonNull(ad)) {
						if(adPriority(ad) > 7) {    // insert 2020.4.21
							ad->hymo = 0;
							slvmDoorCommand(PV_PULSE_UNLOCK, 1);	// rte:0=>1 2019.12.13
						}
					} else {
						buf[0] = 50; buf[1] = 0; buf[2] = 1;
						slvmOutputCommand(0, buf);
					}
				}
				EventAdd(OT_ACCESS_POINT, devId(NULL), (int)cr->accessEvent, cr->accessTime, cr->data);
			} else {

			}
		} else {
			acuReqIdentify(cr);
			//acuSetOnReceivedRsp(AuthOnReceivedRsp);
		}
	}
}

void AuthRequestOnStart(GACTIVITY *act)
{
}

void AuthRequestOnDestroy(GACTIVITY *act)
{
//printf("AuthRequestOnDestroy...\n");
	AdmActExit(act);
}

void AuthRequestOnTimer(GACTIVITY *act, int timerID)
{
	CREDENTIAL_READER	*cr;

	if(timerID == 1) {
		cr = crsGet(0);
		cr->result = 5;
		DestroyActivityForResult(act, RESULT_OK, NULL);
	}
}

void AuthRequestReceived(GACTIVITY *act);

void AuthOnReceivedRsp(unsigned char *buf, int size)
{
	void	*act, *wnd;
	CREDENTIAL_READER	*cr;
	int		rval;

	act = appCurrentActivity();
	wnd = activityWindow(act);
	cr = crsGet(0);
	UItimerStop(1);
	AuthRequestReceived(act);
	if(crResult(cr)) DestroyActivityForResult(act, RESULT_OK, NULL);
	else if(crAccessEvent(cr)) {
		if(cr->evtbuf[0] && camEventWithCamImage(NULL)) {
			rval = evtcamAdd(cr->evtbuf);	
			if(rval > 0) SvrnetSendCamEvent(); 
		}
		AuthResultStart();
	} else	AuthCaptureStart();
}

void AuthRequestOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;
	CREDENTIAL_READER	*cr;
	unsigned char	buf[12];
	int		rval;

printf("AuthRequestOnAppMessage=%d\n", (int)msg[0]);
	wnd = activityWindow(act);
	cr = crsGet(0);
	switch(msg[0]) {
	case GM_IDENTIFY_RESULT:
		UItimerStop(1);
		if(crResult(cr)) DestroyActivityForResult(act, RESULT_OK, NULL);
		else if(crAccessEvent(cr)) AuthResultStart();
		else	AuthCaptureStart();
		break;
	case GM_CONF_CNF:
		UItimerStop(1);
		//SvrProcessResponse();
		AuthRequestReceived(act);
		if(crResult(cr)) DestroyActivityForResult(act, RESULT_OK, NULL);
		else if(crAccessEvent(cr)) {
			if(cr->evtbuf[0] && camEventWithCamImage(NULL)) {
				rval = evtcamAdd(cr->evtbuf);	
				if(rval > 0) SvrnetSendCamEvent(); 
			}
			AuthResultStart();
		} else	AuthCaptureStart();
		break;
	case GM_FPM_RESPONSE:
		UItimerStop(1);
		fpmResponseIdentifyMatch();
		if(crResult(cr)) DestroyActivityForResult(act, RESULT_OK, NULL);
		else if(crAccessEvent(cr)) AuthResultStart();
		else	AuthCaptureStart();
		break;
	}
}

void AuthRequestReceived(GACTIVITY *act)
{
	CREDENTIAL_READER	*cr;

	cr = crsGet(0);
	acuCnfIdentify(cr);
if(cr->result > 1) printf("IndenifyResult: result=%d user=%ld [%s][%s]\n", (int)cr->result, cr->userId, cr->userName, cr->userExternalId);	
else if(cr->userId > 0) printf("IdentifyResult: event=%d user=%ld [%s][%s]\n", (int)cr->accessEvent, cr->userId, cr->userName, cr->userExternalId);
else	 printf("IdentifyResult: event=%d\n", (int)cr->accessEvent);
/*
cr->accessEvent = E_ACCESS_GRANTED_CARD;
cr->userId = 2053412;
cr->userId = 1000105; cr->userName[0] = cr->userExternalId[0] = 0;
*/
	//while(cr->Result < 2 && !cr->AccessEvent) MstVerifyUser(cr);
}

