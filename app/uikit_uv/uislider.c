#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "uiview_.h"
#include "UISlider.h"
#include "uislider_.h"


static void UISliderClass_izer(void *self, va_list *app)
{
	struct UISliderClass *cls = self;

//printf("UISliderClass_izer...\n");
	super_izer(UISliderClass, cls, app);
}

static void UISlider_izerwf(void * self, va_list *app)
{
	struct UISlider	*_self = self;
	CGRect	rt;
	int		v;

	super_izerwf(UISlider, _self, app);
	UIframe(_self, &rt);
	rt.height = 24;
	UIsetFrame(_self, &rt);
	UIsetOpaque(self, FALSE);;	
	UIsetTintColor(self, systemGreen);
//	UIsetBackgroundColor(self, blackColor);
	_self->value		= 0;
	_self->minimumValue	= 0;
	_self->maximumValue	= 100;
	_self->minimumTrackTintColor = grayColor;
	_self->maximumTrackTintColor = grayColor;
	_self->thumbTintColor = grayColor;
}

static void UISlider_draw(void *self, CGRect *rect)
{
	struct UISlider *_self = self;
	CGRect		rt;
	CGPoint		pt;
	int		x, y, w, h, tw, thumb;

UIsetBackgroundColor(_self, greenColor); 
	super_UIdraw(UISlider, _self, rect);
	UIframe(_self, &rt);
//printf("UISlider draw...%d,%d %d %d\n", rt.x, rt.y, rt.width, rt.height);
	pt.x = 0; pt.y = 0;
	UIconvertPointToScreen(_self, &pt, (CGPoint *)&rt);
	lcdPixelClipToRect(&rt, rect);
	x = rt.x + _scale(8); y = rt.y + (rt.height>>1); tw = rt.width - _scale(16);
	w = _self->maximumValue - _self->minimumValue;
	thumb = (tw * _self->value + (w>>1)) / w;
	w = _scale(4); h = _scale(8);
	lcdFillRect(x, y-(w>>1), thumb, w, _self->minimumTrackTintColor);
	lcdFillRect(x+thumb, y-(w>>1), tw-thumb, w, _self->maximumTrackTintColor);
	lcdPutAlphaMaskFile(x+thumb-h, y-h, h+h, h+h, _self->thumbTintColor, "circle.bmp"); 
}

int _SliderClick(void *self, CGPoint *pt)
{
	struct UISlider *_self = self;
	CGRect		rt;
	CGPoint		_pt;
	int		w, tw, thumb, val;
	
	UIframe(_self, &rt);
	_pt.x = 8; _pt.y = 0; rt.width -= 16;
	UIconvertPointToScreen(_self, &_pt, (CGPoint *)&rt);
	thumb = pt->x - rt.x;
	tw = rt.width;
	w = _self->maximumValue - _self->minimumValue;
	val = (w * thumb + (tw>>1)) / tw;
	if(val < _self->minimumValue) val = _self->minimumValue;
	else if(val > _self->maximumValue) val = _self->maximumValue;
	if(val != _self->value) {
		_self->value = val;
		UIsetNeedsDisplay(_self);
		val = 1;
	} else	val = 0;
	return val;
}

struct NSClass		_UISliderClass;
struct UISliderClass	_UISlider;

void *UISliderClass, *UISlider;

void initUISlider(void)
{
	if(!UISliderClass) {
		UISliderClass = &_UISliderClass;
		alloc(NSClass, UISliderClass);
		init(UISliderClass, "UISliderClass", UIControlClass, sizeof(struct UISliderClass),
		izer, UISliderClass_izer,
		0);
	}
	if(!UISlider) {
		UISlider = &_UISlider;
		alloc(UISliderClass, UISlider);
		init(UISlider, "UISlider", UIControl, sizeof(struct UISlider),
		izerwf, UISlider_izerwf, 
		UIdraw, UISlider_draw,
		0);
	}
}

int UIvalue(void *self)
{
	struct UISlider *_self = self;

	return _self->value;
}

void UIsetValue(void *self, int value)
{
	struct UISlider *_self = self;

	_self->value = value;
	UIsetNeedsDisplay(_self);
}

int UIminimumValue(void *self)
{
	struct UISlider *_self = self;

	return _self->minimumValue;
}

void UIsetMinimumValue(void *self, int minimumValue)
{
	struct UISlider *_self = self;

	_self->minimumValue = minimumValue;
	UIsetNeedsDisplay(_self);
}

int UImaximumValue(void *self)
{
	struct UISlider *_self = self;

	return _self->maximumValue;
}

void UIsetMaximumValue(void *self, int maximumValue)
{
	struct UISlider *_self = self;

	_self->maximumValue = maximumValue;
	UIsetNeedsDisplay(_self);
}

UIColor UIminimumTrackTintColor(void *self)
{
	struct UISlider *_self = self;

	return _self->minimumTrackTintColor;
}

void UIsetMinimumTrackTintColor(void *self, UIColor minimumTrackTintColor)
{
	struct UISlider *_self = self;

	_self->minimumTrackTintColor = minimumTrackTintColor;
	UIsetNeedsDisplay(_self);
}

UIColor UImaximumTrackTintColor(void *self)
{
	struct UISlider *_self = self;

	return _self->maximumTrackTintColor;
}

void UIsetMaximumTrackTintColor(void *self, UIColor maximumTrackTintColor)
{
	struct UISlider *_self = self;

	_self->maximumTrackTintColor = maximumTrackTintColor;
	UIsetNeedsDisplay(_self);
}

UIColor UIthumbTintColor(void *self)
{
	struct UISlider *_self = self;

	return _self->thumbTintColor;
}

void UIsetThumbTintColor(void *self, UIColor thumbTintColor)
{
	struct UISlider *_self = self;

	_self->thumbTintColor = thumbTintColor;
	UIsetNeedsDisplay(_self);
}

