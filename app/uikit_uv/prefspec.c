#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prim.h"
#include "gactivity.h"
#include "prefspec.h"

void *PSalloc(int size);
void PSfree(void *buffer);


void PSinit(void *self, int type, char *title)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->type				= type;
	ps->state				= PS_INTERACTION_ENABLED;
	ps->resultCode			= 0;
	ps->viewStyle			= PS_VIEW_STYLE_DEFAULT;
	ps->tag					= 0;
	ps->button				= 0;
	ps->backResultCode		= RESULT_CANCELLED;
	ps->userData			= 0;
	if(title) strcpy(ps->title, title); else ps->title[0] = 0;
	ps->subtitle[0]			= 0;	
	ps->activity			= NULL;
	ps->context				= NULL;
	ps->onPrefChanged		= NULL;
	ps->onValueChanged		= NULL;
	ps->intAction			= NULL;
	ps->userAction			= NULL;
	ps->buttonUserAction	= NULL;
	ps->buttonIntAction		= NULL;
	ps->onUserButtonClick	= NULL;
	ps->onCreate			= NULL;
	ps->onStart				= NULL;
	ps->onActivityResult	= NULL;
	ps->onDestroy			= NULL;
	ps->children			= NULL;
	ps->next				= NULL;
	ps->bufAddress			= NULL;
}

int PStype(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return (int)ps->type;
}

char *PStitle(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return ps->title;
}

void PSsetTitle(void *self, char *title)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(title) strcpy(ps->title, title);
	else	ps->title[0] = 0;
}

char *PSsubtitle(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return ps->subtitle;
}

void PSsetSubtitle(void *self, char *subtitle)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(subtitle) strcpy(ps->subtitle, subtitle);
	else	ps->subtitle[0] = 0;
}

BOOL PSisEnabled(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->state & PS_DISABLED) return FALSE;
	else	return TRUE;
}

void PSsetEnabled(void *self, BOOL isEnabled)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(isEnabled) ps->state &= ~PS_DISABLED;
	else	ps->state |= PS_DISABLED;
}

BOOL PSisHidden(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->state & PS_HIDDEN) return TRUE;
	else	return FALSE;
}

void PSsetHidden(void *self, BOOL isHidden)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(isHidden) ps->state |= PS_HIDDEN;
	else	ps->state &= ~PS_HIDDEN;
}

BOOL PSisInteractionEnabled(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->state & PS_INTERACTION_ENABLED) return TRUE;
	else	return FALSE;
}

void PSsetInteractionEnabled(void *self, BOOL isEnabled)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(isEnabled) ps->state |= PS_INTERACTION_ENABLED;
	else	ps->state &= ~PS_INTERACTION_ENABLED;
}

BOOL PSisAccessoryEnabled(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->state & PS_ACCESSORY_DISABLED) return FALSE;
	else	return TRUE;
}

void PSsetAccessoryEnabled(void *self, BOOL isEnabled)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(isEnabled) ps->state &= ~PS_ACCESSORY_DISABLED;
	else	ps->state |= PS_ACCESSORY_DISABLED;
}

void *PSactivity(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return ps->activity;
}

void PSsetActivity(void *self, void *activity)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->activity = activity;
}

void *PScontext(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return ps->context;
}

void PSsetContext(void *self, void *context)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->context = context;
}

int PStag(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return ps->tag;
}

void PSsetTag(void *self, int tag)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->tag = tag;
}

int PSresultCode(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return (int)ps->resultCode;
}

void PSsetResultCode(void *self, int resultCode)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->resultCode = resultCode;
}

int PSviewStyle(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return (int)ps->viewStyle;
}

void PSsetViewStyle(void *self, int viewStyle)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->viewStyle = viewStyle;
}

int PSbutton(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return (int)ps->button;
}

void PSsetButton(void *self, int button, void (*action)(void *))
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->button = button;
	PSsetButtonIntAction(self, action);
}

int PSbackResultCode(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return (int)ps->backResultCode;
}

void PSsetBackResultCode(void *self, int backResultCode)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->backResultCode = backResultCode;
}

int PSuserData(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return (int)ps->userData;
}

void PSsetUserData(void *self, int userData)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->userData = userData;
}

void PSsetOnPrefChanged(void *self, void (*onPrefChanged)(void *, int, void *))
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->onPrefChanged = onPrefChanged;
}

void PScallOnPrefChanged(void *self, int index, void *act)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->onPrefChanged) (*ps->onPrefChanged)(self, index, act);
}

void PSsetOnValueChanged(void *self, void (*onValueChanged)(void *, int, void *))
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->onValueChanged = onValueChanged;
}

void PScallOnValueChanged(void *self, int index, void *act)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->onValueChanged) (*ps->onValueChanged)(self, index, act);
}

void *PSuserAction(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return (void *)ps->userAction;
}

void PSsetUserAction(void *self, void (*action)(int, void *))
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->userAction = action;
}

void PScallUserAction(void *self, int requestCode)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->userAction) (*ps->userAction)(requestCode, self);
}

void *PSintAction(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return (void *)ps->intAction;
}

void PSsetIntAction(void *self, void (*action)(void *))
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->intAction = action;
}

void PScallIntAction(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->intAction) (*ps->intAction)(self);
}

void *PSbuttonUserAction(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return (void *)ps->buttonUserAction;
}

void PSsetButtonUserAction(void *self, void (*action)(int, void *))
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->buttonUserAction = action;
}

void PScallButtonUserAction(void *self, int requestCode)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->buttonUserAction) (*ps->buttonUserAction)(requestCode, self);
}

void *PSbuttonIntAction(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return (void *)ps->buttonIntAction;
}

void PSsetButtonIntAction(void *self, void (*action)(void *))
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->buttonIntAction = action;
}

void PScallButtonIntAction(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->buttonIntAction) (*ps->buttonIntAction)(self);
}

void PSsetOnUserButtonClick(void *self, void (*onUserButtonClick)(void *))
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->onUserButtonClick = onUserButtonClick;
}

void PScallOnUserButtonClick(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->onUserButtonClick) (*ps->onUserButtonClick)(self);
}

void PSsetOnCreate(void *self, void (*onCreate)(void *))
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->onCreate = onCreate;
}

void PScallOnCreate(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->onCreate) (*ps->onCreate)(self);
}

void PSsetOnStart(void *self, void (*onStart)(void *))
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->onStart = onStart;
}

void PScallOnStart(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->onStart) (*ps->onStart)(self);
}

void PSsetOnActivityResult(void *self, void (*onActivityResult)(void *, int, int, char *))
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->onActivityResult = onActivityResult;
}

void PScallOnActivityResult(void *self, int requestCode, int resultCode, char *intent)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->onActivityResult) (*ps->onActivityResult)(self, requestCode, resultCode, intent);
}

void PSsetOnDestroy(void *self, void (*onDestroy)(void *))
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->onDestroy = onDestroy;
}

