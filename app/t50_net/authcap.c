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

void AuthCaptureOnCreate(GACTIVITY *act);
void AuthCaptureOnStart(GACTIVITY *act);
void AuthCaptureOnDestroy(GACTIVITY *act);
void AuthCaptureOnTimer(GACTIVITY *act, int timerID);
void AuthCaptureOnClick(GACTIVITY *act, void *view);
void AuthCaptureOnAppMessage(GACTIVITY *act, unsigned char *msg);

#define KPD_TIMEOUT		10000

static void	*_kpdLbl;
static KEYPAD_CONTEXT	*kpd, _kpd;
static char	_kpdText[16];


void AuthCaptureStart(void)
{
	GACTIVITY	*act;

	act = CreateActivity(AuthCaptureOnCreate);
	appStartActivityForResult(act, ACT_AUTHCAPTURE, NULL, 1);
}

void AuthCaptureOnCreate(GACTIVITY *act)
{
	void	*wnd, *v;
	CREDENTIAL_READER	*cr;
	CGRect		rect;
	unsigned char	msg[12];
	char	*st, temp[80], iconFile[32];
	int		val, x, y, w, h;

	cr = crsGet(0);
	activitySetOnStart(act, AuthCaptureOnStart);
	activitySetOnDestroy(act, AuthCaptureOnDestroy);
	activitySetOnTimer(act, AuthCaptureOnTimer);
	activitySetOnClick(act, AuthCaptureOnClick);
	activitySetOnAppMessage(act, AuthCaptureOnAppMessage);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, 0xffbbdefb); 
	UIframe(wnd, &rect);
	x = y = 0; h = 48;
	if(cr->captureMode == USER_CARD) w = rect.width; else w = rect.width >> 1;
	v = ViewAddTextButton(wnd, TAG_CANCEL_BTN, admCancelTitle(), x, y, w, h);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, 0xff00acc1);
	UIsetTitleColor(v, TABLE_GREY_50);
	kpd = &_kpd;
	if(cr->captureMode != USER_CARD) {
		x += w;
		v = ViewAddTextButton(wnd, TAG_OK_BTN, admCompleteTitle(), x, y, w, h);
		UIsetOpaque(v, TRUE);
		UIsetBackgroundColor(v, 0xff00acc1);
		UIsetTitleColor(v, TABLE_GREY_50);
	}
	y += h;
	if(cr->captureMode == USER_CARD) {
		if(cr->retryCount) {
			x = 0;
			y += 32;
			st = GetResultMsg(R_DATA_MISMATCHED);
			strcpy_swap(temp, st, '\n', ' ');
			v = ViewAddLabel(wnd, 0, temp, x, y, rect.width-x, h, UITextAlignmentCenter);
			UIsetTextColor(v, redColor);
			y += h;
			y += 20;
		} else {
			y += 60;
		}
		h = 180;
		x = (rect.width - h) >> 1;
		v = ViewAddImageViewAlphaMask(wnd, 0, "capcard.bmp", x, y, h, h);
		UIsetTintColor(v, blueColor);
		y += h;
		h = 22;
		x = 0;
		if(cr->retryCount) {
			val = M_TOUCH_YOUR_CARD_AGAIN;
			y += 32;
		} else {
			val = M_TOUCH_YOUR_CARD;
			y += 40;
		}
		v = ViewAddLabel(wnd, 0, GetPromptMsg(val), x, y, rect.width-x, h, UITextAlignmentCenter);
	} else {
		if(cr->captureMode == USER_PIN) {
			ViewAddKeypad(kpd, wnd, 1);
			UIframe(kpd->view, &rect);
			x = 0;	
			h = 22;
			if(cr->retryCount) {
				x = 0; y = rect.y - 160;
				st = GetResultMsg(R_PW_MISMATCHED);
				strcpy_swap(temp, st, '\n', ' ');
				v = ViewAddLabel(wnd, 0, temp, x, y, rect.width-x, h, UITextAlignmentCenter);
				UIsetTextColor(v, redColor);
				y += h;
			}
			x = 40; y = rect.y - 110; w = 220;
			v = ViewAddLabel(wnd, 0, GetPromptMsg(M_ENTER_PASSWORD), x, y, w, 32, UITextAlignmentLeft);
			y += 40; 
			_kpdLbl = ViewAddLabel(wnd, 0, NULL, rect.x+20, y, rect.width-40, 36, UITextAlignmentCenter);
			UIsetFont(_kpdLbl, 32);
		} else {
			ViewAddKeypad(kpd, wnd, 0);
			if(cr->retryCount) {
				h = 22;
				x = 0;
				y += 4;
				//if(cr->credMode == USER_PIN) val =  R_PW_MISMATCHED;
				//else	val =  R_DATA_MISMATCHED;
				val =  R_DATA_MISMATCHED;
				st = GetResultMsg(val);
				strcpy_swap(temp, st, '\n', ' ');
				v = ViewAddLabel(wnd, 0, temp, x, y, rect.width-x, h, UITextAlignmentCenter);
				UIsetTextColor(v, redColor);
				y += h;
			}
			h = 64;
			x = (rect.width - h) >> 1;
			v = ViewAddImageViewAlphaMask(wnd, 0, "capcard.bmp", x, y, h, h);
			UIsetTintColor(v, blueColor);
			y += h;
			h = 22;
			x = 0;
			if(cr->retryCount) val = M_TOUCH_YOUR_CARD_AGAIN; else val = M_TOUCH_YOUR_CARD;
			v = ViewAddLabel(wnd, 0, GetPromptMsg(val), x, y, rect.width-x, h, UITextAlignmentCenter);
			y += h;
			w = 220;
			x = (rect.width - w) >> 1;
			if(cr->retryCount) y += 10; else y += 30;
			v = ViewAddLabel(wnd, 0, GetPromptMsg(M_ENTER_PASSWORD), x-20, y, w, 32, UITextAlignmentLeft);
			y += 36;
			_kpdLbl = ViewAddLabel(wnd, 0, NULL, rect.x+20, y, rect.width-40, 48, UITextAlignmentCenter);
			UIsetFont(_kpdLbl, 32);
		}
		KeypadChangeDigit(kpd);
		_kpdText[0] = 0;
		UItimerStop(0); UItimerStop(1);
	}
	crSetCaptureMode(cr, 0);
}

