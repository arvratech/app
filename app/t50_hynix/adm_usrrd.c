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
#include "usrprim.h"
#include "actalert.h"
#include "appact.h"


void AdmUsrRdOnCreate(GACTIVITY *act);
void AdmUsrRdOnDestroy(GACTIVITY *act);
void AdmUsrRdOnClick(GACTIVITY *act, void *view);
void AdmUsrRdOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmUsrRdOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);

unsigned long	fpId;


void AdmUsrRdStart(int requestCode, void *user)
{
	GACTIVITY	*act;

printf("AdmUsrRdStart...\n");
	act = CreateActivity(AdmUsrRdOnCreate);
	activitySetPsContext(act, user);
	activitySetTimeout(act, 30);
	appStartActivityForResult(act, requestCode, NULL, 0);
}

void _AdmUsrRdRefresh(GACTIVITY *act, void *view)
{
	FP_USER	*user;
	void	*v;
	char	temp[64];
	int		val;

printf("_AdmUsrRefresh...\n");
	user = activityPsContext(act);
	v = UIviewWithTag(view, TAG_ADM_LABEL+0);
	sprintf(temp, "%ld", user->id);
	UIsetText(v, temp);
	GetUserAccessMode((int)user->accessMode, temp);
	v = UIviewWithTag(view, TAG_ADM_LABEL+1);
	UIsetText(v, temp);
	GetUserAccessRights(user->accessRights, temp);
	v = UIviewWithTag(view, TAG_ADM_LABEL+2);
	UIsetText(v, temp);
	GetUserDate(user->activateDate, temp);
	v = UIviewWithTag(view, TAG_ADM_LABEL+3);
	UIsetText(v, temp);
	GetUserDate(user->expireDate, temp);
	v = UIviewWithTag(view, TAG_ADM_LABEL+4);
	UIsetText(v, temp);
	if(user->fpFlag & 0x01) val = 1; else val = 0;
	GetUserFpData(val, temp);
	v = UIviewWithTag(view, TAG_ADM_LABEL+5);
	UIsetText(v, temp);
	if(user->fpFlag & 0x02) val = 1; else val = 0;
	GetUserFpData(val, temp);
	v = UIviewWithTag(view, TAG_ADM_LABEL+6);
	UIsetText(v, temp);
	GetUserCardData(user->cardData, temp);
	v = UIviewWithTag(view, TAG_ADM_LABEL+7);
	UIsetText(v, temp);
	plainbcd2password(user->pin, 8, temp);
	v = UIviewWithTag(view, TAG_ADM_LABEL+8);
	UIsetText(v, temp);
}

void AdmUsrRdOnCreate(GACTIVITY *act)
{
	FP_USER	*user;
	void	*wnd, *pv, *v; 
	CGRect	rt;
	char	*xmenu[10];
	int		i, y, h;
	
	activitySetOnDestroy(act, AdmUsrRdOnDestroy);
	activitySetOnClick(act, AdmUsrRdOnClick);
	activitySetOnAppMessage(act, AdmUsrRdOnAppMessage);
	activitySetOnActivityResult(act, AdmUsrRdOnActivityResult);
	pv = AdmActInitWithScrollView(act, "  ");
	wnd = activityWindow(act);
	h = 48;
	v = ViewAddButtonAlphaMask(wnd, TAG_DELETE_BTN, "trash.bmp", lcdWidth()-50, 0, h, h, 32);
	UIsetTintColor(v, whiteColor);
	v = ViewAddButtonAlphaMask(wnd, TAG_EDIT_BTN, "edit.bmp", lcdWidth()-100, 0, h, h, 32);
	UIsetTintColor(v, whiteColor);
	UIframe(pv, &rt);
	y = 0;
	v = ViewAddLabel(pv, 0, xmenu_top[0], 0, y, rt.width, h, UITextAlignmentCenter);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_GREY_200);
	y += h + 10;
	xmenu[0] = xmenu_user_reg[0];
	xmenu[1] = xmenu_user_reg[1];
	xmenu[2] = xmenu_user_regex[0];
	xmenu[3] = xmenu_user_regex[1];
	xmenu[4] = xmenu_user_regex[2];
	xmenu[5] = xmenu_user_reg[2];
	xmenu[6] = xmenu_user_reg[3];
	xmenu[7] = xmenu_user_reg[4];
	xmenu[8] = xmenu_user_reg[5];
	for(i = 0;i < 9;i++) { 
		h = 16; AdmAddKeyLabel(pv, y, xmenu[i]); y += h + 4;
		h = 24; AdmAddValueLabel2(pv, TAG_ADM_LABEL+i, y); y += h + 10;
	}
	_AdmUsrRdRefresh(act, pv);
	user = activityPsContext(act);
	fpId = user->id << 1;
	fpmRequestFind(fpId);
