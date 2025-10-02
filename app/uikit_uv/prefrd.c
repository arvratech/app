#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
#include "prefspec.h"
#include "viewprim.h"
#include "actprim.h"
#include "pref.h"

#define TAG_EDIT_BTN		101
#define TAG_DELETE_BTN		102

void PrefRdOnCreate(GACTIVITY *act);
void PrefRdOnDestroy(GACTIVITY *act);
void PrefRdOnTimer(GACTIVITY *act, int timerID);
void PrefRdOnClick(GACTIVITY *act, void *view);
void PrefRdOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);

static int	edited;


void _PrefRdRefresh(GACTIVITY *act, void *view);

void PrefRdOnCreate(GACTIVITY *act)
{
	void	*wnd, *v, *btn, *ps, *pss;
	CGRect	_rt;
	int		i, val, x, y, w, h, count;

	activitySetOnDestroy(act, PrefRdOnDestroy);
	activitySetOnTimer(act, PrefRdOnTimer);
	activitySetOnClick(act, PrefRdOnClick);
	activitySetOnActivityResult(act, PrefRdOnActivityResult);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, TABLE_CYAN_600);
	UIframe(wnd, &_rt);
	y = 0; h = 48;
	pss = activityPsContext(act);
	v = ViewAddTitleImageTextButton(wnd, TAG_PREF_BACK, 0, 0, "arrow_left.bmp", "   ");
	y += h; h = _rt.height - y;
	v = ViewAddScrollView(wnd, TAG_PREF_CHILD_VIEW, 0, y, _rt.width, h);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_GREY_50);
	pss = activityPsContext(act);
	count = PSchildrenCount(pss);
	for(i = val = 0;i < count;i++) {
		ps = PSobjectAtIndex(pss, i);
		if(PStype(ps) != PS_TYPE_BUTTON) break;
		val++;
	}
	if(val > 0) {
		y = 0; w = 48;
		if(val > 1) x = lcdWidth() - 116; else x = lcdWidth() - 58;
		btn = ViewAddButtonAlphaMask(wnd, TAG_EDIT_BTN, "edit.bmp", x, y, w, w, 32);
		UIsetTintColor(btn, whiteColor);
		x += 58;
		if(val > 1) {
			btn = ViewAddButtonAlphaMask(wnd, TAG_DELETE_BTN, "trash.bmp", x, y, w, w, 32);
			UIsetTintColor(btn, whiteColor);
		}
	}
	PSsetViewBuffer(pss, ViewAllocBuffer());
	edited = 0;
	_PrefRdRefresh(act, v);
	PScallOnCreate(pss);
}

