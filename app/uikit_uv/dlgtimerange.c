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
#include "pref.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "dlg.h"


void TimeRangeDialogOnCreate(GACTIVITY *act);
void TimeRangeDialogOnDestroy(GACTIVITY *act);
void TimeRangeDialogOnClick(GACTIVITY *act, void *view);
void TimeRangeDialogOnValueChanged(GACTIVITY *act, void *view);


void *TimeRangeDialog(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(TimeRangeDialogOnCreate);
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 0);
	return act;
}

void TimeRangeDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *v, *lbl, *btn;
	CGRect		rect;
	unsigned char	timeRange[4];
	char	*p, title[64], value[64], temp[24];
	int		i, x, y, w, h, wh;

	activitySetOnDestroy(act, TimeRangeDialogOnDestroy);
	activitySetOnClick(act, TimeRangeDialogOnClick);
	activitySetOnValueChanged(act, TimeRangeDialogOnValueChanged);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, whiteColor); 
	UIframe(wnd, &rect);
	p = act->intent;
	p = get_keyvalue(p, title, value);
	temp[3] = 0;
	memcpy(temp, value, 2);   timeRange[0] = atoi(temp); 
	memcpy(temp, value+2, 2); timeRange[1] = atoi(temp); 
	memcpy(temp, value+4, 2); timeRange[2] = atoi(temp); 
	memcpy(temp, value+6, 2); timeRange[3] = atoi(temp); 
	wh = 140 + 48 + 48;
	rect.x = 0; rect.y = (rect.height - wh) >> 1; rect.height = wh;
	UIsetFrame(wnd, &rect);
	x = 16; y = 0; h = 48; w = rect.width - 32;
	lbl = ViewAddLabel(wnd, TAG_PREF_TITLE, "", x, y, w, h, UITextAlignmentLeft);
	UIsetTextColor(lbl, blackColor);
	y += h; h = 140;
	v = ViewAddTimeRange(wnd, y);
	UIsetTimeRange(v, timeRange);
	UIsetTag(v, TAG_PREF_CELL_CONTROL);
	y += h;
	ViewAddCancelDoneButton(wnd, y);
	TimeRangeDialogOnValueChanged(act, v);
}

void TimeRangeDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

void TimeRangeDialogOnTimer(GACTIVITY *act, int timerID)
{
	void	*wnd;

	wnd = activityWindow(act);
//	if(timerID == 0) ViewClockUpdate(viewClock, TAG_CLOCK_BASE);
}

void TimeRangeDialogOnClick(GACTIVITY *act, void *view)
{
	void	*wnd, *v; 
	unsigned char	timeRange[4];
	char	temp[64];
	int		tag;

	wnd = activityWindow(act);
	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:		DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_CANCEL_BTN:	DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_OK_BTN:
		v = UIviewWithTag(wnd, TAG_PREF_CELL_CONTROL);
		UItimeRange(v, timeRange);
		sprintf(temp, "timeRange=%02d%02d%02d%02d\n", (int)timeRange[0], (int)timeRange[1], (int)timeRange[2], (int)timeRange[3]);
		DestroyActivityForResult(act, RESULT_OK, temp);
		break;
	}
}

void TimeRangeDialogOnValueChanged(GACTIVITY *act, void *view)
{
	void	*wnd, *lbl;
	unsigned char	timeRange[4];
	char	temp[64];

	UItimeRange(view, timeRange);
	sprintf(temp, "%02d:%02d - %02d:%02d", (int)timeRange[0], (int)timeRange[1], (int)timeRange[2], (int)timeRange[3]);
//printf("ValueChanged[%s]\n", temp);
	wnd = activityWindow(act);
	lbl = UIviewWithTag(wnd, TAG_PREF_TITLE);
	UIsetText(lbl, temp);
}