void PScallOnDestroy(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->onDestroy) (*ps->onDestroy)(self);
}

void *PSfirstChild(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return ps->children;
}

void *PSnextChild(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return ps->next;
}

void *PSobjectAtIndex(void *self, int index)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;
	PS_OBJECT	*cps;
	int		idx;

	idx = 0;
	cps = ps->children;
	while(cps) {
		if(idx == index) break;
		idx++;
		cps = cps->next;
	}
	return cps;
}

void *PSobjectWithContext(void *self, void *ctx)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;
	PS_OBJECT	*cps;
	int		idx;

	idx = 0;
	cps = ps->children;
	while(cps) {
		if(cps->context == ctx) break;
		cps = cps->next;
	}
	return cps;
}

int PSindexForObject(void *self, void *psObj)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;
	PS_OBJECT	*cps;
	int		idx;

	idx = 0;
	cps = ps->children;
	while(cps) {
		if(cps == psObj) break;
		idx++;
		cps = cps->next;
	}
	return idx;
}

void *PSviewBuffer(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	return ps->bufAddress;
}

void PSsetViewBuffer(void *self, void *address)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	ps->bufAddress = address;
}

int PSchildrenCount(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;
	PS_OBJECT	*cps;
	int		count;

	count = 0;
	cps = ps->children;
	while(cps) {
		count++;
		cps = cps->next;	
	}
	return count;
}

void PSremoveChildren(void *self)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;

	if(ps->children) {
		PSfree(ps->children);
		ps->children = NULL;
	}
}

void PSaddChild(void *self, void *child)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;
	PS_OBJECT	*cps;

	cps = ps->children;
	if(!cps) ps->children = child;
	else {
		while(cps->next) cps = cps->next;
		cps->next = child;
	}
	cps = child;
	cps->next = NULL;
}

int PScompareChildrenContext(void *self, void *objs[], int count)
{
	PS_OBJECT	*ps = (PS_OBJECT *)self;
	PS_OBJECT	*cps;
	int		idx;

	idx = 0;
	cps = ps->children;
	while(cps && idx < count) {
		if(PScontext(cps) != objs[idx]) break;
		idx++;
		cps = cps->next;
	}
	if(!cps && idx == count) idx = 0;
	else	idx = -1;
	return idx;
}

