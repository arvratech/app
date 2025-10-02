#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
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
#include "actprim.h"
#include "pref.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "appact.h"


void AdmSetDateTimeOnCreate(GACTIVITY *act);
void AdmSetDateTimeOnDestroy(GACTIVITY *act);
void AdmSetDateTimeOnClick(GACTIVITY *act, void *view);
void AdmSetDateTimeOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmSetDateTimeOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
int  AdmSetDateTimeOnNumberOfRowsInTableView(void *self, void *tblv);
void *AdmSetDateTimeOnCellForRowAtIndex(void *self, void *tblv, int index);
void AdmSetDateTimeOnDidSelectRowAtIndex(void *self, void *tblv, int index);


void AdmSetDateTime(int requestCode, void *pss)
{
	GACTIVITY	*act;

	act = CreateActivity(AdmSetDateTimeOnCreate);
	activitySetPsContext(act, pss);
	activitySetTimeout(act, 30);
	appStartActivity(act, NULL, 0);
}

void AdmSetDateTimeOnCreate(GACTIVITY *act)
{
	void	*tblv, *cell, *v;

	activitySetOnDestroy(act, AdmSetDateTimeOnDestroy);
	activitySetOnClick(act, AdmSetDateTimeOnClick);
	activitySetOnAppMessage(act, AdmSetDateTimeOnAppMessage);
	activitySetOnActivityResult(act, AdmSetDateTimeOnActivityResult);
	tblv = AdmActInitWithTableView(act, xmenu_setup[2]);
	UIsetOnNumberOfRowsInTableView(tblv, AdmSetDateTimeOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, AdmSetDateTimeOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, AdmSetDateTimeOnDidSelectRowAtIndex);

	UIreloadData(tblv);
}

void AdmSetDateTimeOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

void AdmSetDateTimeOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_PREF_BACK:		DestroyActivity(act); break;
	}
}

void AdmSetDateTimeOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
}

void AdmSetDateTimeOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	void	*wnd, *tblv, *cell;
	unsigned char	ctm[8];
	char	temp[64];
	int		rval, index;

	wnd = activityWindow(act);
	if(resultCode != RESULT_OK) return;
	index = requestCode - ACT_PREF;
	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	switch(index) {
	case 0:
		cell = UIcellForRowAtIndex(tblv, 0);
		rtcGetDate(ctm);
		sprintf(temp, "%04d/%02d/%02d", (int)ctm[0]+2000, (int)ctm[1], (int)ctm[2]);
		CellSetSubtitle(cell, temp);
		break;
	case 1:
		cell = UIcellForRowAtIndex(tblv, 1);
		rtcGetTime(ctm);
		GetTimeString(temp, (int)ctm[0], (int)ctm[1]);
		CellSetSubtitle(cell, temp);
		break;
	}
}

int AdmSetDateTimeOnNumberOfRowsInTableView(void *self, void *tblv)
{
	return 3;
}

void *AdmSetDateTimeOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	GACTIVITY	*act = self;
	void	*cell, *lbl, *v;
	unsigned char	ctm[8];
	UITableViewCellStyle	style;
	UITableViewCellAccessoryType	accessoryType;
	CGRect		rect;
	char	temp[32];
	int		val;

	if(index == 2) {
		style = UITableViewCellStyleSubtitle;
		if(sys_cfg->dateNotation & TIME_NOTATION) accessoryType = UITableViewCellAccessoryTypeCheckmark;
		else	accessoryType = UITableViewCellAccessoryTypeUncheckmark;
	} else {
		style = UITableViewCellStyleSubtitle;
		accessoryType = UITableViewCellAccessoryTypeNone;
	}
	cell = AdmInitTableViewCell(tblv, style, accessoryType, xmenu_datetime[index]);
	lbl = UIdetailTextLabel(cell);
	switch(index) {
	case 0:
		rtcGetDate(ctm);
		sprintf(temp, "%04d/%02d/%02d", (int)ctm[0]+2000, (int)ctm[1], (int)ctm[2]);
		UIsetText(lbl, temp);
		break;
	case 1:
		rtcGetTime(ctm);
		GetTimeString(temp, (int)ctm[0], (int)ctm[1]);
		UIsetText(lbl, temp);
		break;
	case 2:
		if(sys_cfg->dateNotation & TIME_NOTATION) val = 1; else val = 0;
		UIsetText(lbl, xmenu_time_notation[val]);
		break;
	}
	return cell;
}

void AdmSetDateTimeOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell, *lbl;
	BOOL	bVal;
	unsigned char	ctm[8];
	char	*p, temp[256];
	int		val;

	switch(index) {
	case 0:
		rtcGetDate(ctm);
		DatePickerDialog(ACT_PREF+index, (int)ctm[0]+2000, (int)ctm[1], (int)ctm[2]);
		break;
	case 1:
		rtcGetTime(ctm);
		if(sys_cfg->dateNotation & TIME_NOTATION) bVal = TRUE; else bVal = FALSE;
		TimePickerDialog(ACT_PREF+index, (int)ctm[0], (int)ctm[1], bVal);
		break;
	case 2:
		cell = UIcellForRowAtIndex(tblv, index);
		if(UIaccessoryType(cell) == UITableViewCellAccessoryTypeCheckmark) {
			sys_cfg->dateNotation &= ~TIME_NOTATION;
			UIsetAccessoryType(cell, UITableViewCellAccessoryTypeUncheckmark);
		} else {
			sys_cfg->dateNotation |= TIME_NOTATION;
			UIsetAccessoryType(cell, UITableViewCellAccessoryTypeCheckmark);
		}
		if(sys_cfg->dateNotation & TIME_NOTATION) val = 1; else val = 0;
		lbl = UIdetailTextLabel(cell);
		UIsetText(lbl, xmenu_time_notation[val]);
		break;
	}
}

