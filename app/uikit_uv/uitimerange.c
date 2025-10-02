#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "cgrect.h"
#include "UIView.h"
#include "uiview_.h"
#include "UITimeRange.h"
#include "uitimerange_.h"


static void UITimeRangeClass_izer(void *self, va_list *app)
{
	struct UITimeRangeClass *cls = self;

//printf("UITimeRangeClass_izer...\n");
	super_izer(UITimeRangeClass, cls, app);
}

static void UITimeRange_izerwf(void * self, va_list *app)
{
	struct UITimeRange	*_self = self;
	CGRect	rect;

	super_izerwf(UITimeRange, _self, app);
	UIframe(_self, &rect);
	rect.width = 320; rect.height = 140;
	UIsetFrame(_self, &rect);
	UIsetOpaque(self, FALSE);
	UIsetTintColor(self, systemGreen);
//	UIsetBackgroundColor(self, ARGB(255,0,0,0));
	_self->status		= 0;
	_self->timeRange[0] = _self->timeRange[1] = _self->timeRange[2] = _self->timeRange[3] = 0;
}

void _DrawTimeRangeComp(struct UITimeRange *self, int comp, BOOL bgDraw);

static void UITimeRange_draw(void *self, CGRect *rect)
{
	struct UITimeRange *_self = self;
	void		*v;
	BMPC		*bmp, _bmp;
	CGRect		rt;
	CGPoint		pt;
	char		temp[16];
	int		i, x, y, w, h, dx, dy, font;

	pt.x = pt.y = 0;
	UIconvertPointToScreen(_self, &pt, (CGPoint *)&rt);
	if(_self->status) {
		rt.x += rect->x; rt.y += rect->y; rt.width = rect->width; rt.height = rect->height;
		lcdClipToRect(&rt);
		i = _self->status & 0x03;
		_DrawTimeRangeComp(_self, i, TRUE);
		_self->status = 0;
	} else {
		super_UIdraw(UITimeRange, _self, rect);
		x = rt.x; y = rt.y; w = 80; h = 40;
		rt.x += rect->x; rt.y += rect->y; rt.width = rect->width; rt.height = rect->height;
		lcdClipToRect(&rt);
		bmp = &_bmp;
	//printf("Draw: %d %d %d\n", (int)_self->year, (int)_self->month, (int)_self->day);
		for(i = 0;i < 4;i++) {
			_DrawTimeRangeComp(_self, i, FALSE);
			bmpcInit(bmp, arrowup32_mask, 32, 32);
			dx = (w - 32) >> 1;
//			lcdPutAlphaMask(x+dx, y+4, UItintColor(_self), bmp); 
			bmpcInit(bmp, arrowdown32_mask, 32, 32);
			dy = h + 10 + h + 10;
//			lcdPutAlphaMask(x+dx, y+dy+4, UItintColor(_self), bmp);
			if(i == 1) {
				bmpcInit(bmp, colon_8x32, 8, 32);
				dy = h + 10;
//				lcdPutMask(x+76, y+dy, blackColor, bmp);
			} else if(i == 0 || i == 2) {
				bmpcInit(bmp, colon_8x32, 8, 32);
				dy = h + 10;
//				lcdPutMask(x+76, y+dy, blackColor, bmp);
			}
			x += w;
		}
	}
}

void _DrawTimeRangeComp(struct UITimeRange *self, int comp, BOOL bgDraw)
{
	CGRect		rt;
	CGPoint		pt;
	char	temp[16];
	int		hh, font;

	sprintf(temp, "%02d", (int)self->timeRange[comp]);
	rt.x = 80 * comp;
	rt.y = 50; rt.width = 80; rt.height = 40;
	if(bgDraw) {
		rt.x += 4; rt.width -= 8;
		UIView_draw(self, &rt);
		rt.x -= 4; rt.width += 8;
	}
	pt.x = rt.x; pt.y = rt.y;
	UIconvertPointToScreen(self, &pt, (CGPoint *)&rt);
//printf("[%d] %d,%d => %d,%d [%s]\n", comp, pt.x, pt.y, rt.x, rt.y, temp);
	font = FONT_TYPE_28;
	SetFontSize(font);
	lcdPutText(rt.x + ((80-textWidth(font, temp))>>1), rt.y + ((40-font)>>1), temp, blackColor);
	RestoreFontSize();
}

void _PlusTimeRangeComp(struct UITimeRange *self, int comp)
{
	if(comp == 0 || comp == 2) {
		self->timeRange[comp]++; if(self->timeRange[comp] >= 24) self->timeRange[comp] = 0;
	} else {
		self->timeRange[comp]++; if(self->timeRange[comp] >= 60) self->timeRange[comp] = 0;
	}
}

void _MinusTimeRangeComp(struct UITimeRange *self, int comp)
{
	if(comp == 0 || comp == 2) {
		if(self->timeRange[comp] <= 0) self->timeRange[comp] = 23; else self->timeRange[comp]--;
	} else {
		if(self->timeRange[comp] <= 0) self->timeRange[comp] = 59; else self->timeRange[comp]--;
	}
}

int _TimeRangeClick(void *self, CGPoint *touchPt)
{
	struct UITimeRange *_self = self;
	CGRect		rt;
	CGPoint		pt;
	void		*v;
	int		i, x, mark;
	
	UIconvertPointFromScreen(_self, &pt, touchPt);
//printf("TimeRange: %d,%d %d,%d\n", pt.x, pt.y, touchPt->x, touchPt->y);
	mark = 0;
	x = 0;
	for(i = 0;i < 4;i++, x += 80) {
		if(pt.x >= x && pt.x < x+80) {
			if(pt.y >= 0 && pt.y < 40) {
				_PlusTimeRangeComp(_self, i);  mark = i + 0x80;
			} else if(pt.y >= 100 && pt.y < 140) {
				_MinusTimeRangeComp(_self, i); mark = i + 0x80;
			}
		}
		if(mark) break;
	}
	if(mark) {
		UIsetNeedsDisplay(_self);
		_self->status = mark;
		mark = 1;
	}
	return mark;
}

struct NSClass		_UITimeRangeClass;
struct UITimeRangeClass	_UITimeRange;

void *UITimeRangeClass, *UITimeRange;

void initUITimeRange(void)
{
	if(!UITimeRangeClass) {
		UITimeRangeClass = &_UITimeRangeClass;
		alloc(NSClass, UITimeRangeClass);
		init(UITimeRangeClass, "UITimeRangeClass", UIControlClass, sizeof(struct UITimeRangeClass),
		izer, UITimeRangeClass_izer,
		0);
	}
	if(!UITimeRange) {
		UITimeRange = &_UITimeRange;
		alloc(UITimeRangeClass, UITimeRange);
		init(UITimeRange, "UITimeRange", UIControl, sizeof(struct UITimeRange),
		izerwf, UITimeRange_izerwf, 
		UIdraw, UITimeRange_draw,
		0);
	}
}

void UItimeRange(void *self, unsigned char *timeRange)
{
	struct UITimeRange *_self = self;

	memcpy(timeRange, _self->timeRange, 4);
}

void UIsetTimeRange(void *self, unsigned char *timeRange)
{
	struct UITimeRange *_self = self;

	memcpy(_self->timeRange, timeRange, 4);
}
