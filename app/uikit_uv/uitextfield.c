#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ctfont.h"
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "uiview_.h"
#include "UITextField.h"
#include "uitextfield_.h"


static void UITextFieldClass_izer(void *self, va_list *app)
{
	struct UITextFieldClass *cls = self;

//printf("UITextFieldClass_izer...\n");
	super_izer(UITextFieldClass, cls, app);
}

static void UITextField_izerwf(void * self, va_list *app)
{
	struct UITextField	*_self = self;
	int		v;

	super_izerwf(UITextField, _self, app);
	_self->text[0]			= 0;
	_self->font				= 20;
	_self->textColor		= ARGB(255,0,0,0);
	_self->textAlignment	= UITextAlignmentNatural;
	_self->viewMode			= 0;
	_self->textPosition		= 0;
}

static void UITextField_draw(void *self, CGRect *rect)
{
	struct UITextField *_self = self;
	CGRect		rt;
	CGPoint		pt;
	char	*s, *p, temp[128];
	int		x, y, w, h, x0, c, dw, fh, color, align;

//if(self->tag == 41) printf("StatOnPaint: info=%02x bc=%06x\n", self->info, textFieldBackgroundColor(self));
	super_UIdraw(UITextField, _self, rect);
	s = _self->text;
	if(!s[0]) return;
	UIframe(_self, &rt);
	pt.x = pt.y = 0;
	UIconvertPointToScreen(_self, &pt, (CGPoint *)&rt);
	rt.x += 8; rt.width -= 16; 
	lcdPixelClipToRect(&rt, rect);
	x = rt.x; y = rt.y; w = rt.width; h = rt.height;
	align = UItextAlignment(self);
	if(UIisHighlighted(self)) color = UItintColor(self);
	else if(!UIisEnabled(self)) color = UItextColor(self);
	else	color = UItextColor(self);
	dw = textLineCount(s);
	ftSetSize(UIfont(_self));
	fh = ftPixelHeight();
	c = dw * fh; if(dw > 1) c += (dw-1) * (fh >> 2);
	dw = (h - c) >> 1; if(dw < 0) dw = 0;
//	if(align != UITextAlignmentNatural) y += dw;
	y += dw;
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
		y += fh + (fh >> 2);
	}
}

struct NSClass		_UITextFieldClass;
struct UITextFieldClass	_UITextField;

void *UITextFieldClass, *UITextField;

void initUITextField(void)
{
	if(!UITextFieldClass) {
		UITextFieldClass = &_UITextFieldClass;
		alloc(NSClass, UITextFieldClass);
		init(UITextFieldClass, "UITextFieldClass", UIControlClass, sizeof(struct UITextFieldClass),
		izer, UITextFieldClass_izer,
		0);
	}
	if(!UITextField) {
		UITextField = &_UITextField;
		alloc(UITextFieldClass, UITextField);
		init(UITextField, "UITextField", UIControl, sizeof(struct UITextField),
		izerwf, UITextField_izerwf, 
		UIdraw, UITextField_draw,
		0);
	}
}

int _TextFieldGetTextPosition(void *self, CGPoint *touchPt)
{
	struct UITextField *_self = self;
	CGRect	rt;
	CGPoint	pt;
	int		w;
	
	UIframe(_self, &rt);
	pt.x = pt.y = 0;
	UIconvertPointToScreen(_self, &pt, (CGPoint *)&rt);
	rt.x += 8; rt.width -= 16; 
	lcdPointToPixelRect(&rt);
	w = ftTextPositionFromTouched(_self->text, rt.x, touchPt->x);
	return w;
}

void _TextFieldDrawCursor(void *self, int textPosition, int count)
{
	struct UITextField *_self = self;
	CGRect	rt, rect;
	CGPoint	pt;
	UIColor	color;
	char	*s;
	int		x, y, h, dw, fh;

	UIframe(_self, &rt);
	rect.x = rect.y = 0; rect.width = rt.width; rect.height = rt.height;
	pt.x = pt.y = 0;
	UIconvertPointToScreen(_self, &pt, (CGPoint *)&rt);
	rt.x += 8; rt.width -= 16; 
	lcdPixelClipToRect(&rt, &rect);
	x = rt.x; y = rt.y; h = rt.height;
	ftSetSize(UIfont(_self));
	s = _self->text;
	fh = ftPixelHeight();
	dw = textLineCount(s); if(dw < 1) dw = 1;
	x = dw * fh; if(dw > 1) x += (dw-1) * (fh >> 2);
	dw = (h - x) >> 1; if(dw < 0) dw = 0;
	y += dw;
	x = rt.x + ftTextPixelPosition(s, textPosition); 
	if(count & 1) color = blueColor;
	else	color = whiteColor;
	lcdVerticalLine(x-1, y, fh, color);
	lcdVerticalLine(x, y, fh, color);
}

char *UIplaceHolder(void *self)
{
	struct UITextField	*_self = self;

	return _self->placeHolder;
}

void UIsetPlaceHolder(void *self, char *placeHolder)
{
	struct UITextField	*_self = self;

	if(placeHolder && placeHolder[0]) strcpy(_self->placeHolder, placeHolder);
	else	placeHolder[0] = 0;
	UIsetNeedsDisplay(_self);
}

BOOL UIisEditing(void *self)
{
	return FALSE;
}

UITextBorderStyle UIboderStyle(void *self)
{
	struct UITextField	*_self = self;

	return (UITextBorderStyle)_self->borderStyle;
}

void UIsetBoderStyle(void *self, UITextBorderStyle borderStyle)
{
	struct UITextField	*_self = self;

	_self->borderStyle = (unsigned char)borderStyle;
	UIsetNeedsDisplay(_self);
}

UITextFieldViewMode textFieldClearButtonMode(void *self)
{
	struct UITextField	*_self = self;

	return (UITextFieldViewMode)(_self->viewMode  & 0x03);
}

void UIsetClearButtonMode(void *self, UITextFieldViewMode clearButtonMode)
{
	struct UITextField	*_self = self;

	_self->viewMode = (_self->viewMode  & 0xfc) | clearButtonMode;
	UIsetNeedsDisplay(_self);
}

int UItextPosition(void *self)
{
	struct UITextField	*_self = self;

	return (int)_self->textPosition;
}

void UIsetTextPosition(void *self, int textPosition)
{
	struct UITextField	*_self = self;
	
	_self->textPosition = textPosition;
}

