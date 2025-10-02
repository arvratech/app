#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cgrect.h"
#include "UIKit.h"
#include "gesture.h"
#include "gactivity.h"


void activityInit(GACTIVITY *self)
{
	self->classType			= 0;
	self->window			= NULL;
	self->startForResult	= 0;
	self->isFinishing		= 0;
	self->requestCode		= 0;
	self->resultCode		= 0;
	self->intent[0]			= 0;
	self->param1[0]			= 0;
	self->param2[0]			= 0;
	self->context			= NULL;
	self->psContext			= NULL;
	self->viewBuffer		= NULL;
	self->timeout			= 0;
	self->onCreate = self->onStart = self->onStop = self->onDestroy	= NULL;
	self->onActivityResult	= NULL;
	self->onLongPress		= NULL;
	self->onTimer			= NULL;
	self->onClick			= NULL;
	self->onValueChanged	= NULL;
	self->onAppMessage		= NULL;
	self->onDown			= NULL;
	self->onTabUp			= NULL;
	self->onLongPress		= NULL;
	self->onScroll			= NULL;
	self->onFlying			= NULL;
	self->onTextFieldFocused = NULL;
	self->onDismiss			= NULL;
	self->touchView			= NULL;
}

/*
void activityReset(GACTIVITY *self)
{
	self->classType			= 0;
	self->startForResult	= 0;
	self->requestCode		= 0;
	self->resultCode		= 0;
	self->intent[0]			= 0;
	self->param1[0]			= 0;
	self->param2[0]			= 0;
	self->context			= NULL;
	self->psContext			= NULL;
	self->timeout			= 0;
	self->onStart			= NULL;
	self->onActivityResult	= NULL;
	self->onLongPress		= NULL;
	self->onTimer			= NULL;
	self->onClick			= NULL;
	self->onValueChanged	= NULL;
	self->onAppMessage		= NULL;
	self->onDown			= NULL;
	self->onTabUp			= NULL;
	self->onLongPress		= NULL;
	self->onScroll			= NULL;
	self->onFlying			= NULL;
	self->onTextFieldFocused = NULL;
	self->onDismiss			= NULL;
	self->touchView			= NULL;
}
*/

void *activityWindow(GACTIVITY *self)
{
	return self->window;
}

void activityAddView(GACTIVITY *self, void *view)
{
	if(self->window) UIaddChild(self->window, view);
}

int activityRequestCode(GACTIVITY *self)
{
	return (int)self->requestCode;
}

void activitySetResult(GACTIVITY *self, int resultCode, char *intent)
{
	self->resultCode = resultCode;
	if(intent && intent[0]) strcpy(self->intent, intent);
	else	self->intent[0] = 0;
}	

int activityResultCode(GACTIVITY *self)
{
	return (int)self->resultCode;
}

char *activityIntent(GACTIVITY *act)
{
	return act->intent;
}

char *activityParam1(GACTIVITY *self)
{
	return self->param1;
}

void activitySetParam1(GACTIVITY *self, char *param)
{
	if(param && param[0]) strcpy(self->param1, param);
	else	self->param1[0] = 0;
}

char *activityParam2(GACTIVITY *self)
{
	return self->param2;
}

void activitySetParam2(GACTIVITY *self, char *param)
{
	if(param && param[0]) strcpy(self->param2, param);
	else	self->param2[0] = 0;
}

void *activityContext(GACTIVITY *self)
{
	return self->context;
}

void activitySetContext(GACTIVITY *self, void *context)
{
	self->context = context;
}

void *activityPsContext(GACTIVITY *self)
{
	return self->psContext;
}

void activitySetPsContext(GACTIVITY *self, void *psContext)
{
	self->psContext = psContext;
}

void *activityViewBuffer(GACTIVITY *self)
{
	return self->viewBuffer;
}

void activitySetViewBuffer(GACTIVITY *self, void *viewBuffer)
{
	self->viewBuffer = viewBuffer;
}

void *_AppParentActivity(GACTIVITY *act);

void *activityParent(GACTIVITY *act)
{
	return _AppParentActivity(act);
}

int activityTimeout(GACTIVITY *self)
{
	return self->timeout;
}

void activitySetTimeout(GACTIVITY *self, int timeout)
{
	self->timeout = timeout;
}

int activityIsFinishing(GACTIVITY *self)
{
	return (int)self->isFinishing;
}

void activitySetOnCreate(GACTIVITY *self, void (*onCreate)(GACTIVITY *))
{
	self->onCreate = onCreate;
}

void activitySetOnStart(GACTIVITY *self, void (*onStart)(GACTIVITY *))
{
	self->onStart = onStart;
}

