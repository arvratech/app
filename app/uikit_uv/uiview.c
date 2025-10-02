#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "wallpaper.h"
#include "cgrect.h"
#include "UIView.h"
#include "UIWindow.h"
#include "UIControl.h"
#include "UIImageView.h"
#include "UIScrollView.h"
#include "UITableView.h"
#include "UITableViewCell.h"
#include "uiview_.h"

extern int		_appDraw;


static void UIViewClass_izer(void *self, va_list *app)
{
	struct UIViewClass *cls = self;
	typedef void (* voidf)();
	voidf		selector, method;
	va_list		ap;

//printf("UIViewClass_izer...%x\n", (unsigned int)self);
	super_izer(UIViewClass, cls, app);
#ifdef va_copy
	va_copy(ap, *app);
#else
	ap = *app;
#endif
	while(selector = va_arg(ap, voidf)) {
		method = va_arg(ap, voidf);
		if(selector == (voidf)izerwf) *(voidf *)&cls->izerwf = method;
		else if(selector == (voidf)UIdraw) *(voidf *)&cls->draw = method;
	}
#ifdef va_copy
    va_end(ap);
#endif
}

// Designated Initializer
static void UIView_izerwf(void *self, va_list *app)
{
	struct UIView	*_self = self;

	super_izer(UIView, _self, app);
//	_self->state			= UIV_USER_INTERACTION_ENABLED | UIV_CLIP_TO_BOUNDS;
	_self->state			= UIV_USER_INTERACTION_ENABLED | UIV_CLIP_TO_BOUNDS | UIV_OPAQUE;	// 2022.4.1
	_self->mode				= 0;
	_self->alpha			= 255;		// 0:Transparent  255:Opaque
	_self->tag				= 0;
	if(app) {
		_self->x			= va_arg(*app, int);
		_self->y			= va_arg(*app, int);
		_self->w			= va_arg(*app, int);
		_self->h			= va_arg(*app, int);
	} else {
		_self->x = _self->y = _self->w = _self->h = 0;
	}
	_self->ix = _self->iy	= 0;
	_self->iw				= _self->w;
	_self->ih				= _self->h;
	_self->parent			= NULL;
	_self->children			= NULL;
	_self->next				= NULL;
	_self->backgroundColor	= clearColor;
	_self->tintColor		= clearColor;
	_self->drawRect			= NULL;
	_appDraw = 1;
}

// Super override
static void UIView_izer(void *self, va_list *app)
{
//	initWithFrame(self, 0, 0, 0, 0);
	izerwf(self, NULL);
}

void izerwf(void *self, va_list *app)
{
	struct UIViewClass *cls = class(self);

	assert(cls && cls->izerwf);
	cls->izerwf(self, app);
}

void super_izerwf(void *self_class, void *self, va_list *app)
{
	struct NSClass *cls = self_class;
	struct UIViewClass *supercls;

	supercls = (struct UIViewClass *)cls->super;
	assert(supercls && supercls->izerwf);
	supercls->izerwf(self, app);
}

void initWithFrame(void *self, ...)
{
	struct NSObject *obj = self;
	va_list ap;

	va_start(ap, self);
	izerwf(obj, &ap);
	va_end(ap);
}

static void *_ViewGetApparentOpaqueView(void *self)
{
	struct UIView	*v;

	v = self;
	while(v) {
		if(UIisOpaque(v)) break;
		v = v->parent;
	}
	return v;
}

#include "UIButton.h"
#include "UITableViewCell.h"
#include "UILabel.h"

