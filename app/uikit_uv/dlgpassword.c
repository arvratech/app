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

void PasswordDialogOnCreate(GACTIVITY *act);
void PasswordDialogOnDestroy(GACTIVITY *act);
void PasswordDialogOnTimer(GACTIVITY *act, int timerID);
void PasswordDialogOnClick(GACTIVITY *act, void *view);
static int	_inputType, _limitLength, _qwertyMode, _timerID;
static char	_kpdText[64];
static KEYPAD_CONTEXT	*kpd, _kpd;
static void	*_kpdLbl;
static BOOL (*_doneValidate)(char *intent, char *result);


void *PasswordDialog(int requestCode, char *intent, BOOL (*doneValidate)(char *, char *))
{
	GACTIVITY	*act;

	act = CreateActivity(PasswordDialogOnCreate);
	_doneValidate = doneValidate;
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 0);
	return act;
}

void *PasswordClearTopDialog(int requestCode, char *intent, BOOL (*doneValidate)(char *, char *))
{
	GACTIVITY	*act;

	act = CreateActivity(PasswordDialogOnCreate);
	_doneValidate = doneValidate;
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 1);
	return act;
}

// inputType = 1: password(character, digit)
//             3: password(digit)
// limitLength = 
void PasswordDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *v;
	CGRect	rt;
	char	*p, title[64], temp[64];
	int		x, y, w, h, count;

	activitySetOnDestroy(act, PasswordDialogOnDestroy);
	activitySetOnTimer(act, PasswordDialogOnTimer);
	activitySetOnClick(act, PasswordDialogOnClick);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, TABLE_BLUE_100); 
	UIframe(wnd, &rt);
	x = y = 0; w = rt.width >> 1; h = 48;
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
	p = read_token_ch(p, temp, ','); _inputType = n_atoi(temp);
	p = read_token_ch(p, temp, ','); _limitLength = n_atoi(temp);
	if(_inputType == TI_ALPHANUMERIC_PASSWORD) x = 0;
	else	x = 1;
	kpd = &_kpd;
	ViewAddKeypad(kpd, wnd, x);
	UIframe(kpd->view, &rt);
	x = 40; y = rt.y - 110; w = 220;
	if(_inputType == TI_ALPHANUMERIC_PASSWORD) y -= 30;
	v = ViewAddLabel(wnd, 0, title, x, y, w, 32, UITextAlignmentLeft);
	y += 40; 
	_kpdLbl = ViewAddLabel(wnd, 0, NULL, rt.x+20, y, rt.width-40, 36, UITextAlignmentCenter);
//printf("_kpdLbl=%x\n", _kpdLbl);
	_kpdText[0] = 0;
	if(_inputType == TI_ALPHANUMERIC_PASSWORD) {
		UIsetFont(_kpdLbl, 28);
		_qwertyMode = 0;
		KeypadChangeQwerty(kpd, _qwertyMode);
	} else {
		UIsetFont(_kpdLbl, 32);
		KeypadChangeDigit(kpd);
	}
	UItimerStop(0);
}

void PasswordDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

void PasswordDialogOnTimer(GACTIVITY *act, int timerID)
{
	if(timerID == 0) {
		LabelSetHiddenText(_kpdLbl, _kpdText);
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
			UItimerStop(0);
			len--; _kpdText[len] = 0;
			LabelSetHiddenText(_kpdLbl, _kpdText);
		}
		break;
	case '#':
		UItimerStop(0);
		rval = 1;
		break;
	default:
		len = strlen(_kpdText);
		if(len < _limitLength && code > 0) {
			UItimerStop(0);
			kpdText[len++] = code; kpdText[len] = 0;
			LabelSetHiddenTextExceptLast(kpdLbl, kpdText);
			UItimerStart(0, 1000, 0);
		} else {
			UItimerStop(0);
			LabelSetHiddenText(kpdLbl, kpdText);
		}
	}
	return rval;
}

static int _AlphanumericKeypadPressed(int code, void *kpdLbl, char *kpdText)
{
	int		len;

	if(code >= 0) {
		len = strlen(_kpdText);
		if(len < _limitLength && code > 0) {
			UItimerStop(0);
			kpdText[len++] = code; kpdText[len] = 0;
			LabelSetHiddenTextExceptLast(kpdLbl, kpdText);
			UItimerStart(0, 1000, 0);
		} else {
			UItimerStop(0);
			LabelSetHiddenText(kpdLbl, kpdText);
		}
		if(_qwertyMode == 1) {
			_qwertyMode = 0;
			KeypadChangeQwerty(kpd, _qwertyMode);
		}
	}
	return 0;
}

void PasswordDialogOnClick(GACTIVITY *act, void *view)
{
	void	*wnd;
	unsigned char	temp[128];
	int		tag, len, rval, code;

	wnd = activityWindow(act);
	tag = UItag(view);
//printf("pwsOnCLick=%d\n", tag);
	switch(tag) {
	case TAG_CANCEL_BTN:
		UItimerStop(0);
		DestroyDialogActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	case TAG_OK_BTN:
	case TAG_KEYPAD_FUNC_BTN+3:
		UItimerStop(0);
		sprintf(temp, "pwd=%s\n", _kpdText);
		DestroyDialogActivityForResult(act, RESULT_OK, temp);
		break;
	case TAG_KEYPAD_FUNC_BTN:		// backspace
		len = strlen(_kpdText);
		if(len > 0) {
			UItimerStop(0);
			len--; _kpdText[len] = 0;
			LabelSetHiddenText(_kpdLbl, _kpdText);
		}
		break;
	case TAG_KEYPAD_FUNC_BTN+1:
		_qwertyMode++;
		if(_qwertyMode > 4) _qwertyMode = 3;	
		else if(_qwertyMode == 3) _qwertyMode = 0;
		KeypadChangeQwerty(kpd, _qwertyMode);
		break;
	case TAG_KEYPAD_FUNC_BTN+2:
		if(_qwertyMode < 3) _qwertyMode = 3;	
		else	_qwertyMode = 0;
		KeypadChangeQwerty(kpd, _qwertyMode);
		break;
	default:
		if(_inputType == TI_ALPHANUMERIC_PASSWORD) {
			code = KeypadQwertyCode(view, _qwertyMode);
			rval = _AlphanumericKeypadPressed(code, _kpdLbl, _kpdText);
		} else{
			code = KeypadNumericCode(view);
			rval = _DigitKeypadPressed(code, _kpdLbl, _kpdText);
		}
		if(rval > 0) {
			UItimerStop(0);
			sprintf(temp, "pwd=%s\n", _kpdText);
			DestroyDialogActivityForResult(act, RESULT_OK, temp);
		}
	}
}

/*
void PasswordDialogOnExternalKey(GACTIVITY *act, int code)
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
