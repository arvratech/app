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


void PrefOnCreate(GACTIVITY *act);
void PrefOnStart(GACTIVITY *act);
void PrefOnDestroy(GACTIVITY *act);
void PrefOnTimer(GACTIVITY *act, int timerID);
void PrefOnClick(GACTIVITY *act, void *view);
void PrefOnValueChanged(GACTIVITY *act, void *view);
void PrefOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
int  PrefOnNumberOfRowsInTableView(void *self, void *tblv);
void *PrefOnCellForRowAtIndex(void *self, void *tblv, int index);
void PrefOnDidSelectRowAtIndex(void *self, void *tblv, int index);

void PrefRdOnCreate(GACTIVITY *act);


void PrefCreate(int requestCode, void *ps)
{
	GACTIVITY	*act;
	void (*onCreate)(struct _GACTIVITY *self);

	if(PSuserAction(ps)) PScallUserAction(ps, requestCode);
	else {
		PScallIntAction(ps);
		if(PSviewStyle(ps) == PS_VIEW_STYLE_READONLY) onCreate = PrefRdOnCreate;
		else	onCreate = PrefOnCreate;
		act = CreateActivity(onCreate);
		PSsetActivity(ps, act);
		activitySetPsContext(act, ps);
		activitySetTimeout(act, 30);
		appStartActivityForResult(act, requestCode, NULL, 0);
	}
}

void PrefRecreate(int requestCode, void *ps)
{
	GACTIVITY	*act;
	void (*onCreate)(struct _GACTIVITY *self);

	if(PSuserAction(ps)) PScallUserAction(ps, requestCode);
	else {
		PScallIntAction(ps);
		if(PSviewStyle(ps) == PS_VIEW_STYLE_READONLY) onCreate = PrefRdOnCreate;
		else	onCreate = PrefOnCreate;
		act = CreateActivity(onCreate);
		PSsetActivity(ps, act);
		activitySetPsContext(act, ps);
		activitySetTimeout(act, 30);
		appStartActivityForResult(act, requestCode, NULL, 1);
	}
}

void PrefButtonCreate(int requestCode, void *ps)
{
	GACTIVITY	*act;
	void (*onCreate)(struct _GACTIVITY *self);

	if(PSbuttonUserAction(ps)) PScallButtonUserAction(ps, requestCode);
	else {
		PScallButtonIntAction(ps);
		if(PSviewStyle(ps) == PS_VIEW_STYLE_READONLY) onCreate = PrefRdOnCreate;
		else	onCreate = PrefOnCreate;
		act = CreateActivity(onCreate);
		PSsetActivity(ps, act);
		activitySetPsContext(act, ps);
		activitySetTimeout(act, 30);
		appStartActivityForResult(act, requestCode, NULL, 0);
	}
}

void _PrefRdRefresh(GACTIVITY *act, void *view);

void PrefReloadData(void *act)
{
	void	*wnd, *v, *pss;

	wnd = activityWindow((GACTIVITY *)act);
	pss = activityPsContext(act);
	if(PSviewStyle(pss) == PS_VIEW_STYLE_READONLY) {
		v = UIviewWithTag(wnd, TAG_PREF_CHILD_VIEW);
		_PrefRdRefresh(act, v);
	} else {
		v = UIviewWithTag(wnd, TAG_PREF_TABLE);
		UIremoveChildAll(v);
		UIdequeueReusableCellAll(v);
		ViewFreeBuffer(PSviewBuffer(pss));
		UIreloadData(v);
	}
}

void PrefReload(void *act)
{
	void	*wnd, *tblv, *cell, *v, *ps, *pss, *cells[16];
	UITableViewCellAccessoryType	accessoryType;
	int		i, index, type, count;

	wnd = activityWindow((GACTIVITY *)act);
	pss = activityPsContext(act);
	if(PSviewStyle(pss) == PS_VIEW_STYLE_READONLY) {
		v = UIviewWithTag(wnd, TAG_PREF_CHILD_VIEW);
		_PrefRdRefresh(act, v);
	} else {
		tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
		count = UIvisibleCells(tblv, cells);
		if(count) {
			index = UIindexForCell(tblv, cells[0]);
			for(i = 0;i < count;i++, index++) {
				cell = cells[i];
				ps = PSobjectAtIndex(pss, index);
				CellSetTitleSubtitle(cell, PStitle(ps), PSsubtitle(ps));
				UIsetUserInteractionEnabled(cell, PSisInteractionEnabled(ps));
				type = PStype(ps);
				if(type == PS_TYPE_SWITCH) {
					v = UIaccessoryView(cell);
					UIsetOn(v, PSswitchValue(ps));
					UIsetEnabled(v, PSisAccessoryEnabled(ps));
				} else if(type == PS_TYPE_SCREEN) {
					if(PSisInteractionEnabled(ps)) accessoryType = UITableViewCellAccessoryTypeDisclosureIndicator;
					else	accessoryType = UITableViewCellAccessoryTypeNone;
					UIsetAccessoryType(cell, accessoryType);

				}
			}
		}
	}
}

