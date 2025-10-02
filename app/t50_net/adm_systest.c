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
#include "slvmlib.h"
#include "slvm.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "actprim.h"
#include "pref.h" 
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "ping.h"
#include "appact.h"
#include "av.h" 

extern unsigned char	pingIpAddress[];

void AdmSysTestRunOnCreate(GACTIVITY *act);
void AdmSysTestRunOnStart(GACTIVITY *act);
void AdmSysTestRunOnDestroy(GACTIVITY *act);
void AdmSysTestRunOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmSysTestRunOnTimer(GACTIVITY *act, int timerID);
void AdmSysTestRunOnClick(GACTIVITY *act, void *view);
int  AdmSysTestRunOnNumberOfRowsInTableView(void *self, void *tblv);
void *AdmSysTestRunOnCellForRowAtIndex(void *self, void *tblv, int index);

static void		*tblv;
static int	reqIndex, ment, gx, gpv;


void AdmSysTestRun(int requestCode, void *ps)
{
	GACTIVITY	*act;

	reqIndex = requestCode - ACT_PREF;	
printf("AdmSysTestRun: %d\n", reqIndex);
	act = CreateActivity(AdmSysTestRunOnCreate);
	activitySetTimeout(act, 30);
	devSetTestMode(NULL, 1);
	appStartActivity(act, NULL, 0);
}

void AdmSysTestRunOnCreate(GACTIVITY *act)
{
	char	*p;

	activitySetOnStart(act, AdmSysTestRunOnStart);
	activitySetOnDestroy(act, AdmSysTestRunOnDestroy);
	activitySetOnAppMessage(act, AdmSysTestRunOnAppMessage);
	activitySetOnTimer(act, AdmSysTestRunOnTimer);
	activitySetOnClick(act, AdmSysTestRunOnClick);
	switch(reqIndex) {
	case 0:		p = xmenu_test[0]; ment = 100; break;
	case 1:		p = xmenu_test[1]; ment = 100; break;
	case 2:		p = xmenu_test[2]; ment = 100; break;
	case 3:		p = xmenu_test[3]; ment = 100; break;
	}	
	tblv = AdmActInitWithTableView(act, p);
	UIsetOnNumberOfRowsInTableView(tblv, AdmSysTestRunOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, AdmSysTestRunOnCellForRowAtIndex);
	UIreloadData(tblv);
}

int _InputPresentValue(void)
{
	int		pv;

	pv = 0;
	if(tamperTrackValue(NULL)) pv |= 0x80;
	if(devAdminPass(NULL)) pv |= 0x40;
	return pv;
}

int _DoorIoPresentValue(void)
{
	void	*bo, *bi;
	int		i, pv, msk;

	pv = 0;
	for(i = 0, msk = 0x80;i < 2;i++, msk >>= 1) {
		bo = bosGet(i);
		if(boPresentValue(bo)) pv |= msk;
	}
	for(i = 0;i < 4;i++, msk >>= 1) {
		bi = bisGet(i);
		if(biPresentValue(bi)) pv |= msk;
	}
	return pv;
}

void AdmSysTestRunOnStart(GACTIVITY *act)
{
	unsigned char	msg[12];

	switch(reqIndex) {
	case 0:
		audioFlush();
		msg[0] = ment; msg[1] = 95; msg[2] = 0;
		audioPost(msg);
		UItimerStart(1, 100, 0);
		break;
	case 1:
		gpv = _InputPresentValue();
		gx = 0;
		UItimerStart(1, 20, 0);
		break;
	case 2:
		slvmWriteLocalIoConfig();
		usleep(2000);
		gpv = _DoorIoPresentValue();
		gx = 0;
		UItimerStart(1, 20, 0);
		break;
	}
}

void AdmSysTestRunOnDestroy(GACTIVITY *act)
{
	unsigned char	buf[4];
	int		i;

	AdmActExit(act);
	devSetTestMode(NULL, 0);
	if(reqIndex == 2) {
		// insert at 2022.7.26
		for(i = 0;i < 2;i++) {
			buf[1] = buf[0] = buf[2] = 0;
			usleep(700);
			slvmOutputCommand(i, buf);
		}
		usleep(900);
		slvmWriteLocalIoConfig();
		usleep(2000);
	}
}

void AdmSysTestRunOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;

	wnd = activityWindow(act);
}

