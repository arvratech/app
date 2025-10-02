#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "prim.h"
#include "rtctime.h"
#include "lcdc.h"
#include "lang.h"
#include "jpeg_api.h"
#include "msg.h"
#include "timer.h"
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

void AdmUsrWrOnCreate(GACTIVITY *act);
void AdmUsrWrOnDestroy(GACTIVITY *act);
void AdmUsrWrOnClick(GACTIVITY *act, void *view);
void AdmUsrWrOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmUsrWrOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
int  AdmUsrWrOnNumberOfRowsInTableView(void *self, void *tblv);
void *AdmUsrWrOnCellForRowAtIndex(void *self, void *tblv, int index);
void AdmUsrWrOnDidSelectRowAtIndex(void *self, void *tblv, int index);

static FP_USER	*user;
static long		userId;


void AdmUsrWrStart(int requestCode, void *puser)
{
	GACTIVITY	*act;

	act = CreateActivity(AdmUsrWrOnCreate);
	user = (FP_USER *)puser;
	if(requestCode == ACT_ADM_ADD) {
		user->id = 0L;
		userSetDefault(user);
		user->accessMode = USER_CARD;
	}
	appStartActivityForResult(act, requestCode, NULL);
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
		UIsetTitleColor(v, 0x9e9e9e);
		UIsetUserInteractionEnabled(v, FALSE);
	}
	UIsetOnNumberOfRowsInTableView(tblv, AdmUsrWrOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, AdmUsrWrOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, AdmUsrWrOnDidSelectRowAtIndex);
	UIreloadData(tblv);
}

void AdmUsrWrOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

void AdmUsrWrOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_CANCEL_BTN:
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	case TAG_OK_BTN:
		DestroyActivityForResult(act, RESULT_OK, NULL);
		break;
	}
}

void _UsrWrCapturedCardResult(void *wnd, int msgIndex)
{
	void	*tblv, *cell;
	CREDENTIAL_READER	*cr;
	char	temp[64];
	
	cr = crsGet(0);
	if(msgIndex) {
		AlertResultMsg(xmenu_user_reg[4], msgIndex);
	} else {
		memcpy(user->cardData, cr->data, 9);
		tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
		cell = UIcellForRowAtIndex(tblv, 2);
		GetUserCardData(user->cardData, temp);
		CellSetSubtitle(cell, temp);
	}
}

void _UsrWrUserIdResult(void *wnd, int msgIndex)
{
	void	*tblv, *cell, *v;
	char	temp[32];
	BOOL	bVal;

	if(msgIndex) {
		AlertResultMsg(xmenu_user_reg[0], msgIndex);
	} else {
		tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
		cell = UIcellForRowAtIndex(tblv, 0);
		user->id = userId;
		sprintf(temp, "%d", user->id);
		CellSetSubtitle(cell, temp);
		if(user->id > 0) bVal = TRUE;
		else	bVal = FALSE; 
		v = UIviewWithTag(wnd, TAG_OK_BTN);
		UIsetUserInteractionEnabled(v, bVal);
		if(bVal) UIsetTitleColor(v, 0xfafafa);
		else	 UIsetTitleColor(v, 0x9e9e9e);
	}
}

extern unsigned char	reqCmd, reqOt, reqOp;
void acuReqFindUser(long nID);
int  acuCnfFindUser(void);
void acuReqFindUserCardData(char *data);
int  acuCnfFindUserCardData(void);

void AdmUsrWrCapturedCard(void *wnd)
{
	CREDENTIAL_READER	*cr;
	FP_USER		*user2, _user2;
	int		rval, no;

	cr = crsGet(0);
	user2 = &_user2;
	if(devMode(NULL)) {
		acuReqFindUserCardData(cr->data);
	} else {
		userfsSeek(0L);
		rval = userfsGetCardData(user2, cr->data);
		if(rval > 0 && user->id == user2->id) rval = userfsGetCardData(user2, cr->data);
		if(rval < 0) no = R_R_SYSTEM_ERROR;
		else if(rval > 0) no = R_CARD_DATA_OVERLAPPED;
		else	no = 0;
		_UsrWrCapturedCardResult(wnd, no);
	}
}

void AdmUsrWrOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;
	int		rval, result, index;

	wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_CRED_CAPTURED:
		AdmUsrWrCapturedCard(wnd);
		break;
	case GM_CONF_CNF:
		switch(reqCmd) {
		case P_FIND_OBJECT:
			switch(reqOt) {
			case OT_USER:
				result = acuCnfFindUser();
printf("result=%d\n", result);
				if(!result) index = R_USER_ID_OVERLAPPED;
				else if(result == R_OBJECT_NOT_FOUND) index = 0;
				else if(result == 99) index = R_ACU_TIMEOUT;
				else	index = R_ACU_OTHER_ERROR;
				_UsrWrUserIdResult(wnd, index);
				break;
			}
			break;	
		case P_FIND_PROPERTY:
			switch(reqOt) {
			case OT_USER:
				switch(reqOp) {
				case OP_SECOND_CARD_AUTHENTICATION_FACTOR:
					result = acuCnfFindUserCardData();
					if(!result) index = R_CARD_DATA_OVERLAPPED;
					else if(result == R_OBJECT_NOT_FOUND) index = 0;
					else if(result == 99) index = R_ACU_TIMEOUT;
					else	index = R_ACU_OTHER_ERROR;
					_UsrWrCapturedCardResult(wnd, index);
					break;
				}
				break;
			}
		}
	}
}

void AdmUsrWrOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	void	*wnd, *tblv, *cell, *btn;
	char	*p, pwd[12], temp[64], value[64];
	int		val, no;

	wnd = activityWindow(act);
	if(resultCode != RESULT_OK) return;
	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	cell = UIcellForRowAtIndex(tblv, requestCode - ACT_ADM_INPUT);
	switch(requestCode) {
	case ACT_ADM_INPUT+0:
		get_keyvalue(intent, temp, value);
		userId = n_atol(value);
		if(userId != user->id) {
			if(devMode(NULL)) {
printf("userId=%d\n", userId);
				acuReqFindUser(userId);
			} else {
				val = userfsExist(userId);
				if(val < 0) no = R_R_SYSTEM_ERROR;
				else if(val > 0) no = R_USER_ID_OVERLAPPED;
				else	no = 0;
				_UsrWrUserIdResult(wnd, no);
			}
		}
		break;
	case ACT_ADM_INPUT+1:
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
	case ACT_ADM_INPUT+5:
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
	return 6;
}

void *AdmUsrWrOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell, *lbl1, *lbl2;
	char	idstr[12], temp[128];
	UITableViewCellStyle	style;
	UITableViewCellAccessoryType	accessoryType;
	BOOL	bVal;
	int		val;

	style = UITableViewCellStyleSubtitle;
	accessoryType = UITableViewCellAccessoryTypeNone;
	cell = AdmInitTableViewCell(tblv, style, accessoryType, NULL);
	lbl1 = UItextLabel(cell);
	lbl2 = UIdetailTextLabel(cell);
	switch(index) {
	case 0:
		UIsetText(lbl1, xmenu_user_reg[0]);
		sprintf(idstr, "%ld", user->id);
		UIsetText(lbl2, idstr);
		if(activityRequestCode(self) != ACT_ADM_ADD) UIsetUserInteractionEnabled(cell, FALSE);
		break;
	case 1:
		UIsetText(lbl1, xmenu_user_reg[1]);
		GetUserAccessMode((int)user->accessMode, temp);
		UIsetText(lbl2, temp);
		break;
	case 2:
		UIsetText(lbl1, xmenu_user_reg[2]);
		//GetUserCardData(user->cardData, temp);
		//UIsetText(lbl2, temp);
		break;
	case 3:
		UIsetText(lbl1, xmenu_user_reg[3]);
		//GetUserCardData(user->cardData, temp);
		//UIsetText(lbl2, temp);
		break;
	case 4:
		UIsetText(lbl1, xmenu_user_reg[4]);
		GetUserCardData(user->cardData, temp);
		UIsetText(lbl2, temp);
		break;
	case 5:
		UIsetText(lbl1, xmenu_user_reg[5]);
		plainbcd2password(user->pin, 8, temp);
		UIsetText(lbl2, temp);
		break;
	}
	return cell;
}

void AdmUsrWrOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell;
	char	*p, pwd[12], temp[256];
	int		val;

	switch(index) {
	case 0:
		sprintf(temp, "ID=0\n%s=%ld\n%d,8\n", GetTableViewCellText(tblv, index), user->id, TI_NUMERIC);
		TextInputDialog(ACT_ADM_INPUT+index, temp, AdmDoneValidateUserID);
		break;
	case 1:
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
	case 5:
		bcd2string(user->pin, 8, pwd);
		sprintf(temp, "User=0\n%s=%s\n%d,8\n%s=%s\n%d,8\n", GetPromptMsg(M_ENTER_PASSWORD), pwd, TI_NUMERIC_PASSWORD, GetPromptMsg(M_VERIFY_PASSWORD), pwd, TI_NUMERIC_PASSWORD);
		TextInputDialog(ACT_ADM_INPUT+index, temp, AdmDoneValidatePIN);
		break;
	}
}