void *PSaddScreen(void *self, char *title, void (*action)(void *))
{
	void	*cps;
	
	cps = PSscreenAlloc();
	PSscreenInit(cps, title, action);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddTitle(void *self, char *title, char *subtitle, int style)
{
	void	*cps;
	
	cps = PStitleAlloc();
	PStitleInit(cps, title, subtitle, style);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddButton(void *self, char *title, void (*action)(int, void *))
{
	void	*cps;

	cps = PSbuttonAlloc();
	PSbuttonInit(cps, title, action);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddCheck(void *self, char *title, BOOL (*value)(void *), void (*setValue)(void *, BOOL))
{
	void	*cps;
	
	cps = PScheckAlloc();
	PScheckInit(cps, title, value, setValue);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddRadioGroup(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int))
{
	void	*cps;
	
	cps = PSradioGroupAlloc();
	PSradioGroupInit(cps, title, value, setValue);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddRadio(void *self, char *title, int value, void *radioGroup)
{
	void	*cps;
	
	cps = PSradioAlloc();
	PSradioInit(cps, title, value, radioGroup);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddSwitch(void *self, char *title, BOOL (*value)(void *), void (*setValue)(void *, BOOL), void (*action)(void *))
{
	void	*cps;

	cps = PSswitchAlloc();
	PSswitchInit(cps, title, value, setValue, action);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddStepper(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int), int minValue, int maxValue, int stepValue, char *unitText)
{
	void	*cps;
	
	cps = PSstepperAlloc();
	PSstepperInit(cps, title, value, setValue, minValue, maxValue, stepValue, unitText);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddSlider(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int), int minValue, int maxValue)
{
	void	*cps;
	
	cps = PSsliderAlloc();
	PSsliderInit(cps, title, value, setValue, minValue, maxValue);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddAlert(void *self, char *title, char *value, void (*confirm)(void *))
{
	void	*cps;
	
	cps = PSalertAlloc();
	PSalertInit(cps, title, value, confirm);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddTextField(void *self, char *title, BOOL (*doneValidate)(char *, char *))
{
	void	*cps;
	
	cps = PStextFieldAlloc();
	PStextFieldInit(cps, title, doneValidate);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddSpinner(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int))
{
	void	*cps;
	
	cps = PSspinnerAlloc();
	PSspinnerInit(cps, title, value, setValue);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddCheckList(void *self, char *title)
{
	void	*cps;
	
	cps = PScheckListAlloc();
	PScheckListInit(cps, title);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddPicker(void *self, char *title, void (*getValues)(void *, int *), void (*setValues)(void *, int *))
{
	void	*cps;
	
	cps = PSpickerAlloc();
	PSpickerInit(cps, title, getValues, setValues);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddTimeRange(void *self, char *title, unsigned char *value)
{
	void	*cps;
	
	cps = PStimeRangeAlloc();
	PStimeRangeInit(cps, title, value);
	PSaddChild(self, cps);
	return cps;
}

void *PSaddSliderGroup(void *self, char *title)
{
	void	*cps;
	
	cps = PSsliderGroupAlloc();
	PSsliderGroupInit(cps, title);
	PSaddChild(self, cps);
	return cps;
}

void *PSscreenAlloc(void)
{
	return  PSalloc(sizeof(PS_SCREEN));
}

void PSscreenInit(void *self, char *title, void (*action)(void *))
{
	PS_SCREEN	*ps = (PS_SCREEN *)self;

	PSinit(self, PS_TYPE_SCREEN, title);
	PSsetIntAction(self, action);
	ps->imageFile[0]	= 0;
}

char *PSscreenImageFile(void *self)
{
	PS_SCREEN	*ps = (PS_SCREEN *)self;
	char	*p;

	p = ps->imageFile;
	if(!p[0]) p = NULL;
	return p;
}

void PSscreenSetImageFile(void *self, char *imageFile)
{
	PS_SCREEN	*ps = (PS_SCREEN *)self;

	if(imageFile) strcpy(ps->imageFile, imageFile);
	else	ps->imageFile[0] = 0;
}

void *PStitleAlloc(void)
{
	return  PSalloc(sizeof(PS_TITLE));
}

void PStitleInit(void *self, char *title, char *subtitle, int style)
{
	PS_TITLE	*ps = (PS_TITLE *)self;

	PSinit(self, PS_TYPE_TITLE, title);
	PSsetSubtitle(self, subtitle);
	PSsetInteractionEnabled(ps, FALSE);
	ps->style		= style;
}

int PStitleStyle(void *self)
{
	PS_TITLE	*ps = (PS_TITLE *)self;

	return (int)ps->style;
}

void PStitleGetColumn(void *self, int *column)
{
	PS_TITLE	*ps = (PS_TITLE *)self;
	int		i;

	for(i = 0;i < 4;i++) column[i] = ps->column[i];
}


void PStitleSetColumn(void *self, int *column)
{
	PS_TITLE	*ps = (PS_TITLE *)self;
	int		i;

	for(i = 0;i < 4;i++) ps->column[i] = column[i];
}

void *PSbuttonAlloc(void)
{
	return  PSalloc(sizeof(PS_BUTTON));
}

void PSbuttonInit(void *self, char *title, void (*action)(int, void *))
{
	PS_BUTTON	*ps = (PS_BUTTON *)self;

	PSinit(self, PS_TYPE_BUTTON, title);
	PSsetUserAction(self, action);
	ps->imageFile[0]	= 0;
}

char *PSbuttonImageFile(void *self)
{
	PS_BUTTON	*ps = (PS_BUTTON *)self;
	char	*p;

	p = ps->imageFile;
	if(!p[0]) p = NULL;
	return p;
}

void PSbuttonSetImageFile(void *self, char *imageFile)
{
	PS_BUTTON	*ps = (PS_BUTTON *)self;

	if(imageFile) strcpy(ps->imageFile, imageFile);
	else	ps->imageFile[0] = 0;
}

void *PScheckAlloc(void)
{
	return  PSalloc(sizeof(PS_CHECK));
}

void PScheckInit(void *self, char *title, BOOL (*value)(void *), void (*setValue)(void *, BOOL))
{
	PS_CHECK	*ps = (PS_CHECK *)self;

	PSinit(self, PS_TYPE_CHECK, title);
	ps->value		= value;
	ps->setValue	= setValue;
	ps->onDialog	= NULL;
	ps->offDialog	= NULL;
	ps->interactionCount  = 0;
}

BOOL PScheckValue(void *self)
{
	PS_CHECK	*ps = (PS_CHECK *)self;
	BOOL	bVal;

	if(ps->value) bVal = (*ps->value)(PScontext(ps));
	else	bVal = ps->_value;
	return bVal;
}

void PScheckSetValue(void *self, BOOL value)
{
	PS_CHECK	*ps = (PS_CHECK *)self;

	if(ps->setValue) {
		ps->_value = (*ps->value)(PScontext(ps));
		(*ps->setValue)(PScontext(ps), value);
	} else	ps->_value = value;
}

void PScheckRestoreValue(void *self)
{
	PS_CHECK	*ps = (PS_CHECK *)self;
	BOOL	bVal;

	if(ps->setValue) {
		bVal = ps->_value;
		ps->_value = (*ps->value)(PScontext(ps));
		(*ps->setValue)(PScontext(ps), bVal);
	}
}

void *PScheckOnDialog(void *self)
{
	PS_CHECK	*ps = (PS_CHECK *)self;

	return ps->onDialog;
}

void PScheckSetOnDialog(void *self, void *dialog)
{
	PS_CHECK	*ps = (PS_CHECK *)self;

	ps->onDialog = dialog;
}

void *PScheckOffDialog(void *self)
{
	PS_CHECK	*ps = (PS_CHECK *)self;

	return ps->offDialog;
}

void PScheckSetOffDialog(void *self, void *dialog)
{
	PS_CHECK	*ps = (PS_CHECK *)self;

	ps->offDialog = dialog;
}

int PScheckInteractionCount(void *self)
{
	PS_CHECK	*ps = (PS_CHECK *)self;

	return (int)ps->interactionCount;
}

BOOL PScheckInteractionReverse(void *self)
{
	PS_CHECK	*ps = (PS_CHECK *)self;
	BOOL	reverse;

	if(ps->interactionReverse) reverse = TRUE;
	else	reverse = FALSE;
	return reverse;
}

void PScheckSetInteraction(void *self, int count, BOOL reverse)
{
	PS_CHECK	*ps = (PS_CHECK *)self;

	ps->interactionCount = count;
	if(reverse) ps->interactionReverse = 1;
	else		ps->interactionReverse = 0;
}

void *PSradioGroupAlloc(void)
{
	return  PSalloc(sizeof(PS_RADIO_GROUP));
}

void PSradioGroupInit(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int))
{
	PS_RADIO_GROUP	*ps = (PS_RADIO_GROUP *)self;

	PSinit(self, PS_TYPE_RADIO_GROUP, title);
	ps->value		= value;
	ps->setValue	= setValue;
}

int PSradioGroupValue(void *self)
{
	PS_RADIO_GROUP	*ps = (PS_RADIO_GROUP *)self;
	int		val;

	if(ps->value) val = (*ps->value)(PScontext(ps));
	else	val = ps->_value;
	return val;
}

void PSradioGroupSetValue(void *self, int value)
{
	PS_RADIO_GROUP	*ps = (PS_RADIO_GROUP *)self;

	if(ps->setValue) {
		ps->_value = (*ps->value)(PScontext(ps));
		(*ps->setValue)(PScontext(ps), value);
	} else	ps->_value = value;
}

void PSradioGroupRestoreValue(void *self)
{
	PS_RADIO_GROUP	*ps = (PS_RADIO_GROUP *)self;
	int		val;

	if(ps->setValue) {
		val = ps->_value;
		ps->_value = (*ps->value)(PScontext(ps));
		(*ps->setValue)(PScontext(ps), val);
	}
}

void *PSradioAlloc(void)
{
	return  PSalloc(sizeof(PS_RADIO));
}

void PSradioInit(void *self, char *title, int value, void *radioGroup)
{
	PS_RADIO	*ps = (PS_RADIO *)self;

	PSinit(self, PS_TYPE_RADIO, title);
	ps->_value		= value;
	ps->radioGroup	= radioGroup;
}

void *PSradioRadioGroup(void *self)
{
	PS_RADIO	*ps = (PS_RADIO *)self;

	return ps->radioGroup;
}

BOOL PSradioIsOn(void *self)
{
	PS_RADIO	*ps = (PS_RADIO *)self;
	BOOL	bVal;

	if(PSradioGroupValue(ps->radioGroup) == ps->_value) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void PSradioSetOn(void *self)
{
	PS_RADIO	*ps = (PS_RADIO *)self;

	PSradioGroupSetValue(ps->radioGroup, ps->_value);
}

void *PSswitchAlloc(void)
{
	return  PSalloc(sizeof(PS_SWITCH));
}

void PSswitchInit(void *self, char *title, BOOL (*value)(void *), void (*setValue)(void *, BOOL), void (*action)(void *))
{
	PS_SWITCH	*ps = (PS_SWITCH *)self;

	PSinit(self, PS_TYPE_SWITCH, title);
	ps->value		= value;
	ps->setValue	= setValue;
	PSsetIntAction(ps, action);
	ps->onDialog	= NULL;
	ps->offDialog	= NULL;
}

BOOL PSswitchValue(void *self)
{
	PS_SWITCH	*ps = (PS_SWITCH *)self;
	BOOL	bVal;

	if(ps->value) bVal = (*ps->value)(PScontext(ps));
	else	bVal = ps->_value;
	return bVal;
}

void PSswitchSetValue(void *self, BOOL value)
{
	PS_SWITCH	*ps = (PS_SWITCH *)self;

	if(ps->setValue) {
		ps->_value = (*ps->value)(PScontext(ps));
		(*ps->setValue)(PScontext(ps), value);
	} else	ps->_value = value;
}

void PSswitchRestoreValue(void *self)
{
	PS_SWITCH	*ps = (PS_SWITCH *)self;
	BOOL	bVal;

	if(ps->setValue) {
		bVal = ps->_value;
		ps->_value = (*ps->value)(PScontext(ps));
		(*ps->setValue)(PScontext(ps), bVal);
	}
}

void *PSswitchOnDialog(void *self)
{
	PS_SWITCH	*ps = (PS_SWITCH *)self;

	return ps->onDialog;
}

void PSswitchSetOnDialog(void *self, void *dialog)
{
	PS_SWITCH	*ps = (PS_SWITCH *)self;

	ps->onDialog = dialog;
}

void *PSswitchOffDialog(void *self)
{
	PS_SWITCH	*ps = (PS_SWITCH *)self;

	return ps->offDialog;
}

void PSswitchSetOffDialog(void *self, void *dialog)
{
	PS_SWITCH	*ps = (PS_SWITCH *)self;

	ps->offDialog = dialog;
}

void *PSstepperAlloc(void)
{
	return  PSalloc(sizeof(PS_STEPPER));
}

void PSstepperInit(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int), int minValue, int maxValue, int stepValue, char *unitText)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;

	PSinit(self, PS_TYPE_STEPPER, title);
	ps->value		= value;
	ps->setValue	= setValue;
	ps->minimumValue= minValue;
	ps->maximumValue= maxValue;
	ps->stepValue	= stepValue;
	ps->unitRatio	= 0;
	if(unitText) strcpy(ps->unitText, unitText);
	else	ps->unitText[0] = 0;
	ps->getValueText = NULL;
}

int PSstepperValue(void *self)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;
	int		val;

	if(ps->value) val = (*ps->value)(PScontext(ps));
	else	val = ps->_value;
	return val;
}

void PSstepperSetValue(void *self, int value)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;

	if(ps->setValue) {
		ps->_value = (*ps->value)(PScontext(ps));
		(*ps->setValue)(PScontext(ps), value);
	} else	ps->_value = value;
}

void PSstepperRestoreValue(void *self)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;
	int		val;

	if(ps->setValue) {
		val = ps->_value;
		ps->_value = (*ps->value)(PScontext(ps));
		(*ps->setValue)(PScontext(ps), val);
	}
}

int PSstepperMinimumValue(void *self)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;

	return ps->minimumValue;
}

void PSstepperSetMinimumValue(void *self, int minimumValue)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;

	ps->minimumValue = minimumValue;
	if(PSstepperValue(self) < minimumValue) PSstepperSetValue(self, minimumValue);
}

int PSstepperMaximumValue(void *self)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;

	return ps->maximumValue;
}

void PSstepperSetMaximumValue(void *self, int maximumValue)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;

	ps->maximumValue = maximumValue;
	if(PSstepperValue(self) > maximumValue) PSstepperSetValue(self, maximumValue);
}

int PSstepperStepValue(void *self)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;

	return ps->stepValue;
}

