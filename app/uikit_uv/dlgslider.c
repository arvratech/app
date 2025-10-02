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


void SliderDialogOnCreate(GACTIVITY *act);
void SliderDialogOnDestroy(GACTIVITY *act);
void SliderDialogOnClick(GACTIVITY *act, void *view);

#define MAX_SLIDER_SZ	4

typedef struct _SLIDER {
	char	title[64];
	int		value, minValue, maxValue;
	void	*slider;
} SLIDER; 

SLIDER	slds[MAX_SLIDER_SZ];
	
static int	_contentSize;


void *SliderDialog(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(SliderDialogOnCreate);
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 0);
	return act;
}

void SliderDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *v, *lbl, *btn;
	CGRect		rect;
	SLIDER		*sld;
	char	*p, title[64], value[64], temp[32];
	int		i, x, y, w, h, wh;

	activitySetOnDestroy(act, SliderDialogOnDestroy);
	activitySetOnClick(act, SliderDialogOnClick);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, whiteColor); 
	UIframe(wnd, &rect);
	p = act->intent;
	p = get_keyvalue(p, title, value);
	for(i = 0, sld = slds;p && i < MAX_SLIDER_SZ;i++, sld++) {
		p = get_keyvalue(p, sld->title, value); sld->value = n_atoi(value);
		p = read_token_ch(p, temp, ','); sld->minValue = n_atoi(temp);
		p = read_token_ch(p, temp, ','); sld->maxValue = n_atoi(temp);
//printf("%d [%s] [%d,%d,%d]\n", i, sld->title, sld->value, sld->minValue, sld->maxValue); 
	}
	_contentSize = i;
	wh = 48 + _contentSize * 86 + 48;
	if(wh > rect.height - 32) wh = rect.height - 32;
	rect.x = 20; rect.width -= 40; rect.y = (rect.height - wh) >> 1; rect.height = wh;
	UIsetFrame(wnd, &rect);
	x = 16; y = 0; h = 48; w = rect.width - 32;
	lbl = ViewAddLabel(wnd, 0, title, x, y, w, h, UITextAlignmentLeft);
	UIsetFont(lbl, 24);
	UIsetTextColor(lbl, blackColor);
	y += h;
	for(i = 0, sld = slds;i < _contentSize;i++, sld++) {
		h = 44;
		lbl = ViewAddLabel(wnd, 0, sld->title, x, y, w, h, UITextAlignmentLeft);
		UIsetTextColor(lbl, blackColor);
		y += h;
		sld->slider = ViewAddSlider(wnd, x-16, y, w+32, sld->value, sld->minValue, sld->maxValue);
		y += 32 + 10;
	}
	ViewAddCancelDoneButton(wnd, y);
//UIsetOnCellForRowAtIndex(tblv, DlgSpinnerOnCellForRowAtIndex);
}

void SliderDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

void SliderDialogOnTimer(GACTIVITY *act, int timerID)
{
	void	*wnd;

	wnd = activityWindow(act);
//	if(timerID == 0) ViewClockUpdate(viewClock, TAG_CLOCK_BASE);
}

void SliderDialogOnClick(GACTIVITY *act, void *view)
{
	SLIDER	*sld;
	char	*p, temp[256];
	int		i, tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:		DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_CANCEL_BTN:	DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_OK_BTN:
		p = temp;
		for(i = 0, sld = slds;i < _contentSize;i++, sld++) {
			sprintf(p, "input=%d\n", UIvalue(sld->slider)); p += strlen(p);
		}
		DestroyActivityForResult(act, RESULT_OK, temp);
		break;
	}
}
