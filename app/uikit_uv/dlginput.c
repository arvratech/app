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


void TextInputDialogOnCreate(GACTIVITY *act);
void TextInputDialogOnDestroy(GACTIVITY *act);
void TextInputDialogOnTimer(GACTIVITY *act, int timerID);
void TextInputDialogOnClick(GACTIVITY *act, void *view);
void TextInputDialogOnTextFieldFocused(GACTIVITY *act, void *view, int textPosition);
static int	_inputType[2], _limitLength[2], _qwertyMode, _timerID;
static char	titles[2][64], values[2][64];
static KEYPAD_CONTEXT	*kpd, _kpd;
static void	*_textField;
static BOOL (*_doneValidate)(char *intent, char *result);


void *TextInputDialog(int requestCode, char *intent, BOOL (*doneValidate)(char *, char *))
{
	GACTIVITY	*act;

	act = CreateActivity(TextInputDialogOnCreate);
	_doneValidate = doneValidate;
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 0);
	return act;
}

// inputType = 0: character, digit
//             1: password(character, digit)
//             2: digit
//             3: password(digit)
//             4: hexa digits
//             5: IP address
// limitLength = 
void TextInputDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *v;
	CGRect	rt;
	char	*p, title[64], temp[64];
	int		i, x, y, w, h, count;

	activitySetOnDestroy(act, TextInputDialogOnDestroy);
	activitySetOnTimer(act, TextInputDialogOnTimer);
	activitySetOnClick(act, TextInputDialogOnClick);
	activitySetOnTextFieldFocused(act, TextInputDialogOnTextFieldFocused);
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
	for(i = 0;p && i < 2;i++) {
		p = get_keyvalue(p, titles[i], values[i]);
		p = read_token_ch(p, temp, ','); _inputType[i] = n_atoi(temp);
		p = read_token_ch(p, temp, ','); _limitLength[i] = n_atoi(temp);
	}
	count = i;
printf("0 field [%s] type=%d limit=%d\n", titles[0], _inputType[0], _limitLength[0]);
if(i > 1) {
printf("1 field [%s] type=%d limit=%d\n", titles[1], _inputType[1], _limitLength[1]);
}
	if(_limitLength[0] > 32) w = 320;
	else if(_limitLength[0] > 30) w = 300;
	else	w = 220; 
	x = (rt.width - w) >> 1;
	if(count == 1) {
		y += 32; i = x - 20; if(i < 4) i = 4;
		v = ViewAddLabel(wnd, 0, titles[0], i, y, w, 32, UITextAlignmentLeft);
		y += 48;
		_textField = v = ViewAddTextField(wnd, TAG_TEXT_INPUT, values[0], x, y, w, 48);
if(_limitLength[0] > 30) UIsetFont(_textField, 16);
		y += 80;
	} else {
		y += 16; i = x - 20; if(i < 4) i = 4;
		v = ViewAddLabel(wnd, 0, titles[0], i, y, w, 32, UITextAlignmentLeft);
		y += 34;
		_textField = v = ViewAddTextField(wnd, TAG_TEXT_INPUT, values[0], x, y, w, 40);
		y += 48; i = x - 20; if(i < 4) i = 4;
		v = ViewAddLabel(wnd, 0, titles[1], i, y, w, 32, UITextAlignmentLeft);
		y += 34;
		v = ViewAddTextField(wnd, TAG_TEXT_INPUT+1, values[1], x, y, w, 40);
		y += 48;
	}
	kpd = &_kpd;
	ViewAddKeypad(kpd, wnd, 0);
	if(_inputType[0] == TI_ALPHANUMERIC || _inputType[0] == TI_ALPHANUMERIC_PASSWORD) {
		_qwertyMode = 0;
		KeypadChangeQwerty(kpd, _qwertyMode);
	} else if(_inputType[0] == TI_HEXANUMERIC) {
		KeypadChangeHexaNumeric(kpd);
	} else {
		KeypadChangeNumeric(kpd);
	}
	_timerID = 7;
	UItimerStart(_timerID, 500, 1);
}

void TextInputDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

static int	tmrCount;
void _TextFieldDrawCursor(void *self, int textPosition, int count);

void TextInputDialogOnTimer(GACTIVITY *act, int timerID)
{
	void	*wnd;
	int		textPosition;

	wnd = activityWindow(act);
	if(timerID == _timerID) {
		textPosition = UItextPosition(_textField);
		_TextFieldDrawCursor(_textField, textPosition, tmrCount);
		tmrCount++;
	}
}

static void MoveFocusedTextField(void *wnd, int position);

