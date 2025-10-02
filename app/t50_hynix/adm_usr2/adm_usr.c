#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "prim.h"
#include "rtctime.h"
#include "jpeg_api.h"
#include "msg.h"
#include "timer.h"
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


void AdmUsrOnCreate(GACTIVITY *act);
void AdmUsrOnDestroy(GACTIVITY *act);
void AdmUsrOnClick(GACTIVITY *act, void *view);
void AdmUsrOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmUsrOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
int  AdmUsrOnNumberOfRowsInTableView(void *self, void *tblv);
void *AdmUsrOnCellForRowAtIndex(void *self, void *tblv, int index);
void AdmUsrOnDidSelectRowAtIndex(void *self, void *tblv, int index);

static FP_USER	*user, _user, *user2, _user2;


void AdmUsr(int requestCode, void *ps)
{
	GACTIVITY	*act;

printf("AdmUsrStart...\n");
	user = &_user;
	act = CreateActivity(AdmUsrOnCreate);
	act->psContext = ps;
	appStartActivity(act, NULL);
}

void AdmClearTopUsr(int requestCode, void *ps)
{
	GACTIVITY	*act;

printf("AdmClearTopUsrStart...\n");
	user = &_user;
	act = appCurrentActivity();
	ReuseActivity(act, AdmUsrOnCreate);
	act->psContext = ps;
	appStartActivity(NULL, NULL);
}

void AdmUsrOnCreate(GACTIVITY *act)
{
	void	*wnd, *tblv, *v, *pss; 

printf("AdmUsrOnDestroy...act=%x\n", act);
	pss = act->psContext;
	activitySetOnDestroy(act, AdmUsrOnDestroy);
	activitySetOnClick(act, AdmUsrOnClick);
	activitySetOnAppMessage(act, AdmUsrOnAppMessage);
	activitySetOnActivityResult(act, AdmUsrOnActivityResult);
	tblv = AdmActInitWithTableView(act, PStitle(pss));
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
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_PREF_BACK:		DestroyActivity(act); break;
	case TAG_ADD_BTN:
//		AdmUsrWrStart(ACT_ADM_ADD, user);
		break;
	}
}

extern unsigned char	reqCmd, reqOt, reqOp;
void acuReqWriteUser(FP_USER *user);
int  acuCnfWriteUser(void);

void AdmUsrOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;
	int		rval, result, index;

	wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_CONF_CNF:
		switch(reqCmd) {
		case P_WRITE_OBJECT:
			switch(reqOt) {
			case OT_USER:
				result = acuCnfWriteUser();
				if(!result) index = R_DATA_SAVED;
				else if(result == R_OBJECT_IS_FULL) index = R_USER_ARE_FULL;
				else if(result == 99) index = R_ACU_TIMEOUT;
				else	index = R_ACU_OTHER_ERROR;
				AlertResultMsg(xmenu_user[0], index);
				break;
			}
			break;
		}
	}
}

void AdmUsrRdStart(void *pss);
static PS_BUTTON	_psbutton;

void AdmUsrOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	void	*wnd, *tblv, *ps;
	char	temp[128];
	int		rval, no;

	wnd = activityWindow(act);
