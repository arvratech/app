#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "fsuser.h"
#include "sysdep.h"
#include "subdev.h"
#include "gactivity.h"
#include "gapp.h"
#include "syscfg.h"
#include "pref.h"
#include "prefspec.h"
#include "viewprim.h"
#include "dlg.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "usrprim.h"
#include "actalert.h"
#include "appact.h"

void AdmAcctWrOnCreate(GACTIVITY *act);
void AdmAcctWrOnDestroy(GACTIVITY *act);
void AdmAcctWrOnClick(GACTIVITY *act, void *view);
void AdmAcctWrOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmAcctWrOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
int  AdmAcctWrOnNumberOfRowsInTableView(void *self, void *tblv);
void *AdmAcctWrOnCellForRowAtIndex(void *self, void *tblv, int index);
void AdmAcctWrOnDidSelectRowAtIndex(void *self, void *tblv, int index);


void AdmAcctWrStart(int requestCode, void *acct)
{
	GACTIVITY	*act;

	act = CreateActivity(AdmAcctWrOnCreate);
	activitySetPsContext(act, acct);
	activitySetTimeout(act, 30);
	appStartActivityForResult(act, requestCode, NULL, 0);
}

void AdmAcctWrOnCreate(GACTIVITY *act)
{
	void	*wnd, *tblv, *v; 

	activitySetOnDestroy(act, AdmAcctWrOnDestroy);
	activitySetOnClick(act, AdmAcctWrOnClick);
	activitySetOnAppMessage(act, AdmAcctWrOnAppMessage);
	activitySetOnActivityResult(act, AdmAcctWrOnActivityResult);
	tblv = AdmActInitWithTableButtonView(act, xmenu_okcancel[3]);
	wnd = activityWindow(act);
	if(activityRequestCode(act) == ACT_ADM_ADD) {
		v = UIviewWithTag(wnd, TAG_OK_BTN);
		UIsetTitleColor(v, 0xff9e9e9e);
		UIsetUserInteractionEnabled(v, FALSE);
	}
	UIsetOnNumberOfRowsInTableView(tblv, AdmAcctWrOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, AdmAcctWrOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, AdmAcctWrOnDidSelectRowAtIndex);
	UIreloadData(tblv);
}

void AdmAcctWrOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

void _AdmAcctWrWriteUserOnResult(GACTIVITY *act, int status, unsigned char *buf, int len)
{
	void	*a;
	int		val, result;

	if(status) AcuRequestDestroy(act);
	else {
		result = buf[0];
		if(!result) val = 0;
		else if(result == R_OBJECT_IS_FULL) val = R_USER_ARE_FULL;
		else	val = R_FAILED;
		if(val) AcuRequestSetResult(act, val);
		else {
			a = activityParent(act);
			AcuRequestDestroy(act);
			DestroyActivityForResult(a, RESULT_OK, NULL);
		}
	}
}

extern ACCT		_acct1;

void AdmAcctWrOnClick(GACTIVITY *act, void *view)
{
	ACCT	*acct;	
	int		tag, rval, no;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_CANCEL_BTN:
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	case TAG_OK_BTN:
		acct = activityPsContext(act);
		rval = 0;
		if(activityRequestCode(act) == ACT_ADM_EDIT && !acctCompare(acct, &_acct1)) {
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		} else if(activityRequestCode(act) == ACT_ADM_ADD || acctCompare(acct, &_acct1)) {
			rval = acctfsAdd(acct);
printf("acctfsAdd....%d\n", rval);
			sync();
			if(rval < 0) no = R_MSG_SYSTEM_ERROR;
			else if(!rval) no = R_USER_ARE_FULL;
			else	no = 0;
printf("no = %d\n", no);
			if(no) {
				AlertResultMsg(xmenu_phone[1], no);
				rval = -1;
			} else {
				rval = 0;
			}
		}
printf("rval = %d\n", rval);
		if(!rval) DestroyActivityForResult(act, RESULT_OK, NULL);
		break;
	}
}

void AdmAcctWrOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
}

