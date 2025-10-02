#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "dirent.h"
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "sche.h"
#include "msg.h"
#include "hw.h"
#include "crcred.h"
#include "cf.h"
#include "sysdep.h"
#include "syscfg.h"
#include "tacfg.h"
#include "wpaprim.h"
#include "viewprim.h"
#include "actprim.h"
#include "pref.h"
#include "topprim.h"
#include "admprim.h"

extern int		sleepTimer;


void EnterSleepMode(void)
{
printf("SLEEP..................\n");
	lcdOffBacklight();
}

void WakeupSleepMode()
{
printf("WAKEUP..................\n");
	lcdOnBacklight();
	sleepTimer = 0;
}

void *AdmActInitWithTableView(GACTIVITY *act, char *title)
{
	void	*wnd, *tblv, *v;
	CGRect	rt;
	int		y;

	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, TABLE_CYAN_600); 
	y = 0;
	v = ViewAddTitleImageTextButton(wnd, TAG_PREF_BACK, 0, 0, "arrow_left.bmp", title);
	UIframe(v, &rt);
	y += rt.height;
	UIframe(wnd, &rt);
	tblv = ViewAddTableView(wnd, 0, y, rt.width, rt.height-y);
	UIsetTag(tblv, TAG_PREF_TABLE);
	UIsetOpaque(tblv, TRUE);
	UIsetBackgroundColor(tblv, TABLE_GREY_50);
	UIsetDelegate(tblv, act);
	UIsetDataSource(tblv, act);
	return tblv;
}

void *AdmActInitWithTableButtonView(GACTIVITY *act, char *btnTitle)
{
	void	*wnd, *tblv, *v;
	CGRect	rt;
	int		y;

	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, TABLE_CYAN_600); 
	y = 0;
	v = ViewAddTitleCancelOkButton(wnd, btnTitle);
	UIframe(v, &rt);
	y += rt.height;
	UIframe(wnd, &rt);
	tblv = ViewAddTableView(wnd, 0, y, rt.width, rt.height-y);
	UIsetTag(tblv, TAG_PREF_TABLE);
	UIsetOpaque(tblv, TRUE);
	UIsetBackgroundColor(tblv, TABLE_GREY_50);
	UIsetDelegate(tblv, act);
	UIsetDataSource(tblv, act);
	return tblv;
}

void *AdmActInitWithScrollView(GACTIVITY *act, char *title)
{
	void	*wnd, *v;
	CGRect	rt;
	int		y;

	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, TABLE_CYAN_600);
	y = 0;
	v = ViewAddTitleImageTextButton(wnd, TAG_PREF_BACK, 0, 0, "arrow_left.bmp", title);
	UIframe(v, &rt);
	y += rt.height;
	UIframe(wnd, &rt);
	v = ViewAddScrollView(wnd, TAG_PREF_CHILD_VIEW, 0, y, rt.width, rt.height-y);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_GREY_50);
	return v;
}