void _StepperValueText(void *ps, char *buf)
{
	int		val, ratio;
 
	val = PSstepperCallGetValueText(ps, buf);
	if(!val) {
		val = PSstepperValue(ps);
		ratio = PSstepperUnitRatio(ps);
		if(ratio > -2 && ratio < 2) sprintf(buf, "%d %s", val, PSstepperUnitText(ps));
		else if(ratio > 0) sprintf(buf, "%d %s", val*ratio, PSstepperUnitText(ps));
		else {
			ratio = -ratio;
			sprintf(buf, "%d.%d %s", val/ratio, val%ratio, PSstepperUnitText(ps));
		}
	}
}

void PrefReloadRowAtIndex(void *act, int index)
{
	void	*wnd, *tblv, *cell, *v, *ps;
	char	*p, temp[128];
	UITableViewCellAccessoryType	accessoryType;
	int		val, type;

	wnd = activityWindow((GACTIVITY *)act);
	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	cell = UIcellForRowAtIndex(tblv, index);
//printf("cell=%x %d/%d\n", cell, index, UIchildrenCount(tblv));
	if(cell) {
		ps = PSobjectAtIndex(activityPsContext(act), index);
		type = PStype(ps);
		switch(type) {
		case PS_TYPE_SCREEN:
			if(PSisInteractionEnabled(ps)) accessoryType = UITableViewCellAccessoryTypeDisclosureIndicator;
			else	accessoryType = UITableViewCellAccessoryTypeNone;
			UIsetAccessoryType(cell, accessoryType);
			CellSetTitle(cell, PStitle(ps)); CellSetSubtitle(cell, PSsubtitle(ps));
			break;
		case PS_TYPE_TITLE:
		case PS_TYPE_BUTTON:
			//CellSetTitle(cell, PStitle(ps));
			CellSetTitle(cell, PStitle(ps)); CellSetSubtitle(cell, PSsubtitle(ps));
			break;
		case PS_TYPE_CHECK:
			if(PScheckValue(ps)) accessoryType = UITableViewCellAccessoryTypeCheckmark;
			else	accessoryType = UITableViewCellAccessoryTypeUncheckmark;
			UIsetAccessoryType(cell, accessoryType);
			CellSetTitle(cell, PStitle(ps)); CellSetSubtitle(cell, PSsubtitle(ps));
			break;
		case PS_TYPE_SWITCH:
			CellSetSubtitle(cell, PSsubtitle(ps));
			v = UIaccessoryView(cell);
//printf("Switch ReloadRow: val=%d [%s]\n", PSswitchValue(ps), PSsubtitle(ps));
			UIsetOn(v, PSswitchValue(ps));
			UIsetEnabled(v, PSisAccessoryEnabled(ps));
			break;
		case PS_TYPE_STEPPER:
			accessoryType = UITableViewCellAccessoryTypeNone;
			_StepperValueText(ps, temp);
			PSsetSubtitle(ps, temp); CellSetSubtitle(cell, temp);
			//CellSetSubtitle(cell, PSsubtitle(ps));
            v = UIaccessoryView(cell);		// inserted 2024.1.8
			if(!PSisAccessoryEnabled(ps)) UIsetEnabled(v, FALSE);
			break;
		case PS_TYPE_SLIDER:
			sprintf(temp, "%d", PSsliderValue(ps));
			PSsetSubtitle(ps, temp); CellSetSubtitle(cell, temp);
			break;
		case PS_TYPE_TEXT_FIELD:
			if(PStextFieldIsPassword(ps)) temp[0] = 0;
			else {
				PStextFieldAllValue(ps, temp);
				if(!temp[0]) { temp[0] = ' '; temp[1] = 0; }
			}
			PSsetSubtitle(ps, temp); CellSetSubtitle(cell, temp);
			break;
		case PS_TYPE_SPINNER:
			val = PSspinnerIndexWithValue(ps, PSspinnerValue(ps));
			if(val < 0) p = NULL;
			else	p = PSspinnerTitleAtIndex(ps, val);
			if(p) strcpy(temp, p);
			else {
				temp[0] = ' '; temp[1] = 0;
			}
			PSsetSubtitle(ps, temp); CellSetSubtitle(cell, temp);
			break;
		case PS_TYPE_PICKER:
			PSpickerValuesTitle(ps, temp);
			PSsetSubtitle(ps, temp); CellSetSubtitle(cell, temp);
			break;
		case PS_TYPE_TIME_RANGE:
			PStimeRangeAllValueTitle(ps, temp);
			PSsetSubtitle(ps, temp); CellSetSubtitle(cell, temp);
			break;
		}
		UIsetUserInteractionEnabled(cell, PSisInteractionEnabled(ps));
	}
}

