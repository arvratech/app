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
#include "UITableViewCell.h"
#include "UISlider.h"
#include "uiview_.h"
#include "uitableviewcell_.h"


static void UITableViewCellClass_izer(void *self, va_list *app)
{
	struct UITableViewCellClass *cls = self;

//printf("UITableViewCellClass_izer...\n");
	super_izer(UITableViewCellClass, cls, app);
}

// Designated Initializer
void initWithStyle(void *self, UITableViewCellStyle style, int reuseIdentifier)
{
	struct UITableViewCell	*_self = self;
	CGRect	rt;
	void	*lbl, *imgv;
	int		v;

	super_izerwf(UITableViewCell, _self, NULL);
	_self->state			= 0;
	_self->cellStyle		= style;
	_self->reuseIdentifier	= reuseIdentifier;
	UIsetAccessoryType(_self, UITableViewCellAccessoryTypeNone);
	UIsetSelectionStyle(_self, UITableViewCellSelectionStyleDefault);
	_self->accessoryView	= NULL;
//	_self->highlightedColor = systemBlue;
	lbl = &_self->_textLabel; 
	alloc(UILabel, lbl);
	initWithFrame(lbl, 0, 0, 0, 0);
	UIsetOpaque(lbl, FALSE);
//	UIsetTextAlignment(lbl, UITextAlignmentLeft);
	_ViewSetParent(lbl, _self);
	lbl = &_self->_detailTextLabel;
	alloc(UILabel, lbl);
	initWithFrame(lbl, 0, 0, 0, 0);
	UIsetFont(lbl, 16);
	UIsetOpaque(lbl, FALSE);
	_ViewSetParent(lbl, _self);
	imgv = &_self->_imageView; 
	alloc(UIImageView, imgv);
	initWithFrame(imgv, 0, 0, 0, 0);
	UIsetOpaque(imgv, FALSE);
	_ViewSetParent(imgv, _self);
}

// Super override
static void UITableViewCell_izerwf(void *self, va_list *app)
{
	initWithStyle(self, UITableViewCellStyleDefault, 0);
}

void UITableView_drawend(void *self);

static void UITableViewCell_draw(void *self, CGRect *rect)
{
	struct UITableViewCell	*_self = self;
	void		*tblv, *v, *parent;
	CGRect		rt;
	CGPoint		pt;
	char		fileName[32];
	UITableViewCellAccessoryType accessoryType;
	int		x, y, w, h, iw;

//UIframe(self, &rt);
//printf("UITableViewCell draw.......%d,%d %d %d clip:%d,%d %d %d\n", rt.x, rt.y, rt.width, rt.height, rect->x, rect->y, rect->width, rect->height);
	super_UIdraw(UITableViewCell, self, rect);
	v = &_self->_imageView;
	if(UIimage(v)) {
		UIframe(v, &rt);
		rt.x = rect->x - rt.x; rt.y = rect->y - rt.y; rt.width = rect->width; rt.height = rect->height; 
		UIdraw(v, &rt);
	}
	v = UItextLabel(_self);
	UIframe(v, &rt);
	rt.x = rect->x - rt.x; rt.y = rect->y - rt.y; rt.width = rect->width; rt.height = rect->height; 
	UIdraw(v, &rt);
	if(UIcellStyle(_self) == UITableViewCellStyleSubtitle) {
		v = UIdetailTextLabel(_self);
		UIframe(v, &rt);
		rt.x = rect->x - rt.x; rt.y = rect->y - rt.y; rt.width = rect->width; rt.height = rect->height; 
		UIdraw(v, &rt);
	}
	accessoryType = UIaccessoryType(_self);
	if(!_self->accessoryView && accessoryType != UITableViewCellAccessoryTypeNone)  {
		switch(accessoryType) {
		case UITableViewCellAccessoryTypeDisclosureIndicator:	 strcpy(fileName, "arrowh_right.bmp"); break;
		case UITableViewCellAccessoryTypeDetailDisclosureButton: strcpy(fileName, "detail_disclosure.bmp"); break;
		case UITableViewCellAccessoryTypeDetailButton:	strcpy(fileName, "detail_disclosure.bmp"); break;
		case UITableViewCellAccessoryTypeCheckmark: strcpy(fileName, "check_on.bmp"); break;
			//parent = UIparent(_self);
			//if(UIallowsMultipleSelection(parent)) strcpy(fileName, "check_on.bmp");
			//else	strcpy(fileName, "radio_on.ico");
		case UITableViewCellAccessoryTypeUncheckmark: strcpy(fileName, "check_off.bmp"); break;
			//parent = UIparent(_self);
			//if(UIallowsMultipleSelection(parent)) strcpy(fileName, "check_off.bmp");
			//else	strcpy(fileName, "radio_off.ico");
		case UITableViewCellAccessoryTypeRadiomark: strcpy(fileName, "radio_on.bmp"); break;
		case UITableViewCellAccessoryTypeUnradiomark: strcpy(fileName, "radio_off.bmp"); break;
		}
		UIframe(_self, &rt);
		pt.x = pt.y = 0;
		UIconvertPointToScreen(_self, &pt, (CGPoint *)&rt);
		lcdPixelClipToRect(&rt, rect);
		x = rt.x; y = rt.y; w = rt.width; h = rt.height;
//printf("%d,%d [%s]\n", x+w-32, y+((h-24)>>1), fileName);
		iw = _scale(22);
		lcdPutAlphaMaskFile(x+w-iw-_scale(15), y+((h-iw)>>1), iw, iw, UItintColor(self), fileName);
	}
}