void _AcctWrAcctResult(void *act, int msgIndex, char *url)
{
	void	*wnd, *tblv, *cell, *v;
	ACCT	*acct;
	char	temp[68];
	UIColor	color;
	BOOL	bVal;

	wnd = activityWindow(act);
	acct = activityPsContext(act);
	if(msgIndex) {
		AlertResultMsg(xmenu_phone_reg[1], msgIndex);
	} else {
		tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
		cell = UIcellForRowAtIndex(tblv, 2);
		strcpy(acct->url, url);
		CellSetSubtitle(cell, acct->url);
		if(url[0]) bVal = TRUE;
		else	bVal = FALSE;
		v = UIviewWithTag(wnd, TAG_OK_BTN);
		UIsetUserInteractionEnabled(v, bVal);
		if(bVal) color = 0xfffafafa;
		else	 color = 0xff9e9e9e;
		UIsetTitleColor(v, color);
	}
}

void AdmAcctWrOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	void	*wnd, *tblv, *cell;
	ACCT	*acct, *acct_t, _acct_t;
	char	temp[64], value[128];
	int		rval, index, no;

	wnd = activityWindow(act);
	if(resultCode != RESULT_OK) return;
	acct = activityPsContext(act);
	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	index = requestCode - ACT_ADM_INPUT;
	cell = UIcellForRowAtIndex(tblv, index);
printf("OnActivityResult: Index=%d\n", index);
	switch(index) {
	case 1:
		get_keyvalue(intent, temp, value);
		strcpy(acct->acctName, value);
		CellSetSubtitle(cell, value);
		break;	
	case 2:
		get_keyvalue(intent, temp, value);
printf("url: %s\n", value);
		if(strcmp(acct->url, value)) {
			acct_t = &_acct_t;
			acctfsSeek(0L);
			rval = acctfsGetWithUrl(acct_t, value);
			if(rval > 0 && acct_t->id == acct->id) rval = acctfsGetWithUrl(acct_t, value);
			if(rval < 0) no = R_R_SYSTEM_ERROR;
			else if(rval > 0) no = R_USER_ID_OVERLAPPED;
			else	no = 0;
			_AcctWrAcctResult(act, no, value);
		}
		break;
	}
}

int AdmAcctWrOnNumberOfRowsInTableView(void *self, void *tblv)
{
	return 3;
}

void *AdmAcctWrOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell, *lbl1, *lbl2;
	ACCT	*acct;
	char	temp[128];
	UITableViewCellStyle	style;
	UITableViewCellAccessoryType	accessoryType;
	BOOL	bVal;
	int		val;

	acct = activityPsContext(self);
	if(index == 0) style = UITableViewCellStyleDefault;
	else	style = UITableViewCellStyleSubtitle;
	accessoryType = UITableViewCellAccessoryTypeNone;
	cell = AdmInitTableViewCell(tblv, style, accessoryType, NULL);
	lbl1 = UItextLabel(cell);
	lbl2 = UIdetailTextLabel(cell);
	switch(index) {
	case 0:
		UIsetBackgroundColor(cell, TABLE_GREY_200);
		if(activityRequestCode(self) == ACT_ADM_ADD) val = 1; else val = 2;
		UIsetText(lbl1, xmenu_phone[val]);
printf("val=%d [%s] [%s]\n", val, xmenu_phone[val], UItext(lbl1));
		UIsetTextAlignment(lbl1, UITextAlignmentCenter);
		UIsetUserInteractionEnabled(cell, FALSE);
		break;
	case 1:
		UIsetText(lbl1, xmenu_phone_reg[0]);
		UIsetText(lbl2, acct->acctName);
		break;
	case 2:
		UIsetText(lbl1, xmenu_phone_reg[1]);
		UIsetText(lbl2, acct->url);
//		if(activityRequestCode(self) != ACT_ADM_ADD) UIsetUserInteractionEnabled(cell, FALSE);
		break;
	}
	return cell;
}

void AdmAcctWrOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	ACCT	*acct;
	char	temp[256];

	acct = activityPsContext(self);
	switch(index) {
	case 1:
		sprintf(temp, "ID=0\n%s=%s\n%d,32\n", GetTableViewCellText(tblv, index), acct->acctName, TI_ALPHANUMERIC);
		TextInputDialog(ACT_ADM_INPUT+index, temp, NULL);
		break;
	case 2:
		sprintf(temp, "ID=0\n%s=%s\n%d,64\n", GetTableViewCellText(tblv, index), acct->url, TI_ALPHANUMERIC);
		TextInputDialog(ACT_ADM_INPUT+index, temp, AdmDoneValidateAcctUrl);
		break;
	}
}