void AdmActExit(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

void *AdmInitTableViewCell(void *tblv, UITableViewCellStyle style, UITableViewCellAccessoryType accessoryType, char *title)
{
	void	*cell, *lbl;
	CGRect	rt;

	cell = UIdequeueReusableCellWithIdentifier(tblv, 1);
	if(cell) {
		initWithStyle(cell, style, 1);
	} else {
		cell = ViewAddTableViewCell(style, 1);
	}
	rt.x = rt.y = rt.width = rt.height = 0;
	UIsetFrame(cell, &rt);
	UIsetOpaque(cell, TRUE);
	UIsetBackgroundColor(cell, TABLE_GREY_50);
	UIsetAccessoryType(cell, accessoryType);
	lbl = UItextLabel(cell);
	UIsetBackgroundColor(lbl, TABLE_GREY_50);
	UIsetTextColor(lbl, blackColor); 
	UIsetText(lbl, title);
	if(style == UITableViewCellStyleSubtitle) {
		lbl = UIdetailTextLabel(cell);
		UIsetBackgroundColor(lbl, TABLE_GREY_50);
		UIsetTextColor(lbl, TABLE_GREY_600);
	}
	return cell;
}

void *TableCreateSubtitleCell(void *view, char *title, char *subtitle)
{
	void	*cell, *v;

	cell = AdmInitTableViewCell(view, UITableViewCellStyleSubtitle, UITableViewCellAccessoryTypeNone, title);
	v = UIdetailTextLabel(cell);
	UIsetText(v, subtitle);
	return cell;
}

void *TableCreateLabelCell(void *view, char *title)
{
	void	*cell, *v;

	cell = AdmInitTableViewCell(view, UITableViewCellStyleDefault, UITableViewCellAccessoryTypeNone, title);
	v = UItextLabel(cell);
	UIsetTextColor(v, TABLE_CYAN_700);
	UIsetUserInteractionEnabled(cell, FALSE);
	return cell;
}

void *TableCreateTopLabelCell(void *view, char *title)
{
	void	*cell, *v;

	cell = AdmInitTableViewCell(view, UITableViewCellStyleDefault, UITableViewCellAccessoryTypeNone, title);
	UIsetBackgroundColor(cell, TABLE_GREY_200);
	v = UItextLabel(cell);
	UIsetBackgroundColor(v, TABLE_GREY_200);
	UIsetTextAlignment(v, UITextAlignmentCenter);
	UIsetUserInteractionEnabled(cell, FALSE);
	return cell;
}

char *GetTableViewCellText(void *tblv, int index)
{
	void	*cell, *lbl;

	cell = UIcellForRowAtIndex(tblv, index);
	lbl = UItextLabel(cell);
	return UItext(lbl);
}

int AdmAddTitleLabel(void *view, int y, char *text)
{
	void	*v;
	CGRect	_rect;

	UIframe(view, &_rect);
	v = ViewAddLabel(view, 0, text, 16, y, _rect.width-32, 30, UITextAlignmentLeft);
	UIsetOpaque(v, TRUE);
	UIsetTextColor(v, TABLE_CYAN_700);
	UIsetBackgroundColor(v, TABLE_GREY_50);
	return 40;
}

int AdmAddTopTitleLabel(void *view, int y, char *text)
{
	void	*v;
	CGRect	_rect;

	UIframe(view, &_rect);
	v = ViewAddLabel(view, 0, text, 0, y, _rect.width, 40, UITextAlignmentCenter);
	UIsetOpaque(v, TRUE);
//	UIsetTextColor(v, TABLE_CYAN_700);
	UIsetBackgroundColor(v, TABLE_GREY_200);
	return 50;
}

int AdmAddKeyValueLabel(void *view, int y, char *keyText, char *valueText)
{
	void	*v;
	CGRect	_rect;
	int		py;

	UIframe(view, &_rect);
	py = y; 
	v = ViewAddLabel(view, 0, keyText, 16, py, _rect.width-32, 16, UITextAlignmentLeft);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_GREY_50);
	UIsetFont(v, 16);
	py += 20;
	v = ViewAddLabel(view, 0, valueText, 16, py, _rect.width-32, 24, UITextAlignmentLeft);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_GREY_50);
	py += 34;
	return py - y;
}

int AdmAddKeyLabel(void *view, int y, char *text)
{ 
	void	*v;
	CGRect	_rect;

	UIframe(view, &_rect);
	v = ViewAddLabel(view, 0, text, 16, y, _rect.width-32, 20, UITextAlignmentLeft);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_GREY_50);
	UIsetFont(v, 16);
	return 20;
}

int AdmAddValueLabel(void *view, int y, char *text)
{
	void	*v;
	CGRect	_rect;

	UIframe(view, &_rect);
	v = ViewAddLabel(view, 0, text, 16, y, _rect.width-32, 24, UITextAlignmentLeft);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_GREY_50);
	return 34;
}