void AuthCaptureOnStart(GACTIVITY *act)
{
printf("AuthCaptureOnStart...\n");
	UItimerStart(1, KPD_TIMEOUT, 0);
}

void AuthCaptureOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

void AuthCaptureOnTimer(GACTIVITY *act, int timerID)
{
	void	*cr;

	if(timerID == 0) {
		LabelSetHiddenText(_kpdLbl, _kpdText);
	} else if(timerID == 1) {
printf("timeout....1\n");
		UItimerStop(0);
		cr = crsGet(0);
		if(crCaptureMode(cr)) {
			crSetResult(cr, 3);
			DestroyActivityForResult(act, RESULT_OK, NULL);
		} else {
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		}
	}
}

static int _DigitKeypadPressed(void *view, void *kpdLbl, char *kpdText)
{
	int		c, tag, len, rval;

	rval = 0;
	c = KeypadNumericCode(view);
	switch(c) {
	case '*':
		len = strlen(kpdText);
		if(len > 0) {
			UItimerStop(0); UItimerStop(1);
			len--; kpdText[len] = 0;
			LabelSetHiddenText(kpdLbl, kpdText);
			UItimerStart(1, KPD_TIMEOUT, 0);
		}
		break;
	case '#':
		UItimerStop(0); UItimerStop(1);
		rval = 1;
		break;
	default:
		len = strlen(kpdText);
		if(len < 8 && c > 0) {
			UItimerStop(0); UItimerStop(1);
			kpdText[len++] = c; kpdText[len] = 0;
			LabelSetHiddenTextExceptLast(kpdLbl, kpdText);
			UItimerStart(0, 1000, 0);
			UItimerStart(1, KPD_TIMEOUT, 0);
		} else {
			UItimerStop(0);
			LabelSetHiddenText(kpdLbl, kpdText);
		}
	}
	return rval;
}

void AuthCaptureVerifyRequest(void *cr)
{
	unsigned char	msg[12];

	UItimerStop(0); UItimerStop(1);
	UItimerStart(1, 3000, 0);
	msg[0] = GM_VERIFY_REQUEST; msg[1] = 0;
//	svcWorkPostMessage(msg);
}

void AuthCaptureOnClick(GACTIVITY *act, void *view)
{
	void	*wnd, *cr;
	unsigned char	*p, temp[128];
	int		tag, len, rval;

	wnd = activityWindow(act);
	tag = UItag(view);
	switch(tag) {
	case TAG_CANCEL_BTN:
		UItimerStop(0); UItimerStop(1);
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	case TAG_OK_BTN:
		UItimerStop(0); UItimerStop(1);
		cr = crsGet(0);
		if(strlen(_kpdText) < 1) {
			p = temp;
			sprintf(p, GetPromptMsg(M_ENTER_PASSWORD)); p += strlen(p);
			*p++ = '=';
			strcpy_swap(p, GetResultMsg(R_INVALID_DATA), '\n', ' ');
			AlertDialog(0, temp, FALSE);
		} else if(!crCaptureMode(cr)) {	
			crCapturedPin(cr, _kpdText);
			crSetCaptureMode(cr, USER_PIN);
			AuthCaptureVerifyRequest(cr);
		}
		break;
	default:
		rval = _DigitKeypadPressed(view, _kpdLbl, _kpdText);
		if(rval > 0) {
			UItimerStop(0); UItimerStop(1);
			cr = crsGet(0);
			if(strlen(_kpdText) < 1) {
				p = temp;
				sprintf(p, GetPromptMsg(M_ENTER_PASSWORD)); p += strlen(p);
				*p++ = '=';
				strcpy_swap(p, GetResultMsg(R_INVALID_DATA), '\n', ' ');
				AlertDialog(0, temp, FALSE);
			} else if(!crCaptureMode(cr)) {	
				crCapturedPin(cr, _kpdText);
				crSetCaptureMode(cr, USER_PIN);
				AuthCaptureVerifyRequest(cr);
			}
		}
	}
}

void AuthCaptureOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;
	CREDENTIAL_READER	*cr;
	int		rval;

	wnd = activityWindow(act);
	cr = crsGet(0);
	switch(msg[0]) {
	case GM_CRED_CAPTURED:
		if(crCaptureMode(cr)) {
			crSetCaptureMode(cr, USER_CARD);
			AuthCaptureVerifyRequest(cr);
		}
		break;
	case GM_VERIFY_RESULT:
printf("VerifyResult: authMode=%02x authDone=%02x captureMode=%02x\n", (int)cr->authMode, (int)cr->authDone, (int)cr->captureMode);
		UItimerStop(1);
		if(crResult(cr) > 1) DestroyActivityForResult(act, RESULT_OK, NULL);
		else if(crAccessEvent(cr)) AuthResultStart();
		else	AuthCaptureStart();
		break;
	}
}