void PrefReloadRowAtIndex2(void *act, int index)
{
	void	*wnd, *tblv;

	wnd = activityWindow((GACTIVITY *)act);
	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	UIreloadRowAtIndex(tblv, index);
}

int ggg;

void *PrefCreateTable(GACTIVITY *act)
{
	void	*wnd, *v, *pss;
	CGRect	rect, rt;
	int		x, y, w, h;

	wnd = activityWindow(act);
	pss = activityPsContext(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, TABLE_CYAN_600); 
	UIframe(wnd, &rect);
//printf("wnd: %d,%d %d %d\n", rect.x, rect.y, rect.width, rect.height);
	if(PSviewStyle(pss) == PS_VIEW_STYLE_EDIT) {
		x = y = 0; w = rect.width>>1; h = 48;
		v = ViewAddTextButton(wnd, TAG_PREF_CANCEL_BTN, xmenu_okcancel[1], x, y, w, h);
		UIsetTitleColor(v, enableWhiteColor);
		x += w;
		v = ViewAddTextButton(wnd, TAG_PREF_OK_BTN, xmenu_okcancel[3], x, y, w, h);
		UIsetTitleColor(v, enableWhiteColor);
	} else {
		y = 0;
		v = ViewAddTitleImageTextButton(wnd, TAG_PREF_BACK, 0, 0, "arrow_left.bmp", PStitle(pss));
	}
	UIframe(v, &rt); h = rt.height;
	y += h; h = rect.height - y;
	v = ViewAddTableView(wnd, 0, y, rect.width, h);
	UIsetTag(v, TAG_PREF_TABLE);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, TABLE_GREY_50);
	UIsetDelegate(v, act);
	UIsetDataSource(v, act);
	return v;
}

void PrefOnCreate(GACTIVITY *act)
{
	void	*tblv, *pss, *ps;
	CGRect	rect;
	int		type,  index, count;

	activitySetOnStart(act, PrefOnStart);
	activitySetOnDestroy(act, PrefOnDestroy);
	activitySetOnTimer(act, PrefOnTimer);
	activitySetOnClick(act, PrefOnClick);
	activitySetOnValueChanged(act, PrefOnValueChanged);
	activitySetOnActivityResult(act, PrefOnActivityResult);
//printf("AllocBuffer: %x\n", activityViewBuffer(act));
	activitySetViewBuffer(act, ViewAllocBuffer());
	pss = activityPsContext(act);
	tblv = PrefCreateTable(act);
//	UIsetAllowsMultipleSelection(tblv, PSallowsMultipleSelection(pss));
	UIframe(tblv, &rect);
	UIsetOnNumberOfRowsInTableView(tblv, PrefOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, PrefOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, PrefOnDidSelectRowAtIndex);
	PSsetViewBuffer(pss, ViewAllocBuffer());
/*
	count = 0;
	ps = PSfirstChild(pss);
	while(ps) {
		type = PStype(ps);
		if(type == PS_TYPE_CHECK) {
			count = PScheckInteractionCount(ps);
			if(!PScheckInteractionReverse(ps) && PScheckValue(ps)
					|| PScheckInteractionReverse(ps) && !PScheckValue(ps)) count = 0;
		} else if(count > 0) {
			PSsetInteractionEnabled(ps, FALSE);
			count--;
		}
		ps = PSnextChild(ps);
	}
*/
	UIreloadData(tblv);
	PScallOnCreate(pss);
}

void PrefOnStart(GACTIVITY *act)
{
	void	*ps; 

	ps = activityPsContext(act);
	PScallOnStart(ps);
}

void PrefOnDestroy(GACTIVITY *act)
{
	void	*ps; 

	ps = activityPsContext(act);
	PScallOnDestroy(ps);
	ViewFreeBuffer(activityViewBuffer(act));
	PSremoveChildren(ps);
}

void PrefOnTimer(GACTIVITY *act, int timerID)
{
	if(timerID == 0) {
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
	}
}