int AdmAddValueLabel2(void *view, int tag, int y)
{
	void	*v;
	CGRect	_rect;

	UIframe(view, &_rect);
	v = ViewAddLabel(view, tag, NULL, 16, y, _rect.width-32, 24, UITextAlignmentLeft);
//	UIsetOpaque(v, TRUE);
//	UIsetBackgroundColor(v, TABLE_GREY_50);
	return 34;
}

void AdmSetContentSize(void *view, int contentHeight)
{
	CGRect	_rect;
	CGPoint	_pt;
	CGSize	_size;

	UIframe(view, &_rect);
	_size.width = _rect.width; _size.height = contentHeight;
	UIsetContentSize(view, &_size);
	_pt.x = _pt.y = 0;
	UIsetContentOffset(view, &_pt);
}

BOOL AdmDoneValidateInt8(char *intent, char *result)
{
	char	*p, key[64], value[80];
	unsigned char	data[32];
	int		rval;
	BOOL	bRval;

	get_keyvalue(intent, key, value);
	rval = n_atoi(value);
	if(rval < 255) bRval = TRUE;
	else {
		p = GetResultMsg(R_INVALID_DATA);
		strcpy_swap(value, p, '\n', '&');
		sprintf(result, "%s=%s\n", key, value);
		bRval = FALSE;
	}
	return bRval;
}

BOOL AdmNetDoneValidateDeviceID(char *intent, char *result)
{
	char	*p, key[64], value[80];
	BOOL	bRval;
	int		val;

	p = intent;
	p = get_keyvalue(p, key, value);
	val = atoi(value);
	if(val > 0 && val < 1000) {
		bRval = TRUE;
	} else {
		p = GetResultMsg(R_INVALID_TID);
		strcpy_swap(value, p, '\n', '&');
		sprintf(result, "%s=%s\n", key, value);
		bRval = FALSE;
	}
	return bRval;
}

BOOL AdmDoneValidateIpAddress(char *intent, char *result)
{
	char	*p, key[64], value[80];
	BOOL	bRval;

	get_keyvalue(intent, key, value);
	if(validateIpAddress(value)) bRval = TRUE;
	else {
		p = GetResultMsg(R_INVALID_IP_ADDRESS);
		strcpy_swap(value, p, '\n', '&');
		sprintf(result, "%s=%s\n", key, value);
		bRval = FALSE;
	}
	return bRval;
}

BOOL AdmNetDoneValidateServer(char *intent, char *result)
{
	char	*p, key[64], value[80];
	BOOL	bRval;
	int		val;

	p = intent;
	p = get_keyvalue(p, key, value);
	if(validateIpAddress(value)) {
		get_keyvalue(p, key, value);
		val = n_atoi(value);
		if(val < 100 || val > 65535) {
			p = GetResultMsg(R_INVALID_SERVER_PORT);
			strcpy_swap(value, p, '\n', '&');
			sprintf(result, "%s=%s\n", key, value);
			bRval = FALSE;
		} else	bRval = TRUE;
	} else {
		p = GetResultMsg(R_INVALID_IP_ADDRESS);
		strcpy_swap(value, p, '\n', '&');
		sprintf(result, "%s=%s\n", key, value);
		bRval = FALSE;
	}
	return bRval;
}

BOOL AdmDoneValidatePIN(char *intent, char *result)
{
	char	*p, titles[2][64], values[2][64];
	BOOL	bRval;

	p = intent;
	p = get_keyvalue(p, titles[0], values[0]);
	p = get_keyvalue(p, titles[1], values[1]);
//printf("ValidatePIN [%s]=[%s] [%s]=[%s]\n", titles[0], values[0], titles[1], values[1]);
	if(strcmp(values[0], values[1])) {
		p = GetResultMsg(R_DATA_MISMATCHED);
		strcpy_swap(values[0], p, '\n', '&');
		sprintf(result, "%s=%s\n", titles[0], values[0]);
		bRval = FALSE;
	} else	bRval = TRUE;
	return bRval;
}

