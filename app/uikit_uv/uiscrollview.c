#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "uiview_.h"
#include "UIScrollView.h"
#include "uiscrollview_.h"


static void UIScrollViewClass_izer(void *self, va_list *app)
{
	struct UIScrollViewClass *cls = self;

//printf("UIScrollViewClass_izer...\n");
	super_izer(UIScrollViewClass, cls, app);
}

static void UIScrollView_izerwf(void * self, va_list *app)
{
	struct UIScrollView	*_self = self;

	super_izerwf(UIScrollView, _self, app);
	_self->state			= 0;
	_self->contentOffset.x = _self->contentOffset.y	= 0;
	_self->contentSize.width = _self->contentSize.height = 0;
}

static void UIScrollView_draw(void *self, CGRect *rect)
{
	struct UIScrollView *_self = self;

	super_UIdraw(UIScrollView, self, rect);
}

struct NSClass		_UIScrollViewClass;
struct UIScrollViewClass	_UIScrollView;

void *UIScrollViewClass, *UIScrollView;

void initUIScrollView(void)
{
	if(!UIScrollViewClass) {
		UIScrollViewClass = &_UIScrollViewClass;
		alloc(NSClass, UIScrollViewClass);
		init(UIScrollViewClass, "UIScrollViewClass", UIViewClass, sizeof(struct UIScrollViewClass),
		izer, UIScrollViewClass_izer,
		0);
	}
	if(!UIScrollView) {
		UIScrollView = &_UIScrollView;
		alloc(UIScrollViewClass, UIScrollView);
		init(UIScrollView, "UIScrollView", UIView, sizeof(struct UIScrollView),
		izerwf, UIScrollView_izerwf, 
		UIdraw, UIScrollView_draw,
		0);
	}
}

void _ScrollViewScroll(void *self, int dy)
{
	struct UIScrollView *_self = self;
	CGRect	_rt;
	int		i, y, d;

	UIframe(_self, &_rt);
	d = _self->contentSize.height - _rt.height;
	y = _self->contentOffset.y;
	y -= dy;
	if(y < 0 || d <= 0) y = 0;
	else if(y > d) y = d;
	if(y != _self->contentOffset.y) {
		_self->contentOffset.y = y;
//printf("UIScrollView scroll.......dy=%d y=%d\n", dy, _self->contentOffset.y);
		UIsetNeedsDisplay(_self);
	}
}

void UIcontentOffset(void *self, CGPoint *contentOffset)
{
	struct UIScrollView	*_self = self;

	contentOffset->x = _self->contentOffset.x;
	contentOffset->y = _self->contentOffset.y;
}

void UIsetContentOffset(void *self, CGPoint *contentOffset)
{
	struct UIScrollView	*_self = self;

	_self->contentOffset.x = contentOffset->x;
	_self->contentOffset.y = contentOffset->y;
	_ScrollViewScroll(self, 0);
	UIsetNeedsDisplay(self);
}

void UIcontentSize(void *self, CGSize *contentSize)
{
	struct UIScrollView	*_self = self;

	contentSize->width  = _self->contentSize.width;
	contentSize->height = _self->contentSize.height;
}

void UIsetContentSize(void *self, CGSize *contentSize)
{
	struct UIScrollView	*_self = self;

	_self->contentSize.width  = contentSize->width;
	_self->contentSize.height = contentSize->height;
	_ScrollViewScroll(self, 0);
	UIsetNeedsDisplay(self);
}