void activitySetOnStop(GACTIVITY *self, void (*onStop)(GACTIVITY *))
{
	self->onStop = onStop;
}

void activitySetOnDestroy(GACTIVITY *self, void (*onDestroy)(GACTIVITY *))
{
	self->onDestroy = onDestroy;
}

void activitySetOnActivityResult(GACTIVITY *self, void (*onActivityResult)(GACTIVITY *, int, int, char *))
{
	self->onActivityResult = onActivityResult;
}

void activitySetOnTimer(GACTIVITY *self, void (*onTimer)(GACTIVITY *, int))
{
	self->onTimer = onTimer;
}

void activitySetOnClick(GACTIVITY *self, void (*onClick)(GACTIVITY *, void *))
{
	self->onClick = onClick;
}

void activitySetOnValueChanged(GACTIVITY *self, void (*onValueChanged)(GACTIVITY *, void *))
{
	self->onValueChanged = onValueChanged;
}

void activitySetOnAppMessage(GACTIVITY *self, void (*onAppMessage)(GACTIVITY *, unsigned char *))
{
	self->onAppMessage = onAppMessage;
}

void activitySetOnDown(GACTIVITY *self, void (*onDown)(GACTIVITY *, void *, CGPoint *))
{
	self->onDown = onDown;
}

void activitySetOnTabUp(GACTIVITY *self, void (*onTabUp)(GACTIVITY *, void *))
{
	self->onTabUp = onTabUp;
}

void activitySetOnLongPress(GACTIVITY *self, void (*onLongPress)(GACTIVITY *, void *))
{
	self->onLongPress = onLongPress;
}

void activitySetOnScroll(GACTIVITY *self, void (*onScroll)(GACTIVITY *, void *, CGPoint *))
{
	self->onScroll = onScroll;
}

void activitySetOnFlying(GACTIVITY *self, void (*onFlying)(GACTIVITY *, void *, CGPoint *))
{
	self->onFlying = onFlying;
}

void activitySetOnTextFieldFocused(GACTIVITY *self, void (*onTextFieldFocused)(GACTIVITY *, void *, int))
{
	self->onTextFieldFocused = onTextFieldFocused;
}

void activitySetOnDismiss(GACTIVITY *self, void (*onDismiss)(GACTIVITY *))
{
	self->onDismiss = onDismiss;
}

void _TableViewScroll(void *self, int dy);
void _TableViewSelectRow(void *self, void *cell);
int  _TextFieldGetTextPosition(void *self, CGPoint *pt);
int  _SliderClick(void *self, CGPoint *pt);
int  _StepperClick(void *self, CGPoint *pt);
int  _PickerViewClick(void *self, int btnTag);
int  _DatePickerClick(void *self, int btnTag);
int  _TimePickerClick(void *self, int btnTag);
int  _TimeRangeClick(void *self, CGPoint *pt);

void _AudioPlay(int no)
{
	unsigned char	msg[4];

	audioFlush();
	msg[0] = no; msg[1] = appSoundVolume(); msg[2] = 0;
//printf("### app sound: %d vol=%d\n", (int)msg[0], (int)msg[1]);
	audioPost(msg);
}

#define TAG_VIEW_KEYPAD				50

int _CellIsSelectable(void *cell)
{
	void	*v;
	int		rval;

	v = UIfirstChild(cell);		// 2018.10.4 bug fixed: UIfirstChild(v);
	if(v && (class(v) == UIStepper || class(v) == UISlider)) rval = 0;
	else	rval = 1;
	return rval;
}

#include "rtc.h"
//#define _DEBUG		1