void UIView_draw(void *self, CGRect *rect)
{
	struct UIView	*_self = self;
	struct UIViewClass *cls;
	void	*v;
	CGRect	rt;
	CGPoint	pt;
	UIColor	color;
	int		x, y, w, h, c1, c2, c3;

	UIframe(_self, &rt);
	pt.x = pt.y = 0;
	UIconvertPointToScreen(_self, &pt, (CGPoint *)&rt);
	lcdPixelClipToRect(&rt, rect);
	x = rt.x; y = rt.y; w = rt.width; h = rt.height;
	v = _ViewGetApparentOpaqueView(_self);
	if(v) {
		cls = class(v);
		if(v != _self && cls == UIImageView) {
//printf("UIView_draw...%d,%d %d %d\n", x, y, w, h);
			pt.x = rect->x; pt.y = rect->y; 
			UIconvertPointToView(_self, &pt, v, (CGPoint *)&rt);
			rt.width = rect->width; rt.height = rect->height;
			cls->draw(v, &rt);
		} else {
			color = UIbackgroundColor(v);
			if(cls != UIButton && cls != UITableViewCell) {
				v = UIparent(v);
				if(v) cls = class(v);
			}
			if(v && (cls == UIButton || cls == UITableViewCell) && UIisHighlighted(v)) {
				c1 = (color >> 16) & 0xff; c2 = (color >> 8) & 0xff; c3 = color & 0xff;
				c1 -= 48; if(c1 < 0) c1 = 0;
				c2 -= 48; if(c2 < 0) c2 = 0;
				c3 -= 48; if(c3 < 0) c3 = 0;
				color = (color & 0xff000000) | (c1 << 16) | (c2 << 8) | c3;
			}
			if(color != clearColor) lcdFillRect(x, y, w, h, color);
		}
//if(class(self) == UIWindow) printf("Wnd draw: %d,%d %d %d color=0x%08x\n", pt.x, pt.y, clip.width, clip.height, color);
	} else {
//if(_self->tag == 126) printf("[%d,%d %d %d]\n", clip.x, clip.y, clip.width, clip.height);
		wallpaperPut();
	}
}

void UIdraw(void *self, CGRect *rect)
{
	struct UIView	*_self = self;
	struct UIViewClass *cls = class(self);

	if(_self->drawRect) {
		if(class(self) != UIView) super_UIdraw(class(self), self, rect);
		_self->drawRect(self, rect);
	} else {
		assert(cls->draw);
		cls->draw(self, rect);
	}
}

void super_UIdraw(void *self_class, void *self, CGRect *rect)
{
	struct NSClass *cls = self_class;
	struct UIViewClass *supercls;

	supercls = (struct UIViewClass *)cls->super;
	assert(supercls && supercls->draw);
	supercls->draw(self, rect);
}

struct NSClass		_UIViewClass;
struct UIViewClass	_UIView;

void *UIViewClass, *UIView;

void initUIView(void)
{
	struct	NSObject	*obj;

	if(!UIViewClass) {
		UIViewClass = &_UIViewClass;
		alloc(NSClass, UIViewClass);
		init(UIViewClass, "UIViewClass", NSClass, sizeof(struct UIViewClass),
		izer, UIViewClass_izer,
		0);
	}
	if(!UIView) {
		UIView = &_UIView;
		alloc(UIViewClass, UIView);
		init(UIView, "UIView", NSObject, sizeof(struct UIView),
		izer, UIView_izer,
		izerwf, UIView_izerwf,
		UIdraw, UIView_draw,
		0);
	}
}

void UIinitWithFrame(void *self, CGRect *rect)
{
	struct UIView	*_self = self;

	_self->state			= 0;
	_self->alpha			= 255;		// 0:Transparent  255:Opaque
	_self->tag				= 0;
	_self->x				= rect->x;
	_self->y				= rect->y;
	_self->w				= rect->width;
	_self->h				= rect->height;
	_self->ix = _self->iy	= 0;
	_self->iw				= _self->w;
	_self->ih				= _self->h;
	_self->parent			= NULL;
	_self->children			= NULL;
	_self->next				= NULL;
	_self->backgroundColor	= 0;
	_self->tintColor		= 0;
	_self->drawRect			= NULL;
	_appDraw = 1;
}

int UItag(void *self)
{
	struct UIView	*_self = self;

	return _self->tag;
}

void UIsetTag(void *self, int tag)
{
	struct UIView	*_self = self;

	_self->tag = tag;
}

void UIframe(void *self, CGRect *rect)
{
	struct UIView	*_self = self;

	rect->x = _self->x; rect->y = _self->y; rect->width = _self->w; rect->height = _self->h;
}