void PrefOnClick(GACTIVITY *act, void *view)
{
	void	*ps;
	int		tag, index, resultCode;

	tag = UItag(view);
	switch(tag) {
	//case TAG_BACK_BTN:
	case TAG_PREF_BACK:
	case TAG_PREF_CANCEL_BTN:
		resultCode = PSbackResultCode(activityPsContext(act));
		DestroyActivityForResult(act, resultCode, NULL);
		break;
	case TAG_PREF_OK_BTN:
		DestroyActivityForResult(act, RESULT_OK, NULL);
		break;
	case TAG_PREF_USER_BTN:
		PScallOnUserButtonClick(activityPsContext(act));
		break;
	default:
		index = UItag(view) - TAG_PREF_CELL_BTN;
		ps = PSobjectAtIndex(activityPsContext(act), index);
		PrefButtonCreate(ACT_PREF+index, ps);	
	}
}

/*
void _PrefDisabled(void *tblv)
{
	void	*cell, *v;

	cell = UIfirstChild(tblv);
	while(cell) {
		UIsetInteractionEnabled(cell, FALSE);
		v = UIfirstChild(cell);
		while(cell) {
			UIsetEnabled(cell, FALSE);
			v = UInextChild(v);
		}
		cell = UInextChild(cell);
	}
	UIsetInteractionEnabled(tblv, FALSE);
}
*/

void PrefOnValueChanged(GACTIVITY *act, void *view)
{
	void	*wnd, *tblv, *cell, *pss, *ps, *ps2;
	char	temp[128];
	BOOL	bVal;
	int		index, val, changed;

	wnd = activityWindow(act);
	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	index = UItag(view) - TAG_PREF_CELL_CONTROL;
	pss = activityPsContext(act);
	ps = PSobjectAtIndex(pss, index);
	changed = 0;
	switch(PStype(ps)) {
	case PS_TYPE_SWITCH:
		bVal = UIisOn(view);
		if(bVal) ps2 = PSswitchOnDialog(ps);
		else	 ps2 = PSswitchOffDialog(ps);
		if(ps2) {
			switch(PStype(ps2)) {
			case PS_TYPE_ALERT:
				PSalertEncode(ps2, temp);
				AlertDialog(ACT_PREF_SWITCH+index, temp, TRUE);
				break;
			case PS_TYPE_TEXT_FIELD:
				PStextFieldEncode(ps2, temp);
				TextInputDialog(ACT_PREF_SWITCH+index, temp, PStextFieldDoneValidate(ps2));
				break;
			case PS_TYPE_SPINNER:
				PSspinnerEncode(ps2, temp);
				SpinnerDialog(ACT_PREF_SWITCH+index, temp);
				break;
			case PS_TYPE_CHECK_LIST:
				PScheckListEncode(ps2, temp);
				CheckDialog(ACT_PREF_SWITCH+index, temp);
				break;
			case PS_TYPE_PICKER:
				PSpickerEncode(ps2, temp);
				PickerDialog(ACT_PREF_SWITCH+index, temp);
				break;
//			case PS_TYPE_TIME_RANGE:
//				PStimeRangeEncode(ps2, temp);
//				TimeRangeDialog(ACT_PREF_SWITCH+index, temp);
//				break;
			case PS_TYPE_SLIDER_GROUP:
				PSsliderGroupEncode(ps2, temp);
				SliderDialog(ACT_PREF_SWITCH+index, temp);
				break;
			}
		} else {
			PSswitchSetValue(ps, bVal);
			changed = 1;
		}
//		cell = UIcellForRowAtIndex(tblv, index);
//		UIsetUserInteractionEnabled(cell, bVal);
		break;
	case PS_TYPE_STEPPER:
		PSstepperSetValue(ps, UIvalue(view));
		cell = UIcellForRowAtIndex(tblv, index);
		_StepperValueText(ps, temp);
		CellSetSubtitle(cell, temp);
		changed = 1;
		break;
	case PS_TYPE_SLIDER:
		PSsliderSetValue(ps, UIvalue(view));
		cell = UIcellForRowAtIndex(tblv, index);
		sprintf(temp, "%d", PSsliderValue(ps));
		CellSetSubtitle(cell, temp);
		changed = 1;
		break;
	}
	PSsetResultCode(pss, RESULT_OK);
	if(changed) {
		PScallOnValueChanged(ps, index, act);
		PScallOnPrefChanged(pss, index, act);
	}
}

void PrefOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	void	*wnd, *tblv, *cell, *v, *pss, *ps, *ps2;
	UITableViewCellAccessoryType	accessoryType;
	char	*p, temp[256];
	BOOL	bVal;
	int		index, val, val2, changed;

	wnd = activityWindow(act);
	pss = activityPsContext(act);