void _PrefRdRefresh(GACTIVITY *act, void *view)
{
	CGRect	_rt;
	CGSize	_sz;
	void	*ps, *pss, *v;
	CGRect	_rect;
	char	*p, temp[64];
	int		i, j, x, y, w, h, first, count, style, column[4];

	UIremoveChildAll(view);
	pss = activityPsContext(act);
	ViewFreeBuffer(PSviewBuffer(pss));
	y = 0; first = 1;
	count = PSchildrenCount(pss);
	for(i = 0;i < count;i++) {
		ps = PSobjectAtIndex(pss, i);
		if(PStype(ps) == PS_TYPE_TITLE) {
			style = PStitleStyle(ps);
			UIframe(view, &_rect);
			if(first) {
				h = 48;
				v = ViewAddLabel(view, 0, PStitle(ps), 0, y, _rect.width, h, UITextAlignmentCenter);
				UIsetOpaque(v, TRUE);
				UIsetBackgroundColor(v, TABLE_GREY_200);
				y += h + 10;
				first = 0;
			} else if(style & PS_TITLE_STYLE_SUBTITLE) {
				h = 20;
				v = ViewAddLabel(view, 0, PStitle(ps), 16, y, _rect.width-32, h, UITextAlignmentLeft);
				UIsetOpaque(v, TRUE);
				if(style & PS_TITLE_STYLE_GROUP) UIsetTextColor(v, TABLE_CYAN_700);
				UIsetBackgroundColor(v, TABLE_GREY_50);
				UIsetFont(v, 16);
				y += h;
				if(style & PS_TITLE_STYLE_COLUMN) {
					p = PSsubtitle(ps);
					PStitleGetColumn(ps, column);
					for(j = 0;p && j < 4;j++) {
						p = read_token_ch(p, temp, ',');
						if(j < 3) w = column[j+1] - column[j];
						else	w = _rect.width - column[j];
						v = ViewAddLabel(view, 0, temp, column[j], y, w, 24, UITextAlignmentLeft);
						UIsetOpaque(v, TRUE);
						if(style & PS_TITLE_STYLE_GROUP) {
							UIsetTextColor(v, TABLE_CYAN_700);
							UIsetFont(v, 16);
						}
						UIsetBackgroundColor(v, TABLE_GREY_50);
					}
				} else {
					v = ViewAddLabel(view, 0, PSsubtitle(ps), 16, y, _rect.width-32, 24, UITextAlignmentLeft);
				}
				y += 34;
			} else {
				if(style & PS_TITLE_STYLE_COLUMN) {
					p = PStitle(ps);
					PStitleGetColumn(ps, column);
					for(j = 0;p && j < 4;j++) {
						p = read_token_ch(p, temp, ',');
						if(j < 3) w = column[j+1] - column[j];
						else	w = _rect.width - column[j];
						v = ViewAddLabel(view, 0, temp, column[j], y, w, 30, UITextAlignmentLeft);
						UIsetOpaque(v, TRUE);
						if(style & PS_TITLE_STYLE_GROUP) UIsetTextColor(v, TABLE_CYAN_700);
						UIsetBackgroundColor(v, TABLE_GREY_50);
					}
				} else {
					v = ViewAddLabel(view, 0, PStitle(ps), 16, y, _rect.width-32, 30, UITextAlignmentLeft);
					UIsetOpaque(v, TRUE);
					if(style & PS_TITLE_STYLE_GROUP) UIsetTextColor(v, TABLE_CYAN_700);
					UIsetBackgroundColor(v, TABLE_GREY_50);
				}
				y += 40;
			}
		}
	}
	UIframe(view, &_rt);
	_sz.width = _rt.width; _sz.height = y;
	UIsetContentSize(view, &_sz);
}

void PrefRdOnDestroy(GACTIVITY *act)
{
	void	*ps; 

	ps = activityPsContext(act);
	PScallOnCreate(ps);
	ViewFreeBuffer(activityViewBuffer(act));
	PSremoveChildren(ps);
}

void PrefRdOnTimer(GACTIVITY *act, int timerID)
{
	if(timerID == 0) {
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
	}
}

void PrefRdOnClick(GACTIVITY *act, void *view)
{
	void	*ps, *pss;
	char	temp[128], temp2[64];
	int		tag;

	pss = activityPsContext(act);
	tag = UItag(view);
	switch(tag) {
	case TAG_PREF_BACK:
		if(edited) DestroyActivityForResult(act, RESULT_PREF_EDITED, NULL);
		else	DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	case TAG_EDIT_BTN:
		ps = PSobjectAtIndex(pss, 0);
		if(PSuserAction(ps)) PScallUserAction(ps, ACT_PREF+0);
		else	PrefCreate(ACT_PREF+0, ps);	
		break;
	case TAG_DELETE_BTN:
		sprintf(temp, "%s=%s&%s", "Delete", PStitle(pss), GetPromptMsg(M_DELETE_CONFIRM));
		AlertDialog(ACT_PREF+1, temp, TRUE); 
		break;
	}
}

void PrefRdOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	void	*pss;
	int		index;

//printf("PrefRdOnActivityResult: requestCode=%d resultCode=%d\n", requestCode, resultCode);
	if(resultCode < RESULT_OK) return;
	pss = activityPsContext(act);
	PSsetResultCode(pss, resultCode);
	index = requestCode - ACT_PREF;
	switch(index) {
	case 0:
		edited = 1;
		PScallOnPrefChanged(pss, index, act);
		break;
	case 1:
		PScallOnPrefChanged(pss, index, act);
		DestroyActivityForResult(act, RESULT_PREF_DELETED, NULL);
		break;
	}
}

