#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ctfont.h"
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "uiview_.h"
#include "UIStepper.h"
#include "uistepper_.h"


static void UIStepperClass_izer(void *self, va_list *app)
{
	struct UIStepperClass *cls = self;

//printf("UIStepperClass_izer...\n");
	super_izer(UIStepperClass, cls, app);
}

static void UIStepper_izerwf(void * self, va_list *app)
{
	struct UIStepper	*_self = self;
	CGRect	rt;

	super_izerwf(UIStepper, _self, app);
	UIframe(_self, &rt);
	rt.width	= 110;	// 94+16
	rt.height	= 45;	// 29+16
	UIsetFrame(_self, &rt);
	UIsetOpaque(self, FALSE);;	
	UIsetTintColor(self, systemGreen);
	_self->value		= 0;
	_self->minimumValue	= 0;
	_self->maximumValue	= 100;
	_self->flags		= 0x03;
	_self->stepValue	= 1;
}

static void UIStepper_draw(void *self, CGRect *rect)
{
	struct UIStepper *_self = self;
	CGRect		rt;
	CGPoint		pt;
	UIColor		color;
	int		x, y, w, h, cw, ch, dx, dy;

	UIframe(_self, &rt);
	pt.x = pt.y = 0;
	UIconvertPointToScreen(_self, &pt, (CGPoint *)&rt);
	lcdPixelClipToRect(&rt, rect);
	x = rt.x; y = rt.y; w = rt.width; h = rt.height;
	dx = _scale(94); dy = _scale(29);
	x += (w - dx) >> 1; y += (h - dy) >> 1;
	w = dx; h = dy;
	color = UItintColor(_self);
	lcdRect(x, y, w, h, color);
	lcdRect(x+1, y+1, w-2, h-2, color);  
	w >>= 1;
	lcdVerticalLine(x+w, y+1, h-2, color);
	lcdVerticalLine(x+w+1, y+1, h-2, color);
	cw = _scale(17); ch = _scale(3);
	dx = (w-cw)>>1; dy = (h-ch)>>1;
	lcdFillRect(x+dx, y+dy, cw, ch, color);
	x += w;
	lcdFillRect(x+dx, y+dy, cw, ch, color);
	dx = (w-ch)>>1; dy = (h-cw)>>1;
	lcdFillRect(x+dx, y+dy, ch, cw, color);
}

int _StepperClick(void *self, CGPoint *touchPt)
{
	struct UIStepper *_self = self;
	CGRect		rt;
	CGPoint		pt;
	int		val;
	
	UIconvertPointFromScreen(_self, &pt, touchPt);
	UIframe(_self, &rt);
	val = _self->value;;
	if(pt.x >= 0 && pt.x < (rt.width >> 1)) {
		if(_self->stepValue >= 0) {
			if(val <= _self->minimumValue) {
				if(UIwraps(_self)) val = _self->maximumValue;
			} else {
				val -= _self->stepValue;
				if(val < _self->minimumValue) val = _self->minimumValue;
			}
		} else{
			if(val >= _self->maximumValue) {
				if(UIwraps(_self)) val = _self->minimumValue;
			} else {
				val -= _self->stepValue;;
				if(val > _self->maximumValue) val = _self->maximumValue;
			}
		}
	} else {
		if(_self->stepValue >= 0) {
			if(val >= _self->maximumValue) {
				if(UIwraps(_self)) val = _self->minimumValue;
			} else {
				val += _self->stepValue;;
				if(val > _self->maximumValue) val = _self->maximumValue;
			}
		} else {
			if(val <= _self->minimumValue) {
				if(UIwraps(_self)) val = _self->maximumValue;
			} else {
				val += _self->stepValue;
				if(val < _self->minimumValue) val = _self->minimumValue;
			}
		}
	}
	if(val != _self->value) {
		_self->value = val;
		UIsetNeedsDisplay(_self);
		val = 1;
	} else	val = 0;
	return val;
}

struct NSClass		_UIStepperClass;
struct UIStepperClass	_UIStepper;

void *UIStepperClass, *UIStepper;

void initUIStepper(void)
{
	if(!UIStepperClass) {
		UIStepperClass = &_UIStepperClass;
		alloc(NSClass, UIStepperClass);
		init(UIStepperClass, "UIStepperClass", UIControlClass, sizeof(struct UIStepperClass),
		izer, UIStepperClass_izer,
		0);
	}
	if(!UIStepper) {
		UIStepper = &_UIStepper;
		alloc(UIStepperClass, UIStepper);
		init(UIStepper, "UIStepper", UIControl, sizeof(struct UIStepper),
		izerwf, UIStepper_izerwf, 
		UIdraw, UIStepper_draw,
		0);
	}
}

int UIstepValue(void *self)
{
	struct UIStepper *_self = self;

	return _self->stepValue;
}

void UIsetStepValue(void *self, int stepValue)
{
	struct UIStepper *_self = self;

	_self->stepValue = stepValue;
		UIsetNeedsDisplay(_self);
}

BOOL UIisContinuous(void *self)
{
	struct UIStepper *_self = self;

	if(_self->flags & 0x01) return TRUE; else return FALSE;
}

void UIsetContinuous(void *self, BOOL continuous)
{
	struct UIStepper *_self = self;

	if(continuous) _self->flags += 0x01; else _self->flags &= 0xfe;	
}

BOOL UIautorepeat(void *self)
{
	struct UIStepper *_self = self;

	if(_self->flags & 0x02) return TRUE; else return FALSE;
}

void UIsetAutorepeat(void *self, BOOL autorepeat)
{
	struct UIStepper *_self = self;

	if(autorepeat) _self->flags += 0x02; else _self->flags &= 0xfd;	
}

BOOL UIwraps(void *self)
{
	struct UIStepper *_self = self;

	if(_self->flags & 0x04) return TRUE; else return FALSE;
}

void UIsetWraps(void *self, BOOL wraps)
{
	struct UIStepper *_self = self;

	if(wraps) _self->flags += 0x04; else _self->flags &= 0xfb;	
}
