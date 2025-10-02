#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "cr.h"
#include "fsuser.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "cf.h"
#include "dlg.h"
#include "topprim.h"
#include "admprim.h"
#include "admtitle.h"
#include "actprim.h"
#include "svcwork.h"
#include "appact.h"

void AdmUsrCredOnCreate(GACTIVITY *act);
void AdmUsrCredOnDestroy(GACTIVITY *act);
void AdmUsrCredOnTimer(GACTIVITY *act, int timerID);
void AdmUsrCredOnClick(GACTIVITY *act, void *view);
void AdmUsrCredOnAppMessage(GACTIVITY *act, unsigned char *msg);
int  fpmEnrollStage;
unsigned long	fpIds[2];
unsigned char	fpCapStage, fpCapFlag;
unsigned char	fpCapTmpl[800];


void AdmUsrCredStart(int requestCode, void *ps)
{
	GACTIVITY	*act;

	act = CreateActivity(AdmUsrCredOnCreate);
//	activitySetPsContext(act, (void *)index);
	activitySetTimeout(act, 30);
	if(ps) devSetTestMode(NULL, 1);
	appStartActivityForResult(act, requestCode, NULL, 0);
}

void AdmUsrCredOnCreate(GACTIVITY *act)
{
	void	*wnd, *v;
	CREDENTIAL_READER	*cr;
	CGRect		rt;
	char	*p, temp[80];
	int		val, index, x, y, w, h;

	cr = crsGet(0);
	activitySetOnDestroy(act, AdmUsrCredOnDestroy);
	activitySetOnTimer(act, AdmUsrCredOnTimer);
	activitySetOnClick(act, AdmUsrCredOnClick);
	activitySetOnAppMessage(act, AdmUsrCredOnAppMessage);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, 0xffbbdefb); 
	UIframe(wnd, &rt);
	x = y = 0; h = 48;
	if(devTestMode(NULL)) w = rt.width;
	else	w = rt.width >> 1;
	v = ViewAddTextButton(wnd, TAG_CANCEL_BTN, admCancelTitle(), x, y, w, h);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, 0xff00acc1);
	UIsetTitleColor(v, TABLE_GREY_50);
	if(!devTestMode(NULL)) {
		x += w;
		v = ViewAddTextButton(wnd, TAG_OK_BTN, admCompleteTitle(), x, y, w, h);
		UIsetOpaque(v, TRUE);
		UIsetBackgroundColor(v, 0xff00acc1);
		UIsetTitleColor(v, TABLE_GREY_50);
		UIsetUserInteractionEnabled(v, FALSE);
		UIsetTitleColor(v, 0xff9e9e9e);
	}
	y += h;
	if(cr->captureMode == USER_CARD) {
		if(devTestMode(NULL)) temp[0] = 0;
		else	GetUserTitleCardData(cr->data, temp);
	} else {
		index = (int)activityPsContext(act);
		if(index == 0) val = 2; else val = 3;
		strcpy(temp, xmenu_user_reg[val]);
	}
	v = ViewAddLabel(wnd, TAG_ADM_LABEL, temp, 0, y, rt.width, h, UITextAlignmentCenter);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_GREY_200);

	y += h + 32;
	if(cr->captureMode == USER_CARD) {
		h = 180; x = (rt.width - h) >> 1;
		strcpy(temp, "capcard.bmp");
		v = ViewAddImageViewAlphaMask(wnd, 0, temp, x, y, h, h);
		UIsetTintColor(v, blueColor);
	} else{
		h = 140;
		w = rt.width - h - h - 20;
		w >>= 1;
		if(cr->fpFlag) strcpy(temp, "fingerprint.bmp"); else temp[0] = 0;
		v = ViewAddImageViewAlphaMask(wnd, TAG_ADM_IMAGEVIEW1, temp, w, y, h, h);
		UIsetTintColor(v, blueColor);
		v = ViewAddImageViewAlphaMask(wnd, TAG_ADM_IMAGEVIEW2, temp, rt.width-w-h, y, h, h);
		UIsetTintColor(v, blueColor);
	}
	y += h + 32;
	h = 30; x = 0;
	if(cr->captureMode == USER_CARD) val = M_TOUCH_YOUR_CARD;
	else	val = M_PLACE_YOUR_FINGER;
	v = ViewAddLabel(wnd, TAG_ADM_LABEL2, GetPromptMsg(val), x, y, rt.width-x-x, h, UITextAlignmentCenter);
	UItimerStart(0, 60000, 0);
	if(cr->captureMode == USER_FP) {
		fpCapStage = fpCapFlag = 0;
		fpmRequestCapture();
	}
}

void AdmUsrCredOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
	devSetTestMode(NULL, 0);
}

void AdmUsrCredOnTimer(GACTIVITY *act, int timerID)
{
	if(timerID == 0) {
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
	}
}

void AdmUsrCredOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_CANCEL_BTN:
		UItimerStop(0);
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	case TAG_OK_BTN:
		UItimerStop(0);
		DestroyActivityForResult(act, RESULT_OK, NULL);
	}
}

extern unsigned char	reqCmd, reqOt, reqOp;
void acuReqFindUserCardData(char *data);
int  acuCnfFindUserCardData(void);