char *PSstepperUnitText(void *self)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;

	return ps->unitText;
}

void PSstepperSetUnitText(void *self, char *unitText)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;

	if(unitText && unitText[0]) strcpy(ps->unitText, unitText);
	else	ps->unitText[0] = 0;
}

int PSstepperUnitRatio(void *self)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;

	return ps->unitRatio;
}

void PSstepperSetUnitRatio(void *self, int unitRatio)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;

	ps->unitRatio = unitRatio;
}

void PSstepperSetGetValueText(void *self, void (*getValueText)(int, char *))
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;

	ps->getValueText = getValueText;
}

int PSstepperCallGetValueText(void *self, char *text)
{
	PS_STEPPER	*ps = (PS_STEPPER *)self;
	int		rval;

	if(ps->getValueText) {
		(*ps->getValueText)(PSstepperValue(self), text); 
		rval = 1;
	} else	rval = 0;
	return rval;
}

void *PSsliderAlloc(void)
{
	return  PSalloc(sizeof(PS_SLIDER));
}

void PSsliderInit(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int), int minValue, int maxValue)
{
	PS_SLIDER	*ps = (PS_SLIDER *)self;

	PSinit(self, PS_TYPE_SLIDER, title);
	ps->value		= value;
	ps->setValue	= setValue;
	ps->minimumValue= minValue;
	ps->maximumValue= maxValue;
}

int PSsliderValue(void *self)
{
	PS_SLIDER	*ps = (PS_SLIDER *)self;
	int		val;

	if(ps->value) val = (*ps->value)(PScontext(ps));
	else	val = ps->_value;
	return val;
}

void PSsliderSetValue(void *self, int value)
{
	PS_SLIDER	*ps = (PS_SLIDER *)self;

	if(ps->setValue) {
		ps->_value = (*ps->value)(PScontext(ps));
		(*ps->setValue)(PScontext(ps), value);
	} else	ps->_value = value;
}

void PSsliderRestoreValue(void *self)
{
	PS_SLIDER	*ps = (PS_SLIDER *)self;
	int		val;

	if(ps->setValue) {
		val = ps->_value;
		ps->_value = (*ps->value)(PScontext(ps));
		(*ps->setValue)(PScontext(ps), val);
	}
}

int PSsliderMinimumValue(void *self)
{
	PS_SLIDER	*ps = (PS_SLIDER *)self;

	return ps->minimumValue;
}

int PSsliderMaximumValue(void *self)
{
	PS_SLIDER	*ps = (PS_SLIDER *)self;

	return ps->maximumValue;
}

void *PSalertAlloc(void)
{
	return  PSalloc(sizeof(PS_ALERT));
}

void PSalertInit(void *self, char *title, char *value, void (*confirm)(void *))
{
	PS_ALERT	*ps = (PS_ALERT *)self;

	PSinit(self, PS_TYPE_ALERT, title);
	PSsetSubtitle(self, value);
	ps->confirm		= confirm;
}

void PSalertEncode(void *self, char *buf)
{
	PS_ALERT	*ps = (PS_ALERT *)self;

	sprintf(buf, "%s=%s\n", PStitle(ps), PSsubtitle(ps));	
}

