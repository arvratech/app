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
#include "admtitle.h"
#include "usrprim.h"
#include "actalert.h"
#include "appact.h"

void AdmUsrWrOnCreate(GACTIVITY *act);
void AdmUsrWrOnDestroy(GACTIVITY *act);
void AdmUsrWrOnClick(GACTIVITY *act, void *view);
void AdmUsrWrOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmUsrWrOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
int  AdmUsrWrOnNumberOfRowsInTableView(void *self, void *tblv);
void *AdmUsrWrOnCellForRowAtIndex(void *self, void *tblv, int index);
void AdmUsrWrOnDidSelectRowAtIndex(void *self, void *tblv, int index);

static long		_userId;
static unsigned long  fpId;
static unsigned char  fpFlag;
static unsigned char  fpTmpls[2][800];


void AdmUsrWrStart(int requestCode, void *user)
{
	GACTIVITY	*act;

	act = CreateActivity(AdmUsrWrOnCreate);
	activitySetPsContext(act, user);
	activitySetTimeout(act, 30);
	appStartActivityForResult(act, requestCode, NULL, 0);
}

void AdmUsrWrOnCreate(GACTIVITY *act)
{
	void	*wnd, *tblv, *v; 

	activitySetOnDestroy(act, AdmUsrWrOnDestroy);
	activitySetOnClick(act, AdmUsrWrOnClick);
	activitySetOnAppMessage(act, AdmUsrWrOnAppMessage);
	activitySetOnActivityResult(act, AdmUsrWrOnActivityResult);
	tblv = AdmActInitWithTableButtonView(act, xmenu_okcancel[3]);
	wnd = activityWindow(act);
	if(activityRequestCode(act) == ACT_ADM_ADD) {
		v = UIviewWithTag(wnd, TAG_OK_BTN);
		UIsetTitleColor(v, 0xff9e9e9e);
		UIsetUserInteractionEnabled(v, FALSE);
	}
	UIsetOnNumberOfRowsInTableView(tblv, AdmUsrWrOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, AdmUsrWrOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, AdmUsrWrOnDidSelectRowAtIndex);
	UIreloadData(tblv);
	fpFlag = 0;
}

void AdmUsrWrOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

void _AdmUsrWrWriteUserOnResult(GACTIVITY *act, int status, unsigned char *buf, int len)
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

extern FP_USER	_user1;

void AdmUsrWrOnClick(GACTIVITY *act, void *view)
{
	FP_USER	*user;	
	int		tag, rval, no;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_CANCEL_BTN:
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	case TAG_OK_BTN:
		user = activityPsContext(act);
		rval = 0;
		if(activityRequestCode(act) == ACT_ADM_EDIT && !userCompare(user, &_user1) && !fpFlag) {
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		} else if(activityRequestCode(act) == ACT_ADM_ADD || userCompare(user, &_user1)) {
			if(devMode(NULL)) {
				user->cardStatus = 1;
				AcuRequestStart(NULL, NULL, _AdmUsrWrWriteUserOnResult);
				acuReqWriteUser(user);
				rval = 1;
			} else {
				user->cardStatus = 1;
				rval = userfsAdd(user);
for(no = 0;no < 9;no++) printf("-%02x", user->cardData[no]); printf("\n");
printf("userfsAdd()=%d\n", rval);
				if(rval < 0) no = R_MSG_SYSTEM_ERROR;
				else if(!rval) no = R_USER_ARE_FULL;
				else	no = 0;
//for(rval = 114;rval <= 160;rval++) { user->id = rval; fsAddUser(user); }
				if(no) {
					AlertResultMsg(xmenu_user[0], no);
					rval = -1;
				} else {
					rval = 0;
				}
			}
		}
		if(!rval && fpFlag) {
			if(fpFlag & 0x01) fpId = user->id << 1;
			else	fpId = (user->id << 1) + 1;
			fpmRequestAddTemplate(fpId, fpTmpls[fpId & 1]);
			rval = 1;
		}
		if(!rval) DestroyActivityForResult(act, RESULT_OK, NULL);
		break;
	}
}

void acuReqFindUser(long nID);

void AdmUsrWrOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;

	wnd = activityWindow(act);
}

void _AdmUsrWrFindUserOnResult(GACTIVITY *act, int status, unsigned char *buf, int len)
{
	void	*wnd, *tblv, *cell, *v, *a;
	FP_USER	*user;
	char	temp[80];
	UIColor	color;
	BOOL	bVal;
	int		val, result;

	if(status) AcuRequestDestroy(act);
	else {
		result = buf[0];
		if(!result) val = R_USER_ID_OVERLAPPED;
		else if(result == R_OBJECT_NOT_FOUND) val = 0;
		else	val = R_FAILED;
		if(val) AcuRequestSetResult(act, val);
		else {
			a = activityParent(act);
			AcuRequestDestroy(act);
			user = activityPsContext(a);
			wnd = activityWindow(a);
			tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
			cell = UIcellForRowAtIndex(tblv, 1);
			user->id = _userId;
			sprintf(temp, "%lu", user->id);
printf("##### %s #####\n", temp);
			CellSetSubtitle(cell, temp);
			if(user->id > 0) bVal = TRUE;
			else	bVal = FALSE; 
			v = UIviewWithTag(wnd, TAG_OK_BTN);
			UIsetUserInteractionEnabled(v, bVal);
			if(bVal) color = 0xfffafafa;
			else	 color = 0xff9e9e9e;
			UIsetTitleColor(v, color);
		}
	}
}

