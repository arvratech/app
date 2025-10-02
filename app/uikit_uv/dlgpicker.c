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
#include "actalert.h"
#include "dlg.h"


void PickerDialogOnCreate(GACTIVITY *act);
void PickerDialogOnDestroy(GACTIVITY *act);
void PickerDialogOnTimer(GACTIVITY *act, int timerID);
void PickerDialogOnClick(GACTIVITY *act, void *view);
int  PickerDialogView_rowHeightForComponent(void *self, void *pickerView, int component);
int  PickerDialogView_widthForComponent(void *self, void *pickerView, int component);
char *PickerDialogView_titleForRow_forComponent(void *self, void *pickerView, int row, int component);
void *PickerDialogView_didSelectRow_inComponent(void *self, void *pickerView, int row, int component);
int  DlgNumberOfComponentsInPickerView(void *self, void *pickerView);
int  PickerDialogView_numberOfRowsInComponent(void *self, void *pickerView, int component);

#define COMP_BTN_HEIGHT		60
#define COMP_TITLE_HEIGHT	80	

#define MAX_COMP_SZ			4
#define MAX_COMP_ROW_SZ		32

static int	numberOfComponent;
static char	values[MAX_COMP_SZ][MAX_COMP_ROW_SZ][64];
static int  numberOfRow[MAX_COMP_SZ];


void *PickerDialog(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(PickerDialogOnCreate);
	activitySetTimeout(act, 20);
	appStartActivityForResult(act, requestCode, intent, 0);
	return act;
}

void PickerDialogOnCreate(GACTIVITY *act)
{
	void	*wnd, *pkv, *v;
	CGRect	rt, rt2;
	char	*p, title[64], value[64], temp[24];
	int		i, j, x, y, w, h, selectedRow[4];

	activitySetOnDestroy(act, PickerDialogOnDestroy);
	activitySetOnTimer(act, PickerDialogOnTimer);
	activitySetOnClick(act, PickerDialogOnClick);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, whiteColor); 
	UIframe(wnd, &rt);
	p = act->intent;
	p = get_keyvalue(p, title, temp);
	for(i = 0;p && i < MAX_COMP_SZ;i++) {
		p = get_keyvalue(p, value, temp);
		selectedRow[i] = n_atoi(temp);
//printf("[%d] [%s]=[%s]\n", i, value, temp);
		for(j = 0;p && j < MAX_COMP_ROW_SZ;j++) {
			p = read_token_ch(p, values[i][j], ',');
//printf("[%d,%d] [%s]\n", i, j, values[i][j]);
			if(p && (*(p-1)) == '\n') {  
				j++; break;
			}
		}
		numberOfRow[i] = j;
	}
	numberOfComponent = i;
	h = COMP_BTN_HEIGHT + COMP_TITLE_HEIGHT + COMP_BTN_HEIGHT;
	pkv = ViewAddPickerView(wnd, 15, 48, 0, h);
	UIsetTag(pkv, TAG_PICKERVIEW);
	UIsetPickerView_rowHeightForComponent(pkv, PickerDialogView_rowHeightForComponent);
	UIsetPickerView_widthForComponent(pkv, PickerDialogView_widthForComponent);
	UIsetPickerView_titleForRow_forComponent(pkv, PickerDialogView_titleForRow_forComponent);
	UIsetPickerView_didSelectRow_inComponent(pkv, PickerDialogView_didSelectRow_inComponent);
	UIsetNumberOfComponentsInPickerView(pkv, DlgNumberOfComponentsInPickerView);
	UIsetPickerView_numberOfRowsInComponent(pkv, PickerDialogView_numberOfRowsInComponent);
	for(i = w = 0;i < numberOfComponent;i++)
		w += PickerDialogView_widthForComponent(act, pkv, i);
	UIframe(pkv, &rt2);
	rt2.width = w;
	UIsetFrame(pkv, &rt2);
	w += 30; h += 48 + 48;
	rt.x = (rt.width - w) >> 1; rt.y = (rt.height - h) >> 1; rt.width = w; rt.height = h;
	UIsetFrame(wnd, &rt);
//printf("wnd: %d,%d %d %d\n", rect.x, rect.y, rect.width, rect.height);
	y = 0;
	v = ViewAddLabel(wnd, 0, title, 30, y, w-30, 48, UITextAlignmentLeft);
	UIsetTextColor(v, blackColor);
	y += h - 48;
	ViewAddCancelDoneButton(wnd, y);

	UIreloadAllComponents(pkv);
	for(i = 0;i < numberOfComponent;i++)
		UIselectRow_inComponent(pkv, selectedRow[i], i);
}

void PickerDialogOnDestroy(GACTIVITY *act)
{
	ViewFreeBuffer(activityViewBuffer(act));
}

void PickerDialogOnTimer(GACTIVITY *act, int timerID)
{
	void	*wnd;

	wnd = activityWindow(act);
//	if(timerID == 0) ViewClockUpdate(viewClock, TAG_CLOCK_BASE);
}

void PickerDialogOnClick(GACTIVITY *act, void *view)
{
	void	*wnd, *pkv;
	char	*p, temp[256];
	int		tag, i, num;

	wnd = activityWindow(act);
	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:		DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_CANCEL_BTN:	DestroyActivityForResult(act, RESULT_CANCELLED, NULL); break;
	case TAG_OK_BTN:
		p = temp;
		pkv = UIviewWithTag(wnd, TAG_PICKERVIEW);
		num = UInumberOfComponent(pkv);
		for(i = 0;i < num;i++) {
			sprintf(p, "comp=%d\n", UIselectedRowInComponent(pkv, i));
			p += strlen(p);
		}
		DestroyActivityForResult(act, RESULT_OK, temp);
		break;
	}
}

int PickerDialogView_rowHeightForComponent(void *self, void *pickerView, int component)
{
	return COMP_TITLE_HEIGHT;
}

int PickerDialogView_widthForComponent(void *self, void *pickerView, int component)
{
	int		i, w, width;

	ftSetSize(30);
	width = 0;
	for(i = 0;i < numberOfRow[component];i++) {
		w = ftTextWidth(values[component][i]);
		if(w > width) width = w;
	}
	width += 30;	// 15
	if(width < 60) width = 60;
//printf("comp=%d width=%d\n", component, width);
	return width;
}

char *PickerDialogView_titleForRow_forComponent(void *self, void *pickerView, int row, int component)
{
	return values[component][row]; 
}

void *PickerDialogView_didSelectRow_inComponent(void *self, void *pickerView, int row, int component)
{
}

int DlgNumberOfComponentsInPickerView(void *self, void *pickerView)
{
	return numberOfComponent;
}

int PickerDialogView_numberOfRowsInComponent(void *self, void *pickerView, int component)
{
	return numberOfRow[component];
}

