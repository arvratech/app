#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "defs_obj.h"
#include "NSEnum.h"
#include "defs_pkt.h"
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
#include "appact.h"


void AdmAcctOnCreate(GACTIVITY *act);
void AdmAcctOnDestroy(GACTIVITY *act);
void AdmAcctOnClick(GACTIVITY *act, void *view);
void AdmAcctOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmAcctOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
int  AdmAcctOnNumberOfRowsInTableView(void *self, void *tblv);
void *AdmAcctOnCellForRowAtIndex(void *self, void *tblv, int index);
void AdmAcctOnDidSelectRowAtIndex(void *self, void *tblv, int index);

ACCT		_acct1, _acct2;


void AdmAcct(int requestCode, void *pss)
{
	GACTIVITY	*act;

	act = CreateActivity(AdmAcctOnCreate);
	activitySetPsContext(act, pss);
	activitySetTimeout(act, 30);
	appStartActivity(act, NULL, 0);
}

void AdmClearTopAcct(int requestCode, void *pss)
{
	GACTIVITY	*act;

	act = CreateActivity(AdmAcctOnCreate);
	activitySetPsContext(act, pss);
	activitySetTimeout(act, 30);
	appStartActivity(act, NULL, 1);
}

void AdmAcctOnCreate(GACTIVITY *act)
{
	void	*wnd, *tblv, *v, *ps; 

printf("AdmAcctOnCreate...\n");
	ps = act->psContext;
	activitySetOnDestroy(act, AdmAcctOnDestroy);
	activitySetOnClick(act, AdmAcctOnClick);
	activitySetOnAppMessage(act, AdmAcctOnAppMessage);
	activitySetOnActivityResult(act, AdmAcctOnActivityResult);
	tblv = AdmActInitWithTableView(act, PStitle(ps));
	wnd = activityWindow(act);
	v = ViewAddButtonAlphaMask(wnd, TAG_ADD_BTN, "add_user.bmp", lcdWidth()-58, 0, 48, 48, 32);
	UIsetTintColor(v, whiteColor);
	UIsetOnNumberOfRowsInTableView(tblv, AdmAcctOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, AdmAcctOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, AdmAcctOnDidSelectRowAtIndex);
	UIreloadData(tblv);
}

void AdmAcctOnDestroy(GACTIVITY *act)
{
printf("AdmAcctOnDestroy...act=%x\n", act);
	AdmActExit(act);
}

void AdmAcctOnClick(GACTIVITY *act, void *view)
{
	ACCT	*acct;
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_PREF_BACK:
			DestroyActivity(act);
			break;
	case TAG_ADD_BTN:
		acct = &_acct2;
		acctSetDefault(acct);
		AdmAcctWrStart(ACT_ADM_ADD, acct);
		break;
	}
}

void AdmAcctOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
}

void AdmAcctOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	void	*wnd, *tblv;
	ACCT	*acct;

	wnd = activityWindow(act);
printf("AdmAcctOnActivityResult...request=%d result=%d\n", requestCode, resultCode);
	if(resultCode != RESULT_OK) return;
	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	if(requestCode == ACT_ADM_ADD) {
		UIreloadData(tblv);
		acct = &_acct1;
		acctCopy(acct, &_acct2);	
		AdmAcctRdStart(0, acct);
	} else if(requestCode >= ACT_ADM_INPUT) {
		UIreloadData(tblv);
	}
}

int AdmAcctOnNumberOfRowsInTableView(void *self, void *tblv)
{
	int		rval;

	rval = acctfsGetCount();
	if(rval < 0) rval = 0;
	return rval;
}

void *AdmAcctOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell;
	ACCT	*acct, _acct;
	char	title[64], subtitle[68];;
	int		rval;

	acct = &_acct;
	rval = acctfsReadAtIndex(acct, index);
	if(rval > 0) {
		sprintf(title, "%s", acct->acctName);
		sprintf(subtitle, "%s", acct->url);
	} else {
		title[0] = subtitle[0] = 0;
	}
	cell = AdmInitTableViewCell(tblv, UITableViewCellStyleSubtitle, UITableViewCellAccessoryTypeNone, title);
	CellSetSubtitle(cell, subtitle);
	return cell;
}

void AdmAcctOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell;
	ACCT	*acct;
	int		rval;
	
	cell = UIcellForRowAtIndex(tblv, index);
	if(cell) {
		acct = &_acct1;
		strcpy(acct->url, CellSubtitle(cell));
		acctfsSeek(0L);
		rval = acctfsGetWithUrl(acct, acct->url);
printf("######## rval=%d [%s] [%s]\n", rval, acct->acctName, acct->url);
		AdmAcctRdStart(ACT_ADM_INPUT+index, acct);
	}
}