void PSalertCallConfirm(void *self)
{
	PS_ALERT	*ps = (PS_ALERT *)self;

	if(ps->confirm) (*ps->confirm)(PScontext(ps));
}

void *PStextFieldAlloc(void)
{
	return  PSalloc(sizeof(PS_TEXT_FIELD));
}

void PStextFieldInit(void *self, char *title, BOOL (*doneValidate)(char *, char *))
{
	PS_TEXT_FIELD	*ps = (PS_TEXT_FIELD *)self;
	int		i;

	PSinit(self, PS_TYPE_TEXT_FIELD, title);
	ps->doneValidate	= doneValidate;
	for(i = 0;i < PS_TEXT_FIELD_SZ;i++) ps->items[i].title[0] = 0;
}

int PStextFieldAddItem(void *self, char *title, char *(*value)(void *), void (*setValue)(void *, char *), int inputType, int limitLength)
{
	PS_TEXT_FIELD	*ps = (PS_TEXT_FIELD *)self;
	PS_TEXT_FIELD_ITEM	*item;
	int		i;

	for(i = 0, item = ps->items;i < PS_TEXT_FIELD_SZ;i++, item++)
		if(!item->title[0]) break;
	if(i < PS_TEXT_FIELD_SZ) {
		if(title) strcpy(item->title, title); else item->title[0] = 0;
		item->_value[0]		= 0;
		item->value			= value;
		item->setValue		= setValue;
		item->inputType		= inputType;
		item->limitLength	= limitLength;
	} else	i = -1;
	return i;
}

char *PStextFieldValueAtIndex(void *self, int index)
{
	PS_TEXT_FIELD	*ps = (PS_TEXT_FIELD *)self;
	PS_TEXT_FIELD_ITEM	*item;
	char	*p;
	int		i;

	for(i = 0, item = ps->items;i < PS_TEXT_FIELD_SZ;i++, item++)
		if(!item->title[0]) break;
	if(index < i) {
		if(item->value) p = (*item->value)(PScontext(ps));
		else	p = item->_value;
	} else	p = NULL;
	return p;
}

void PStextFieldSetValueAtIndex(void *self, int index, char *value)
{
	PS_TEXT_FIELD	*ps = (PS_TEXT_FIELD *)self;
	PS_TEXT_FIELD_ITEM	*item;
	int		i;

	for(i = 0, item = ps->items;i < PS_TEXT_FIELD_SZ;i++, item++)
		if(!item->title[0]) break;
	if(index < i) {
		if(item->setValue) (*item->setValue)(PScontext(ps), value);
		else if(value) strcpy(item->_value, value);
		else	item->_value[0] = 0;
	}
}

void PStextFieldAllValue(void *self, char *buf)
{
	PS_TEXT_FIELD	*ps = (PS_TEXT_FIELD *)self;
	PS_TEXT_FIELD_ITEM	*item;
	char	*p, *s;
	int		i;

	p = buf;
	for(i = 0, item = ps->items;i < PS_TEXT_FIELD_SZ;i++, item++) {
		if(!item->title[0]) break;
		if(item->value) s = (*item->value)(PScontext(ps));
		else	s = item->_value;
		strcpy(p, s); p += strlen(p);
		*p++ = ' ';
	}
	if(i && (*(p-1)) == ' ') p--;
	*p = 0;
}

int PStextFieldIsPassword(void *self)
{
	PS_TEXT_FIELD	*ps = (PS_TEXT_FIELD *)self;
	PS_TEXT_FIELD_ITEM	*item;
	BOOL	bVal;
	int		i;

	bVal = FALSE;
	for(i = 0, item = ps->items;i < PS_TEXT_FIELD_SZ;i++, item++) {
		if(!item->title[0]) break;
		if(item->inputType == 1 || item->inputType == 3) {
			bVal = TRUE;
			break;
		}
	}
	return bVal;
}

int PStextFieldInputTypeAtIndex(void *self, int index)
{
	PS_TEXT_FIELD	*ps = (PS_TEXT_FIELD *)self;
	PS_TEXT_FIELD_ITEM	*item;
	int		i;

	for(i = 0, item = ps->items;i < PS_TEXT_FIELD_SZ;i++, item++)
		if(!item->title[0]) break;
	if(index < i) i = ps->items[index].inputType;
	else	i  -1;
	return i;
}

void PStextFieldSetInputTypeAtIndex(void *self, int index, int inputType)
{
	PS_TEXT_FIELD	*ps = (PS_TEXT_FIELD *)self;
	PS_TEXT_FIELD_ITEM	*item;
	int		i;

	for(i = 0, item = ps->items;i < PS_TEXT_FIELD_SZ;i++, item++)
		if(!item->title[0]) break;
	if(index < i) ps->items[index].inputType = inputType;
}

BOOL (*PStextFieldDoneValidate(void *self))(char *, char *)
{
	PS_TEXT_FIELD	*ps = (PS_TEXT_FIELD *)self;

	return ps->doneValidate;
}

void PStextFieldEncode(void *self, char *buf)
{
	PS_TEXT_FIELD	*ps = (PS_TEXT_FIELD *)self;
	PS_TEXT_FIELD_ITEM	*item;
	char	*p, *s;
	int		i;

	p = buf;
	sprintf(p, "%s=0\n", PStitle(ps)); p += strlen(p);
	for(i = 0, item = ps->items;i < PS_TEXT_FIELD_SZ;i++, item++) {
		if(!item->title[0]) break;
		if(item->value) s = (*item->value)(PScontext(ps));
		else	s = item->_value;
		sprintf(p, "%s=%s\n", item->title, s); p += strlen(p);
		sprintf(p, "%d,%d\n", (int)item->inputType, (int)item->limitLength); p += strlen(p);
	}
}

void PStextFieldDecode(void *self, char *buf)
{
	PS_TEXT_FIELD	*ps = (PS_TEXT_FIELD *)self;
	PS_TEXT_FIELD_ITEM	*item;
	char	*p, key[PS_TEXT_SZ], temp[PS_TEXT_SZ];
	int		i;
	
	p = buf;
	for(i = 0, item = ps->items;i < PS_TEXT_FIELD_SZ;i++, item++) {
		if(!item->title[0]) break;
		p = get_keyvalue(p, key, temp);
		if(item->setValue) (*item->setValue)(PScontext(ps), temp);
		else	strcpy(item->_value, temp);
	}
}

void *PSspinnerAlloc(void)
{
	return  PSalloc(sizeof(PS_SPINNER));
}

void PSspinnerInit(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int))
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	int		i;

	PSinit(self, PS_TYPE_SPINNER, title);
	ps->value = value;
	ps->setValue = setValue;
	for(i = 0;i < PS_SPINNER_SZ;i++) ps->titles[i][0] = 0;
	for(i = 0;i < PS_SPINNER_SZ;i++) ps->values[i] = 0;
	ps->enables = 0L;
}