//printf("PrefOnActivityResult: requestCode=%d resultCode=%d [%s]\n", requestCode, resultCode, intent);
	if(requestCode >= ACT_PREF_SWITCH) {
		index = requestCode - ACT_PREF_SWITCH;
	} else {
		index = requestCode - ACT_PREF;
		if(resultCode < RESULT_OK) {
//printf("PrefOnActivityResult: requestCode=%d resultCode=%d [%s]\n", requestCode, resultCode, intent);
			PScallOnActivityResult(pss, requestCode, resultCode, intent);
			return;
		}
	}
	PSsetResultCode(pss, resultCode);
	if(index < 0) {
//printf("PrefOnActivityResult: index=%d requestCode=%d resultCode=%d [%s]\n", index, requestCode, resultCode, intent);
		PScallOnActivityResult(pss, requestCode, resultCode, intent);
		return;
	}

	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	cell = UIcellForRowAtIndex(tblv, index);
	ps = PSobjectAtIndex(pss, index);
	changed = 0;
	switch(PStype(ps)) {
	case PS_TYPE_SCREEN:
	case PS_TYPE_BUTTON:
	case PS_TYPE_RADIO:
		// append 2021.3.16
		if(intent && intent[0]) strcpy(act->intent, intent);
		else	act->intent[0] = 0;
		changed = 1;
		break;
	case PS_TYPE_CHECK:
		if(requestCode >= ACT_PREF_SWITCH) {
			bVal = PScheckValue(ps);
			if(bVal) ps2 = PScheckOffDialog(ps);
			else	 ps2 = PScheckOnDialog(ps);
			if(resultCode >= RESULT_OK) {
				switch(PStype(ps2)) {
				case PS_TYPE_SPINNER:
					PSspinnerDecode2(ps2, intent);
					break;
				case PS_TYPE_ALERT:
					PSalertCallConfirm(ps2);
					break;
				}
				PScheckSetValue(ps, !bVal);
				changed = 1;
			} else {
				//v = UIviewWithTag(cell, TAG_PREF_CELL_CONTROL+index);
				accessoryType = UITableViewCellAccessoryTypeUncheckmark;
				accessoryType = UITableViewCellAccessoryTypeCheckmark;
				//UIsetAccessoryType(cell, accessoryType);
			}
		} else {
			changed = 1;
		}
		break;
	case PS_TYPE_SWITCH:
		if(requestCode >= ACT_PREF_SWITCH) {
			bVal = PSswitchValue(ps);
			if(bVal) ps2 = PSswitchOffDialog(ps);
			else	 ps2 = PSswitchOnDialog(ps);
			if(resultCode >= RESULT_OK) {
				switch(PStype(ps2)) {
				case PS_TYPE_SPINNER:
					PSspinnerDecode2(ps2, intent);
					break;
				case PS_TYPE_ALERT:
					PSalertCallConfirm(ps2);
					break;
				}
				PSswitchSetValue(ps, !bVal);
				changed = 1;
			} else {
				v = UIviewWithTag(cell, TAG_PREF_CELL_CONTROL+index);
				UIsetOn(v, bVal);
			}
		} else {
			changed = 1;
		}
		break;
	case PS_TYPE_TEXT_FIELD:
		PStextFieldDecode(ps, intent);
		if(UIcellStyle(cell) == UITableViewCellStyleSubtitle) {
			PStextFieldAllValue(ps, temp);
			PSsetSubtitle(ps, temp);
			CellSetSubtitle(cell, temp);
		}
		changed = 1;
		break;
	case PS_TYPE_SPINNER:
		bVal = PSspinnerDecode(ps, intent);
		if(bVal) {
			val = PSspinnerIndexWithValue(ps, PSspinnerValue(ps));
			if(val < 0) p = NULL;
			else	p = PSspinnerTitleAtIndex(ps, val);
			if(p) strcpy(temp, p); else temp[0] = 0;
			PSsetSubtitle(ps, temp);
			CellSetSubtitle(cell, temp);
			changed = 1;
		}
		break;
	case PS_TYPE_CHECK_LIST:
		bVal = PScheckListDecode(ps, intent);
		if(bVal) changed = 1;
		break;
	case PS_TYPE_PICKER:
		bVal = PSpickerDecode(ps, intent);
		if(bVal) {
			PSpickerValuesTitle(ps, temp);
			PSsetSubtitle(ps, temp);
			CellSetSubtitle(cell, temp);
			changed = 1;
		}
		break;
	case PS_TYPE_TIME_RANGE:
		bVal = PStimeRangeDecode(ps, intent);
		if(bVal) {
			PStimeRangeAllValueTitle(ps, temp);
			PSsetSubtitle(ps, temp);
			CellSetSubtitle(cell, temp);
			changed = 1;
		}
		break;
	case PS_TYPE_SLIDER_GROUP:
		bVal = PSsliderGroupDecode(ps, intent);
		if(bVal) changed = 1;
		break;
	}
	if(changed) {
		PScallOnValueChanged(ps, index, act);
		PScallOnPrefChanged(pss, index, act);
	}
	// inserted at 2025-04-24
	PScallOnActivityResult(pss, requestCode, resultCode, intent);
}

