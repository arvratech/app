#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "ctfont.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "dlg.h"

#define KPD_TIMEOUT		15000

void DigitInputDialogOnCreate(GACTIVITY *act);
void DigitInputDialogOnDestroy(GACTIVITY *act);
void DigitInputDialogOnTimer(GACTIVITY *act, int timerID);
void DigitInputDialogOnClick(GACTIVITY *act, void *view);
static int	_inputType, _limitLength, _qwertyMode, _timerID, _entryStopSentinel;
static char	_kpdText[64];
static KEYPAD_CONTEXT	*kpd, _kpd;
static void	*_kpdLbl;
static BOOL (*_doneValidate)(char *intent, char *result);


void *DigitInputDialog(int requestCode, char *intent, BOOL (*doneValidate)(char *, char *))
{
	GACTIVITY	*act;

	act = CreateActivity(DigitInputDialogOnCreate);
	_doneValidate = doneValidate;
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 0);
	return act;
}

void *DigitInputClearTopDialog(int requestCode, char *intent, BOOL (*doneValidate)(char *, char *))
{
	GACTIVITY	*act;

	act = CreateActivity(DigitInputDialogOnCreate);
	_doneValidate = doneValidate;
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 1);
	return act;
}

// limitLength = 
void DigitInputDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *v;
	CGRect	rect;
	char	*p, title[64], temp[64];
	int		x, y, w, h, count;

	activitySetOnDestroy(act, DigitInputDialogOnDestroy);
	activitySetOnTimer(act, DigitInputDialogOnTimer);
	activitySetOnClick(act, DigitInputDialogOnClick);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, TABLE_BLUE_100); 
	UIframe(wnd, &rect);
	x = y = 0; w = rect.width >> 1; h = 48;
	v = ViewAddTextButton(wnd, TAG_CANCEL_BTN, xmenu_okcancel[1], x, y, w, h);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_CYAN_600);
	UIsetTitleColor(v, enableWhiteColor);
	x += w;
	v = ViewAddTextButton(wnd, TAG_OK_BTN, xmenu_okcancel[2], x, y, w, h);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_CYAN_600);
	UIsetTitleColor(v, enableWhiteColor);
	y += h;
	p = act->intent;
	p = get_keyvalue(p, title, temp);
	p = get_keyvalue(p, title, temp);
	p = read_token_ch(p, temp, ','); _limitLength = n_atoi(temp);
	kpd = &_kpd;
	ViewAddKeypad(kpd, wnd, 1);
	UIframe(kpd->view, &rect);
	x = 40; y = rect.y - 110; w = 220;
	v = ViewAddLabel(wnd, 0, title, x, y, w, 32, UITextAlignmentLeft);
	y += 40; 
	_kpdLbl = ViewAddLabel(wnd, 0, NULL, rect.x+20, y, rect.width-40, 36, UITextAlignmentCenter);
	_kpdText[0] = 0;
	UIsetFont(_kpdLbl, 32);
	KeypadChangeDigit(kpd);
	UItimerStop(1);
	UItimerStart(1, KPD_TIMEOUT, 0);
}

void DigitInputDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

void DigitInputDialogOnTimer(GACTIVITY *act, int timerID)
{
	if(timerID == 1) {
		UItimerStop(0);
		DestroyDialogActivityForResult(act, RESULT_CANCELLED, NULL);
	}
}

static int _DigitKeypadPressed(int code, void *kpdLbl, char *kpdText)
{
	int		tag, len, rval;

	rval = 0;
	switch(code) {
	case '*':
		len = strlen(_kpdText);
		if(len > 0) {
			UItimerStop(1);
			len--; _kpdText[len] = 0;
			UIsetText(_kpdLbl, _kpdText);
			UItimerStart(1, KPD_TIMEOUT, 0);
		}
		break;
	case '#':
		UItimerStop(1);
		rval = 1;
		break;
	default:
		len = strlen(_kpdText);
		if(len < _limitLength && code > 0) {
			UItimerStop(1);
			kpdText[len++] = code; kpdText[len] = 0;
			UIsetText(kpdLbl, kpdText);
			UItimerStart(1, KPD_TIMEOUT, 0);
		}
	}
	return rval;
}

void DigitInputDialogOnClick(GACTIVITY *act, void *view)
{
	void	*wnd;
	unsigned char	temp[128];
	int		tag, len, rval, code;

	wnd = activityWindow(act);
	tag = UItag(view);
	switch(tag) {
	case TAG_CANCEL_BTN:
		UItimerStop(1);
		DestroyDialogActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	case TAG_OK_BTN:
		UItimerStop(1);
		sprintf(temp, "digit=%s\n", _kpdText);
		DestroyDialogActivityForResult(act, RESULT_OK, temp);
		break;
	default:
		code = KeypadNumericCode(view);
		rval = _DigitKeypadPressed(code, _kpdLbl, _kpdText);
		if(rval > 0) {
			UItimerStop(1);
			sprintf(temp, "digit=%s\n", _kpdText);
			DestroyDialogActivityForResult(act, RESULT_OK, temp);
		}
	}
}

/*
void DigitInputDialogOnExternalKey(GACTIVITY *act, int code)
{
	unsigned char	temp[128];
	int		rval;

	rval = _DigitKeypadPressed(code, _kpdLbl, _kpdText);
	if(rval > 0) {
		UItimerStop(1);
		sprintf(temp, "digit=%s\n", _kpdText);
		DestroyDialogActivityForResult(act, RESULT_OK, temp);
	}
}
*/