void TextInputDialogOnClick(GACTIVITY *act, void *view)
{
	void	*wnd, *txtfld;
	char	*p, temp[256], result[256], ch[4];
	BOOL	bRval;
	int		c, add, tag, position, inputType, limitLength;

	wnd = activityWindow(act);
	tag = UItag(view);
	position = UItextPosition(_textField);
	if(UItag(_textField) == TAG_TEXT_INPUT) {
		inputType = _inputType[0]; limitLength = _limitLength[0];
	} else {
		inputType = _inputType[1]; limitLength = _limitLength[1];
	}
	if(inputType == TI_ALPHANUMERIC || inputType == TI_ALPHANUMERIC_PASSWORD) {
		c = KeypadQwertyCode(view, _qwertyMode);
	} else if(inputType == TI_HEXANUMERIC) {
		c = KeypadHexaNumericCode(view);
	} else {
		c = KeypadNumericCode(view);
	}
	if(c >= 0) {
		p = UItext(_textField);
		if(c == '*' && inputType == TI_IP_ADDRESS) c = '.';
		if(strlen(p) < limitLength) {
			_TextFieldDrawCursor(_textField, position, 0);
			ch[0] = c; ch[1] = 0;
			strcpy(temp, p);
			position = textInsertCharAtPosition(temp, position, ch); 
			UIsetTextPosition(_textField, position);
			UIsetText(_textField, temp);
		}
		if((inputType == TI_ALPHANUMERIC || inputType == TI_ALPHANUMERIC_PASSWORD) && _qwertyMode == 1) {
			_qwertyMode = 0;
			KeypadChangeQwerty(kpd, _qwertyMode);
		}
	} else if(tag == TAG_KEYPAD_FUNC_BTN) { 
		position = UItextPosition(_textField);
		_TextFieldDrawCursor(_textField, position, 0);
		strcpy(temp, UItext(_textField));
		position = textDeleteCharBelowPosition(temp, position); 
		UIsetTextPosition(_textField, position);
		UIsetText(_textField, temp);
	} else if(tag == TAG_KEYPAD_FUNC_BTN+1) { 
		if(inputType == TI_ALPHANUMERIC || inputType == TI_ALPHANUMERIC_PASSWORD) {
			_qwertyMode++;
			if(_qwertyMode > 4) _qwertyMode = 3;	
			else if(_qwertyMode == 3) _qwertyMode = 0;
			KeypadChangeQwerty(kpd, _qwertyMode);
		} else {
			MoveFocusedTextField(wnd, position);
		}
	} else if(tag == TAG_KEYPAD_FUNC_BTN+2) { 
		if(inputType == TI_ALPHANUMERIC || inputType == TI_ALPHANUMERIC_PASSWORD) {
			if(_qwertyMode < 3) _qwertyMode = 3;	
			else	_qwertyMode = 0;
			KeypadChangeQwerty(kpd, _qwertyMode);
		} else {
			MoveFocusedTextField(wnd, position);
		}
	} else if(tag == TAG_KEYPAD_FUNC_BTN+3) { 
		MoveFocusedTextField(wnd, position);
	} else {
		switch(tag) {
		case TAG_BACK_BTN:
			UItimerStop(_timerID);
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
		case TAG_CANCEL_BTN:
			UItimerStop(_timerID);
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
		case TAG_OK_BTN:
			txtfld = UIviewWithTag(wnd, TAG_TEXT_INPUT);
			p = temp;
			sprintf(p, "%s=%s\n", titles[0], UItext(txtfld)); p += strlen(p);
			txtfld = UIviewWithTag(wnd, TAG_TEXT_INPUT+1);
			if(txtfld) sprintf(p, "%s=%s\n", titles[1], UItext(txtfld));
			if(_doneValidate) bRval = (*_doneValidate)(temp, result);
			else	bRval = TRUE;
			if(bRval) {
				UItimerStop(_timerID);
				DestroyActivityForResult(act, RESULT_OK, temp);
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
	int		inputType;

	txtfld = UIviewWithTag(wnd, TAG_TEXT_INPUT+1);
	if(txtfld) {
		_TextFieldDrawCursor(_textField, position, 0);
		if(txtfld == _textField) {
			_textField = UIviewWithTag(wnd, TAG_TEXT_INPUT);
			inputType = _inputType[0];
		} else {
			_textField = txtfld;	
			inputType = _inputType[1];
		}
		if(inputType == TI_ALPHANUMERIC || inputType == TI_ALPHANUMERIC_PASSWORD) {
			_qwertyMode = 0;
			KeypadChangeQwerty(kpd, _qwertyMode);
		} else if(inputType == TI_HEXANUMERIC) {
			KeypadChangeHexaNumeric(kpd);
		} else {
			KeypadChangeNumeric(kpd);
		}
	}
}

void TextInputDialogOnTextFieldFocused(GACTIVITY *act, void *view, int textPosition)
{
	_TextFieldDrawCursor(_textField, UItextPosition(_textField), 0);
	_textField = view;
	UIsetTextPosition(view, textPosition);
}