int PSspinnerItemCount(void *self)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	int		i, count;

	for(i = count = 0;i < PS_SPINNER_SZ;i++) {
		if(!ps->titles[i][0]) break;
		count++;
	}
	return count;
}

int PSspinnerAddTitle(void *self, char *title)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	int		i;

	for(i = 0;i < PS_SPINNER_SZ;i++)
		if(!ps->titles[i][0]) break;
	if(i < PS_SPINNER_SZ) {
		if(title) strcpy(ps->titles[i], title); else ps->titles[i][0] = 0;
		ps->values[i] = i;
		ps->enables |= 0x80000000 >> i;	
	} else	i = -1;
	return i;
}

int PSspinnerAddTitleValue(void *self, char *title, int value)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	int		i;

	for(i = 0;i < PS_SPINNER_SZ;i++)
		if(!ps->titles[i][0]) break;
	if(i < PS_SPINNER_SZ) {
		if(title) strcpy(ps->titles[i], title); else ps->titles[i][0] = 0;
		ps->values[i] = value;		
		ps->enables |= 0x80000000 >> i;	
	} else	i = -1;
	return i;
}

int PSspinnerValue(void *self)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	int		val;

	if(ps->value) val = (*ps->value)(PScontext(ps));
	else	val = ps->_value;
	return val;
}

void PSspinnerSetValue(void *self, int value)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;

	if(ps->setValue) {
		if(ps->value) ps->_value = (*ps->value)(PScontext(ps));
		(*ps->setValue)(PScontext(ps), value);
	} else	ps->_value = value;
}

void PSspinnerRestoreValue(void *self)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	int		val;

printf("PSspinnerRestoreValue...\n");
	if(ps->setValue) {
		val = ps->_value;
		if(ps->value) ps->_value = (*ps->value)(PScontext(ps));
		(*ps->setValue)(PScontext(ps), val);
	}
}

char *PSspinnerTitleAtIndex(void *self, int index)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	char	*p;
	int		i;

	for(i = 0;i < PS_SPINNER_SZ;i++)
		if(!ps->titles[i][0]) break;
	if(index < i) p = ps->titles[index];
	else	p = NULL;
	return p;
}

int PSspinnerValuetAtIndex(void *self, int index)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	int		i, value;

	for(i = 0;i < PS_SPINNER_SZ;i++)
		if(!ps->titles[i][0]) break;
	if(index < i) value = ps->values[index];
	else	value = -1;
	return value;
}

void PSspinnerSetTitleValueAtIndex(void *self, char *title, int value, int index)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	int		i;

	for(i = 0;i < PS_SPINNER_SZ;i++)
		if(!ps->titles[i][0]) break;
	if(index < i) {
		strcpy(ps->titles[index], title);
		ps->values[index] = value;
	}
}

BOOL PSspinnerEnableAtIndex(void *self, int index)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	BOOL	bVal;
	int		i;

	for(i = 0;i < PS_SPINNER_SZ;i++)
		if(!ps->titles[i][0]) break;
	if(index < i) {
		if(ps->enables & (0x80000000 >> index)) bVal = TRUE; else bVal = FALSE;
	} else	bVal = FALSE;
	return bVal;
}

void PSspinnerSetEnableAtIndex(void *self, BOOL enable, int index)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	int		i;

	for(i = 0;i < PS_SPINNER_SZ;i++)
		if(!ps->titles[i][0]) break;
	if(index < i) {
		if(enable) ps->enables |= 0x80000000 >> index;
		else	   ps->enables &= ~(0x80000000 >> index);
	}
}

int PSspinnerIndexWithValue(void *self, int value)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	int		i, index;

	index = -1;
	for(i = 0;i < PS_SPINNER_SZ;i++) {
		if(!ps->titles[i][0]) break;
		if(ps->values[i] == value) {
			index = i;
			break;
		}
	}
	return index;
}

int PSspinnerEnableIndexWithValue(void *self, int value)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	int		i, index;

	index = -1;
	for(i = 0;i < PS_SPINNER_SZ;i++) {
		if(!ps->titles[i][0]) break;
		if(PSspinnerEnableAtIndex(ps, i) && ps->values[i] == value) {
			index = i;
			break;
		}
	}
	return index;
}

void PSspinnerEncode(void *self, char *buf)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	PS_SPINNER	*ps2;
	char	*p;
	int		i, val, count;

	p = buf;
	sprintf(p, "%s=%d\n", PStitle(ps), PSspinnerValue(ps)); p += strlen(p);
	for(i = count = 0;i < PS_SPINNER_SZ;i++) {
		if(!ps->titles[i][0]) break;
		if(PSspinnerEnableAtIndex(ps, i)) {
			sprintf(p, "%s,", ps->titles[i], i); p += strlen(p);
			count++;
		}
	}
	if(count) *(p-1) = '\n';
	for(i = 0;i < PS_SPINNER_SZ;i++) {
		if(!ps->titles[i][0]) break;
		if(PSspinnerEnableAtIndex(ps, i)) {
			sprintf(p, "%d,", ps->values[i], i); p += strlen(p);
		}
	}
	if(count) *(p-1) = '\n';
	else	*p++ = '\n';
	*p = 0;
}

BOOL PSspinnerDecode(void *self, char *buf)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	char	key[PS_TEXT_SZ], value[PS_TEXT_SZ];
	BOOL	bChanged;
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(PSspinnerValue(ps) == val) bChanged = FALSE;
	else {
		PSspinnerSetValue(ps, val); bChanged = TRUE;
	}
	return bChanged;
}

void PSspinnerDecode2(void *self, char *buf)
{
	PS_SPINNER	*ps = (PS_SPINNER *)self;
	char	key[PS_TEXT_SZ], value[PS_TEXT_SZ];
	int		val, count;

	get_keyvalue(buf, key, value);
printf("key=[%s] value=[%s]\n", key, value);
	val = n_atoi(value);
printf("val=%d\n", val);
	PSspinnerSetValue(ps, val);
}

void *PScheckListAlloc(void)
{
	return  PSalloc(sizeof(PS_CHECK_LIST));
}

void PScheckListInit(void *self, char *title)
{
	PS_CHECK_LIST	*ps = (PS_CHECK_LIST *)self;
	int		i;

	PSinit(self, PS_TYPE_CHECK_LIST, title);
	for(i = 0;i < PS_CHECK_LIST_SZ;i++) {
		ps->titles[i][0] = 0; ps->values[i] = FALSE;
	}
}

int PScheckListAddTitle(void *self, char *title, BOOL value)
{
	PS_CHECK_LIST	*ps = (PS_CHECK_LIST *)self;
	int		i;

	for(i = 0;i < PS_CHECK_LIST_SZ;i++)
		if(!ps->titles[i][0]) break;
	if(i < PS_CHECK_LIST_SZ) {
		if(title) strcpy(ps->titles[i], title); else ps->titles[i][0] = 0;
		ps->values[i] = value;
	} else	i = -1;
	return i;
}

