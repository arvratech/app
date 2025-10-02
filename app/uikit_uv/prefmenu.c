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
#include "psprim.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "pref.h"


void PrefMenuOnCreate(GACTIVITY *act);
void PrefMenuOnStart(GACTIVITY *act);
void PrefMenuOnDestroy(GACTIVITY *act);
void PrefMenuOnClick(GACTIVITY *act, void *view);
int  PrefMenuOnNumberOfRowsInTableView(void *self, void *tblv);
void *PrefMenuOnCellForRowAtIndex(void *self, void *tblv, int index);
void PrefMenuOnDidSelectRowAtIndex(void *self, void *tblv, int index);


void PrefMenuStart(void *pss)
{
	GACTIVITY	*act;

printf("PrefMenuStart: %s %d\n", PStitle(pss), PSscreenChildrenCount(pss));
	act = CreateActivity(PrefMenuOnCreate);
	act->psContext = pss;
	activitySetTimeout(act, 30);
	appStartActivity(act, NULL, 0);
}

void PrefMenuOnCreate(GACTIVITY *act)
{
	void	*tblv, *ps;

printf("PrefMenuOnCreate...\n");
	activitySetOnDestroy(act, PrefMenuOnStart);
	activitySetOnDestroy(act, PrefMenuOnDestroy);
	activitySetOnClick(act, PrefMenuOnClick);
	ps = act->psContext;
	tblv = AdmActInitWithTableView(act, PStitle(ps));
	UIsetOnNumberOfRowsInTableView(tblv, PrefMenuOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, PrefMenuOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, PrefMenuOnDidSelectRowAtIndex);
	UIreloadData(tblv);
	PScallOnCreate(pss);
}

void PrefMenuOnStart(GACTIVITY *act)
{
	PScallOnStart(ps, act);
}

void PrefMenuOnDestroy(GACTIVITY *act)
{
	void	*ps; 

	PScallOnDestroy(ps);
	AdmActExit(act);
	ps = activityPSContext(act);
	PSscreenCallOnEnd(ps);
}

void PrefMenuOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
printf("PrefMenuOnClick...tag=%d\n", tag);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_SETTING_BACK:
		DestroyActivity(act); break;
	}
}

int PrefMenuOnNumberOfRowsInTableView(void *self, void *tblv)
{
	GACTIVITY	*act = self;
	void	*ps;

	ps = act->psContext;
	return PSscreenChildrenCount(ps);
}

void *PrefMenuOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	GACTIVITY	*act = self;
	void	*cell, *img, *imgv, *ps;
	UITableViewCellStyle	style;
	UITableViewCellAccessoryType	accessoryType;

	style = UITableViewCellStyleDefault;
	accessoryType = UITableViewCellAccessoryTypeDisclosureIndicator;
	ps = PSscreenObjectAtIndex((PS_OBJECT *)act->psContext, index);
	cell = AdmInitTableViewCell(tblv, style, accessoryType, PStitle(ps));
	if(PSscreenImageFile(ps)) {
		img	= ImageIconCreate(PSscreenImageFile(ps), 32);
		imgv = UIimageView(cell);
		UIsetImage(imgv, img);
	}
	return cell;
}

void PrefMenuOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	GACTIVITY	*act = self;
	void	*ps;

printf("Select: %d...\n", index);
	ps = PSscreenObjectAtIndex(act->psContext, index);
	PSscreenCallOnStart(ps);
}