BOOL AdmDoneValidateUserID(char *intent, char *result)
{
	char	*p, key[64], value[80];
	BOOL	bRval;
	long	userId;
	int		rval, no;

	get_keyvalue(intent, key, value);
	userId = n_atol(value);
	if(userValidateId(userId)) no = 0;
	else	no = R_INVALID_USER_ID;
	if(no) {
		p = GetResultMsg(no);
		strcpy_swap(value, p, '\n', '&');
		sprintf(result, "%s=%s\n", key, value);
		bRval = FALSE;
	} else	bRval = TRUE;
	return bRval;
}

BOOL AdmDoneValidateNewUserID(char *intent, char *result)
{
	char	*p, key[64], value[80];
	BOOL	bRval;
	long	userId;
	int		rval, no;

	get_keyvalue(intent, key, value);
	userId = n_atol(value);
	if(userValidateId(userId)) {
		rval = userfsExist(userId);
		if(rval < 0) no = R_MSG_SYSTEM_ERROR;
		else if(rval) no = R_USER_ID_OVERLAPPED;
		else	no = 0;
	} else	no = R_INVALID_USER_ID;
	if(no) {
		p = GetResultMsg(no);
		strcpy_swap(value, p, '\n', '&');
		sprintf(result, "%s=%s\n", key, value);
		bRval = FALSE;
	} else	bRval = TRUE;
	return bRval;
}

BOOL AdmDoneValidateMifareKey(char *intent, char *result)
{
	char	*p, key[64], value[80];
	unsigned char	data[32];
	int		rval, no;
	BOOL	bRval;

	get_keyvalue(intent, key, value);
	if(strlen(value) & 1) rval = -1;
	else	rval = hexstr2bin(value, data);
	if(rval == 6) no = 0;
	else	no = R_INVALID_DATA;
	if(no) {
		p = GetResultMsg(no);
		strcpy_swap(value, p, '\n', '&');
		sprintf(result, "%s=%s\n", key, value);
		bRval = FALSE;
	} else	bRval = TRUE;
	return bRval;
}

BOOL AdmDoneValidateAid(char *intent, char *result)
{
	char	*p, key[64], value[80];
	unsigned char	data[32];
	int		rval, no;
	BOOL	bRval;

	get_keyvalue(intent, key, value);
	if(strlen(value) & 1) rval = -1;
	else	rval = hexstr2bin(value, data);
	if(rval == 0 || rval >= 6 && rval <= 10) no = 0;
	else	no = R_INVALID_DATA;
	if(no) {
		p = GetResultMsg(no);
		strcpy_swap(value, p, '\n', '&');
		sprintf(result, "%s=%s\n", key, value);
		bRval = FALSE;
	} else	bRval = TRUE;
	return bRval;
}

BOOL AdmDoneValidateMacFilter(char *intent, char *result)
{
	char	*p, key[64], value[80];
	unsigned char	data[32];
	int		rval, no;
	BOOL	bRval;

	get_keyvalue(intent, key, value);
	no = strlen(value);
	if(no <= 0) rval = 0;
	else if(strlen(value) != 6) rval = -1;
	else	rval = hexstr2bin(value, data);
	if(rval == 0 || rval == 3) no = 0;
	else	no = R_INVALID_DATA;
	if(no) {
		p = GetResultMsg(no);
		strcpy_swap(value, p, '\n', '&');
		sprintf(result, "%s=%s\n", key, value);
		bRval = FALSE;
	} else	bRval = TRUE;
	return bRval;
}

BOOL AdmDoneValidateAcctUrl(char *intent, char *result)
{
	char	*p, key[64], value[80];
	BOOL	bRval;
	long	acctId;
	int		rval, no;

	get_keyvalue(intent, key, value);
	if(value[0]) no = 0;
	else	no = R_INVALID_USER_ID;
	if(no) {
		p = GetResultMsg(no);
		strcpy_swap(value, p, '\n', '&');
		sprintf(result, "%s=%s\n", key, value);
		bRval = FALSE;
	} else	bRval = TRUE;
	return bRval;
}