void _UsrWrUserIdResult(void *act, int msgIndex)
{
	void	*wnd, *tblv, *cell, *v;
	FP_USER	*user;
	char	temp[32];
	UIColor	color;
	BOOL	bVal;

	wnd = activityWindow(act);
	user = activityPsContext(act);
	if(msgIndex) {
		AlertResultMsg(xmenu_user_reg[0], msgIndex);
	} else {
		tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
		cell = UIcellForRowAtIndex(tblv, 1);
		user->id = _userId;
		sprintf(temp, "%d", user->id);
		CellSetSubtitle(cell, temp);
		if(user->id > 0) bVal = TRUE;
		else	bVal = FALSE; 
		v = UIviewWithTag(wnd, TAG_OK_BTN);
		UIsetUserInteractionEnabled(v, bVal);
		if(bVal) color = 0xfffafafa;
		else	 color = 0xff9e9e9e;
		UIsetTitleColor(v, color);
	}
}

extern unsigned char  fpCapTmpl[];

void AdmUsrWrOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	void	*wnd, *tblv, *cell, *btn;
	FP_USER	*user;
	CREDENTIAL_READER	*cr;
	char	*p, pwd[12], temp[64], value[64];
	int		val, index, no;

	cr = crsGet(0);
	wnd = activityWindow(act);
	if(resultCode != RESULT_OK) return;
	user = activityPsContext(act);
	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	index = requestCode - ACT_ADM_INPUT;
	cell = UIcellForRowAtIndex(tblv, index);
printf("OnActivityResult: Index=%d\n", index);
	switch(index) {
	case 1:
		get_keyvalue(intent, temp, value);
		_userId = n_atol(value);
		if(_userId != user->id) {
			AcuRequestStart(NULL, NULL, _AdmUsrWrFindUserOnResult);
			acuReqFindUser(_userId);
		}
		break;
	case 2:
		get_keyvalue(intent, temp, value);
		p = value;
		p = read_token_ch(p, temp, ',');
		val = n_atoi(temp);
		if(val) user->accessMode |= USER_CARD; else user->accessMode &= ~USER_CARD;
		p = read_token_ch(p, temp, ',');
		val = n_atoi(temp);
		if(val) user->accessMode |= USER_PIN; else user->accessMode &= ~USER_PIN;
		p = read_token_ch(p, temp, ',');
		val = n_atoi(temp);
		if(val) user->accessMode |= USER_CARD_PIN; else user->accessMode &= ~USER_CARD_PIN;
		GetUserAccessMode((int)user->accessMode, temp);
		CellSetSubtitle(cell, temp);
		break;
	case 3:
		memcpy(fpTmpls[0], fpCapTmpl, 800);
		fpFlag |= 0x01;
		if((user->fpFlag & 1) || (fpFlag & 1)) val = 1; else val = 0;
		GetUserFpData(val, temp);
		CellSetSubtitle(cell, temp);
		break;	
	case 4:
		memcpy(fpTmpls[1], fpCapTmpl, 800);
		fpFlag |= 0x02;
		if((user->fpFlag & 2) || (fpFlag & 2)) val = 1; else val = 0;
		GetUserFpData(val, temp);
		CellSetSubtitle(cell, temp);
		break;	
	case 5:
		if(memcmp(user->cardData, cr->data, 9)) {
			memcpy(user->cardData, cr->data, 9);
			GetUserCardData(user->cardData, temp);
			CellSetSubtitle(cell, temp);
		}
		break;
	case 6:
		get_keyvalue(intent, temp, value);
		bcd2string(user->pin, 8, pwd);
		if(strcmp(value, pwd)) {
			string2bcd(value, 8, user->pin);
			plainstr2password(value, temp);
			CellSetSubtitle(cell, temp);
		}
		break;
	}
}

int AdmUsrWrOnNumberOfRowsInTableView(void *self, void *tblv)
{
	return 7;
}

