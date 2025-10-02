#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "fsuser.h"
#include "sysdep.h"
#include "cr.h"
#include "gactivity.h"
#include "gapp.h"
#include "syscfg.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "dlg.h"
#include "appact.h"

extern CREDENTIAL_READER _gCredentialReaders[];

void AdmUserRegOnCreate(GACTIVITY *act);
void AdmUserRegOnDestroy(GACTIVITY *act);
void AdmUserRegOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmUserRegOnTimer(GACTIVITY *act, int timerID);
void AdmUserRegOnClick(GACTIVITY *act, void *view);
void AdmUserRegOnTextFieldFocused(GACTIVITY *act, void *view, int textPosition);
static void	*_textField;
static int	_timerID, _timerCount;
static FP_USER	*user, _user;


void AdmUserRegStart(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(AdmUserRegOnCreate);
	activitySetTimeout(act, 30);
	appStartActivityForResult(act, requestCode, intent, 0);
	user = &_user;
	user->UserID[0] = 0;
	userSetDefault(user);
	user->AccessMode = USER_CARD;
}

void AdmUserRegOnCreate(GACTIVITY *act)
{
	void	*wnd, *v, *lbl, *txtfld, *imgv;
	CGRect		rect;
	int		x, y, w, count;

	activitySetOnDestroy(act, AdmUserRegOnDestroy);
	activitySetOnTimer(act, AdmUserRegOnTimer);
	activitySetOnClick(act, AdmUserRegOnClick);
	activitySetOnAppMessage(act, AdmUserRegOnAppMessage);
	activitySetOnTextFieldFocused(act, AdmUserRegOnTextFieldFocused);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIframe(wnd, &rect);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, systemPurple); 
	w = 220;
	x = (rect.width - w) >> 1;
	y = 16;
	lbl = ViewAddLabel(wnd, 0, GetPromptMsg(M_ENTER_USER_ID), x-20, y, w, 32, UITextAlignmentLeft);
	y += 34;
	_textField = txtfld = ViewAddTextField(wnd, TAG_TEXT_INPUT, "", x, y, w, 40);
	y += 48;
	lbl = ViewAddLabel(wnd, 0, GetPromptMsg(M_TOUCH_YOUR_CARD), x-20, y, w, 32, UITextAlignmentLeft);
	y += 34;
	imgv = ViewAddImageViewAlphaMask(wnd, TAG_CARD_CAP_ICON, "check_off.bmp", x+32, y+8, 32, 32);
	y += 48;
	ViewAddButtonText(wnd, TAG_CANCEL_BTN, xmenu_okcancel[1], 100, y, 80, 32);
	ViewAddButtonText(wnd, TAG_OK_BTN, xmenu_okcancel[2], 220, y, 80, 32);
	ViewAddBackButton(wnd);
	v = ViewAddKeypad(wnd);
	ViewChangeNumericKeypad(v);
	_timerID = timerCreate(-1);
	timerStart(_timerID, 500, 1);
}

void AdmUserRegOnDestroy(GACTIVITY *act)
{
	AmdActExit(act);
}

void _TextFieldDrawCursor(void *self, int textPosition, int count);

void AdmUserRegOnTimer(GACTIVITY *act, int timerID)
{
	void	*wnd;
	int		textPosition;

	wnd = activityWindow(act);
	if(timerID == _timerID) {
		textPosition = UItextPosition(_textField);
		_TextFieldDrawCursor(_textField, textPosition, _timerCount);
		_timerCount++;
	}
}