void UIsetFrame(void *self, CGRect *rect)
{
	struct UIView	*_self = self;

	_self->x = rect->x; _self->y = rect->y; _self->w = rect->width; _self->h = rect->height;
	_self->ix = _self->iy = 0; _self->iw = _self->w; _self->ih = _self->h;
	_appDraw = 1;
}

void *UIparent(void *self)
{
	struct UIView	*_self = self;

	return _self->parent;
}

int UIchildren(void *self, void **children)
{
	struct UIView	*_self = self;
	struct UIView	*v;
	void	**p;
	int		count;

	p = children;
	count = 0;
	v = _self->children;
	while(v) {
		*p++ = v; count++;
		v = v->next;
	}
	return count;
}

int UIchildrenCount(void *self)
{
	struct UIView	*_self = self;
	struct UIView	*v;
	int		count;

	count = 0;
	v = _self->children;
	while(v) {
		count++;
		v = v->next;
	}
	return count;
}

void *UIfirstChild(void *self)
{
	struct UIView	*_self = self;

	return _self->children;
}

void *UInextChild(void *self)
{
	struct UIView	*_self = self;

	return _self->next;
}

void *UIwindow(void *self)
{
	struct UIView	*v = self;

	while(v && class(v) != UIWindow) v = v->parent;
	return v;
}

UIColor UIbackgroundColor(void *self)
{
	struct UIView	*_self = self;

	return _self->backgroundColor;
}

void UIsetBackgroundColor(void *self, UIColor backgroundColor)
{
	struct UIView	*_self = self;

	_self->backgroundColor = backgroundColor;
	_self->ix = _self->iy = 0; _self->iw = _self->w; _self->ih = _self->h;
	_appDraw = 1;
}

BOOL UIisHidden(void *self)
{
	struct UIView	*_self = self;

	if(_self->state & UIV_HIDDEN) return TRUE; else return FALSE;
}

void UIsetHidden(void *self, BOOL isHidden)
{
	struct UIView	*_self = self;
	void	*parent;

	if(isHidden) {
		_self->state |= UIV_HIDDEN;
		parent = UIparent(_self);
		if(parent) UIsetNeedsDisplay(parent);
	} else {
		_self->state &= ~UIV_HIDDEN;
		_self->ix = _self->iy = 0; _self->iw = _self->w; _self->ih = _self->h;
		_appDraw = 1;
	}
}

int UIalpha(void *self)
{
	struct UIView	*_self = self;

	return (int)_self->alpha;
}

void UIsetAlpha(void *self, int alpha)
{
	struct UIView	*_self = self;

	_self->alpha = (unsigned char)alpha;
}

BOOL UIisOpaque(void *self)
{
	struct UIView	*_self = self;

	if(_self->state & UIV_OPAQUE) return TRUE; else return FALSE;
}

void UIsetOpaque(void *self, BOOL isOpaque)
{
	struct UIView	*_self = self;

	if(isOpaque) _self->state |= UIV_OPAQUE;
	else	_self->state &= ~UIV_OPAQUE;
	_self->ix = _self->iy = 0; _self->iw = _self->w; _self->ih = _self->h;
	_appDraw = 1;
}

UIColor UItintColor(void *self)
{
	struct UIView	*v = self;;
	UIColor	color;

	while(v && !v->tintColor) v = v->parent;
	if(!v || !v->tintColor) color = ARGB(255,0,125,255);
	else	color = v->tintColor;
	return color;
}

void UIsetTintColor(void *self, UIColor tintColor)
{
	struct UIView	*_self = self;

	_self->tintColor = tintColor;
}

UIViewTintAdjustmentMode UItintAdjustmentMode(void *self)
{
	struct UIView	*_self = self;

	return (UIViewTintAdjustmentMode)((_self->mode) & 0x3);
}

void viewSetTintAdjustmentMode(void *self, UIViewTintAdjustmentMode tintAdjustmentMode)
{
	struct UIView	*_self = self;

	_self->mode = (_self->mode  & 0xfc) | tintAdjustmentMode;
}
	
