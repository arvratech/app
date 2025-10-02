#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "ctfont.h"
#include "cgtext.h"
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


void UuidInputDialogOnCreate(GACTIVITY *act);
void UuidInputDialogOnDestroy(GACTIVITY *act);
void UuidInputDialogOnTimer(GACTIVITY *act, int timerID);
void UuidInputDialogOnClick(GACTIVITY *act, void *view);
void UuidInputDialogOnTextFieldFocused(GACTIVITY *act, void *view, int textPosition);
static int	_timerID;
static char	title[64];
static KEYPAD_CONTEXT	*kpd, _kpd;
static void	*_textField;


void *UuidInputDialog(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(UuidInputDialogOnCreate);
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 0);
	return act;
}

void UuidInputDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *v;
	CGRect	rect;
	char	*p, value[64], temp[32];
	int		i, x, y, w, h;

	activitySetOnDestroy(act, UuidInputDialogOnDestroy);
	activitySetOnTimer(act, UuidInputDialogOnTimer);
	activitySetOnClick(act, UuidInputDialogOnClick);
	activitySetOnTextFieldFocused(act, UuidInputDialogOnTextFieldFocused);
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
	p = get_keyvalue(p, temp, value);
	y += 12; i = 10;
	v = ViewAddLabel(wnd, 0, title, 10, y, 220, 32, UITextAlignmentLeft);
	y += 42;
	p = value;
	for(i = 0;i < 5;i++) { 
		p = strcpy_chr(temp, p, '-');
		if(i == 1 || i == 3) x = 30 + w + 30;
		else	x = 30;
		if(i == 0) w = 130;
		else if(i == 4) w = 180;
		else	w = 70;
		v = ViewAddTextField(wnd, TAG_TEXT_INPUT+i, temp, x, y, w, 40);
		if(i == 1 || i == 3) y += 50;
	}
	_textField = UIviewWithTag(wnd, TAG_TEXT_INPUT);
	kpd = &_kpd;
	ViewAddKeypad(kpd, wnd, 0);
	KeypadChangeHexaNumeric(kpd);
	_timerID = 7;
	UItimerStart(_timerID, 500, 1);
}

void UuidInputDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

static int	tmrCount;
void _TextFieldDrawCursor(void *self, int textPosition, int count);

void UuidInputDialogOnTimer(GACTIVITY *act, int timerID)
{
	void	*wnd;
	int		position;

	wnd = activityWindow(act);
	if(timerID == _timerID) {
		position = UItextPosition(_textField);
		_TextFieldDrawCursor(_textField, position, tmrCount);
		tmrCount++;
//printf("[%d %d]\n", position, tmrCount);
	}
}

static void MoveBackwardFocusedTextField(void *wnd, int position)
{
	int		idx;

	_TextFieldDrawCursor(_textField, position, 0);
	idx = UItag(_textField) - TAG_TEXT_INPUT;
	idx--; if(idx < 0) idx = 4;	
	_textField = UIviewWithTag(wnd, TAG_TEXT_INPUT+idx);
//	position = textWidth(UIfont(_textField), UItext(_textField));
//	UIsetTextPosition(_textField, position);
}

static void MoveForwardFocusedTextField(void *wnd, int position)
{
	int		idx;

	_TextFieldDrawCursor(_textField, position, 0);
	idx = UItag(_textField) - TAG_TEXT_INPUT;
	idx++; if(idx > 4) idx = 0;	
	_textField = UIviewWithTag(wnd, TAG_TEXT_INPUT+idx);
//	position = textWidth(UIfont(_textField), UItext(_textField));
//	UIsetTextPosition(_textField, position);
}

void UuidInputDialogOnClick(GACTIVITY *act, void *view)
{
	void	*wnd, *txtfld;
	char	*p, temp[256], ch[4];
	BOOL	bVal;
	int		c, add, tag, font, i, position, limitLen;

	wnd = activityWindow(act);
	tag = UItag(view);
	font = UIfont(_textField); 
	position = UItextPosition(_textField);
	c = KeypadHexaNumericCode(view);
	if(c >= 0) {
		i = UItag(_textField) - TAG_TEXT_INPUT;
		if(i == 0) limitLen = 9;  
		else if(i == 4) limitLen = 12;
		else	limitLen = 4;
		p = UItext(_textField);
		if(strlen(p) < limitLen) {
			position = UItextPosition(_textField);
			_TextFieldDrawCursor(_textField, position, 0);
			if(c >= 'A' && c <= 'F')  c = c - 'A' + 'a';
			ch[0] = c; ch[1] = 0;
			strcpy(temp, p);
			position = textInsertCharAtPosition(temp, position, ch); 
			UIsetTextPosition(_textField, position);
			UIsetText(_textField, temp);
		}
	} else if(tag == TAG_KEYPAD_FUNC_BTN) {		// backspace 
		position = UItextPosition(_textField);
		_TextFieldDrawCursor(_textField, position, 0);
		strcpy(temp, UItext(_textField));
		position = textDeleteCharBelowPosition(temp, position); 
		UIsetTextPosition(_textField, position);
		UIsetText(_textField, temp);
	} else if(tag == TAG_KEYPAD_FUNC_BTN+1) {	// arrow right 
		MoveForwardFocusedTextField(wnd, position);
	} else if(tag == TAG_KEYPAD_FUNC_BTN+2) {	// arrow left
		MoveBackwardFocusedTextField(wnd, position);
	} else if(tag == TAG_KEYPAD_FUNC_BTN+3) {	// return
		MoveForwardFocusedTextField(wnd, position);
	} else {
		switch(tag) {
		case TAG_BACK_BTN:
		case TAG_CANCEL_BTN:
			UItimerStop(_timerID);
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
			break;
		case TAG_OK_BTN:
			bVal = FALSE;
			txtfld = UIviewWithTag(wnd, TAG_TEXT_INPUT);
			if(strlen(UItext(txtfld)) == 8) {
				p = temp;
				sprintf(p, "uuid=%s-", UItext(txtfld)); p += strlen(p);
				for(i = 0;i < 3;i++) {
					txtfld = UIviewWithTag(wnd, TAG_TEXT_INPUT+1+i);
					if(strlen(UItext(txtfld)) != 4) break;
					strcpy(p, UItext(txtfld)); p += strlen(p); *p++ = '-';
				}
				if(i >= 3) {
					txtfld = UIviewWithTag(wnd, TAG_TEXT_INPUT+4);
					if(strlen(UItext(txtfld)) == 12) {
						strcpy(p, UItext(txtfld)); p += strlen(p); *p++ = '\n'; *p = 0;
						bVal = TRUE;
					}
				}
			}
			if(bVal) {
				UItimerStop(_timerID);
				DestroyActivityForResult(act, RESULT_OK, temp);
			} else {
				sprintf(temp, "%s=Invalid UUID\n", title);
				AlertDialog(0, temp, FALSE);
			}
			break;
		}
	}
}

void UuidInputDialogOnTextFieldFocused(GACTIVITY *act, void *view, int textPosition)
{
	_TextFieldDrawCursor(_textField, UItextPosition(_textField), 0);
	_textField = view;
	UIsetTextPosition(view, textPosition);
}