int _DrawWave(int index, int h, int pv, int msk)
{
	void	*cell;
	CGRect	rt;
	CGPoint	pt;
	UIColor	color;
	int		x, y, w, val;

	cell = UIcellForRowAtIndex(tblv, index);
	UIframe(cell, &rt);
	pt.x = 20; pt.y = 38;
	UIconvertPointToScreen(cell, &pt, (CGPoint *)&rt);
	lcdPointToPixelRect(&rt);
	x = rt.x; y = rt.y; w = rt.width - x - x;
	color = blackColor;
	if(gx >= w) lcdFillRect(x, y, w, h, UIbackgroundColor(cell));
	val = pv & msk;
	if(gx < w) x += gx;
	if(val != (gpv & msk)) lcdVerticalLine(x, y, h, color);
	else if(val) lcdPixel(x, y, color);
	else	lcdPixel(x, y+h-1, color);
	return w;
}

void AdmSysTestRunOnTimer(GACTIVITY *act, int timerId)
{
	void	*wnd, *cell;
	unsigned char	buf[12];
	char	text[80];
	int		i, w, h, pv, msk, val;

	wnd = activityWindow(act);
	if(timerId == 1) {
		switch(reqIndex) {
		case 0:
			if(!audioActive()) {
				ment++;
				if(ment == 117) ment = 120;
				else if(ment == 147) ment = 150;
				else if(ment == 162) ment = 100;
				buf[0] = ment; buf[1] = 95; buf[2] = 0;
				audioPost(buf);
				cell = UIcellForRowAtIndex(tblv, reqIndex);
				sprintf(text, "%s %d", xmenu_test[0], ment);
				CellSetTitle(cell, text);
				appStartActivityTimer(act);
			}
			UItimerStart(1, 100, 0);
			break;
		case 1:
			// 10 + 24(textLabel) + 4 + 16(detailLabel) + 10 = 64
			pv = _InputPresentValue();
			h = _scale(16);
 			lcdSetDefaultClipRect();
			for(i = 0, msk = 0x80;i < 2;i++, msk >>= 1) {
				w = _DrawWave(i, h, pv, msk);
			}
			gpv = pv;
			if(gx >= w) {
				gx = 0;
				appStartActivityTimer(act);
			}
			gx++;
			UItimerStart(1, 20, 0);
			break;
		case 2:
			pv = _DoorIoPresentValue();
			h = _scale(16);
 			lcdSetDefaultClipRect();
			for(i = 0, msk = 0x80;i < 6;i++, msk >>= 1) {
				w = _DrawWave(i, h, pv, msk);
				if(i < 2 && !(gx % 64)) {
					val = gx / 64;
					if(val & 1) buf[1] = 1;
					else	buf[1] = 0;
					buf[0] = buf[2] = 0;
                    usleep(700);    // insert at 2022.5.9
					slvmOutputCommand(i, buf);
				}
			}
			gpv = pv;
			if(gx >= w) {
				gx = 0;
				appStartActivityTimer(act);
			}
			gx++;
			UItimerStart(1, 20, 0);
			break;
		}
	}
}

void AdmSysTestRunOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_PREF_BACK:
		switch(reqIndex) {
		case 0:
			audioFlush();
			UItimerStop(1);
			break;
		case 1:
		case 2:
			UItimerStop(1);
			break;
		}
		DestroyActivity(act);
		break;
	}
}

int AdmSysTestRunOnNumberOfRowsInTableView(void *self, void *tblv)
{
	return 6;
}

void *AdmSysTestRunOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell;
	char	text[40], detailText[40];
	UITableViewCellStyle	style;
	UITableViewCellAccessoryType	accessoryType;

	accessoryType = UITableViewCellAccessoryTypeNone;
	switch(reqIndex) {
	case 0:
		style = UITableViewCellStyleDefault;
		if(index == 0) sprintf(text, "%s %d", xmenu_test[0], ment);
		else	text[0] = 0;
		break;
	case 1:
		style = UITableViewCellStyleSubtitle;
		if(index == 0) strcpy(text, "Tamper");
		else if(index == 1) strcpy(text, "Pass");
		else	text[0] = 0;
		detailText[0] = 'I'; detailText[1] = 0;
		break;
	case 2:
		style = UITableViewCellStyleSubtitle;
		if(index == 0) strcpy(text, "Lock");
		else if(index == 1) strcpy(text, "Relay");
		else if(index == 2) strcpy(text, "Exit");
		else if(index == 3) strcpy(text, "DS");
		else if(index == 4) strcpy(text, "Input1");
		else if(index == 5) strcpy(text, "Input2");
		else	text[0] = 0;
		detailText[0] = 'I'; detailText[1] = 0;
		break;
	}
//printf("cell: %d [%s]\n", index, text);
	cell = AdmInitTableViewCell(tblv, style, accessoryType, text);
	if(style == UITableViewCellStyleSubtitle && text[0]) CellSetSubtitle(cell, detailText);
	return cell;
}