char *PScheckListTitleAtIndex(void *self, int index)
{
	PS_CHECK_LIST	*ps = (PS_CHECK_LIST *)self;
	char	*p;
	int		i;

	for(i = 0;i < PS_CHECK_LIST_SZ;i++)
		if(!ps->titles[i][0]) break;
	if(index < i) p = ps->titles[index];
	else	p = NULL;
	return p;
}

BOOL PScheckListValueAtIndex(void *self, int index)
{
	PS_CHECK_LIST	*ps = (PS_CHECK_LIST *)self;
	BOOL	bVal;
	int		i;

	for(i = 0;i < PS_CHECK_LIST_SZ;i++)
		if(!ps->titles[i][0]) break;
	if(index < i) bVal = ps->values[index];
	else	bVal = FALSE;
	return bVal;
}

void PScheckListSetValueAtIndex(void *self, BOOL value, int index)
{
	PS_CHECK_LIST	*ps = (PS_CHECK_LIST *)self;
	BOOL	bVal;
	int		i;

	for(i = 0;i < PS_CHECK_LIST_SZ;i++)
		if(!ps->titles[i][0]) break;
	if(index < i) ps->values[index] = value;
}

int PScheckListCount(void *self)
{
	PS_CHECK_LIST	*ps = (PS_CHECK_LIST *)self;
	int		i, count;

	for(i = count = 0;i < PS_CHECK_LIST_SZ;i++) {
		if(!ps->titles[i][0]) break;
		count++;
	}
	return count;
}

void PScheckListEncode(void *self, char *buf)
{
	PS_CHECK_LIST	*ps = (PS_CHECK_LIST *)self;
	char	*p;
	int		i, count;

	p = buf;
	sprintf(p, "%s=0\n", PStitle(ps)); p += strlen(p);
	for(i = count = 0;i < PS_CHECK_LIST_SZ;i++) {
		if(!ps->titles[i][0]) break;
		sprintf(p, "%s,", ps->titles[i]); p += strlen(p);
		count++;
	}
	if(count) *(p-1) = '\n';
	for(i = 0;i < PS_CHECK_LIST_SZ;i++) {
		if(!ps->titles[i][0]) break;
		if(ps->values[i]) *p++ = '1'; else *p++ = '0';
		*p++ = ',';
	}
	if(count) *(p-1) = '\n';
	else	*p++ = '\n';
	*p = 0;
}

BOOL PScheckListDecode(void *self, char *buf)
{
	PS_CHECK_LIST	*ps = (PS_CHECK_LIST *)self;
	char	*p, temp[PS_TEXT_SZ], value[PS_TEXT_SZ];
	BOOL	bChanged, bVal;
	int		i, val;

	get_keyvalue(buf, temp, value);
	bChanged = FALSE;
	p = value;
	for(i = 0;p && i < PS_CHECK_LIST_SZ;i++) {
		p = read_token_ch(p, temp, ',');
		val = n_atoi(temp);
		if(val) bVal = TRUE; else bVal = FALSE;
		if(ps->values[i] != bVal) {
			ps->values[i] = bVal; bChanged = TRUE;
		}
	}
	return bChanged;
}

void *PSpickerAlloc(void)
{
	return  PSalloc(sizeof(PS_PICKER));
}

void PSpickerInit(void *self, char *title, void (*getValues)(void *, int *), void (*setValues)(void *, int *))
{
	PS_PICKER	*ps = (PS_PICKER *)self;
	int		i;

	PSinit(self, PS_TYPE_PICKER, title);
	ps->getValues	= getValues;
	ps->setValues	= setValues;
	for(i = 0;i < PS_PICKER_SZ;i++) ps->items[i].title[0] = 0;
}

int PSpickerAddItem(void *self, char *title)
{
	PS_PICKER	*ps = (PS_PICKER *)self;
	PS_PICKER_ITEM	*item;
	int		i, j;

	for(i = 0, item = ps->items;i < PS_PICKER_SZ;i++, item++)
		if(!item->title[0]) break;
	if(i < PS_PICKER_SZ) {
		if(title) strcpy(item->title, title); else item->title[0] = 0;
		item->_value = 0;
		for(j = 0;j < PS_PICKER_ITEM_SZ;j++) item->titles[j][0] = 0;
	} else	i = -1;
	return i;
}

int PSpickerAtIndexAddTitle(void *self, int index, char *title)
{
	PS_PICKER	*ps = (PS_PICKER *)self;
	PS_PICKER_ITEM	*item;
	int		i;

	for(i = 0, item = ps->items;i < PS_PICKER_SZ;i++, item++)
		if(!item->title[0]) break;
	if(index >= i) return -1;
	item = &ps->items[index];
	for(i = 0;i < PS_PICKER_ITEM_SZ;i++)
		if(!item->titles[i][0]) break;
	if(i < PS_PICKER_ITEM_SZ) {
		if(title) strcpy(item->titles[i], title); else item->titles[i][0] = 0;
	} else	i = -1;
	return i;
}

void PSpickerGetValues(void *self, int *values)
{
	PS_PICKER	*ps = (PS_PICKER *)self;
	PS_PICKER_ITEM	*item;
	int		i;

	if(ps->getValues) (*ps->getValues)(PScontext(ps), values);
	else {
		for(i = 0, item = ps->items;i < PS_PICKER_SZ;i++, item++) {
			if(!item->title[0]) break;
			values[i] = item->_value;
		}
	}
}

void PSpickerSetValues(void *self, int *values)
{
	PS_PICKER	*ps = (PS_PICKER *)self;
	PS_PICKER_ITEM	*item;
	int		i;

	if(ps->setValues) (*ps->setValues)(PScontext(ps), values);
	else {
		for(i = 0, item = ps->items;i < PS_PICKER_SZ;i++, item++) {
			if(!item->title[0]) break;
			item->_value = values[i];
		}
	}
}

void PSpickerValuesTitle(void *self, char *buf)
{
	PS_PICKER	*ps = (PS_PICKER *)self;
	PS_PICKER_ITEM	*item;
	char	*p;
	int		i, values[PS_PICKER_SZ];

	PSpickerGetValues(ps, values);
	p = buf;
	for(i = 0, item = ps->items;i < PS_PICKER_SZ;i++, item++) {
		if(!item->title[0]) break;
		strcpy(p, item->titles[values[i]]); p += strlen(p);
		*p++ = ' ';
	}
	if(i && (*(p-1)) == ' ') p--;
	*p = 0;
} 

