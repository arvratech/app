#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "UIControl.h"
#include "uicontrol_.h"


static void UIControlClass_izer(void *self, va_list *app)
{
	struct UIControlClass *cls = self;

//printf("UIControlClass_izer...\n");
	super_izer(UIControlClass, cls, app);
}

static void UIControl_izerwf(void * self, va_list *app)
{
	struct UIControl	*_self = self;

	super_izerwf(UIControl, _self, app);
	_self->state			= 0;
	_self->contentAlignment	= 0;
}

struct NSClass			_UIControlClass;
struct UIControlClass	_UIControl;

void *UIControlClass, *UIControl;

void initUIControl(void)
{
	if(!UIControlClass) {
		UIControlClass = &_UIControlClass;
		alloc(NSClass, UIControlClass);
		init(UIControlClass, "UIControlClass", UIViewClass, sizeof(struct UIControlClass),
		izer, UIControlClass_izer,
		0);
	}
	if(!UIControl) {
		UIControl = &_UIControl;
		alloc(UIControlClass, UIControl);
		init(UIControl, "UIControl", UIView, sizeof(struct UIControl),
		izerwf, UIControl_izerwf, 
		0);
	}
}

UIControlState UIstate(void *self)
{
	struct UIControl	*_self = self;
	int		state;
	
	state = _self->state & 0x07;
	if(UIisFocused(self)) state |= 0x08;
	return (UIControlState)state;
}

BOOL UIisEnabled(void *self)
{
	struct UIControl	*_self = self;

	if(_self->state & UIC_DISABLED) return FALSE; else return TRUE;
}

void UIsetEnabled(void *self, BOOL isEnabled)
{
	struct UIControl	*_self = self;

	if(isEnabled) _self->state &= ~UIC_DISABLED;
	else	_self->state |= UIC_DISABLED;
	UIsetNeedsDisplay(_self);
}

BOOL UIisHighlighted(void *self)
{
	struct UIControl	*_self = self;

	if(_self->state & UIC_HIGHLIGHTED) return TRUE; else return FALSE;
}

void UIsetHighlighted(void *self, BOOL isHighlighted)
{
	struct UIControl	*_self = self;

	if(isHighlighted) _self->state |= UIC_HIGHLIGHTED;
	else	_self->state &= ~UIC_HIGHLIGHTED;
	UIsetNeedsDisplay(_self);
}

BOOL UIisSelected(void *self)
{
	struct UIControl	*_self = self;

	if(_self->state & UIC_SELECTED) return TRUE; else return FALSE;
}

void UIsetSelected(void *self, BOOL isSelected)
{
	struct UIControl	*_self = self;

	if(isSelected) _self->state |= UIC_SELECTED;
	else	_self->state &= ~UIC_SELECTED;
	UIsetNeedsDisplay(_self);
}

UIContentVerticalAlignment UIcontentVerticalAlignment(void *self)
{
	struct UIControl	*_self = self;

	return (UIContentVerticalAlignment)(_self->contentAlignment & 0xf);
}

void UIsetContentVerticalAlignment(void *self, UIContentVerticalAlignment contentVerticalAlignment)
{
	struct UIControl	*_self = self;

	_self->contentAlignment = (_self->contentAlignment  & 0xf0) | contentVerticalAlignment;
}

UIContentHorizontalAlignment UIcontentHorizontalAlignment(void *self)
{
	struct UIControl	*_self = self;

	return (UIContentHorizontalAlignment)((_self->contentAlignment >> 4) & 0xf);
}

void UIsetContentHorizontalAlignment(void *self, UIContentHorizontalAlignment contentHorizontalAlignment)
{
	struct UIControl	*_self = self;

	_self->contentAlignment = (_self->contentAlignment  & 0x0f) | (contentHorizontalAlignment << 4);
}

