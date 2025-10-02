#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "ctfont.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "dlg.h"


void FieldAlertDialogOnCreate(GACTIVITY *act);
void FieldAlertDialogOnDestroy(GACTIVITY *act);
void FieldAlertDialogOnClick(GACTIVITY *act, void *view);
static char	keys[8][64], vals[8][64];


void *FieldAlertDialog(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(FieldAlertDialogOnCreate);
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 0);
	return act;
}

void FieldAlertDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *lbl, *btn;
	char	*p, temp[64], title[64], keys[6][64], vals[6][64], bvals[3][64];
	CGRect		rect;
	int		i, x, y, w, h, fldCnt, btnCnt;

	activitySetOnDestroy(act, FieldAlertDialogOnDestroy);
	activitySetOnClick(act, FieldAlertDialogOnClick);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIframe(wnd, &rect);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, whiteColor); 
	p = act->intent;
	p = get_keyvalue(p, temp, title);
	for(i = 0;p && i < 6;i++) {
		p = get_keyvalue(p, keys[i], vals[i]);
		if(!strcmp(keys[i], "button")) break;
	}
	fldCnt = i;
	strcpy(bvals[0], vals[i]); btnCnt = 1;
	for( ;p && btnCnt < 3;btnCnt++) {
		p = get_keyvalue(p, temp, bvals[btnCnt]);
		if(strcmp(temp, "button")) break;
	}
	w = rect.width - 40;
	h = 48 + fldCnt * 64 + 48;
	rect.x = (rect.width - w) >> 1;
	rect.y = (rect.height - h) >> 1;
	rect.width = w; rect.height = h;
	UIsetFrame(wnd, &rect);
	x = 16; y = 0; w = rect.width - 32; h = 48;
	lbl = ViewAddLabel(wnd, 0, title, x, y, w, h, UITextAlignmentLeft);
	UIsetOpaque(lbl, TRUE);
	UIsetBackgroundColor(lbl, whiteColor);
	y += h;
	for(i = 0;i < fldCnt;i++) {
		y += 10; h = 16;
		lbl = ViewAddLabel(wnd, 0, keys[i], x, y, w, h, UITextAlignmentLeft);
		UIsetOpaque(lbl, TRUE);
		UIsetBackgroundColor(lbl, whiteColor);
		UIsetFont(lbl, 16);
		y += h + 4;
		h = 24;
		lbl = ViewAddLabel(wnd, 0, vals[i], x, y, w, h, UITextAlignmentLeft);
		UIsetOpaque(lbl, TRUE);
		UIsetBackgroundColor(lbl, whiteColor);
		y += h + 10;
	}
	y += 4; h = 40;
	x = rect.width - 10;
	ftSetSize(20);
	for(i = 0;i < btnCnt;i++) {
		w = ftTextWidth(bvals[i]) + 20;
		btn = ViewAddTextButton(wnd, TAG_DIALOG_BTN+i, bvals[i], x-w, y, w, h);
		UIsetTitleColor(btn, TABLE_CYAN_700);
		x -= w;
	}
}

void FieldAlertDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

void FieldAlertDialogOnClick(GACTIVITY *act, void *view)
{
	char	*p, temp[128];
	int		tag, no;

	tag = UItag(view);
	if(tag >= TAG_DIALOG_BTN && tag < TAG_DIALOG_BTN+3) {
		no = tag - TAG_DIALOG_BTN;
		DestroyActivityForResult(act, RESULT_FIRST_USER+no, NULL);
	}
}
