#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "msg.h"
#include "fsuser.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "pref.h"
#include "prefspec.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "uv.h"
#include "beacon.h"
#include "appact.h"


unsigned char	bleUpdated;

void AdmSetBleBeaconOnCreate(GACTIVITY *act);
void AdmSetBleBeaconOnDestroy(GACTIVITY *act);
void AdmSetBleBeaconOnClick(GACTIVITY *act, void *view);
void AdmSetBleBeaconOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmSetBleBeaconOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
int  AdmSetBleBeaconOnNumberOfRowsInTableView(void *self, void *tblv);
int  AdmSetBleBeaconOnHeightForRowAtIndex(void *self, void *tblv, int index);
void *AdmSetBleBeaconOnCellForRowAtIndex(void *self, void *tblv, int index);
void AdmSetBleBeaconOnDidSelectRowAtIndex(void *self, void *tblv, int index);

void *MainLoop(void);
static void		*tblView;
static uv_timer_t	*timerBleBeacon, _timerBleBeacon;
unsigned char	bleUpdated;


void AdmSetBleBeacon(int requestCode, void *pss)
{
	GACTIVITY	*act;

	act = CreateActivity(AdmSetBleBeaconOnCreate);
	activitySetPsContext(act, pss);
	activitySetTimeout(act, 60);
	appStartActivity(act, NULL, 0);
}

void BleBeaconOnTimer(uv_timer_t *handle)
{
	if(bleUpdated) {
		UIreloadData(tblView);
		appDrawCurrentActivity();
		bleUpdated = 0;
	}
	uv_timer_start(timerBleBeacon, BleBeaconOnTimer, 500, 0);
}

void AdmSetBleBeaconOnCreate(GACTIVITY *act)
{
	void	*wnd, *tblv, *v, *ps; 

	ps = act->psContext;
	activitySetOnDestroy(act, AdmSetBleBeaconOnDestroy);
	activitySetOnClick(act, AdmSetBleBeaconOnClick);
	activitySetOnAppMessage(act, AdmSetBleBeaconOnAppMessage);
	activitySetOnActivityResult(act, AdmSetBleBeaconOnActivityResult);
	tblView = tblv = AdmActInitWithTableView(act, PStitle(ps));
	wnd = activityWindow(act);
	UIsetOnNumberOfRowsInTableView(tblv, AdmSetBleBeaconOnNumberOfRowsInTableView);
	UIsetOnHeightForRowAtIndex(tblv, AdmSetBleBeaconOnHeightForRowAtIndex);
	UIsetOnCellForRowAtIndex(tblv, AdmSetBleBeaconOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, AdmSetBleBeaconOnDidSelectRowAtIndex);
	UIreloadData(tblv);
	bleUpdated = 0;
	timerBleBeacon = &_timerBleBeacon;
	uv_timer_init(MainLoop(), timerBleBeacon);
	uv_timer_start(timerBleBeacon, BleBeaconOnTimer, 500, 0);
}

void AdmSetBleBeaconOnDestroy(GACTIVITY *act)
{
	uv_timer_stop(timerBleBeacon);
	AdmActExit(act);
}

void AdmSetBleBeaconOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_PREF_BACK:		DestroyActivity(act); break;
		break;
	}
}

void AdmSetBleBeaconOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd, *tblv, *cell;
	BEACON	*bc, _bc;
	char	temp[80];
	int		rval, index;

	wnd = activityWindow(act);
	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	switch(msg[0]) {
	case GM_BEACONS_UPDATED:
		switch(msg[1]) {
		case 0: UIreloadData(tblv); break;
		case 1: printf("ReloadlastRow=%d\n", beaconsCount()); UIloadLastRow(tblv); break;
		case 2:
			index = msg[2];
			bc = &_bc;
			cell = UIcellForRowAtIndex(tblv, index);
			rval = beaconsReadAtIndex(bc, index);
			if(rval > 0 && cell) {
				sprintf(temp, "%02d:%02d:%02d   %d dB", (int)bc->ctm[0], (int)bc->ctm[1], (int)bc->ctm[2], (int)bc->rssi);
				CellSetSubtitle(cell, temp);
			}	
			break;
		}
		break;
	}
}

void AdmSetBleBeaconOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
}

int AdmSetBleBeaconOnNumberOfRowsInTableView(void *self, void *tblv)
{
	return beaconsCount();
}

int AdmSetBleBeaconOnHeightForRowAtIndex(void *self, void *tblv, int index)
{
	return 68;
}

void *AdmSetBleBeaconOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell;
	BEACON	*bc, _bc;
	char	title[64], subtitle[64];;
	int		val, duration, hh, mm, ss;

	bc = &_bc;
	val = beaconsReadAtIndex(bc, index);
	if(val > 0) {
		sprintf(title, "%s", mac2addr(bc->addr));
		sprintf(subtitle, "%02d:%02d:%02d   %d dB", (int)bc->ctm[0], (int)bc->ctm[1], (int)bc->ctm[2], (int)bc->rssi);
	} else {
		title[0] = subtitle[0] = ' ';
		title[1] = subtitle[1] = 0;
	}
	cell = AdmInitTableViewCell(tblv, UITableViewCellStyleSubtitle, UITableViewCellAccessoryTypeNone, title);
	CellSetSubtitle(cell, subtitle);
	return cell;
}

void AdmSetBleBeaconOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
}