int activityProcGesture(GACTIVITY *self, GESTURE *gst)
{
	void	*v, *parent, *cls, *v2;
	CGRect	rect;
	CGPoint	pt, originPt;
	int		rval, temp;
	
	rval = 0;
	v = self->touchView;
	UIframe(self->window, &rect);
	originPt.x = rect.x; originPt.y = rect.y;
	switch(gst->type) {
	case ON_DOWN:
		appStopActivityTimer(self);
		pt.x = gst->x; pt.y = gst->y;
		v = UIhitTestView(self->window, &pt);
		self->touchView = v;
#ifdef _DEBUG
if(v) printf("%u onDown: %d,%d %s\n", MS_TIMER, pt.x, pt.y, className(v));
else  printf("%u onDown: %d,%d\n", MS_TIMER, pt.x, pt.y);
#endif
		if(v) {
			cls = class(v);
			if(cls == UIButton && UIisEnabled(v) || cls == UITableViewCell && _CellIsSelectable(v)) UIsetHighlighted(v, TRUE);
		}
		if(self->onDown) (*self->onDown)(self, v, &pt);
		break;
	case ON_TAB_UP:
		appStartActivityTimer(self);
#ifdef _DEBUG
if(v) printf("%ld onTabUp: %s\n", MS_TIMER, className(v));
else  printf("%ld onTabUp\n", MS_TIMER);
#endif
		if(v) {
			cls = class(v);
			if(cls == UIButton) {
				UIsetHighlighted(v, FALSE);
				parent = UIparent(v);
				cls = class(parent);
				if(cls == UIPickerView) {
					_PickerViewClick(parent, UItag(v));
				} else if(cls == UIDatePicker) {
					rval = _DatePickerClick(parent, UItag(v));
					if(rval && self->onValueChanged) (*self->onValueChanged)(self, parent);
				} else if(cls == UITimePicker) {
					rval = _TimePickerClick(parent, UItag(v));
					if(rval && self->onValueChanged) (*self->onValueChanged)(self, parent);
				} else {
					if(UItag(parent) == TAG_VIEW_KEYPAD) {
						if(appKeypadTone()) _AudioPlay(124);
					} else {
						if(appTouchSounds()) _AudioPlay(124);
					}
					if(self->onClick) (*self->onClick)(self, v);
				}
			} else if(cls == UITextField) {
				UIsetFocused(v, TRUE);
				pt.x = gst->x; pt.y = gst->y;
				rval = _TextFieldGetTextPosition(v, &pt);
				if(self->onTextFieldFocused) (*self->onTextFieldFocused)(self, v, rval);
			} else if(cls == UISwitch) {
				UIsetOn(v, !UIisOn(v));
				if(self->onValueChanged) (*self->onValueChanged)(self, v);
			} else if(cls == UISlider) {
				pt.x = gst->x; pt.y = gst->y;
				rval = _SliderClick(v, &pt);
				if(rval && self->onValueChanged) (*self->onValueChanged)(self, v);
			} else if(cls == UIStepper) {
				pt.x = gst->x; pt.y = gst->y;
				rval = _StepperClick(v, &pt);
				if(rval && self->onValueChanged) (*self->onValueChanged)(self, v);
		//	} else if(cls == UITimeRange) {
		//		pt.x = gst->x; pt.y = gst->y;
		//		rval = _TimeRangeClick(v, &pt);
		//		if(rval && self->onValueChanged) (*self->onValueChanged)(self, v);
			} else if(cls == UITableViewCell) {
				//v2 = UIaccessoryView(v);
				if(_CellIsSelectable(v)) {
					UIsetHighlighted(v, FALSE);
					parent = UIparent(v);
					_TableViewSelectRow(parent, v);
					if(appTouchSounds()) _AudioPlay(124);
				}
			} else {
				if(self->onTabUp) (*self->onTabUp)(self, v);
			}
		}
		self->touchView = NULL;
		break;
	case ON_SHOW_PRESS:
#ifdef _DEBUG
if(v) printf("%ld onShowPress: %s\n", MS_TIMER, className(v));
else  printf("%ld onShowPress\n", MS_TIMER);
#endif
		break;
	case ON_LONG_PRESS:
#ifdef _DEBUG
if(v) printf("onLongPress: %s\n", className(v));
else  printf("onLongPress\n");
#endif
		if(v) {
			if(self->onLongPress) (*self->onLongPress)(self, v);
		}
		break;
	case ON_SCROLL:
		pt.x = gst->x; pt.y = gst->y;
		if(self->onScroll) (*self->onScroll)(self, v, &pt);
#ifdef _DEBUG
if(v) printf("onScroll: %s %d,%d\n", className(v), pt.x, pt.y);
else  printf("onScroll: %d,%d\n", pt.x, pt.y);
#endif
		if(v) {
			cls = class(v);
			if((cls == UIButton || cls == UITableViewCell) && UIisHighlighted(v)) UIsetHighlighted(v, FALSE);
			if(cls == UITableViewCell) {
				parent = UIparent(v);
				if(v != parent) {
					self->touchView = parent; v = self->touchView;
				}
			}
			if(cls == UITableView && pt.y) _TableViewScroll(v, pt.y);
			else if(cls == UIScrollView) _ScrollViewScroll(v, pt.y);
		} else {
		}
		break;
	case ON_FLING:
		appStartActivityTimer(self);
		pt.x = gst->x; pt.y = gst->y;
		if(self->onFlying) (*self->onFlying)(self, v, &pt);
#ifdef _DEBUG
if(v) printf("onFlying: %s %d,%d\n", className(v), pt.x, pt.y);
else  printf("onFlying: %d, %d\n", pt.x, pt.y);
#endif
		break;
	}
	return 1;
}

void activityClearTouchView(GACTIVITY *self)
{
	self->touchView = self->window;
}