void AdmUserRegOnClick(GACTIVITY *act, void *view)
{
	void	*wnd, *txtfld;
	char	*p, temp[256], result[256], ch[4];
	int		c, rval, add, tag, font, position, format, maxlen;

	wnd = activityWindow(act);
	tag = UItag(view);
	font = UIfont(_textField); 
	position = UItextPosition(_textField);
	format = 2; maxlen = 8;
	c = ViewNumericKeypadCode(view);
	if(c >= 0) { 
		p = UItext(_textField);
		if(c >= '0' && c <= '9') add = c;
		else	add = 0;
		if(add && strlen(p) < maxlen) {
			_TextFieldDrawCursor(_textField, position, 0);
			ch[0] = add; ch[1] = 0;
			strcpy(temp, p);
			position = textInsertCharAtPosition(font, temp, position, ch); 
			UIsetTextPosition(_textField, position);
			UIsetText(_textField, temp);
		}
	} else if(tag == TAG_KEYPAD_FUNC_BTN) { 
		_TextFieldDrawCursor(_textField, position, 0);
		strcpy(temp, UItext(_textField));
		position = textDeleteCharBelowPosition(font, temp, position); 
		UIsetTextPosition(_textField, position);
		UIsetText(_textField, temp);
	} else if(tag == TAG_KEYPAD_FUNC_BTN+1 || tag == TAG_KEYPAD_FUNC_BTN+2 || tag == TAG_KEYPAD_FUNC_BTN+3) { 
		MoveFocusedTextField(wnd, position);
	} else {
		switch(tag) {
		case TAG_BACK_BTN:		
			timerDelete(_timerID);
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
		case TAG_CANCEL_BTN:
			timerDelete(_timerID);
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
		case TAG_OK_BTN:
			txtfld = UIviewWithTag(wnd, TAG_TEXT_INPUT);
			sprintf(user->UserID, "%s", UItext(txtfld));
			rval = ValidateRegUser(user, result);
printf("UserID=[%s] Mode=%02x\n", user->UserID, (int)user->AccessMode);
printf("[%02x", (int)user->CardData[0]); for(c = 1;c < 9;c++) printf("-%02x", (int)user->CardData[c]); printf("]\n");
printf("Validate RVAL=%d [%s]\n", rval, result);
			if(rval) {
				timerDelete(_timerID);
				DestroyActivityForResult(act, RESULT_OK, result);
			} else {
				AlertDialog(0, result, FALSE);
			}
			break;
		}
	}
}

static void MoveFocusedTextField(void *wnd, int position)
{
	void	*txtfld;

	txtfld = UIviewWithTag(wnd, TAG_TEXT_INPUT+1);
	if(txtfld) {
		_TextFieldDrawCursor(_textField, position, 0);
		if(txtfld == _textField) {
			_textField = UIviewWithTag(wnd, TAG_TEXT_INPUT);
		} else {
			_textField = txtfld;	
		}
	}
}

void AdmUserRegOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;

	wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_CRED_CAPTURED:
		AdmUserRegCapturedCard(wnd);
		break;
	}
}

static void MoveFocusedTextField(void *wnd, int position);
int ValidateRegUser(FP_USER *user, char *msg);
void AdmUserRegOnTextFieldFocused(GACTIVITY *act, void *view, int textPosition)
{
	_textField = view;
	UIsetTextPosition(view, textPosition);
}

int ValidateRegUser(FP_USER *user, char *msg)
{
	FP_USER	*user2, _user2;
	unsigned char	buf[64];
	char	*st, temp[80];
	int		rval, no;

	no = 0;
	if(!userValidateID(user->UserID)) no = R_INVALID_USER_ID;
	if(!no) {
		rval = fsExistUser(user->UserID);
		if(rval < 0) no = R_MSG_SYSTEM_ERROR;
		else if(rval) no = R_USER_ID_OVERLAPPED;
	}
	if(!no) {
		if(!user->CardData[0]) no = R_INVALID_DATA;
		else {
			user2 = &_user2;
			rval = userfsGetCardData(user2, user->CardData);
			if(rval > 0 && !strcmp(user->UserID, user2->UserID)) rval = userfsGetCardData(user2, user->CardData);
			if(rval) no = R_CARD_DATA_OVERLAPPED;
		}
	}
	if(!no) {
		user->CardStatus = 1;
		userEncode(user, buf);
		userEncodeId(user->UserID, buf+1);
		rval = fsAddEncodedUser(buf);
		if(rval < 0) no = R_MSG_SYSTEM_ERROR;
		else if(!rval) no = R_USER_ARE_FULL;
		else	no = R_DATA_SAVED;
		rval = 1;
	} else {
		rval = 0;
	}
	st = GetResultMsg(no);
	strswap_copy(temp, st, '\n', '&');
	sprintf(msg, "%s=%s\n", xmenu_user[0], temp);
	return rval;
}
