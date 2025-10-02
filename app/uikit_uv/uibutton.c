#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "uiview_.h"
#include "UILabel.h"
#include "UIImage.h"
#include "UIImageView.h"
#include "UIButton.h"
#include "uiview_.h"
#include "uibutton_.h"


static void UIButtonClass_izer(void *self, va_list *app)
{
	struct UIButtonClass *cls = self;

//printf("UIButtonClass_izer...\n");
	super_izer(UIButtonClass, cls, app);
}

static void UIButton_izerwf(void * self, va_list *app)
{
	struct UIButton	*_self = self;
	CGRect	rt;
	void	*lbl, *imgv;

	super_izerwf(UIButton, _self, app);
	UIsetContentVerticalAlignment(self, UIContentVerticalAlignmentTop);
	UIsetContentHorizontalAlignment(self, UIContentHorizontalAlignmentLeft);
	UIframe(_self, &rt);
	lbl = &_self->_titleLabel; 
	alloc(UILabel, lbl);
	initWithFrame(lbl, 0, 0, rt.width, rt.height);
	UIsetTextAlignment(lbl, UITextAlignmentCenter);
	UIsetOpaque(lbl, FALSE);
//	UIsetBackgroundColor(lbl, clearColor);
	_ViewSetParent(lbl, _self);
	imgv = &_self->_imageView; 
	alloc(UIImageView, imgv);
	initWithFrame(imgv, 0, 0, rt.width, rt.height);
	UIsetOpaque(imgv, FALSE);
//	UIsetBackgroundColor(imgv, clearColor);
	_ViewSetParent(imgv, _self);
	_self->highlightedColor = systemBlue;
}

static void UIButton_draw(void *self, CGRect *rect)
{
	struct UIButton *_self = self;
	void	*v;

//	if(self->info & GV_OPAQUE) {
	super_UIdraw(UIButton, _self, rect);
	if(_self->_titleLabel.text[0]) {
		UIsetNeedsDisplay(&_self->_titleLabel);
		UIdrawChild(&_self->_titleLabel, _self, rect);
		//UIframe(&_self->_titleLabel, &rt);
		//rt.x = rect->x - rt.x; rt.y = rect->y - rt.y; rt.width = rect->width; rt.height = rect->height; 
		//UIdraw(&_self->_titleLabel, &rt);
	}
	if(_self->_imageView.image) {
		UIsetNeedsDisplay(&_self->_imageView);
		UIdrawChild(&_self->_imageView, _self, rect);
	}
}

struct NSClass			_UIButtonClass;
struct UIButtonClass	_UIButton;

void *UIButtonClass, *UIButton;

void initUIButton(void)
{
	if(!UIButtonClass) {
		UIButtonClass = &_UIButtonClass;
		alloc(NSClass, UIButtonClass);
		init(UIButtonClass, "UIButtonClass", UIControlClass, sizeof(struct UIButtonClass),
		izer, UIButtonClass_izer,
		0);
	}
	if(!UIButton) {
		UIButton = &_UIButton;
		alloc(UIButtonClass, UIButton);
		init(UIButton, "UIButton", UIControl, sizeof(struct UIButton),
		izerwf, UIButton_izerwf, 
		UIdraw, UIButton_draw,
		0);
	}
}

// Property
void *UItitleLabel(void *self)
{
	struct UIButton *_self = self;

	return &_self->_titleLabel;
}

void *UIimageView(void *self)
{
	struct UIButton *_self = self;

	return &_self->_imageView;
}

UIColor UIhighlightedColor(void *self)
{
	struct UIButton *_self = self;

	return (UIColor)_self->highlightedColor;
}

void UIsetHighlightedColor(void *self, UIColor highlightedColor)
{
	struct UIButton *_self = self;

	_self->highlightedColor = highlightedColor;
	UIsetNeedsDisplay(_self);
}

// Method
char *UItitle(void *self)
{
	struct UIButton *_self = self;

	return UItext(&_self->_titleLabel);
}

void UIsetTitle(void *self, char *title)
{
	struct UIButton *_self = self;

	UIsetText(&_self->_titleLabel, title);
	UIsetNeedsDisplay(_self);
}

UIColor UItitleColor(void *self)
{
	struct UIButton *_self = self;

	return UItextColor(&_self->_titleLabel);
}

void UIsetTitleColor(void *self, UIColor textColor)
{
	struct UIButton *_self = self;

	UIsetTextColor(&_self->_titleLabel, textColor);
	UIsetNeedsDisplay(_self);
}

UIColor UItitleShadowColor(void *self)
{
	struct UIButton *_self = self;

	return UIshadowColor(&_self->_titleLabel);
}

void UIsetTitleShadowColor(void *self, UIColor shadowColor)
{
	struct UIButton *_self = self;

	UIsetShadowColor(&_self->_titleLabel, shadowColor);
	UIsetNeedsDisplay(_self);
}

void *UIbuttonImage(void *self)
{
	struct UIButton *_self = self;

	return _self->_imageView.image;
}

void UIsetButtonImage(void *self, void *image)
{
	struct UIButton *_self = self;

	UIsetImage(&_self->_imageView, image);
	UIsetNeedsDisplay(_self);
}

void *UIbuttonHighlightedImage(void *self)
{
	struct UIButton *_self = self;

	return _self->_imageView.highlightedImage;
}

void UIsetButtonHighlightedImage(void *self, void *image)
{
	struct UIButton *_self = self;

	UIsetHighlightedImage(&_self->_imageView, image);
	UIsetNeedsDisplay(_self);
}

