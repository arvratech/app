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
#include "actprim.h"
#include "pref.h" 
#include "admtitle.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "ping.h"
#include "appact.h"

extern unsigned char	pingIpAddress[];

void AdmNetPingRunOnCreate(GACTIVITY *act);
void AdmNetPingRunOnStart(GACTIVITY *act);
void AdmNetPingRunOnDestroy(GACTIVITY *act);
void AdmNetPingRunOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmNetPingRunOnTimer(GACTIVITY *act, int timerID);
void AdmNetPingRunOnClick(GACTIVITY *act, void *view);
int  AdmNetPingRunOnNumberOfRowsInTableView(void *self, void *tblv);
void *AdmNetPingRunOnCellForRowAtIndex(void *self, void *tblv, int index);

static void		*tblv;
static int		seqNo;


void AdmNetPingRun(int requestCode, void *ps)
{
	GACTIVITY	*act;

	act = CreateActivity(AdmNetPingRunOnCreate);
	activitySetTimeout(act, 30);
	appStartActivity(act, NULL, 0);
}

void AdmNetPingRunOnCreate(GACTIVITY *act)
{
	int		i;

	activitySetOnStart(act, AdmNetPingRunOnStart);
	activitySetOnDestroy(act, AdmNetPingRunOnDestroy);
	activitySetOnAppMessage(act, AdmNetPingRunOnAppMessage);
	activitySetOnTimer(act, AdmNetPingRunOnTimer);
	activitySetOnClick(act, AdmNetPingRunOnClick);
	tblv = AdmActInitWithTableView(act, admPingRunTitle());
	UIsetOnNumberOfRowsInTableView(tblv, AdmNetPingRunOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, AdmNetPingRunOnCellForRowAtIndex);
	UIreloadData(tblv);
}

void AdmNetPingRunOnStart(GACTIVITY *act)
{
	int		rval;

	rval = pingOpen(); 
	seqNo = 1;
	pingSend(pingIpAddress, devId(NULL), seqNo);
}

void AdmNetPingRunOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

void AdmNetPingRunOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd, *cell;
	char	text[80];
	int		rval, index, result;

	wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_PING_RESULT:
		result = msg[1];
		index = seqNo;
		if(index < 5) {
			pingResultText(text, result, msg+2);
			cell = UIcellForRowAtIndex(tblv, index);
			CellSetTitle(cell, text);
			if(index < 4) {
				seqNo++;
				UItimerStart(1, 1000, 0);
			}
		}
		break;
	}
}


void AdmNetPingRunOnTimer(GACTIVITY *act, int timerId)
{
	void	*wnd;
	int		mode, val;

	wnd = activityWindow(act);
	if(timerId == 1) {
		pingSend(pingIpAddress, devId(NULL), seqNo);
	}
}

void AdmNetPingRunOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_PREF_BACK:
		UItimerStop(1);
		pingClose();
		DestroyActivity(act);
		break;
	}
}

int AdmNetPingRunOnNumberOfRowsInTableView(void *self, void *tblv)
{
	return 5;
}

void *AdmNetPingRunOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell;
	char	text[80];
	UITableViewCellStyle	style;
	UITableViewCellAccessoryType	accessoryType;

	style = UITableViewCellStyleDefault;
	accessoryType = UITableViewCellAccessoryTypeNone;
	if(index == 0) {
		sprintf(text, "PING %s 64 bytes", inet2addr(pingIpAddress));
	} else {
		text[0] = 0;
	}
//printf("cell: %d [%s]\n", index, text);
	cell = AdmInitTableViewCell(tblv, style, accessoryType, text);
	return cell;
}