BOOL UIclipToBounds(void *self)
{
	struct UIView	*_self = self;

	if(_self->state & UIV_CLIP_TO_BOUNDS) return TRUE; else return FALSE;
}

void UIsetClipToBounds(void *self, BOOL clipToBounds)
{
	struct UIView	*_self = self;

	if(clipToBounds) _self->state |= UIV_CLIP_TO_BOUNDS;
	else	_self->state &= ~UIV_CLIP_TO_BOUNDS;
	_self->ix = _self->iy = 0; _self->iw = _self->w; _self->ih = _self->h;
	_appDraw = 1;
}

UIViewContentMode UIcontentMode(void *self)
{
	struct UIView	*_self = self;

	return (UIViewContentMode)((_self->mode >> 2) & 0xf);
}

void UIsetContentMode(void *self, UIViewContentMode contentMode)
{
	struct UIView	*_self = self;

	_self->mode = (_self->mode  & 0xc3) | (contentMode << 2);
}

int UIisFocused(void *self)
{
	struct UIView	*_self = self;

	if(_self->state & UIV_FOCUSED) return TRUE; else return FALSE;
}

void UIsetFocused(void *self, BOOL isFocused)
{
	struct UIView	*_self = self;

	if(isFocused) _self->state |= UIV_FOCUSED;
	else	_self->state &= ~UIV_FOCUSED;
	_self->ix = _self->iy = 0;
	_self->iw = _self->w; _self->ih = _self->h;
	_appDraw = 1;
}

BOOL UIcanBecomeFocused(void *self)
{
	struct UIView	*_self = self;

	if(_self->state & UIV_CAN_BECOME_FOCUSED) return TRUE; else return FALSE;
}

void UIsetCanBecomeFocused(void *self, BOOL canBecomeFocused)
{
	struct UIView	*_self = self;

	if(canBecomeFocused) _self->state |= UIV_CAN_BECOME_FOCUSED;
	else	_self->state &= ~UIV_CAN_BECOME_FOCUSED;
	_self->ix = _self->iy = 0; _self->iw = _self->w; _self->ih = _self->h;
	_appDraw = 1;
}

BOOL UIisUserInteractionEnabled(void *self)
{
	struct UIView	*_self = self;

	if(_self->state & UIV_USER_INTERACTION_ENABLED) return TRUE; else return FALSE;
}

void UIsetUserInteractionEnabled(void *self, BOOL isUserInteractionEnabled)
{
	struct UIView	*_self = self;

	if(isUserInteractionEnabled) _self->state |= UIV_USER_INTERACTION_ENABLED;
	else	_self->state &= ~UIV_USER_INTERACTION_ENABLED;
}

void UIsetDrawRect(void *self, void (*drawRect)(void *, CGRect *))
{
	struct UIView	*_self = self;

	_self->drawRect = drawRect;
}

static void _ClearView(void *self)
{
	struct UIView	*_self = self;
	struct UIView	*v;

	v = _self->children;
	while(v) {
		_ClearView(v);
		v = v->next;
	}
	_self->iw = _self->ih = 0;
}

#include "UIButton.h"
#include "../t50_test/defs.h"