//fpmRequestDeleteAllTemplate();
}

void AdmUsrRdOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

extern FP_USER	_user2;

void AdmUsrRdOnClick(GACTIVITY *act, void *view)
{
	FP_USER	*user;
	char	temp[128];
	int		tag;

	user = activityPsContext(act);
	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_PREF_BACK:
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_DELETE_BTN:
		sprintf(temp, "%s=%ld&%s", xmenu_user[2], user->id, GetPromptMsg(M_DELETE_CONFIRM));
		AlertDialog(ACT_ADM_DELETE, temp, TRUE); 
		break;
	case TAG_EDIT_BTN:
		userCopy(&_user2, user);	
		AdmUsrWrStart(ACT_ADM_EDIT, &_user2);
		break;
	}
}

void _UsrRdUserFindFp(void *wnd, unsigned long fpId, int result)
{
	void	*v, *pv;
	char	temp[64];
	int		index, val;

	pv = UIviewWithTag(wnd, TAG_PREF_CHILD_VIEW);
	if(fpId & 1) index  = 6; else index = 5;
	if(result) val = 0; else val = 1;
	GetUserFpData(val, temp);
	v = UIviewWithTag(pv, TAG_ADM_LABEL+index);
	UIsetText(v, temp);
}

extern unsigned char	reqCmd, reqOt, reqOp;
void acuReqFindUser(long nID);
int  acuCnfFindUser(void);
void acuReqFindUserCardData(char *data);
int  acuCnfFindUserCardData(void);

void AdmUsrRdOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	FP_USER	*user;
	void	*wnd;
	int		rval, cmd, result, index;

	wnd = activityWindow(act);
	user = activityPsContext(act);
	switch(msg[0]) {
	case GM_FPM_RESPONSE:
		cmd = fpmCommand();
		switch(cmd) {
		case 0x60:
			rval = fpmResponseFind();
printf("%u-%u: fp=0x%x\n", fpId>>1, fpId&1, rval);
			_UsrRdUserFindFp(wnd, fpId, rval);
			if(!(fpId & 1)) {
				fpId = (user->id << 1) + 1;
				fpmRequestFind(fpId);
			}
			break;
		case 0x72:
			rval = fpmResponseDeleteTemplate();
			if(rval) AlertResultMsg(xmenu_user[0], R_FAILED);
			else if(!(fpId & 1) && (user->fpFlag & 0x02)) fpmRequestDeleteTemplate(fpId+1);
			else	DestroyActivityForResult(act, RESULT_OK, NULL);
			break;
		case 0x70:
			rval = fpmResponseTemplateCount();
			printf("Total FP count=%d\n", rval);
			break;
		case 0x76:
			rval = fpmResponseDeleteAllTemplate();
			printf("DeleteAllTemplate=%d\n", rval);
			break;
		}
		break;
	}
}

void AdmUsrRdDeleteOnResult(GACTIVITY *act, int status, unsigned char *buf, int len)
{
	void	*ps, *pss, *a;
	int		index;

	ps = activityPsContext(act);
	if(status) {
		AcuRequestDestroy(act);
	} else {
		if(buf[0]) AcuRequestSetResult(act, R_FAILED);
		else {
			a = activityParent(act);
			AcuRequestDestroy(act);
			DestroyActivityForResult(a, RESULT_OK, NULL);
		}
	}
}

void AdmUsrRdOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	FP_USER	*user;
	void	*wnd, *v;
	int		rval, no;

	wnd = activityWindow(act);
	user = activityPsContext(act);
printf("AdmUsrRdOnActivityResult...request=%d result=%d\n", requestCode, resultCode);
	if(resultCode != RESULT_OK) return;
	switch(requestCode) {
	case ACT_ADM_EDIT:
		v = UIviewWithTag(wnd, TAG_PREF_CHILD_VIEW);
		_AdmUsrRdRefresh(act, v);
		break;
	case ACT_ADM_DELETE:
		if(devMode(NULL)) {
			AcuRequestStart(NULL, user, AdmUsrRdDeleteOnResult);
			acuReqDeleteUser(user->id);
		} else {
			rval = userfsRemove(user->id);
			if(user->fpFlag) {
				if(user->fpFlag & 0x01) fpId = user->id << 1;
				else	fpId = (user->id << 1) + 1;
				fpmRequestDeleteTemplate(fpId);
			} else {
				DestroyActivityForResult(act, RESULT_OK, NULL);
			}
		}
		break;
	}
}

