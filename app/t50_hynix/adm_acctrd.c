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
#include "usrprim.h"
#include "actalert.h"
#include "appact.h"


void AdmAcctRdOnCreate(GACTIVITY *act);
void AdmAcctRdOnDestroy(GACTIVITY *act);
void AdmAcctRdOnClick(GACTIVITY *act, void *view);
void AdmAcctRdOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmAcctRdOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);

unsigned long	fpId;


void AdmAcctRdStart(int requestCode, void *acct)
{
	GACTIVITY	*act;

printf("AdmAcctRdStart...\n");
	act = CreateActivity(AdmAcctRdOnCreate);
	activitySetPsContext(act, acct);
	activitySetTimeout(act, 30);
	appStartActivityForResult(act, requestCode, NULL, 0);
}

void _AdmAcctRdRefresh(GACTIVITY *act, void *view)
{
	ACCT	*acct;
	void	*v;

printf("_AdmAcctRefresh...\n");
	acct = activityPsContext(act);
	v = UIviewWithTag(view, TAG_ADM_LABEL+0);
	UIsetText(v, acct->acctName);
	v = UIviewWithTag(view, TAG_ADM_LABEL+1);
	UIsetText(v, acct->url);
}

void AdmAcctRdOnCreate(GACTIVITY *act)
{
	ACCT	*acct;
	void	*wnd, *pv, *v; 
	CGRect	rt;
	char	*xmenu[10];
	int		i, y, h;
	
	activitySetOnDestroy(act, AdmAcctRdOnDestroy);
	activitySetOnClick(act, AdmAcctRdOnClick);
	activitySetOnAppMessage(act, AdmAcctRdOnAppMessage);
	activitySetOnActivityResult(act, AdmAcctRdOnActivityResult);
	pv = AdmActInitWithScrollView(act, "  ");
	wnd = activityWindow(act);
	h = 48;
	v = ViewAddButtonAlphaMask(wnd, TAG_DELETE_BTN, "trash.bmp", lcdWidth()-50, 0, h, h, 32);
	UIsetTintColor(v, whiteColor);
	v = ViewAddButtonAlphaMask(wnd, TAG_EDIT_BTN, "edit.bmp", lcdWidth()-100, 0, h, h, 32);
	UIsetTintColor(v, whiteColor);
	UIframe(pv, &rt);
	y = 0;
	v = ViewAddLabel(pv, 0, xmenu_phone[0], 0, y, rt.width, h, UITextAlignmentCenter);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_GREY_200);
	y += h + 10;
	xmenu[0] = xmenu_phone_reg[0];
	xmenu[1] = xmenu_phone_reg[1];
	for(i = 0;i < 2;i++) { 
		h = 16; AdmAddKeyLabel(pv, y, xmenu[i]); y += h + 4;
		h = 24; AdmAddValueLabel2(pv, TAG_ADM_LABEL+i, y); y += h + 10;
	}
	_AdmAcctRdRefresh(act, pv);
	acct = activityPsContext(act);
}

void AdmAcctRdOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

extern ACCT		_acct2;

void AdmAcctRdOnClick(GACTIVITY *act, void *view)
{
	ACCT	*acct;
	char	temp[128];
	int		tag;

	acct = activityPsContext(act);
	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_PREF_BACK:
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_DELETE_BTN:
		sprintf(temp, "%s=%s&%s", xmenu_phone[3], acct->url, GetPromptMsg(M_DELETE_CONFIRM));
		AlertDialog(ACT_ADM_DELETE, temp, TRUE); 
		break;
	case TAG_EDIT_BTN:
		acctCopy(&_acct2, acct);	
		AdmAcctWrStart(ACT_ADM_EDIT, &_acct2);
		break;
	}
}

void AdmAcctRdOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
}

void AdmAcctRdOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	ACCT	*acct;
	void	*wnd, *v;
	int		rval, no;

	wnd = activityWindow(act);
	acct = activityPsContext(act);
printf("AdmAcctRdOnActivityResult...request=%d result=%d\n", requestCode, resultCode);
	if(resultCode != RESULT_OK) return;
	switch(requestCode) {
	case ACT_ADM_EDIT:
		v = UIviewWithTag(wnd, TAG_PREF_CHILD_VIEW);
		_AdmAcctRdRefresh(act, v);
		break;
	case ACT_ADM_DELETE:
		rval = acctfsRemove(acct->id);
		DestroyActivityForResult(act, RESULT_OK, NULL);
		break;
	}
}

