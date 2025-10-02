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


void SpinnerDialogOnCreate(GACTIVITY *act);
void SpinnerDialogOnDestroy(GACTIVITY *act);
void SpinnerDialogOnTimer(GACTIVITY *act, int timerID);
void SpinnerDialogOnClick(GACTIVITY *act, void *view);
int  SpinnerDialogOnNumberOfRowsInTableView(void *self, void *tblv);
void *SpinnerDialogOnCellForRowAtIndex(void *self, void *tblv, int index);
void SpinnerDialogOnDidSelectRowAtIndex(void *self, void *tblv, int index);

#define MAX_SPINNER_ROW_SZ	 32

static int	_value;
static char	titles[MAX_SPINNER_ROW_SZ][64];
static unsigned char values[MAX_SPINNER_ROW_SZ];
int	   contentSize;


void *SpinnerDialog(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(SpinnerDialogOnCreate);
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 0);
	return act;
}

void SpinnerDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *tblv, *v;
	CGRect	rt;
	char	*p, title[64], temp[64];
	int		i, x, y, h, wh, th;

	activitySetOnDestroy(act, SpinnerDialogOnDestroy);
	activitySetOnTimer(act, SpinnerDialogOnTimer);
	activitySetOnClick(act, SpinnerDialogOnClick);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, whiteColor); 
	UIframe(wnd, &rt);
	p = act->intent;
	p = get_keyvalue(p, title, temp);
	_value = n_atoi(temp);
	for(i = 0;p && i < MAX_SPINNER_ROW_SZ;i++) {
		p = read_token_ch(p, titles[i], ',');
		if((*(p-1)) == '\n') {  
			i++; break;
		}
	}
	contentSize = i;
	for(i = 0;p && i < MAX_SPINNER_ROW_SZ;i++) {
		p = read_token_ch(p, temp, ',');
		values[i] = n_atoi(temp);
	}
	for(i = 0;i < contentSize;i++)
		if(values[i] == _value) break;
	if(i >= contentSize) i = 0;
	_value = i;	
	th = 48; x = 20; y = 32;
	wh = th + contentSize * th + th;
	if(wh > rt.height - y) wh = rt.height - y;
	rt.x = x; rt.width -= x+x; rt.y = (rt.height - wh) >> 1; rt.height = wh;
	UIsetFrame(wnd, &rt);
	y = 0; h = th;
	v = ViewAddLabel(wnd, 0, title, 0, y, rt.width, h, UITextAlignmentLeft);
	UIsetFont(v, 24);
	UIsetTextColor(v, blackColor);
	y += h; h = wh - th - th;
	tblv = ViewAddTableView(wnd, 0, y, rt.width, h);
	y += h;
	y += (th - ViewConfirmButtonHeight()) >> 1;
	ViewAddCancelButton(wnd, y);
	UIsetOpaque(tblv, TRUE);
	UIsetBackgroundColor(tblv, whiteColor);
	UIsetAllowsMultipleSelection(tblv, FALSE);
	UIsetDelegate(tblv, act);
	UIsetDataSource(tblv, act);
	UIsetOnNumberOfRowsInTableView(tblv, SpinnerDialogOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, SpinnerDialogOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, SpinnerDialogOnDidSelectRowAtIndex);
	UIreloadData(tblv);
}

void SpinnerDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

void SpinnerDialogOnTimer(GACTIVITY *act, int timerID)
{
	void	*wnd;

	wnd = activityWindow(act);
//	if(timerID == 0) ViewClockUpdate(viewClock, TAG_CLOCK_BASE);
}

void SpinnerDialogOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:		DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_CANCEL_BTN:	DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	}
}

int SpinnerDialogOnNumberOfRowsInTableView(void *self, void *tblv)
{
	return contentSize;
}

void *SpinnerDialogOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell, *lbl, *img, *imgv;

	cell = UIdequeueReusableCellWithIdentifier(tblv, 1);
	if(!cell) {
		cell = ViewAddTableViewCell(UITableViewCellStyleDefault, 1);
	}
	UIsetOpaque(cell, TRUE);
	UIsetBackgroundColor(cell, whiteColor);
	if(index == _value) UIsetAccessoryType(cell, UITableViewCellAccessoryTypeRadiomark);
	else	UIsetAccessoryType(cell, UITableViewCellAccessoryTypeUnradiomark);
	lbl = UItextLabel(cell);
	UIsetOpaque(lbl, TRUE);
	UIsetBackgroundColor(lbl, whiteColor);
	UIsetTextColor(lbl, blackColor); 
	UIsetText(lbl, titles[index]);
	return cell;
}

void SpinnerDialogOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	GACTIVITY	*act = self;
	char	temp[128];

	sprintf(temp, "value=%d\n", values[index]);
	DestroyActivityForResult(act, RESULT_OK, temp);
}