void parseKeyValuePairs(char *buf, void (*Callback)(char *key, char *value))
{
	char	*p, key[64], value[64];

	p = buf;
	while(p && (*p)) {
		p = get_keyvalue(p, key, value);
		(*Callback)(key, value);
	}
}

int  convert12hour(int hour);

void GetTimeString(char *buf, int hh, int mm)
{
	char	*p;
	int		lang;

	lang = GetLanguage();
	p = buf;
	if(sys_cfg->dateNotation & TIME_NOTATION) { 
		sprintf(p, "%02d:%02d", hh, mm);
	} else if(lang == LANG_ENG) {
		sprintf(p, "%d:%02d", convert12hour(hh), mm); p += strlen(p); *p++ = ' ';
		dtGetNoon(lang, hh, p);
	} else {
		dtGetNoon(lang, hh, p); p += strlen(p); *p++ = ' ';
		sprintf(p, "%d:%02d", convert12hour(hh), mm);
	}
}

void AlertResultMsg(char *title, int msgIndex)
{
	char	*p, temp[128], msg[64];

	p = GetResultMsg(msgIndex);
	strcpy_swap(msg, p, '\n', '&');
	sprintf(temp, "%s=%s\n", title, msg);
	AlertDialog(0, temp, FALSE);
}

void *ViewAddFuncLabel(void *wnd)
{
	void	*v, *child;
	BOOL	bVal;
	int		i, x, y, w, h;

	x = 0; y = 200; w = lcdWidth(); h = 64;
	child = ViewAddChild(wnd, 0, x, y, w, h);
	v = ViewAddLabel(child, TAG_FUNC_LBL, "", 0, 0, 8, h, UITextAlignmentCenter);
	UIsetFont(v, 48);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_GREY_600);
	return child;
}

void ViewRefreshFuncLabel(void *view, int funcKey)
{
	void	*v;
	CGRect	rect;
	char	*p;
	int		w;
	
	v = UIviewWithTag(view, TAG_FUNC_LBL);
	if(funcKey > 1) {
		p = taGetTextAt(taGetItemTextIndex(funcKey-1));
		if(p) {
			UIsetHidden(view, FALSE);
			UIframe(v, &rect);
			ftSetSize(48);
			w = ftTextWidth(p);
			w += 16;
			rect.x = (lcdWidth() - w) >> 1; rect.width = w;
			UIsetFrame(v, &rect);
			UIsetText(v, p);
		} else {
			UIsetHidden(view, TRUE);
		}
	} else {
		UIsetHidden(view, TRUE);
	}
}

void ViewAddFuncButtons(void *wnd, void *funcBtns[])
{
	void	*v;
	int		i, x, y, w, h;

	x = 8; y = lcdHeight() - 90; w = 70; h = 40;
	for(i = 0;i < 4;i++) {
		v = funcBtns[i+1] = ViewAddTextButton(wnd, TAG_FUNC_BTN+i+1, NULL, x, y, w, h);
		UIsetOpaque(v, TRUE);
		UIsetBackgroundColor(v, TABLE_GREY_50);
		x += w + 8;
	}
	x = (lcdWidth() >> 1) - 20; y = lcdHeight() - 40; h = 40;
	funcBtns[0] = ViewAddButtonAlphaMask(wnd, TAG_FUNC_BTN, "cancel.bmp", x, y, h, h, h);
}

void ViewRefreshFuncButtons(void *funcBtns[])
{
	void	*v;
	char	*pa[4];
	BOOL	bVal;
	int		i;

	for(i = 0;i < 4;i++) {
		v = funcBtns[i+1];
		if(taItemIsEnable(i+1)) pa[i] = taGetTextAt(taGetItemTextIndex(i+1));
		else	pa[i] = NULL;
		UIsetTitle(v, pa[i]);
		if(pa[i]) bVal = FALSE; else bVal = TRUE;
		UIsetHidden(v, bVal);
	}
	if(taItemIsEnable(0) && (pa[0] || pa[1] || pa[2] || pa[3])) bVal = FALSE; else bVal = TRUE;
	UIsetHidden(funcBtns[0], bVal);
}