void PSpickerEncode(void *self, char *buf)
{
	PS_PICKER	*ps = (PS_PICKER *)self;
	PS_PICKER_ITEM	*item;
	char	*p;
	int		i, j, values[PS_PICKER_SZ];

	PSpickerGetValues(ps, values);
	p = buf;
	sprintf(p, "%s=0\n", PStitle(ps)); p += strlen(p);
	for(i = 0, item = ps->items;i < PS_PICKER_SZ;i++, item++) {
		if(!item->title[0]) break;
		sprintf(p, "%s=%d\n", item->title, values[i]); p += strlen(p);
		for(j = 0;j < PS_PICKER_ITEM_SZ;j++) {
			if(!item->titles[j][0]) break;
			sprintf(p, "%s,", item->titles[j]); p += strlen(p);
		}
		*(p-1) = '\n';
	}
}

BOOL PSpickerDecode(void *self, char *buf)
{
	PS_PICKER	*ps = (PS_PICKER *)self;
	PS_PICKER_ITEM	*item;
	char	*p, key[PS_TEXT_SZ], value[PS_TEXT_SZ];
	BOOL	bChanged;
	int		i, count, oldValues[PS_PICKER_SZ], newValues[PS_PICKER_SZ];

	p = buf;
	for(i = 0, item = ps->items;i < PS_PICKER_SZ;i++, item++) {
		if(!item->title[0]) break;
		p = get_keyvalue(p, key, value);
		newValues[i] = n_atoi(value);
	}
	count = i;
	PSpickerGetValues(ps, oldValues);
	for(i = 0;i < count;i++)
		if(oldValues[i] != newValues[i]) break;  
	if(i < count) {
		PSpickerSetValues(ps, newValues);
		bChanged = TRUE;
	} else	bChanged = FALSE;
	return bChanged;
}

void *PStimeRangeAlloc(void)
{
	return  PSalloc(sizeof(PS_TIME_RANGE));
}

void PStimeRangeInit(void *self, char *title, unsigned char *value)
{
	PS_TIME_RANGE	*ps = (PS_TIME_RANGE *)self;
	int		i;

	PSinit(self, PS_TYPE_TIME_RANGE, title);
	memcpy(ps->value, value, 4);
}

void PStimeRangeValue(void *self, unsigned char *value)
{
	PS_TIME_RANGE	*ps = (PS_TIME_RANGE *)self;

	memcpy(value, ps->value, 4);
}

void PStimeRangeAllValueTitle(void *self, char *buf)
{
	PS_TIME_RANGE	*ps = (PS_TIME_RANGE *)self;

	sprintf(buf, "%02d:%02d - %02d:%02d", (int)ps->value[0], (int)ps->value[1], (int)ps->value[2], (int)ps->value[3]);
}

void PStimeRangeEncode(void *self, char *buf)
{
	PS_TIME_RANGE	*ps = (PS_TIME_RANGE *)self;

	sprintf(buf, "timeRange=%02d%02d%02d%02d\n", (int)ps->value[0], (int)ps->value[1], (int)ps->value[2], (int)ps->value[3]);
}

BOOL PStimeRangeDecode(void *self, char *buf)
{
	PS_TIME_RANGE	*ps = (PS_TIME_RANGE *)self;
	char	temp[PS_TEXT_SZ], value[PS_TEXT_SZ];
	unsigned char	val[4];
	BOOL	bChanged;

	bChanged = FALSE;
	get_keyvalue(buf, temp, value);
	temp[2] = 0;
	memcpy(temp, value, 2);   val[0] = n_atoi(temp);
	memcpy(temp, value+2, 2); val[1] = n_atoi(temp);
	memcpy(temp, value+4, 2); val[2] = n_atoi(temp);
	memcpy(temp, value+6, 2); val[3] = n_atoi(temp);
	if(memcmp(ps->value, val, 4)) {
		memcpy(ps->value, val, 4); bChanged = TRUE;
//printf("timeRangeDecode:%02d:%02d - %02d:%02d\n", (int)ps->value[0], (int)ps->value[1], (int)ps->value[2], (int)ps->value[3]);
	}
	return bChanged;
}

void *PSsliderGroupAlloc(void)
{
	return  PSalloc(sizeof(PS_SLIDER_GROUP));
}

void PSsliderGroupInit(void *self, char *title)
{
	PS_SLIDER_GROUP	*ps = (PS_SLIDER_GROUP *)self;
	int		i;

	PSinit(self, PS_TYPE_SLIDER_GROUP, title);
	for(i = 0;i < PS_SLIDER_GROUP_SZ;i++) ps->items[i].title[0] = 0;
}

int PSsliderGroupAddItem(void *self, char *title, int value, int minValue, int maxValue)
{
	PS_SLIDER_GROUP	*ps = (PS_SLIDER_GROUP *)self;
	PS_SLIDER_ITEM	*item;
	int		i;

	for(i = 0, item = ps->items;i < PS_SLIDER_GROUP_SZ;i++, item++)
		if(!item->title[0]) break;
	if(i < PS_SLIDER_GROUP_SZ) {
		if(title) strcpy(item->title, title); else item->title[0] = 0;
		if(value < minValue) value = minValue;
		else if(value > maxValue) value = maxValue;
		item->value			= value;
		item->minimumValue	= minValue;
		item->maximumValue	= maxValue;
	} else	i = -1;
	return  i;
}

int PSsliderGroupValueAtIndex(void *self, int index)
{
	PS_SLIDER_GROUP	*ps = (PS_SLIDER_GROUP *)self;
	PS_SLIDER_ITEM	*item;
	int		i, val;

	for(i = 0, item = ps->items;i < PS_SLIDER_GROUP_SZ;i++, item++)
		if(!item->title[0]) break;
	if(index < i) val = ps->items[index].value;
	else	val = -1;
	return val;
}

void PSsliderGroupEncode(void *self, char *buf)
{
	PS_SLIDER_GROUP	*ps = (PS_SLIDER_GROUP *)self;
	PS_SLIDER_ITEM	*item;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=0\n", PStitle(ps)); p += strlen(p);
	for(i = 0, item = ps->items;i < PS_SLIDER_GROUP_SZ;i++, item++) {
		if(!item->title[0]) break;
		sprintf(p, "%s=%d\n", item->title, item->value); p += strlen(p);
		sprintf(p, "%d,%d\n", item->minimumValue, item->maximumValue); p += strlen(p);
	}
}

BOOL PSsliderGroupDecode(void *self, char *buf)
{
	PS_SLIDER_GROUP	*ps = (PS_SLIDER_GROUP *)self;
	PS_SLIDER_ITEM	*item;
	char	*p, key[PS_TEXT_SZ], value[PS_TEXT_SZ];
	BOOL	bChanged;
	int		i, val;

	p = buf;
	bChanged = FALSE;
	for(i = 0, item = ps->items;i < PS_SLIDER_GROUP_SZ;i++, item++) {
		if(!item->title[0]) break;
		p = get_keyvalue(p, key, value);
		val = n_atoi(value);
		if(item->value != val) {
			item->value = val; bChanged = TRUE;
		}
//printf("%d [%s=%d]\n", i, key, item->value);
	}
	return bChanged;
}