void *AdmUsrWrOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell, *lbl1, *lbl2;
	FP_USER	*user;
	char	temp[128];
	UITableViewCellStyle	style;
	UITableViewCellAccessoryType	accessoryType;
	BOOL	bVal;
	int		val;

	user = activityPsContext(self);
	if(index == 0) style = UITableViewCellStyleDefault;
	else	style = UITableViewCellStyleSubtitle;
	accessoryType = UITableViewCellAccessoryTypeNone;
	cell = AdmInitTableViewCell(tblv, style, accessoryType, NULL);
	lbl1 = UItextLabel(cell);
	lbl2 = UIdetailTextLabel(cell);
	switch(index) {
	case 0:
		UIsetBackgroundColor(cell, TABLE_GREY_200);
		if(activityRequestCode(self) == ACT_ADM_ADD) val = 0; else val = 1;
		UIsetText(lbl1, xmenu_user[val]);
		UIsetTextAlignment(lbl1, UITextAlignmentCenter);
		UIsetUserInteractionEnabled(cell, FALSE);
		break;
	case 1:
		UIsetText(lbl1, xmenu_user_reg[0]);
		if(user->id > 0) sprintf(temp, "%lu", user->id);
		else	temp[0] = 0;
		UIsetText(lbl2, temp);
		if(activityRequestCode(self) != ACT_ADM_ADD) UIsetUserInteractionEnabled(cell, FALSE);
		break;
	case 2:
		UIsetText(lbl1, xmenu_user_reg[1]);
		GetUserAccessMode((int)user->accessMode, temp);
		UIsetText(lbl2, temp);
		break;
	case 3:
		UIsetText(lbl1, xmenu_user_reg[2]);
		if((user->fpFlag & 1) || (fpFlag & 1)) val = 1; else val = 0;
		GetUserFpData(val, temp);
		UIsetText(lbl2, temp);
		break;
	case 4:
		UIsetText(lbl1, xmenu_user_reg[3]);
		if((user->fpFlag & 2) || (fpFlag & 2)) val = 1; else val = 0;
		GetUserFpData(val, temp);
		UIsetText(lbl2, temp);
		break;
	case 5:
		UIsetText(lbl1, xmenu_user_reg[4]);
		GetUserCardData(user->cardData, temp);
		UIsetText(lbl2, temp);
		break;
	case 6:
		UIsetText(lbl1, xmenu_user_reg[5]);
		plainbcd2password(user->pin, 8, temp);
		UIsetText(lbl2, temp);
		break;
	}
	return cell;
}

void AdmUsrWrOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	CREDENTIAL_READER	*cr;
	FP_USER	*user;
	void	*cell;
	char	*p, pwd[12], temp[256];
	int		val;

	user = activityPsContext(self);
	switch(index) {
	case 1:
		if(user->id > 0) sprintf(temp, "ID=0\n%s=%ld\n%d,8\n", GetTableViewCellText(tblv, index), user->id, TI_NUMERIC);
		else	sprintf(temp, "ID=0\n%s=\n%d,8\n", GetTableViewCellText(tblv, index), TI_NUMERIC);
		TextInputDialog(ACT_ADM_INPUT+index, temp, AdmDoneValidateUserID);
		break;
	case 2:
		p = temp;
		sprintf(p, "%s=0\n", GetTableViewCellText(tblv, index)); p += strlen(p);
		sprintf(p, "%s,%s,%s\n", xmenu_access_mode[1], xmenu_access_mode[2], xmenu_access_mode[4]); p += strlen(p);
		if(user->accessMode & USER_CARD) *p++ = '1'; else *p++ = '0';
		*p++ = ',';
		if(user->accessMode & USER_PIN) *p++ = '1'; else *p++ = '0';
		*p++ = ',';
		if(user->accessMode & USER_CARD_PIN) *p++ = '1'; else *p++ = '0';
		*p++ = '\n';
		CheckDialog(ACT_ADM_INPUT+index, temp);
		break;
	case 3:
	case 4:
		cr = crsGet(0);
		cr->userId = user->id;
		cr->data[0] = 0;
		if(index == 3) val = 0x01; else val = 0x02;
		if((user->fpFlag & val) || (fpFlag & val)) cr->fpFlag = 1;
		else	cr->fpFlag = 0;
		cr->captureMode = USER_FP;
		//AdmUsrCredStart(ACT_ADM_INPUT+index, index-3);
		AdmUsrCredStart(ACT_ADM_INPUT+index, NULL);
		break;
	case 5:
		cr = crsGet(0);
		cr->userId = user->id;
		memcpy(cr->data, user->cardData, 9);
		cr->captureMode = USER_CARD;
		AdmUsrCredStart(ACT_ADM_INPUT+index, NULL);
		break;
	case 6:
		bcd2string(user->pin, 8, pwd);
		sprintf(temp, "User=0\n%s=%s\n%d,8\n%s=%s\n%d,8\n", GetPromptMsg(M_ENTER_PASSWORD), pwd, TI_NUMERIC_PASSWORD, GetPromptMsg(M_VERIFY_PASSWORD), pwd, TI_NUMERIC_PASSWORD);
		TextInputDialog(ACT_ADM_INPUT+index, temp, AdmDoneValidatePIN);
		break;
	}
}

