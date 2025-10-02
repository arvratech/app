#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "uiview_.h"
#include "UISwitch.h"
#include "uiswitch_.h"


static void UISwitchClass_izer(void *self, va_list *app)
{
	struct UISwitchClass *cls = self;

//printf("UISwitchClass_izer...\n");
	super_izer(UISwitchClass, cls, app);
}

static void UISwitch_izerwf(void * self, va_list *app)
{
	struct UISwitch	*_self = self;
	CGRect	rt;

	super_izerwf(UISwitch, _self, app);
	UIframe(_self, &rt);
	rt.width	= 57;	// 41+16
	rt.height	= 40;	// 24+16
	UIsetFrame(_self, &rt);
	UIsetOpaque(self, FALSE);;	
	UIsetTintColor(self, systemGreen);
	//UIsetself->contentAlignment	= 0;
	UIsetContentVerticalAlignment(self, UIContentVerticalAlignmentTop);
	UIsetContentHorizontalAlignment(self, UIContentHorizontalAlignmentLeft);
	_self->onOff = 0;
}

static void UISwitch_draw(void *self, CGRect *rect)
{
	struct UISwitch *_self = self;
	CGRect		rt;
	CGPoint		pt;
	char	temp[16];
	int		x, y, w, h, track, thumb, dx, c1, c2;

	super_UIdraw(UISwitch, self, rect);
	UIframe(_self, &rt);
	pt.x = pt.y = 0;
	UIconvertPointToScreen(_self, &pt, (CGPoint *)&rt);
	lcdPixelClipToRect(&rt, rect);
	x = rt.x; y = rt.y; w = rt.width; h = rt.height;
	track = _scale(23); thumb = _scale(24);
	dx = _scale(17);
	x += (w - dx - thumb) >> 1;
	if(_self->onOff) {
		if(UIisEnabled(_self)) {
			c1 = 0xff59d05f; c2 = 0xff649a80;
		} else {
			c1 = TABLE_GREY_400; c2 = TABLE_GREY_350; 
		}
		lcdPutAlphaMaskFile(x, y+((h-track)>>1), track, track, c2 | (128<<24), "track.bmp");
		lcdPutAlphaMaskFile(x+dx, y+((h-thumb)>>1), thumb, thumb, c1, "circle.bmp");
	} else {
		if(UIisEnabled(_self)) {
			c1 = 0xffa3a3a3; c2 = 0xffa3a3a3;
		} else {
			c1 = TABLE_GREY_300; c2 = TABLE_GREY_350; 
		}
		lcdPutAlphaMaskFile(x+dx+thumb-track, y+((h-track)>>1), track, track, c2, "track.bmp");
		lcdPutAlphaMaskFile(x, y+((h-thumb)>>1), thumb, thumb, TABLE_GREY_50, "circle.bmp");
		lcdPutAlphaMaskFile(x, y+((h-thumb)>>1), thumb, thumb, c1, "circle_bdr.bmp");
	}
}

struct NSClass		_UISwitchClass;
struct UISwitchClass	_UISwitch;

void *UISwitchClass, *UISwitch;

void initUISwitch(void)
{
	if(!UISwitchClass) {
		UISwitchClass = &_UISwitchClass;
		alloc(NSClass, UISwitchClass);
		init(UISwitchClass, "UISwitchClass", UIControlClass, sizeof(struct UISwitchClass),
		izer, UISwitchClass_izer,
		0);
	}
	if(!UISwitch) {
		UISwitch = &_UISwitch;
		alloc(UISwitchClass, UISwitch);
		init(UISwitch, "UISwitch", UIControl, sizeof(struct UISwitch),
		izerwf, UISwitch_izerwf, 
		UIdraw, UISwitch_draw,
		0);
	}
}

BOOL UIisOn(void *self)
{
	struct UISwitch *_self = self;

	if(_self->onOff) return TRUE; else return FALSE;
}

void UIsetOn(void *self, BOOL on)
{
	struct UISwitch *_self = self;

	if(on) _self->onOff = 1; else _self->onOff = 0;
	UIsetNeedsDisplay(_self);
}

