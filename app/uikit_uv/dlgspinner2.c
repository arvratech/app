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
#include "fsuser.h"
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


void SpinnerDialog(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(SpinnerDialogOnCreate);
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 0);
}

void SpinnerDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *tblv, *lbl, *btn;
	CGRect		rect;
	char	*p, title[64], temp[64];
	int		i, x, y, h, wh;

	activitySetOnDestroy(act, SpinnerDialogOnDestroy);
	activitySetOnTimer(act, SpinnerDialogOnTimer);
	activitySetOnClick(act, SpinnerDialogOnClick);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, whiteColor); 
	UIframe(wnd, &rect);
	p = act->intent;
	p = get_keyvalue(p, title, temp);
	_value = n_atoi(temp);
	for(i = 0;p && i < MAX_SPINNER_SZ;i++) {
		p = read_token_ch(p, titles[i], ',');
		if((*(p-1)) == '\n') {  
			i++; break;
		}
	}
	contentSize = i;
	for(i = 0;p && i < MAX_SPINNER_SZ;i++) {
		p = read_token_ch(p, temp, ',');
		values[i] = n_atoi(temp);
	}
	for(i = 0;i < contentSize;i++)
		if(values[i] == _value) break;
	if(i >= contentSize) i = 0;
	_value = i;	
	wh = 48 + contentSize * 44 + 48;
	if(wh > rect.height - 32) wh = rect.height - 32;
	rect.x = 20; rect.width -= 40; rect.y = (rect.height - wh) >> 1; rect.height = wh;
	UIsetFrame(wnd, &rect);

	y = 0; h = 48;
	lbl = ViewAddLabel(wnd, 0, title, 0, y, rect.width, h, UITextAlignmentLeft);
	UIsetFont(lbl, FONT_TYPE_28);
	UIsetTextColor(lbl, blackColor);
	y += h; h = wh - 48 - 48;
	tblv = ViewAddTableView(wnd, 0, y, rect.width, h);
	y += h;
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
	if(index == _value) UIsetAccessoryType(cell, UITableViewCellAccessoryTypeCheckmark);
	else	UIsetAccessoryType(cell, UITableViewCellAccessoryTypeUncheckmark);
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
	char	temp[80];

printf("Select: %d...\n", index);
	sprintf(temp, "value=%d\n", values[index]);
	DestroyActivityForResult(act, RESULT_OK, temp);
}