printf("AdmUsrOnActivityResult...request=%d result=%d\n", requestCode, resultCode);
	if(resultCode != RESULT_OK) return;
	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	if(requestCode == ACT_ADM_ADD) {
		if(devMode(NULL)) {
printf(".....userId=%d\n", user->id);
			user->cardStatus = 1;
			acuReqWriteUser(user);
printf(".....userId=%d\n", user->id);
		} else {
			user->cardStatus = 1;
			rval = userfsAdd(user);
			if(rval < 0) no = R_MSG_SYSTEM_ERROR;
			else if(!rval) no = R_USER_ARE_FULL;
			else	no = 0;
//for(rval = 114;rval <= 160;rval++) { user->id = rval; fsAddUser(user); }
			if(no) {
				AlertResultMsg(xmenu_user[0], no);
			} else {
				UIreloadData(tblv);
				ps = &_psbutton;
				PSbuttonInit(ps, "AAA", NULL);
				PSsetOnStart(ps, AdmWfRdStart);
				PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
				PSsetContext(ps, user);
				PrefStart(requestCode, ps);
			}
		}
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
	void	*cell, *v, *ps;
	int		rval;
	
	cell = UIcellForRowAtIndex(tblv, index);
	if(cell) {
		v = UItextLabel(cell);
		user->id = n_atol(UItext(v));
		rval = userfsGet(user);
		ps = &_psbutton;
		PSbuttonInit(ps, xmenu_top[0], NULL);
		PSsetOnStart(ps, AdmUsrRdStart);
		PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
		PSsetContext(ps, user);
		PrefStart(ACT_ADM_INPUT+index, ps);
	}
}

void AdmUsrWrStart(void *pss)
{
}

void _AdmUsrRdLoadData(void *pss)
{
	void	*ps;
	char	temp[64];
	int		i, id;

printf("AdmUsrRdLoadData...\n");
	user = PScontext(pss);
	ps = PSaddButton(pss, NULL, NULL);	// Edit Button
	PSsetViewStyle(ps, PS_VIEW_STYLE_EDIT);
	user2 = &_user2;
	PSsetContext(ps, &user2);
	PSsetOnStart(ps, AdmUsrWrStart);
	ps = PSaddButton(pss, NULL, NULL);	// Trash Button
	ps = PSaddTitle(pss, PStitle(pss), NULL, PS_TITLE_STYLE_DEFAULT);
	sprintf(temp, "%ld", user->id);
	ps = PSaddTitle(pss, xmenu_user_reg[0], temp, PS_TITLE_STYLE_SUBTITLE);
	GetUserAccessMode((int)user->accessMode, temp);
	ps = PSaddTitle(pss, xmenu_user_reg[1], temp, PS_TITLE_STYLE_SUBTITLE);
	GetUserAccessRights(user->accessRights, temp);
	ps = PSaddTitle(pss, xmenu_user_regex[0], temp, PS_TITLE_STYLE_SUBTITLE);
	GetUserDate(user->activateDate, temp);
	ps = PSaddTitle(pss, xmenu_user_regex[1], temp, PS_TITLE_STYLE_SUBTITLE);
	GetUserDate(user->expireDate, temp);
	ps = PSaddTitle(pss, xmenu_user_regex[2], temp, PS_TITLE_STYLE_SUBTITLE);
	ps = PSaddTitle(pss, xmenu_user_reg[2], " ", PS_TITLE_STYLE_SUBTITLE);
	ps = PSaddTitle(pss, xmenu_user_reg[3], " ", PS_TITLE_STYLE_SUBTITLE);
	GetUserCardData(user->cardData, temp);
	ps = PSaddTitle(pss, xmenu_user_reg[4], temp, PS_TITLE_STYLE_SUBTITLE);
	plainbcd2password(user->pin, 8, temp);
	ps = PSaddTitle(pss, xmenu_user_reg[5], temp, PS_TITLE_STYLE_SUBTITLE);
}

void _AdmUsrRdPrefChanged(void *pss, int index, void *act)
{
	FP_USER	*user;

printf("AdmUsrRdPrefChanged...%d\n", index);
	user = PScontext(pss);
	if(index == 0) {
		userCopy(user, user2);
		PSremoveChildren(pss);
		_AdmUsrRdLoadData(pss);
		PrefReloadData(act);
	}
}

void AdmUsrRdStart(void *pss)
{
printf("AdmUsrRdStart...\n");
	PSsetOnPrefChanged(pss, _AdmUsrRdPrefChanged);
	user = PScontext(pss);
	_AdmUsrRdLoadData(pss);
}