void _AdmUsrCredFindCardDataOnResult(GACTIVITY *act, int status, unsigned char *buf, int len)
{
	void	*wnd, *v, *a;
	CREDENTIAL_READER	*cr;
	char	temp[64];
	int		val, result;

	if(status) AcuRequestDestroy(act);
	else {
		result = buf[0];
		if(!result) val = R_CARD_DATA_OVERLAPPED;
		else if(result == R_OBJECT_NOT_FOUND) val = 0;
		else	val = R_FAILED;
		if(val) AcuRequestSetResult(act, val);
		else {
			a = activityParent(act);
			AcuRequestDestroy(act);
			wnd = activityWindow(a);
			v = UIviewWithTag(wnd, TAG_ADM_LABEL);
			cr = crsGet(0);
			GetUserCardData(cr->data, temp);
			UIsetText(v, temp);
			v = UIviewWithTag(wnd, TAG_OK_BTN);
			UIsetUserInteractionEnabled(v, TRUE);
			UIsetTitleColor(v, 0xfffafafa);
		}
	}
}

void _UsrCredCapturedCardResult(GACTIVITY *act, int msgIndex)
{
	void	*wnd, *v;
	CREDENTIAL_READER	*cr;
	char	temp[64];
	
	cr = crsGet(0);
	wnd = activityWindow(act);
	if(msgIndex) {
		AlertResultMsg(xmenu_user_reg[4], msgIndex);
	} else {
		v = UIviewWithTag(wnd, TAG_ADM_LABEL);
		GetUserCardData(cr->data, temp);
		UIsetText(v, temp);
		if(!devTestMode(NULL)) {
			v = UIviewWithTag(wnd, TAG_OK_BTN);
			UIsetUserInteractionEnabled(v, TRUE);
			UIsetTitleColor(v, 0xfffafafa);
		}
	}
}

void AdmUsrCredCapturedCard(GACTIVITY *act)
{
	CREDENTIAL_READER	*cr;
	FP_USER		*user, _user;
	int		rval, no;

	cr = crsGet(0);
	if(devTestMode(NULL)) {		// Test mode
		_UsrCredCapturedCardResult(act, 0);
	} else {
		user = activityPsContext(act);
		if(devMode(NULL)) {
			AcuRequestStart(NULL, NULL, _AdmUsrCredFindCardDataOnResult);
			acuReqFindUserCardData(cr->data);
		} else {
			user = &_user;
			userfsSeek(0L);
			rval = userfsGetCardData(user, cr->data);
			if(rval > 0 && user->id == cr->userId) rval = userfsGetCardData(user, cr->data);
			if(rval < 0) no = R_R_SYSTEM_ERROR;
			else if(rval > 0) no = R_CARD_DATA_OVERLAPPED;
			else	no = 0;
			_UsrCredCapturedCardResult(act, no);
		}
	}
	UItimerStart(0, 60000, 0);
}

void AdmUsrCredOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd, *v;
	CREDENTIAL_READER	*cr;
	unsigned char	fpTmpl[800];
	char	temp[64];
	int		rval, index, result;

	wnd = activityWindow(act);
	cr = crsGet(0);
	switch(msg[0]) {
	case GM_CRED_CAPTURED:
		if(cr->captureMode == USER_CARD) AdmUsrCredCapturedCard(act);
		break;
	case GM_FPM_TOUCHED:
		if(!fpmCommand()) {
printf("fpmTouched.....\n");
			fpmRequestCapture();
			UItimerStop(0); UItimerStart(0, 60000, 0);
		}
		break;
	case GM_FPM_RESPONSE:
		if(fpCapStage < 2) {
			fpmResponseCapture();
			if(cr->credType == CREDENTIAL_FP) {
				if(fpCapStage == 0) {
					fpmCopyTemplate(fpCapTmpl);
					fpCapStage = 1;
					UItimerStop(0); UItimerStart(0, 60000, 0);
					v = UIviewWithTag(wnd, TAG_ADM_LABEL2);
					UIsetText(v, GetPromptMsg(M_ONE_MORE_TIME_PLEASE));
					v = UIviewWithTag(wnd, TAG_OK_BTN);
					UIsetUserInteractionEnabled(v, FALSE);
					UIsetTitleColor(v, 0xff9e9e9e);
					ViewChangeImageViewAlphaMask(wnd, TAG_ADM_IMAGEVIEW1, "fingerprint.bmp");
					ViewChangeImageViewAlphaMask(wnd, TAG_ADM_IMAGEVIEW2, "");
printf("fpmResponseCaptutre...0\n");
				} else {
//printf("[%02x-%02x-%02x-%02x-%02x...]\n", (int)fpCapTmpl[0], (int)fpCapTmpl[1], (int)fpCapTmpl[2], (int)fpCapTmpl[3], (int)fpCapTmpl[4]);
					fpmCopyTemplate(fpCapTmpl+400);
					// a, b, a1, a2 ...
					// 거꾸로 하지 않으면 다른 지문 b 이후 동일 지문이라도 계속 0x04 오류 발생
					memcpy(fpTmpl, fpCapTmpl+400, 400);
					memcpy(fpTmpl+400, fpCapTmpl, 400);
					fpmRequestCreateTemplate(fpTmpl);
					fpCapStage = 2;
					UItimerStop(0); UItimerStart(0, 60000, 0);
printf("fpmResponseCaptutre...1\n");
				}
			}
		} else {
			rval = fpmResponseCreateTemplate();	
printf("fpmResponseCreateTemplate...%d\n", rval);
			if(rval) {
				AlertResultMsg(xmenu_user[1], R_DATA_MISMATCHED);
				fpCapStage = 1;
			} else {
				ViewChangeImageViewAlphaMask(wnd, TAG_ADM_IMAGEVIEW2, "fingerprint.bmp");
				v = UIviewWithTag(wnd, TAG_ADM_LABEL2);
				UIsetText(v, GetPromptMsg(M_PLACE_YOUR_FINGER));
				v = UIviewWithTag(wnd, TAG_OK_BTN);
				UIsetUserInteractionEnabled(v, TRUE);
				UIsetTitleColor(v, 0xfffafafa);
				fpCapFlag = 1;
				fpCapStage = 0;
			}
		}
		break;
	}
}

