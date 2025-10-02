#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ctfont.h"
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "uiview_.h"
#include "UILabel.h"
#include "uilabel_.h"


static void UILabelClass_izer(void *self, va_list *app)
{
	struct UILabelClass *cls = self;

//printf("UILabelClass_izer...\n");
	super_izer(UILabelClass, cls, app);
}

static void UILabel_izerwf(void * self, va_list *app)
{
	struct UILabel	*_self = self;
	int		font;

	super_izerwf(UILabel, _self, app);
	UIsetUserInteractionEnabled(_self, FALSE);
	_self->state			= 0;
	_self->text[0]			= 0;
	_self->font				= 20;
	_self->textColor		= ARGB(255,0,0,0);
	_self->highlightedTextColor	= 0;
	_self->shadowColor		= 0;
	_self->shadowOffsetX	= 0;
	_self->shadowOffsetY	= -1;
	_self->textAlignment	= UITextAlignmentNatural;
	_self->lineSpacing		= 0xff;
}

static void UILabel_draw(void *self, CGRect *rect)
{
	struct UILabel *_self = self;
	CGRect	rt;
	CGPoint	pt;
	char	*s, *p, temp[128];
	int		x, y, w, h, x0, c, dw, fh, dh, color, align;

//printf("UILabelDraw: [%s]\n", _self->text);
	super_UIdraw(UILabel, self, rect);
	s = _self->text;
	if(!s[0]) return;
	UIframe(_self, &rt);
	pt.x = pt.y = 0;
	UIconvertPointToScreen(_self, &pt, (CGPoint *)&rt);
	lcdPixelClipToRect(&rt, rect);
	x = rt.x; y = rt.y; w = rt.width; h = rt.height;
	align = UItextAlignment(self);
	if(UIisHighlighted(_self)) color = UIhighlightedTextColor(_self);
	else if(!UIisEnabled(self)) color = UItextColor(_self);
	else	color = UItextColor(_self);
	ftSetSize(UIfont(_self));
	fh = ftPixelHeight();
	c = UIlineSpacing(self);
	if(c == 0xff) dh = fh >> 2;
	else	dh = _scale(c);
	x0 = textLineCount(s);
	c = x0 * fh;
	if(x0 > 1) c += (x0-1) * dh;
	c = (h - c) >> 1; if(c < 0) c = 0;
	y += c;
	x0 = x;
	while(1) {
		p = temp;
		while(1) {
			c = *s++;
			if(!c || c == '\n') break;
			*p++ = c;
		}
		*p = 0;
		if(temp[0]) {
			x = x0;
			dw = ftTextPixelWidth(temp);
			if(dw <= w) {
				if(align == UITextAlignmentRight) x += w - dw;
				else if(align == UITextAlignmentCenter) x += (w - dw) >> 1; 
			}
			lcdPutText(x, y, temp, color);
		}
		if(!c) break;
		y += fh + dh;
	}
}

struct NSClass		_UILabelClass;
struct UILabelClass	_UILabel;

void *UILabelClass, *UILabel;

void initUILabel(void)
{
	if(!UILabelClass) {
		UILabelClass = &_UILabelClass;
		alloc(NSClass, UILabelClass);
		init(UILabelClass, "UILabelClass", UIViewClass, sizeof(struct UILabelClass),
		izer, UILabelClass_izer,
		0);
	}
	if(!UILabel) {
		UILabel = &_UILabel;
		alloc(UILabelClass, UILabel);
		init(UILabel, "UILabel", UIView, sizeof(struct UILabel),
		izerwf, UILabel_izerwf, 
		UIdraw, UILabel_draw,
		0);
	}
}

char *UItext(void *self)
{
	struct UILabel	*_self = self;

	return _self->text;
}

void UIsetText(void *self, char *text)
{
	struct UILabel	*_self = self;

	if(text && text[0]) strcpy(_self->text, text);
	else	_self->text[0] = 0;
	UIsetNeedsDisplay(self);
}

int UIfont(void *self)
{
	struct UILabel	*_self = self;

	return (int)_self->font;
}

void UIsetFont(void *self, int font)
{
	struct UILabel	*_self = self;

	_self->font = (unsigned char)font;
	UIsetNeedsDisplay(self);
}

UIColor UItextColor(void *self)
{
	struct UILabel	*_self = self;

	return _self->textColor;
}

void UIsetTextColor(void *self, UIColor textColor)
{
	struct UILabel	*_self = self;

	_self->textColor = textColor;
	UIsetNeedsDisplay(self);
}

UITextAlignment UItextAlignment(void *self)
{
	struct UILabel	*_self = self;

	return (UITextAlignment)_self->textAlignment;
}

void UIsetTextAlignment(void *self, UITextAlignment textAlignment)
{
	struct UILabel	*_self = self;

	_self->textAlignment = (unsigned char)textAlignment;
	UIsetNeedsDisplay(self);
}

int UIlineSpacing(void *self)
{
	struct UILabel	*_self = self;

	return (int)_self->lineSpacing;
}

void UIsetLineSpacing(void *self, int lineSpacing)
{
	struct UILabel	*_self = self;

	_self->lineSpacing = (unsigned char)lineSpacing;
	UIsetNeedsDisplay(self);
}

UIColor UIhighlightedTextColor(void *self)
{
	struct UILabel	*_self = self;

	return _self->highlightedTextColor;
}

void UIsetHighlightedTextColor(void *self, UIColor highlightedTextColor)
{
	struct UILabel	*_self = self;

	_self->highlightedTextColor = highlightedTextColor;
}

UIColor UIshadowColor(void *self)
{
	struct UILabel	*_self = self;

	return _self->shadowColor;
}

void UIsetShadowColor(void *self, UIColor shadowColor)
{
	struct UILabel	*_self = self;

	_self->shadowColor = shadowColor;
}

void UIshadowOffset(void *self, CGSize *shadowOffset)
{
	struct UILabel	*_self = self;

	shadowOffset->width = _self->shadowOffsetX; shadowOffset->height = _self->shadowOffsetY;
}

void UIsetShadowOffset(void *self, CGSize *shadowOffset)
{
	struct UILabel	*_self = self;

	_self->shadowOffsetX = shadowOffset->width; _self->shadowOffsetY = shadowOffset->height;
	UIsetNeedsDisplay(self);
}
