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


void CheckDialogOnCreate(GACTIVITY *act);
void CheckDialogOnDestroy(GACTIVITY *act);
void CheckDialogOnTimer(GACTIVITY *act, int timerID);
void CheckDialogOnClick(GACTIVITY *act, void *view);
int  CheckDialogOnNumberOfRowsInTableView(void *self, void *tblv);
void *CheckDialogOnCellForRowAtIndex(void *self, void *tblv, int index);
void CheckDialogOnDidSelectRowAtIndex(void *self, void *tblv, int index);

#define MAX_CHECK_SZ	 16

static char	titles[MAX_CHECK_SZ][64];
static unsigned char values[MAX_CHECK_SZ];

static int	   contentSize;


void *CheckDialog(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(CheckDialogOnCreate);
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 0);
	return act;
}

void CheckDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *tblv, *v;
	CGRect		rt;
	char	*p, title[200], temp[200];
	int		i, x, y, h, wh, th;

	activitySetOnDestroy(act, CheckDialogOnDestroy);
	activitySetOnTimer(act, CheckDialogOnTimer);
	activitySetOnClick(act, CheckDialogOnClick);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, whiteColor); 
	UIframe(wnd, &rt);
	p = act->intent;
	p = get_keyvalue(p, title, temp);
	for(i = 0;p && i < MAX_CHECK_SZ;i++) {
		p = read_token_ch(p, titles[i], ',');
		if((*(p-1)) == '\n') {  
			i++; break;
		}
	}
printf("check items=%d\n", i);
	contentSize = i;
	for(i = 0;p && i < MAX_CHECK_SZ;i++) {
		p = read_token_ch(p, temp, ',');
		values[i] = n_atoi(temp);
	}
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
	ViewAddCancelDoneButton(wnd, y);
	UIsetOpaque(tblv, TRUE);
	UIsetBackgroundColor(tblv, whiteColor);
	UIsetAllowsMultipleSelection(tblv, TRUE);
	UIsetDelegate(tblv, act);
	UIsetDataSource(tblv, act);
	UIsetOnNumberOfRowsInTableView(tblv, CheckDialogOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, CheckDialogOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, CheckDialogOnDidSelectRowAtIndex);
	UIreloadData(tblv);
}

void CheckDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

void CheckDialogOnTimer(GACTIVITY *act, int timerID)
{
	void	*wnd;

	wnd = activityWindow(act);
//	if(timerID == 0) ViewClockUpdate(viewClock, TAG_CLOCK_BASE);
}

void CheckDialogOnClick(GACTIVITY *act, void *view)
{
	char	*p, temp[128];
	int		i, tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:		DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_CANCEL_BTN:	DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_OK_BTN:
		p = temp;
		strcpy(p, "Values="); p += strlen(p);
		for(i = 0;i < contentSize;i++) {
			if(values[i]) *p++ = '1'; else *p++ = '0';
			*p++ = ',';
		}
		*(p-1) = '\n';
		DestroyActivityForResult(act, RESULT_OK, temp);
		break;
	}
}

int CheckDialogOnNumberOfRowsInTableView(void *self, void *tblv)
{
	return contentSize;
}

void *CheckDialogOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell, *lbl, *img, *imgv;

	cell = UIdequeueReusableCellWithIdentifier(tblv, 1);
	if(!cell) {
		cell = ViewAddTableViewCell(UITableViewCellStyleDefault, 1);
	}
	UIsetOpaque(cell, TRUE);
	UIsetBackgroundColor(cell, whiteColor);
	if(values[index]) UIsetAccessoryType(cell, UITableViewCellAccessoryTypeCheckmark);
	else	UIsetAccessoryType(cell, UITableViewCellAccessoryTypeUncheckmark);
	lbl = UItextLabel(cell);
	UIsetOpaque(lbl, TRUE);
	UIsetBackgroundColor(lbl, whiteColor);
	UIsetTextColor(lbl, blackColor); 
	UIsetText(lbl, titles[index]);
	return cell;
}

void CheckDialogOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	GACTIVITY	*act = self;
	void	*cell;
	int		val;

printf("Select: %d...\n", index);
	cell = UIcellForRowAtIndex(tblv, index);
	if(UIaccessoryType(cell) == UITableViewCellAccessoryTypeCheckmark) {
		UIsetAccessoryType(cell, UITableViewCellAccessoryTypeUncheckmark);
		val = 0;
	} else {
		UIsetAccessoryType(cell, UITableViewCellAccessoryTypeCheckmark);
		val = 1;
	}
	values[index] = val;
}