void UIdrawView(void *self, CGRect *rect)
{
	struct UIView	*_self = self;
	struct UIView	*v;
	CGPoint		_pt;
	CGRect		*rt, _rt;;
	int	 	x, y, w, h, n, invalid;

	rt = &_rt;
	if(_self->state & UIV_HIDDEN) {
		_ClearView(_self);
	} else {
		if(_self->iw > 0 && _self->ih > 0) {
			// invalidRect => x,y,w,h
			x = _self->ix; y = _self->iy; w = _self->iw; h = _self->ih;
			invalid = 1;
		} else {
			// bound => x,y,w,h
			x = y = 0; w = _self->w; h = _self->h;
			invalid = 0;
		}
		// Intersection: x,y,w,h rect => x,y,w,h
		n = x + w - 1; w = rect->x + rect->width - 1;
		if(n > w) n = w;
		if(rect->x > x) x = rect->x;
		w = n - x + 1;
		n = y + h - 1; h = rect->y + rect->height - 1;
		if(n > h) n = h;
		if(rect->y > y) y = rect->y;
		h = n - y + 1; 
		if(invalid) {
			if(w > 0 && h > 0) {
				rt->x = x; rt->y = y; rt->width = w; rt->height = h;
				UIdraw(_self, rt);
			} else	invalid = 0;
		}
		v = _self->children;
		if(!(_self->state & UIV_CLIP_TO_BOUNDS)) {
			while(v) {
				if(invalid) {
					v->ix = v->iy = 0; v->iw = v->w; v->ih = v->h; 
				}
				// rect in parent space => rt in child space
				rt->x = rect->x - v->x; rt->y = rect->y - v->y;
				rt->width = rect->width; rt->height = rect->height;
				if(class(_self) == UIScrollView || class(_self) == UITableView) {
					UIcontentOffset(_self, &_pt);
					rt->x += _pt.x; rt->y += _pt.y;
				}
				UIdrawView(v, rt);
				v = v->next;
			}
		} else if(w > 0 && h > 0) {
			while(v) {
				if(invalid) {
					v->ix = v->iy = 0; v->iw = v->w; v->ih = v->h; 
				}
				// x,y,w,h in parent space => rt in child space
				rt->x = x - v->x; rt->y = y - v->y; rt->width = w; rt->height = h;
				if(class(_self) == UIScrollView || class(_self) == UITableView) {
					UIcontentOffset(_self, &_pt);
					rt->x += _pt.x; rt->y += _pt.y;
				}
				UIdrawView(v, rt);
				v = v->next;
			}
		} else {
			while(v) {
				_ClearView(v);
				v = v->next;
			}
		}
	}
	_self->iw = _self->ih = 0;;
}

void UIdrawChild(void *self, void *parentView, CGRect *rect)
{
	struct UIView	*_self = self;
	struct UIView	*parent = parentView;
	CGPoint		_pt;
	CGRect		*rt, _rt;;
	int	 	x, y, w, h;

	rt = &_rt;
	if(_self->state & UIV_HIDDEN) {
		_ClearView(_self);
	} else {
		rt->x = rect->x - _self->x; rt->y = rect->y - _self->y;
		rt->width = rect->width; rt->height = rect->height;
		if(class(parent) == UIScrollView || class(parent) == UITableView) {
			UIcontentOffset(parent, &_pt);
			rt->x += _pt.x; rt->y += _pt.y;
		}
//printf("Child...%d,%d %d %d rect=%d,%d %d %d\n", _self->x, _self->y, _self->w, _self->h, rt->x, rt->y, rt->width, rt->height);
		UIdrawView(_self, rt);
	}
	_self->iw = _self->ih = 0;;
}

// pt's coordinates is super view
void *UIhitTestView(void *self, CGPoint *pt)
{
	struct UIView	*_self = self;
	struct UIView	*v;
	void	*hitView, *parent; 
	CGPoint		_pt;
	CGRect		*rt, _rt;
	void	*cls;

	hitView = (void *)0;
	rt = &_rt;
	if(_self->state & UIV_CLIP_TO_BOUNDS) {
		if(!(_self->state & UIV_HIDDEN) && (_self->state & UIV_USER_INTERACTION_ENABLED)
							&& (!isKindOfClass(_self, UIControl) || UIisEnabled(_self))) {
			UIframe(_self, rt);
			parent = _self->parent;
			if(parent && (class(parent) == UIScrollView || class(parent) == UITableView)) {
				UIcontentOffset(_self->parent, &_pt);
//printf("ScrollView1:%s: %d,%d %d,%d\n", className(self), rt->x, rt->y, _pt.x, _pt.y);
				rt->x -= _pt.x; rt->y -= _pt.y;
			}
			pt->x -= rt->x; pt->y -= rt->y;
			if(pt->x >= 0 && pt->x < rt->width && pt->y >= 0 && pt->y < rt->height) {
				v = _self->children;
				while(v) {
					hitView = UIhitTestView(v, pt);
					if(hitView) break;
					v = v->next;
				}
				if(!hitView) hitView = _self;
			}
			pt->x += rt->x; pt->y += rt->y;
		}
	} else {
		UIframe(_self, rt);
		parent = _self->parent;
		if(parent && (class(parent) == UIScrollView || class(parent) == UITableView)) {
			UIcontentOffset(_self->parent, &_pt);
			rt->x -= _pt.x; rt->y -= _pt.y;
//printf("ScrollView2: %d,%d %d,%d\n", rt->x, rt->y, _pt.x, _pt.y);
		}
		pt->x -= rt->x; pt->y -= rt->y;
		v = _self->children;
		while(v) {
			hitView = UIhitTestView(v, pt);
			if(hitView) break;
			v = v->next;
		}
		if(!hitView && pt->x >= 0 && pt->x < rt->width && pt->y >= 0 && pt->y < rt->height
				&& !(_self->state & UIV_HIDDEN) && (_self->state & UIV_USER_INTERACTION_ENABLED)
				&& (!isKindOfClass(_self, UIControl) || UIisEnabled(_self))) hitView = _self;
		pt->x += rt->x; pt->y += rt->y;
	}
	return hitView;
}

