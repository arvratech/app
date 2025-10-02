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
#include "admtitle.h"
#include "actalert.h"
#include "appact.h"


void AdmUsrOnCreate(GACTIVITY *act);
void AdmUsrOnDestroy(GACTIVITY *act);
void AdmUsrOnClick(GACTIVITY *act, void *view);
void AdmUsrOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmUsrOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
int  AdmUsrOnNumberOfRowsInTableView(void *self, void *tblv);
void *AdmUsrOnCellForRowAtIndex(void *self, void *tblv, int index);
void AdmUsrOnDidSelectRowAtIndex(void *self, void *tblv, int index);

FP_USER		_user1, _user2;


void AdmUsr(int requestCode, void *pss)
{
	GACTIVITY	*act;

printf("AdmUsrStart...\n");
	act = CreateActivity(AdmUsrOnCreate);
	activitySetPsContext(act, pss);
	activitySetTimeout(act, 30);
	appStartActivity(act, NULL, 0);
}

void AdmClearTopUsr(int requestCode, void *ps)
{
	GACTIVITY	*act;

printf("AdmClearTopUsrStart...ps=%x\n", ps);
	act = CreateActivity(AdmUsrOnCreate);
	activitySetPsContext(act, ps);
	activitySetTimeout(act, 30);
	appStartActivity(act, NULL, 1);
}

void AdmUsrOnCreate(GACTIVITY *act)
{
	void	*wnd, *tblv, *v, *ps; 

printf("AdmUsrOnCreate...\n");
	ps = act->psContext;
	activitySetOnDestroy(act, AdmUsrOnDestroy);
	activitySetOnClick(act, AdmUsrOnClick);
	activitySetOnAppMessage(act, AdmUsrOnAppMessage);
	activitySetOnActivityResult(act, AdmUsrOnActivityResult);
	tblv = AdmActInitWithTableView(act, PStitle(ps));
	wnd = activityWindow(act);
	v = ViewAddButtonAlphaMask(wnd, TAG_ADD_BTN, "add_user.bmp", lcdWidth()-58, 0, 48, 48, 32);
	UIsetTintColor(v, whiteColor);
	UIsetOnNumberOfRowsInTableView(tblv, AdmUsrOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, AdmUsrOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, AdmUsrOnDidSelectRowAtIndex);
	UIreloadData(tblv);
}

void AdmUsrOnDestroy(GACTIVITY *act)
{
printf("AdmUsrOnDestroy...act=%x\n", act);
	AdmActExit(act);
}

void AdmUsrOnClick(GACTIVITY *act, void *view)
{
	FP_USER	*user;
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_PREF_BACK:		DestroyActivity(act); break;
	case TAG_ADD_BTN:
		user = &_user2;
		userSetDefault(user);
		user->id = 0L; user->accessMode = USER_CARD;
		AdmUsrWrStart(ACT_ADM_ADD, user);
		break;
	}
}

extern unsigned char	reqCmd, reqOt, reqOp;
void acuReqWriteUser(FP_USER *user);
int  acuCnfWriteUser(void);

void AdmUsrOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
}

void AdmUsrOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	void	*wnd, *tblv;
	FP_USER	*user;

	wnd = activityWindow(act);
printf("AdmUsrOnActivityResult...request=%d result=%d\n", requestCode, resultCode);
	if(resultCode != RESULT_OK) return;
	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	if(requestCode == ACT_ADM_ADD) {
		UIreloadData(tblv);
		user = &_user1;
		userCopy(user, &_user2);
		AdmUsrRdStart(0, user);
	} else if(requestCode >= ACT_ADM_INPUT) {
		UIreloadData(tblv);
	}
}

int AdmUsrOnNumberOfRowsInTableView(void *self, void *tblv)
{
	int		rval;

	rval = userfsGetCount();
	if(rval < 0) rval = 0;
	return rval;
}

void *AdmUsrOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell;
	FP_USER	*user, _user;
	char	title[64], subtitle[64];;
	int		rval;

	user = &_user;
	rval = userfsReadAtIndex(user, index);
	if(rval > 0) {
		sprintf(title, "%u", user->id);
		subtitle[0] = 0;
	} else {
		title[0] = subtitle[0] = 0;
	}
	cell = AdmInitTableViewCell(tblv, UITableViewCellStyleSubtitle, UITableViewCellAccessoryTypeNone, title);
	CellSetSubtitle(cell, subtitle);
	return cell;
}

void AdmUsrOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell, *v;
	FP_USER	*user;
	int		rval;
	
	cell = UIcellForRowAtIndex(tblv, index);
	if(cell) {
		v = UItextLabel(cell);
		user = &_user1;
		user->id = n_atol(UItext(v));
		rval = userfsGet(user);
		AdmUsrRdStart(ACT_ADM_INPUT+index, user);
	}
}

