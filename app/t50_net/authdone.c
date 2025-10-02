#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NSEnum.h"
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "dlg.h"
#include "topprim.h"
#include "admprim.h"
#include "actprim.h"
#include "appact.h"


void AuthDoneOnCreate(GACTIVITY *act);
void AuthDoneOnDestroy(GACTIVITY *act);
void AuthDoneOnTimer(GACTIVITY *act, int timerID);
void AuthDoneOnClick(GACTIVITY *act, void *view);
void AuthDoneOnAppMessage(GACTIVITY *act, unsigned char *msg);


void AuthDoneStart(void)
{
	GACTIVITY	*act;

	act = CreateActivity(AuthDoneOnCreate);
	appStartActivityForResult(act, ACT_AUTHDONE, NULL, 0);
}

void AuthDoneOnCreate(GACTIVITY *act)
{
/*
	void	*wnd, *lbl, *imgv;
	CGRect	rect;
	unsigned char	*p;
	char	*st, temp[100];
	int		x, y, w, h;

printf("AuthDoneOnCreate...\n");
	activitySetOnDestroy(act, AuthDoneOnDestroy);
	activitySetOnTimer(act, AuthDoneOnTimer);
	activitySetOnClick(act, AuthDoneOnClick);
	activitySetOnAppMessage(act, AuthDoneOnAppMessage);
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	if(doneUser->accessEvent == 0) {
		x = greenColor;
	} else {
		x = redColor;	
	}
	UIsetBackgroundColor(wnd, x); 
	activitySetViewBuffer(act, ViewAllocBuffer());
	ViewAddBackButton(wnd);
	UIsetTintColor(wnd, blackColor);
	UIframe(wnd, &rect);
printf("Wndow: %d,%d %d %d\n", rect.x, rect.y, rect.width, rect.height);
	w = 240; x = (rect.width - w) >> 1;
	y = 100;
	p = doneUser->accessTime;
	sprintf(temp, "%04d-%02d-%02d %02d:%02d:%02d", p+2000, (int)p[1],  (int)p[2], (int)p[3], (int)p[4], (int)p[5]);
//printf("time=[%s]\n", temp);
	h = FONT_TYPE_22;
	lbl = ViewAddLabel(wnd, TAG_RESULT_TIME, temp, x, y, w, h, UITextAlignmentCenter);
	UIsetTextColor(lbl, blackColor);
	h = 40;
	y += h + 8; h = h = FONT_TYPE_22;
//printf("y=%d msg=[%s]\n", y, GetResultMsg(doneUser->msgIndex));
	st = GetResultMsg(doneUser->msgIndex);
	strcpy_swap(temp, st, '\n', ' ');
	lbl = ViewAddLabel(wnd, TAG_RESULT_MSG, temp, x, y, w, h, UITextAlignmentCenter);
	//statSetLinePitch(stat, 8);
	UIsetTextColor(lbl, blackCololr);
	UItimerStart(1, 5000, 0);
*/
}

void AuthDoneOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

void AuthDoneOnTimer(GACTIVITY *act, int timerID)
{
	void	*wnd;

printf("AuthDoneOnTimer...%d\n", timerID);
	wnd = activityWindow(act);
	if(timerID == 1) {
		DestroyActivityForResult(act, RESULT_TIMEOUT, NULL);
	}
}

void AuthDoneOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
printf("AuthDoneOnClick...tag=%d\n", tag);
	switch(tag) {
	case TAG_BACK_BTN:
		UItimerStop(1);
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	}
}

void AuthDoneOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;

	wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_CRED_CAPTURED:
		UItimerStop(1);
		DestroyActivityForResult(act, RESULT_OK, NULL);
		break;
	}
}
