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
#include "pref.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "dlg.h"

#define COMP_WIDTH			90
#define COMP_BTN_HEIGHT		60
#define COMP_TITLE_HEIGHT	80	

void DatePickerDialogOnCreate(GACTIVITY *act);
void DatePickerDialogOnDestroy(GACTIVITY *act);
void DatePickerDialogOnClick(GACTIVITY *act, void *view);
void DatePickerDialogOnValueChanged(GACTIVITY *act, void *view);

static int	_year, _month, _day;


void *DatePickerDialog(int requestCode, int year, int month, int day)
{
	GACTIVITY	*act;

	act = CreateActivity(DatePickerDialogOnCreate);
	_year = year; _month = month; _day = day;
//printf("Date: %d %d %d\n", _year, _month, _day);
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, NULL, 0);
	return act;
}

void DatePickerDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *v, *lbl, *btn;
	CGRect		rt;
	char	*p, title[64], value[64], temp[24];
	int		i, x, y, w, h, wh;

	activitySetOnDestroy(act, DatePickerDialogOnDestroy);
	activitySetOnClick(act, DatePickerDialogOnClick);
	activitySetOnValueChanged(act, DatePickerDialogOnValueChanged);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, whiteColor); 
	UIframe(wnd, &rt);
	h = COMP_BTN_HEIGHT + COMP_TITLE_HEIGHT + COMP_BTN_HEIGHT;
	wh = 48 + h + 48;
	w = 15 + COMP_WIDTH + COMP_WIDTH + COMP_WIDTH + 15;
	rt.x = (rt.width-w)>>1; rt.width = w; rt.y = (rt.height - wh) >> 1; rt.height = wh;
	UIsetFrame(wnd, &rt);
	x = 30; y = 0; w = rt.width - 30;
	lbl = ViewAddLabel(wnd, TAG_PREF_TITLE, "", x, y, w, 48, UITextAlignmentLeft);
	UIsetTextColor(lbl, blackColor);
	y += 48;
	v = ViewAddDatePicker(wnd, 15, y);
	UIsetTag(v, TAG_PREF_CELL_CONTROL);
	UIsetYear(v, _year); UIsetMonth(v, _month); UIsetDay(v, _day);
	UIsetLocale(v, GetLanguage());
	y += h;
	ViewAddCancelDoneButton(wnd, y);
	DatePickerDialogOnValueChanged(act, v);
}

void DatePickerDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

void DatePickerDialogOnTimer(GACTIVITY *act, int timerID)
{
	void	*wnd;

	wnd = activityWindow(act);
//	if(timerID == 0) ViewClockUpdate(viewClock, TAG_CLOCK_BASE);
}

void DatePickerDialogOnClick(GACTIVITY *act, void *view)
{
	void	*wnd, *v; 
	char	temp[64];
	int		tag;

	wnd = activityWindow(act);
	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:		DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_CANCEL_BTN:	DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_OK_BTN:
		v = UIviewWithTag(wnd, TAG_PREF_CELL_CONTROL);
		//rtcGetDateTime(ctm);
		//ctm[0] = UIyear(v) - 2000; ctm[1] = UImonth(v); ctm[2] = UIday(v);
		//rtcSetDateTime(ctm);
		sprintf(temp, "date=%04d/%02d/%02d\n", UIyear(v), UImonth(v), UIday(v)); 
		DestroyActivityForResult(act, RESULT_OK, temp);
		break;
	}
}

void DatePickerDialogOnValueChanged(GACTIVITY *act, void *view)
{
	void	*wnd, *v;
	char	*p, temp[64];
	int		val, lang, year, mm, dd;

	year = UIyear(view); mm = UImonth(view); dd = UIday(view);
	lang = UIlocale(view);
	p = temp;
	if(lang == LANG_KOR || lang == LANG_JPN) {
		sprintf(p, "%d", year); p += strlen(p);
		strcpy(p, GetCJKYear()); p += strlen(p); *p++ = ' ';
		dtGetLongMonth(lang, mm, p); p += strlen(p); *p++ = ' ';
		sprintf(p, "%d", dd); p += strlen(p);
		strcpy(p, GetCJKDay()); p += strlen(p); *p++ = ' ';
		*p++ = '(';
		dtGetShortDayOfWeek(lang, get_week_day(year, mm, dd), p); p += strlen(p);
		*p++ = ')';
		*p = 0;
	} else {
		dtGetShortDayOfWeek(lang, get_week_day(year, mm, dd), p); p += strlen(p);
		*p++ = ','; *p++ = ' ';
		dtGetShortMonth(lang, mm, p); p += strlen(p); *p++ = ' ';
		sprintf(p, "%d", dd); p += strlen(p); *p++ = ','; *p++ = ' ';
		sprintf(p, "%d", year);
	}
//printf("ValueChanged[%s]\n", temp);
	wnd = activityWindow(act);
	v = UIviewWithTag(wnd, TAG_PREF_TITLE);
	UIsetText(v, temp);
}

