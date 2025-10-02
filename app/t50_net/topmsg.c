#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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


void TopMsgOnCreate(GACTIVITY *act);
void TopMsgOnDestroy(GACTIVITY *act);
void TopMsgOnTimer(GACTIVITY *act, int timerID);
void TopMsgOnClick(GACTIVITY *act, void *view);
void TopMsgOnAppMessage(GACTIVITY *act, unsigned char *msg);

static int	resultIndex, count;


void TopMsgStart(char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(TopMsgOnCreate);
	appStartActivityForResult(act, ACT_TOPMSG, intent, 0);
}

void TopMsgRestart(char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(TopMsgOnCreate);
	appStartActivityForResult(act, ACT_TOPMSG, intent, 1);
}

void TopMsgOnCreate(GACTIVITY *act)
{
	void	*wnd, *lbl, *imgv;
	CGRect	rt;
	UIColor color;
	char	temp[32];
	int		x, y, w, h, val;

	activitySetOnDestroy(act, TopMsgOnDestroy);
	activitySetOnTimer(act, TopMsgOnTimer);
	activitySetOnClick(act, TopMsgOnClick);
	activitySetOnAppMessage(act, TopMsgOnAppMessage);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIframe(wnd, &rt);
	w = rt.width - 40;  h = 180;
	x = (rt.width - w) >> 1; y = (rt.height - h) >> 1;
	rt.x = x; rt.y = y; rt.width = w; rt.height = h;
	UIsetFrame(wnd, &rt);
	UIsetOpaque(wnd, TRUE);
//	viewSetTintColor(v, blackColor);
	resultIndex = atoi(act->intent);		
	val = xrsp_msgs_icon[resultIndex];
	switch(val) {
	case MB_ICONINFORMATION: color = whiteColor; break;
	case MB_ICONWARNING:	 color = whiteColor; break;
	case MB_ICONERROR:		 color = magentaColor; break;
	case MB_ICONWAITING:	 color = whiteColor;
	}
	UIsetBackgroundColor(wnd, color);
	y = 0;
	if(resultIndex < R_TAMPER_ALARM) strcpy(temp, " Authentication");
	else	strcpy(temp, " Alarm");	
	h = 50;
	lbl = ViewAddLabel(wnd, 0, temp, 0, y, w, h, UITextAlignmentLeft);
	UIsetBackgroundColor(lbl, lightGrayColor);
	UIsetOpaque(lbl, TRUE);
	UIsetFont(lbl, 22);
	UIsetTextColor(lbl, blackColor);
	y += h;
	x = 4;
//	imgv = ViewAddImageViewIcon(wnd, 0, iconFile, x, y+16, 48);
//	x += 48;
	lbl = ViewAddLabel(wnd, 0, GetResultMsg(resultIndex), x, y, w-x, rt.height-h, UITextAlignmentCenter);
	UIsetFont(lbl, 24);
	UIsetTextColor(lbl, blackColor);
	if(resultIndex == R_TAMPER_ALARM) MentTamperAlarm();
	else if(resultIndex == R_SLAVE_INITIALIZING) ;
	else {
		if(resultIndex == R_SYSTEM_RESTART || resultIndex == R_POWER_FAIL || resultIndex == R_POWER_OFF) val = 200;
		else	val = 2500;
		UItimerStart(1, val, 0);
	}
	count = 0;
}

void TopMsgOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

void MainShutdown(void);

void TopMsgOnTimer(GACTIVITY *act, int timerID)
{
	if(timerID == 1) {
		if(resultIndex == R_SYSTEM_RESTART || resultIndex == R_POWER_FAIL || resultIndex == R_POWER_OFF) {
printf("topMsgTimer...\n");
			if(count) MainShutdown();
			else {
#ifndef _WIEGAND_READER
				fsCloseFiles();
#endif
				UItimerStart(1, 800, 0);
			}
		} else
			DestroyActivityForResult(act, RESULT_TIMEOUT, NULL);
		count++;
	}
}

void TopMsgOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
		if(resultIndex < R_TAMPER_ALARM) {
			UItimerStop(1);
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		}
	}
}

void AuthIdentify(GACTIVITY *act, int clearTop);

void TopMsgOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;

	wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_CRED_CAPTURED:
		if(resultIndex < R_TAMPER_ALARM) {
			UItimerStop(1);
			AuthIdentify(act, 1);
		}
		break;
	case GM_TAMPER_NORMAL:
		if(resultIndex == R_TAMPER_ALARM) {
			audioFlushAlarm();
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		}
		break;
	case GM_SS_CHANGED:
		if(resultIndex == R_SLAVE_INITIALIZING && devSystemStatus(NULL) == SS_OPERATIONAL) {
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		}
		break;
	}
}