struct NSClass		_UITableViewCellClass;
struct UITableViewCellClass	_UITableViewCell;

void *UITableViewCellClass, *UITableViewCell;

void initUITableViewCell(void)
{
	if(!UITableViewCellClass) {
		UITableViewCellClass = &_UITableViewCellClass;
		alloc(NSClass, UITableViewCellClass);
		init(UITableViewCellClass, "UITableViewCellClass", UIViewClass, sizeof(struct UITableViewCellClass),
		izer, UITableViewCellClass_izer,
		0);
	}
	if(!UITableViewCell) {
		UITableViewCell = &_UITableViewCell;
		alloc(UITableViewCellClass, UITableViewCell);
		init(UITableViewCell, "UITableViewCell", UIView, sizeof(struct UITableViewCell),
		izerwf, UITableViewCell_izerwf, 
		UIdraw, UITableViewCell_draw,
		0);
	}
}

UITableViewCellStyle UIcellStyle(void *self)
{
	struct UITableViewCell *_self = self;

	return (UITableViewCellStyle)_self->cellStyle;
}

int UIreuseIdentifier(void *self)
{
	struct UITableViewCell *_self = self;

	return _self->reuseIdentifier;
}

void *UItextLabel(void *self)
{
	struct UITableViewCell *_self = self;

	return &_self->_textLabel;
}

void *UIdetailTextLabel(void *self)
{
	struct UITableViewCell *_self = self;

	return &_self->_detailTextLabel;
}

UITableViewCellAccessoryType UIaccessoryType(void *self)
{
	struct UITableViewCell *_self = self;

	return (UITableViewCellAccessoryType)_self->accessoryType;
}

void UIsetAccessoryType(void *self, UITableViewCellAccessoryType accessoryType)
{
	struct UITableViewCell *_self = self;

	_self->accessoryType = accessoryType;
	UIsetNeedsDisplay(_self);
}

void *UIaccessoryView(void *self)
{
	struct UITableViewCell *_self = self;

	return _self->accessoryView;
}

void UIsetAccessoryView(void *self, void *accessoryView)
{
	struct UITableViewCell *_self = self;
	CGRect	rt;
	int		w, h;

	if(class(accessoryView) != UISlider) {
		UIframe(accessoryView, &rt);
		w = rt.width; h = rt.height;
		UIframe(_self, &rt);
		rt.x = rt.width - w - 15;
		rt.y = (rt.height - h) >> 1;
		rt.width = w; rt.height = h;
		UIsetFrame(accessoryView, &rt);
	} else {
	}
	_self->accessoryView = accessoryView;
	UIaddChild(_self, accessoryView);
	UIsetNeedsDisplay(accessoryView);
}

UITableViewCellSelectionStyle UIselectionStyle(void *self)
{
	struct UITableViewCell *_self = self;

	return (UITableViewCellSelectionStyle)_self->selectionStyle;
}

void UIsetSelectionStyle(void *self, UITableViewCellSelectionStyle selectionStyle)
{
	struct UITableViewCell *_self = self;

	_self->selectionStyle = selectionStyle;
}