int PrefOnNumberOfRowsInTableView(void *self, void *tblv)
{
	GACTIVITY	*act = self;
	void	*ps;

	ps = activityPsContext(act);
	return PSchildrenCount(ps);
}

void *PrefInitCell(void *tblv, UITableViewCellStyle style, UITableViewCellAccessoryType accessoryType, char *title)
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

void *PrefOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	GACTIVITY	*act = self;
	void	*cell, *ps, *v, *v2;
	UITableViewCellStyle	style;
	UITableViewCellAccessoryType	accessoryType;
	CGRect		rt, rt2;
	BOOL		bVal;
	int			type, w, h, btn;
	char		*p, temp[128];

	v = NULL;
	ps = PSobjectAtIndex(act->psContext, index);
	type = PStype(ps);
	switch(type) {
	case PS_TYPE_SCREEN:
		if(!PSintAction(ps) && !PSuserAction(ps)) PSsetInteractionEnabled(ps, FALSE);
		if(PSisInteractionEnabled(ps)) accessoryType = UITableViewCellAccessoryTypeDisclosureIndicator;
		else	accessoryType = UITableViewCellAccessoryTypeNone;
		break;
	case PS_TYPE_TITLE:
		accessoryType = UITableViewCellAccessoryTypeNone;
		break;
	case PS_TYPE_BUTTON:
		accessoryType = UITableViewCellAccessoryTypeNone;
		break;
	case PS_TYPE_CHECK:
		if(PScheckValue(ps)) accessoryType = UITableViewCellAccessoryTypeCheckmark;
		else	accessoryType = UITableViewCellAccessoryTypeUncheckmark;
		break;
	case PS_TYPE_RADIO_GROUP:
		accessoryType = UITableViewCellAccessoryTypeNone;
		break;
	case PS_TYPE_RADIO:
		if(PSradioIsOn(ps)) accessoryType = UITableViewCellAccessoryTypeRadiomark;
		else	accessoryType = UITableViewCellAccessoryTypeUnradiomark;
		break;
	case PS_TYPE_SWITCH:
		accessoryType = UITableViewCellAccessoryTypeNone;
		v = CreateCellAccessorySwitch(PSswitchValue(ps));
		if(!PSisAccessoryEnabled(ps)) UIsetEnabled(v, FALSE);
		break;
	case PS_TYPE_STEPPER:
		accessoryType = UITableViewCellAccessoryTypeNone;
		v = CreateCellAccessoryStepper(PSstepperValue(ps), PSstepperMinimumValue(ps), PSstepperMaximumValue(ps), PSstepperStepValue(ps));
		if(!PSisAccessoryEnabled(ps)) UIsetEnabled(v, FALSE);
		_StepperValueText(ps, temp);
		PSsetSubtitle(ps, temp);
		break;
	case PS_TYPE_SLIDER:
		accessoryType = UITableViewCellAccessoryTypeNone;
		v = CreateCellSlider(PSsliderValue(ps), PSsliderMinimumValue(ps), PSsliderMaximumValue(ps));
		sprintf(temp, "%d", PSsliderValue(ps));
		PSsetSubtitle(ps, temp);
		break;
	case PS_TYPE_TEXT_FIELD:
		accessoryType = UITableViewCellAccessoryTypeNone;
		if(PStextFieldIsPassword(ps)) temp[0] = 0;
		else {
			PStextFieldAllValue(ps, temp);
			if(!temp[0]) { temp[0] = ' '; temp[1] = 0; }
		}
		PSsetSubtitle(ps, temp);
		break;
	case PS_TYPE_ALERT:
		accessoryType = UITableViewCellAccessoryTypeNone;
		break;
	case PS_TYPE_SPINNER:
		accessoryType = UITableViewCellAccessoryTypeNone;
		w = PSspinnerIndexWithValue(ps, PSspinnerValue(ps));
		if(w < 0) p = NULL;
		else	p = PSspinnerTitleAtIndex(ps, w);
		if(p) strcpy(temp, p);
		else {
			temp[0] = ' '; temp[1] = 0;
		}
		PSsetSubtitle(ps, temp);
		break;
	case PS_TYPE_CHECK_LIST:
		accessoryType = UITableViewCellAccessoryTypeNone;
		break;
	case PS_TYPE_PICKER:
		accessoryType = UITableViewCellAccessoryTypeNone;
		PSpickerValuesTitle(ps, temp);
		PSsetSubtitle(ps, temp);
		break;
	case PS_TYPE_TIME_RANGE:
		accessoryType = UITableViewCellAccessoryTypeNone;
		PStimeRangeAllValueTitle(ps, temp);
		PSsetSubtitle(ps, temp);
		break;
	case PS_TYPE_SLIDER_GROUP:
		accessoryType = UITableViewCellAccessoryTypeNone;
		break;
	}
	p = PSsubtitle(ps);
	// modified 2020.3.14
	//if(p[0] && type != PS_TYPE_BUTTON && type != PS_TYPE_ALERT) style = UITableViewCellStyleSubtitle;
	if(p[0] && type != PS_TYPE_ALERT) style = UITableViewCellStyleSubtitle;
	else	style = UITableViewCellStyleDefault;
	cell = PrefInitCell(tblv, style, accessoryType, PStitle(ps));
	//if(style == UITableViewCellStyleSubtitle && type != PS_TYPE_SLIDER) CellSetSubtitle(cell, p);
	if(style == UITableViewCellStyleSubtitle) CellSetSubtitle(cell, p);
	if(type == PS_TYPE_SCREEN || type == PS_TYPE_BUTTON) {
		p = PSscreenImageFile(ps);
		if(p) CellSetImageAlphaMask(cell, p);
	} else if(type == PS_TYPE_TITLE) {
		v2 = UItextLabel(cell);
		if(PStitleStyle(ps) == PS_TITLE_STYLE_GROUP) UIsetTextColor(v2, TABLE_CYAN_700);
		else if(index == 0 && PSviewStyle(activityPsContext(act)) == PS_VIEW_STYLE_EDIT) {
			UIsetBackgroundColor(cell, TABLE_GREY_200);
			UIsetTextAlignment(v2, UITextAlignmentCenter);
		}
	} else if(type == PS_TYPE_RADIO_GROUP) {
		v2 = UItextLabel(cell);
		UIsetTextColor(v2, TABLE_CYAN_700);
	}
	if(v) {
		if(type == PS_TYPE_SLIDER) UIaddChild(cell, v);
		else	UIsetAccessoryView(cell, v);
		UIsetTag(v, TAG_PREF_CELL_CONTROL+index);
	}
	btn = PSbutton(ps);
	if(btn > 0) {
		UIframe(tblv, &rt);
		w = 44;
		rt.width -= w; rt.x = rt.y = 0; rt.height = 0;
		UIsetFrame(cell, &rt);
		UIsetClipToBounds(cell, FALSE);
		if(btn != 9) {
			if(UIcellStyle(cell) == UITableViewCellStyleSubtitle) h = 68; else h = 48;
			v = ViewAddButtonAlphaMask(cell, TAG_PREF_CELL_BTN+index, "settings.bmp", rt.width, 0, w, h, 28);
			UIsetOpaque(v, TRUE);
			UIsetBackgroundColor(v, UIbackgroundColor(cell));
			if(type == PS_TYPE_CHECK || type == PS_TYPE_RADIO) {
				if(type == PS_TYPE_CHECK) {
					if(PScheckValue(ps)) bVal = TRUE; else bVal = FALSE;
				} else {
					if(PSradioIsOn(ps)) bVal = TRUE; else bVal = FALSE;
				}
				UIsetEnabled(v, bVal);
			}
		}
	}
	if(PSisHidden(ps)) UIsetHidden(cell, TRUE);
	if(!PSisEnabled(ps)) UIsetEnabled(cell, FALSE);
	UIsetUserInteractionEnabled(cell, PSisInteractionEnabled(ps));
	return cell;
}

void PrefOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	GACTIVITY	*act = self;
	UITableViewCellAccessoryType	accessoryType;
	void	*cell, *v, *ps, *pss, *psg, *ps2;
	char	*p, temp[1024];
	BOOL	bVal;
	int		val, count, idx;

	pss = activityPsContext(act);	// 2018.10.4 bug fixed: activityPsContext
	ps = PSobjectAtIndex(pss, index);
//printf("PrefOnDidSelect: index=%d type=%d\n", index, PStype(ps));
	switch(PStype(ps)) {
	case PS_TYPE_SCREEN:	
		PrefCreate(ACT_PREF+index, ps);	
		break;
	case PS_TYPE_BUTTON:
		PrefCreate(ACT_PREF+index, ps);	
		break;
	case PS_TYPE_SWITCH:
		if(PSswitchValue(ps) && PSisAccessoryEnabled(ps) && (PSintAction(ps) || PSuserAction(ps))) PrefCreate(ACT_PREF+index, ps);
		break;
	case PS_TYPE_CHECK:
		cell = UIcellForRowAtIndex(tblv, index);
		if(UIaccessoryType(cell) == UITableViewCellAccessoryTypeCheckmark) {
			accessoryType = UITableViewCellAccessoryTypeUncheckmark;
			bVal = FALSE;
		} else {
			accessoryType = UITableViewCellAccessoryTypeCheckmark;
			bVal = TRUE;
		}
		if(PScheckValue(ps) != bVal) {
			if(bVal)  ps2 = PScheckOnDialog(ps);
			else	  ps2 = PScheckOffDialog(ps);
			if(ps2) {
				switch(PStype(ps2)) {
				case PS_TYPE_ALERT:
					PSalertEncode(ps2, temp);
					AlertDialog(ACT_PREF_SWITCH+index, temp, TRUE);
					break;
				case PS_TYPE_SPINNER:
					PSspinnerEncode(ps2, temp);
					SpinnerDialog(ACT_PREF_SWITCH+index, temp);
					break;
				}
			} else {
				PScheckSetValue(ps, bVal);
				UIsetAccessoryType(cell, accessoryType);
				v = UIviewWithTag(cell, TAG_PREF_CELL_BTN+index);
				if(v) UIsetEnabled(v, bVal);
				PSsetResultCode(pss, 0);
				PScallOnValueChanged(ps, index, act);
				PScallOnPrefChanged(pss, index, act);
				/*
				if(PScheckBoxInteractionReverse(ps)) bVal = !bVal;
				count = index + 1 + PScheckBoxInteractionCount(ps);
				for(val = index+1;val < count;val++) {
					ps = PSobjectAtIndex(act->psContext, val);
					PSsetInteractionEnabled(ps,  bVal);
					cell = UIcellForRowAtIndex(tblv, val);
					if(cell) UIsetUserInteractionEnabled(cell, bVal);
				}
				*/
			}
		}
		break;
	case PS_TYPE_RADIO:
		cell = UIcellForRowAtIndex(tblv, index);
		PSradioSetOn(ps);
		psg = PSradioRadioGroup(ps);
		idx = val = PSindexForObject(pss, psg);
		ps = psg;
		while(1) {
			ps = PSnextChild(ps);
			if(!ps || PStype(ps) != PS_TYPE_RADIO) break;
			idx++;
			cell = UIcellForRowAtIndex(tblv, idx);
			if(PSradioIsOn(ps)) {
				accessoryType = UITableViewCellAccessoryTypeRadiomark;
				bVal = TRUE;
			} else {
				accessoryType = UITableViewCellAccessoryTypeUnradiomark;
				bVal = FALSE;
			}
			UIsetAccessoryType(cell, accessoryType);
			v = UIviewWithTag(cell, TAG_PREF_CELL_BTN+idx);
			if(v) UIsetEnabled(v, bVal);
		}
		PSsetResultCode(pss, 0);
		PScallOnValueChanged(psg, val, act);
		PScallOnPrefChanged(pss, val, act);
		break;
	case PS_TYPE_TEXT_FIELD:
		PStextFieldEncode(ps, temp);
		if(PStextFieldInputTypeAtIndex(ps, 0) == 7) UuidInputDialog(ACT_PREF+index, temp);
		else	TextInputDialog(ACT_PREF+index, temp, PStextFieldDoneValidate(ps));
		break;
	case PS_TYPE_ALERT:
		PSalertEncode(ps, temp);
		AlertDialog(ACT_PREF+index, temp, TRUE);
		break;
	case PS_TYPE_SPINNER:
		PSspinnerEncode(ps, temp);
		SpinnerDialog(ACT_PREF+index, temp);
		break;
	case PS_TYPE_CHECK_LIST:
		PScheckListEncode(ps, temp);
		CheckDialog(ACT_PREF+index, temp);
		break;
	case PS_TYPE_PICKER:
		PSpickerEncode(ps, temp);
//printf("len=%d [%s]\n", strlen(temp), temp);
		PickerDialog(ACT_PREF+index, temp);
		break;
//	case PS_TYPE_TIME_RANGE:
//		PStimeRangeEncode(ps, temp);
//		TimeRangeDialog(ACT_PREF+index, temp);
//		break;
	case PS_TYPE_SLIDER_GROUP:
		PSsliderGroupEncode(ps, temp);
		SliderDialog(ACT_PREF+index, temp);
		break;
	}
}