void _ViewListAdd(void **list, void *view)
{
	struct UIView	*v;

	v = *list;
	if(!v) *list = view;
	else {
		while(v->next) v = v->next;
		v->next = view;
	}
	v = view;
	v->next = NULL;	
}

void _ViewListInsert(void **list, void *view, void *siblingView)
{
	struct UIView	*v;

	v = *list;
	if(!v) {
		*list = view;
		v = view;
		v->next = NULL;	
	} else if(siblingView) {
		while(v->next && v->next != siblingView) v = v->next;
		if(v->next) {
			v->next = view;
			v = view;
			v->next = siblingView;
		} else {
			v->next = view;
			v = view;
			v->next = NULL;	
		}
	} else {
		while(v->next) v = v->next;
		v->next = view;
		v = view;
		v->next = NULL;	
	}
}

void _ViewListRemove(void **list, void *view)
{
	struct UIView	*v, *v1;

	v1 = NULL;
	v = *list;
	while(v && v != view) {
		v1 = v;	v = v->next;
	}
	if(v) {
		if(v1) v1->next = v->next;
		else	*list = v->next;
		v->next = NULL;
	}
}

void UIaddChild(void *self, void *childView)
{
	struct UIView	*_self = self;
	struct UIView	*v;

	_ViewListAdd(&_self->children, childView);
	v = childView;
	v->parent = _self;
	_self->ix = _self->iy = 0; _self->iw = _self->w; _self->ih = _self->h;
	_appDraw = 1;
}

void _ViewSetParent(void *self, void *parent)
{
	struct UIView	*_self = self;

	_self->parent = parent;
}

BOOL _ViewIsLastChild(void *self)
{
	struct UIView	*_self = self;

	if(_self->next) return FALSE; else return TRUE;
}

void _ViewInvalidRect(void *self, CGRect *rect)
{
	struct UIView	*_self = self;

	rect->x = _self->ix; rect->y = _self->iy; rect->width = _self->iw; rect->height = _self->ih;
	_appDraw = 1;
}

void UIaddChildFirst(void *self, void *childView)
{
	struct UIView	*_self = self;
	struct UIView	*v;

	v = childView;
	v->next = _self->children;
	_self->children =  childView;
	v->parent = _self;
	_self->ix = _self->iy = 0; _self->iw = _self->w; _self->ih = _self->h;
}

void UIinsertChildAboveChild(void *self, void *childView, void *siblingView)
{
	struct UIView	*_self = self;
	struct UIView	*v;

	_ViewListInsert(&_self->children, childView, siblingView);
	v = childView;
	v->parent = _self;
	_self->ix = _self->iy = 0; _self->iw = _self->w; _self->ih = _self->h;
	_appDraw = 1;
}

//void UIinsertChildBelowChild(void *self, void *childView, void *siblingView)
//{
//}

