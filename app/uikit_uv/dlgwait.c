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


void WaitDialogOnCreate(GACTIVITY *act);
void WaitDialogOnDestroy(GACTIVITY *act);


void *WaitDialog(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(WaitDialogOnCreate);
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 0);
	return act;
}

void *WaitClearTopDialog(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(WaitDialogOnCreate);
	activitySetTimeout(act, 20);
	appStartActivityForResult(NULL, requestCode, intent, 1);
	return act;
}

void WaitDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *lbl, *btn;
	char	title[80], value[100];
	CGRect	rt;
	int		y, w, h;

	activitySetOnDestroy(act, WaitDialogOnDestroy);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, whiteColor); 
	UIframe(wnd, &rt);
	h = 40 + 80 + 40 + 20;
	rt.x = 20; rt.width -= 40; rt.y = (rt.height - h) >> 1; rt.height = h;
	UIsetFrame(wnd, &rt);
	title[0] = ' ';
	get_keyvalue(act->intent, title+1, value);
	strswap(value, '&', '\n');
	y = 0; h = 40;
	lbl = ViewAddLabel(wnd, 0, title, 0, y, rt.width, h, UITextAlignmentLeft);
	UIsetOpaque(lbl, TRUE);
	UIsetBackgroundColor(lbl, systemBlue);
	UIsetTextColor(lbl, blackColor);
	y += h; h = 80;
	lbl = ViewAddLabel(wnd, 0, value, 0, y, rt.width, h, UITextAlignmentCenter);
	UIsetOpaque(lbl, FALSE);
	UIsetTextColor(lbl, blackColor);
	UIsetTextAlignment(lbl, UITextAlignmentCenter);
}

void WaitDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

