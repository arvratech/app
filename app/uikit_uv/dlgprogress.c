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
#include "dlg.h"

void ProgressDialogOnCreate(GACTIVITY *act);
void ProgressDialogOnDestroy(GACTIVITY *act);
void ProgressDialogOnClick(GACTIVITY *act, void *view);
void ProgressDialogOnAppMessage(GACTIVITY *act, unsigned char *msg);
void ProgressDialogOnTimer(GACTIVITY *act, int timerId);

static void	*lblTarget, *lblResult, *lblDetail, *slider, *okbtn;
static int	_timerId, completed;


void *ProgressDialog(int requestCode, char *title, char *message)
{
	GACTIVITY	*act;
	char	temp[80];

	act = CreateActivity(ProgressDialogOnCreate);
	sprintf(temp, "%s=%s\n", title, message);
	activitySetTimeout(act, 0);
	appStartActivityForResult(act, requestCode, temp, 0);
	completed = 0;
	return act;
}

void ProgressDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *v;
	char	title[64], value[64];
	CGRect		rect;
	int		y, w, h;

	activitySetOnDestroy(act, ProgressDialogOnDestroy);
	activitySetOnClick(act, ProgressDialogOnClick);
	activitySetOnAppMessage(act, ProgressDialogOnAppMessage);
	activitySetOnTimer(act, ProgressDialogOnTimer);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIframe(wnd, &rect);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, whiteColor); 
	h = 280;	
	rect.x = 20; rect.width -= 40; rect.y = (rect.height - h) >> 1; rect.height = h;
	UIsetFrame(wnd, &rect);
	title[0] = ' ';
	get_keyvalue(act->intent, title+1, value);
	y = 0; h = 40;
	v = ViewAddLabel(wnd, 0, title, 0, y, rect.width, h, UITextAlignmentLeft);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, systemBlue);
	UIsetTextColor(v, blackColor);
	y += h + 20; h = 30;
	v = ViewAddLabel(wnd, 0, NULL, 0, y, rect.width, h, UITextAlignmentCenter);
	UIsetOpaque(v, FALSE);
	UIsetTextColor(v, blackColor);
	UIsetTextAlignment(v, UITextAlignmentCenter);
	lblTarget = v;
	y += 40;
	slider = ViewAddSlider(wnd, 20, y, rect.width-40, 0, 0, 100);
	UIsetValue(slider, 0);
	y += 40;
	v = ViewAddLabel(wnd, 0, NULL, 0, y, rect.width, h, UITextAlignmentCenter);
	UIsetOpaque(v, FALSE);
	UIsetTextColor(v, blackColor);
	UIsetTextAlignment(v, UITextAlignmentCenter);
	lblResult = v;
	y += 40;
	v = ViewAddLabel(wnd, 0, NULL, 0, y, rect.width, h, UITextAlignmentCenter);
	UIsetOpaque(v, FALSE);
	UIsetTextColor(v, blackColor);
	UIsetTextAlignment(v, UITextAlignmentCenter);
	UIsetFont(v, 18);
	lblDetail = v;
	y += 40;
	okbtn = ViewAddOkButton(wnd, y);
	UIsetHidden(okbtn, TRUE);
	_timerId = 7;
	UItimerStart(_timerId, 9000, 0);
}

void ProgressDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

void ProgressDialogOnClick(GACTIVITY *act, void *view)
{
	int		tag, code;

	tag = UItag(view);
	switch(tag) {
	case TAG_OK_BTN:
		if(completed) code = RESULT_OK;
		else	code = RESULT_CANCELLED;
		DestroyActivityForResult(act, code, NULL);
		break;
	}
}

void ProgressDialogOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;
	char	*p, temp[64];
	int		rval, val;

	wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_USER:			// continuous
			val = msg[1];
printf("Progress: %d\n", val);
			if(UIvalue(slider) != val) UIsetValue(slider, val);
			UItimerStart(_timerId, 9000, 0);
			break;
	case GM_USER+1:			// start
printf("### Progress start: %s\n", activityParam1(act));
			UIsetText(lblTarget, activityParam1(act));
			UIsetValue(slider, 0);
			UItimerStart(_timerId, 9000, 0);
			break;
	case GM_USER+2:			// end
		rval = msg[1];
printf("### Progress end: %d\n", msg[1]);
		if(rval == 0) {
			UIsetValue(slider, 100);
			p = GetResultMsg(R_COMPLETED); strcpy_swap(temp, p, '\n', ' ');
			completed = 1;
		} else {
			p = GetResultMsg(R_FAILED); strcpy_swap(temp, p, '\n', ' ');
		}
		UIsetText(lblResult, temp);
		UIsetText(lblDetail, activityParam2(act));
		UIsetHidden(okbtn, FALSE);
		UItimerStart(_timerId, 10000, 0);
		break;
	}
}

void ProgressDialogOnTimer(GACTIVITY *act, int timerId)
{
	int		code;

	if(timerId == _timerId) {
		if(completed) code = RESULT_OK;
		else	code = RESULT_CANCELLED;
		DestroyActivityForResult(act, code, NULL);
	}
}