void UIremoveFromChild(void *self)
{
	struct UIView	*_self = self;
	struct UIView	*parent;

	parent = _self->parent;
	if(parent) {
		_ViewListRemove(&parent->children, _self);
		_self->parent = NULL;
	}
}

void UIremoveChildAll(void *self)
{
	struct UIView	*_self = self;

	_self->children = NULL;
}

void *UIviewWithTag(void *self, int tag)
{
	struct UIView	*_self = self;
	struct UIView	*v;

	v = _self->children;
	while(v && v->tag != tag) v = v->next;
	return v;
}

void UIsetNeedsDisplay(void *self)
{
	struct UIView	*_self = self;

	_self->ix = _self->iy = 0; _self->iw = _self->w; _self->ih = _self->h;
	_appDraw = 1;
}

void UIsetNeedsDisplayInRect(void *self, CGRect *rect)
{
	struct UIView	*_self = self;

	if(rect) {
		_self->ix = rect->x; _self->iy = rect->y; _self->iw = rect->width; _self->ih = rect->height;
	} else {
		_self->ix = _self->iy = 0; _self->iw = _self->w; _self->ih = _self->h;
	}
	_appDraw = 1;
}

void UIconvertPointToView(void *self, CGPoint *pt, void *view, CGPoint *ptView)
{
	struct UIView	*v = self;
	CGPoint	_pt;
	int		x, y;

	x = pt->x; y = pt->y;
	do {
		x += v->x; y += v->y;
		if(v->parent && (class(v->parent) == UIScrollView || class(v->parent) == UITableView)) {
			UIcontentOffset(v->parent, &_pt);
			x += _pt.x; y += _pt.y;
		}
		v = v->parent;
	} while(v != view);
	ptView->x = x; ptView->y = y;
}

void UIconvertPointToWindow(void *self, CGPoint *pt, CGPoint *ptWindow)
{
	struct UIView	*v = self;
	CGPoint	_pt;
	int		x, y;

	x = pt->x; y = pt->y;
	do {
		x += v->x; y += v->y;
		if(v->parent && (class(v->parent) == UIScrollView || class(v->parent) == UITableView)) {
			UIcontentOffset(v->parent, &_pt);
			x += _pt.x; y += _pt.y;
		}
		v = v->parent;
	} while(v && class(v) != UIWindow);
	ptWindow->x = x; ptWindow->y = y;
}

void UIconvertPointFromWindow(void *self, CGPoint *pt, CGPoint *ptWindow)
{
	struct UIView	*v = self;
	CGPoint	_pt;
	int		x, y;

	x = ptWindow->x; y = ptWindow->y;
	do {
		x -= v->x; y -= v->y;
		if(v->parent && (class(v->parent) == UIScrollView || class(v->parent) == UITableView)) {
			UIcontentOffset(v->parent, &_pt);
			x += _pt.x; y += _pt.y;
		}
		v = v->parent;
	} while(v && class(v) != UIWindow);
	pt->x = x; pt->y = y;
}

void UIconvertPointToScreen(void *self, CGPoint *pt, CGPoint *ptScreen)
{
	struct UIView	*v = self;
	CGPoint	_pt;
	int		x, y;

	x = pt->x; y = pt->y;
	do {
		x += v->x; y += v->y;
		if(v->parent && (class(v->parent) == UIScrollView || class(v->parent) == UITableView)) {
			UIcontentOffset(v->parent, &_pt);
			x -= _pt.x; y -= _pt.y;
		}
		v = v->parent;
	} while(v);
	ptScreen->x = x; ptScreen->y = y;
}

void UIconvertPointFromScreen(void *self, CGPoint *pt, CGPoint *ptScreen)
{
	struct UIView	*v = self;
	CGPoint	_pt;
	int		x, y;

	x = ptScreen->x; y = ptScreen->y;
	do {
		x -= v->x; y -= v->y;
		if(v->parent && (class(v->parent) == UIScrollView || class(v->parent) == UITableView)) {
			UIcontentOffset(v->parent, &_pt);
			x += _pt.x; y += _pt.y;
		}
		v = v->parent;
	} while(v);
	pt->x = x; pt->y = y;
}

