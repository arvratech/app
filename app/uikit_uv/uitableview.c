#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "uiview_.h"
#include "UILabel.h"
#include "UIImageView.h"
#include "UIButton.h"
#include "UISlider.h"
#include "UIScrollView.h"
#include "UITableViewCell.h"
#include "UITableView.h"
#include "uitableview_.h"


static void UITableViewClass_izer(void *self, va_list *app)
{
	struct UITableViewClass *cls = self;

//printf("UITableViewClass_izer...\n");
	super_izer(UITableViewClass, cls, app);
}

static void UITableView_izerwf(void * self, va_list *app)
{
	struct UITableView	*_self = self;
	CGRect		rt;
	void		*lbl, *imgv;
	int		style;

	super_izerwf(UITableView, _self, app);
	if(app) _self->style	= va_arg(*app, int);
	else	_self->style	= 0;
	UIsetSeperatorStyle(_self, UITableViewCellSeperatorStyleNone);
	_self->allowsMultipleSelection = 0;
	_self->delegate			= NULL;
	_self->dataSource		= NULL;
	_self->contentIndex		= 0;
	_self->contentSize		= 0;
	_self->rowHeight		= 0;
	_self->scrollDy			= 0;
	_self->seperatorColor	= systemBlue;
	_self->tableHeaderView	= NULL;
	_self->tableFooterView	= NULL;
	_self->reuseCells		= NULL;
	_self->onNumberOfRowsInTableView = NULL;
	_self->onCellForRowAtIndex		 = NULL;
	_self->onPrefetchRowsAtIndexs	 = NULL;
	_self->onDidSelectRowAtIndex	 = NULL;
	_self->onHeightForRowAtIndex	 = NULL;
}

static void UITableView_draw(void *self, CGRect *rect)
{
	struct UITableView *_self = self;
	CGRect		rt, rt2;
	CGPoint		pt;
	G2D_BUFFER	*g2d;
	int		x, y, w, h, dy;

	dy = _self->scrollDy;
	if(dy) {
		UIframe(_self, &rt);
//printf("TableView draw....%d,%d %d %d scroll=%d %d,%d %d %d\n", rt.x, rt.y, rt.width, rt.height, dy, rect->x, rect->y, rect->width, rect->height);
		pt.x = pt.y = 0;
		UIconvertPointToScreen(_self, &pt, (CGPoint *)&rt);
		lcdPointToPixelRect(&rt);
//printf("%d,%d %d %d scroll=%d\n", rt.x, rt.y, rt.width, rt.height, _scale(dy));
		g2d = g2dLcdBuffer();
		g2dMove(g2d, &rt, _scale(dy));
		if(dy > 0) {
			rt.x = rt.y = 0; rt.height = dy;
		} else {
			rt.x = 0; rt.y = rt.height + dy; rt.height = -dy;
		}
		_self->scrollDy = 0;
	} else {
		rt.x = rect->x; rt.y = rect->y; rt.width = rect->width; rt.height = rect->height;
//printf("UITableView draw....%d,%d %d %d %d,%d %d %d\n", rt.x, rt.y, rt.width, rt.height, rect->x, rect->y, rect->width, rect->height);
	}
	super_UIdraw(UITableView, self, &rt);
}

struct NSClass		_UITableViewClass;
struct UITableViewClass	_UITableView;

void *UITableViewClass, *UITableView;

void initUITableView(void)
{
	if(!UITableViewClass) {
		UITableViewClass = &_UITableViewClass;
		alloc(NSClass, UITableViewClass);
		init(UITableViewClass, "UITableViewClass", UIScrollViewClass, sizeof(struct UITableViewClass),
		izer, UITableViewClass_izer,
		0);
	}
	if(!UITableView) {
		UITableView = &_UITableView;
		alloc(UITableViewClass, UITableView);
		init(UITableView, "UITableView", UIScrollView, sizeof(struct UITableView),
		izerwf, UITableView_izerwf, 
		UIdraw, UITableView_draw,
		0);
	}
}

void _ViewListAdd(void **list, void *view);
void _ViewListRemove(void **list, void *view);

int _TableViewCellMake(void *cell, int cellWidth)
{
	void	*v, *v2;
	CGRect	rt;
	UITableViewCellAccessoryType accessoryType;
	int		x, y, w, h, cw, ch; 

	UIframe(cell, &rt);
	if(rt.width > 0) cw = rt.width; else cw = cellWidth;
	if(UIcellStyle(cell) == UITableViewCellStyleSubtitle) ch = 68;
	else	ch = 48;
	x = 15;
	v = UIimageView(cell);
	if(UIimage(v)) {
		w = h = 29;
		rt.x = x; rt.y = (ch - h) >> 1; rt.width = w; rt.height = h;
		UIsetFrame(v, &rt);
		x += rt.width + 15;
	}
	w = cw - x - 15;
	accessoryType = UIaccessoryType(cell);
	v = UIaccessoryView(cell);
	if(v) {
		UIframe(v, &rt);
		rt.x = cw - (rt.width-8) - 15; rt.y = (ch - rt.height) >> 1;
		UIsetFrame(v, &rt);
		w -= rt.width - 8;
	} else if(accessoryType != UITableViewCellAccessoryTypeNone) w -= 24;	// 24
	v = UItextLabel(cell);
	ftSetSize(UIfont(v));
	rt.height = ftHeight();
	v2 = UIdetailTextLabel(cell);
	ftSetSize(UIfont(v2));
	h = ftHeight();
	if(UIcellStyle(cell) == UITableViewCellStyleSubtitle) {
		y = (ch - rt.height - 2 - h) >> 1;
	} else {
		y = (ch - rt.height) >> 1;
	}
	rt.x = x; rt.y = y; rt.width = w;
	UIsetFrame(v, &rt);
	if(UIcellStyle(cell) == UITableViewCellStyleSubtitle) {
		v = UIfirstChild(cell);
		if(v && class(v) == UISlider) {	// slider
			v2 = UIdetailTextLabel(cell);
			rt.x = x; rt.y = y + rt.height + 2; rt.width = 27; rt.height = h;
			UIsetFrame(v2, &rt);
			rt.x = x + 27; rt.y = y + rt.height + 2; rt.width = w - 27; rt.height = 28;
		} else {
			v = UIdetailTextLabel(cell);
			rt.x = x; rt.y = y + rt.height + 2; rt.width = w; rt.height = h;
		}
		UIsetFrame(v, &rt);
	}
	return ch;
}

void UIreloadData(void *self)
{
	struct UITableView *_self = self;
	void		*cell, *cell2;
	CGRect		rt;
	CGPoint		pt;
	int		index, y, h, yt, cellWidth, tabHeight;

	cell = UIfirstChild(_self);
	while(cell) {
		cell2 = UInextChild(cell);
		if(class(cell) == UITableViewCell) _ViewListAdd(&_self->reuseCells, cell);
		cell = cell2;
	}
	UIremoveChildAll(_self);
	UIframe(_self, &rt);
	cellWidth = rt.width; tabHeight = rt.height;
	_self->contentSize = (*_self->onNumberOfRowsInTableView)(_self->dataSource, _self);
//printf("rowHeight=%d\n", _self->rowHeight);
	if(_self->onHeightForRowAtIndex) {
		yt = _self->_.contentOffset.y;
		h = 0;
		for(index = 0;index < _self->contentSize;index++) {
			h += (*_self->onHeightForRowAtIndex)(_self->dataSource, _self, index);
		}
	} else if(_self->rowHeight > 0) {
		yt = _self->_.contentOffset.y;
		h = _self->rowHeight * _self->contentSize;
	} else {
		h = 0;
	}
	if(h > 0) {
		if(yt + tabHeight > h) yt = h - tabHeight;
		if(yt < 0) yt = 0;
		y = 0;
		for(index = 0;index < _self->contentSize;index++) {
			if(_self->onHeightForRowAtIndex)
				h = (*_self->onHeightForRowAtIndex)(_self->dataSource, _self, index);
			else	h = _self->rowHeight; 
			if(yt >= y && yt < yt+h) break;
			index++;
		}
//printf("index=%d offset=%d y=%d\n", index, yt, y);
	} else {
		yt = y = index = 0;
	}
	_self->_.contentOffset.y = yt;
	_self->contentIndex = index;
	_self->scrollDy = 0;
	yt += tabHeight;
	while(y < yt && index < _self->contentSize) {
		cell = (*_self->onCellForRowAtIndex)(_self->dataSource, _self, index);
		if(!cell) break;
		h = _TableViewCellMake(cell, cellWidth);
		UIframe(cell, &rt);
		if(rt.width <= 0) rt.width = cellWidth;
		rt.x = 0; rt.y = y; rt.height = h;
		UIsetFrame(cell, &rt);
		UIaddChild(_self, cell);
//printf("reloadData: index=%d cell=%x\n", index, cell);
		y += rt.height; index++;
	}
	y = _self->contentIndex - 10; if(y < 0) y = 0;		
	if(_self->contentSize > 0) {
		index += 10; if(index > _self->contentSize-1) index = _self->contentSize - 1;
	}
	if(_self->onPrefetchRowsAtIndexs) (*_self->onPrefetchRowsAtIndexs)(_self->dataSource, _self, y, index);
	UIsetNeedsDisplay(self);
}

void UIreloadRowAtIndex(void *self, int index)
{
	struct UITableView *_self = self;
	void	*cell, *cell2;
	CGRect	rt;

	cell = UIcellForRowAtIndex(_self, index);
//printf("reloadRow cell=%x index=%d\n", cell, index);
	if(!cell) return;
	UIframe(cell, &rt);
	// Bug fixed at 2018.12.26: order change from _ViewLiadAdd, UIremoveChild
	cell2 = UInextChild(cell);
	UIremoveFromChild(cell);
	_ViewListAdd(&_self->reuseCells, cell);	
	cell = (*_self->onCellForRowAtIndex)(_self->dataSource, _self, index);
	UIsetFrame(cell, &rt);
	// Bug fixed at 2018.12.26: UIaddChild
	UIinsertChildAboveChild(_self, cell, cell2);
}

int UIloadLastRow(void *self)
{
	struct UITableView *_self = self;
	void	*cell, *cells[24];
	CGRect	rt;
	int		y, h, rval, count, index, cellWidth, tabHeight, offsetY;

	UIframe(_self, &rt);
	cellWidth = rt.width; tabHeight = rt.height;
	offsetY = _self->_.contentOffset.y;
	index = _self->contentIndex;
	count = UIvisibleCells(_self, cells);
	index += count - 1;
	UIframe(cells[count-1], &rt);
	y = rt.y + rt.height;
	if(index == _self->contentSize-2 && y < offsetY + tabHeight) {
		index++;
		cell = (*_self->onCellForRowAtIndex)(_self->dataSource, _self, index);
		if(cell) {
			h = _TableViewCellMake(cell, cellWidth);
			UIframe(cell, &rt);
			if(rt.width <= 0) rt.width = cellWidth;
			rt.x = 0; rt.y = y; rt.height = h;
			UIsetFrame(cell, &rt);
			UIaddChild(_self, cell);
			rval = 1;
		} else	rval = -1;
	} else	rval = 0;
	return rval;
}

void _TableViewScroll(void *self, int dy)
{
	struct UITableView *_self = self;
	void	*cell, *cells[24];
	CGRect	rt;
	int		i, y, yt, h, count, index, cellWidth, tabHeight, offsetY;

	if(!_self->onNumberOfRowsInTableView) return;
	UIframe(_self, &rt);
	cellWidth = rt.width; tabHeight = rt.height;
	offsetY = _self->_.contentOffset.y;
	index = _self->contentIndex;
	if(dy > 0) {
		if(index == 0 && offsetY <= 0) return;
		cell = UIfirstChild(_self);
		UIframe(cell, &rt);
		y = rt.y;
		yt = offsetY - dy;
		while(index > 0 && y > yt) {
			index--;
			cell = (*_self->onCellForRowAtIndex)(_self->dataSource, _self, index);
			if(!cell) break;
			h = _TableViewCellMake(cell, cellWidth);
			y -= h;
			UIframe(cell, &rt);
			if(rt.width <= 0) rt.width = cellWidth;
			rt.x = 0; rt.y = y; rt.height = h;
			UIsetFrame(cell, &rt);
			UIaddChildFirst(_self, cell);
		}
		if(index <= 0 && y > yt) dy = offsetY - y;
		offsetY -= dy;
		yt = offsetY + tabHeight;
		count = UIvisibleCells(_self, cells);
		for(i = 0;i < count;i++) {
			cell = cells[i];
			UIframe(cell, &rt);
			if(rt.y >= yt) {
				UIremoveFromChild(cell);
				_ViewListAdd(&_self->reuseCells, cell);
			}
		}
	} else {
		count = UIvisibleCells(_self, cells);
		if(_self->contentSize < 1 || count < 1) return;
		index += count - 1;
		UIframe(cells[count-1], &rt);
		y = rt.y + rt.height;
		if(index >= _self->contentSize-1 && y <= offsetY + tabHeight) return;
		yt = offsetY + tabHeight - dy;
		while(index < _self->contentSize-1 && y < yt) {
			index++;
			cell = (*_self->onCellForRowAtIndex)(_self->dataSource, _self, index);
			if(!cell) break;
			h = _TableViewCellMake(cell, cellWidth);
			UIframe(cell, &rt);
			if(rt.width <= 0) rt.width = cellWidth;
			rt.x = 0; rt.y = y; rt.height = h;
			UIsetFrame(cell, &rt);
			UIaddChild(_self, cell);
			y += h;
			cells[count] = cell; count++;
		}
		if(index >= _self->contentSize-1 && y < yt) dy = offsetY + tabHeight - y;
		offsetY -= dy;
		yt = offsetY;
 		index = _self->contentIndex;
		for(i = 0;i < count;i++) {
			cell = cells[i];
			UIframe(cell, &rt);
			if(rt.y + rt.height < yt) {
				UIremoveFromChild(cell);
				_ViewListAdd(&_self->reuseCells, cell);
				index++;
			}
		}
	}
	_self->_.contentOffset.y = offsetY;
	_self->contentIndex = index;
	_self->scrollDy = dy;
	if(dy) {
		rt.x = 0;
		rt.width = cellWidth; rt.height = dy;
		if(dy > 0) {
			rt.y = 0; rt.height = dy;
		} else {
			rt.y = tabHeight + dy; rt.height = -dy;
		}
		y = _self->contentIndex - 10; if(y < 0) y = 0;		
		if(_self->contentSize > 0) {
			index = _self->contentIndex + count + 10;
			if(index > _self->contentSize-1) index = _self->contentSize - 1;
		}
		if(_self->onPrefetchRowsAtIndexs) (*_self->onPrefetchRowsAtIndexs)(_self->dataSource, _self, y, index);
		UIsetNeedsDisplayInRect(_self, &rt);
	}
}

void _TableViewSelectRow(void *self, void *cell)
{
	struct UITableView	*_self = self;
	int		index;

//	UIsetSelected(_self, TRUE);		commented 2018.4.2
	index = UIindexForCell(_self, cell);
	if(_self->delegate && _self->onDidSelectRowAtIndex) (*_self->onDidSelectRowAtIndex)(_self->delegate, _self, index);
}	

void *UIdequeueReusableCellWithIdentifier(void *self, int identifier)
{
	struct UITableView	*_self = self;
	void	*v;

	v = _self->reuseCells;
	while(v) {
//printf("resuse cell=%x %d %d\n", v, UIreuseIdentifier(v), identifier);
		if(UIreuseIdentifier(v) == identifier) break;
		v = UInextChild(v);
	}
	if(v) _ViewListRemove(&_self->reuseCells, v);
	return v;
}

void UIdequeueReusableCellAll(void *self)
{
	struct UITableView	*_self = self;

	_self->reuseCells = NULL;
}

void *UIdelegate(void *self)
{
	struct UITableView	*_self = self;

	return _self->delegate;
}

void UIsetDelegate(void *self, void *delegate)
{
	struct UITableView	*_self = self;

	_self->delegate = delegate;
}

void *UIdataSource(void *self)
{
	struct UITableView	*_self = self;

	return _self->dataSource;
}

void UIsetDataSource(void *self, void *dataSource)
{
	struct UITableView	*_self = self;

	_self->dataSource = dataSource;
}

BOOL UIallowsMultipleSelection(void *self)
{
	struct UITableView	*_self = self;
	
	if(_self->allowsMultipleSelection) return TRUE;
	else	return FALSE;
}

void UIsetAllowsMultipleSelection(void *self, BOOL allowsMultipleSelection)
{
	struct UITableView	*_self = self;

	if(allowsMultipleSelection) _self->allowsMultipleSelection = 1;
	else	_self->allowsMultipleSelection = 0;
}

int UIrowHeight(void *self)
{
	struct UITableView	*_self = self;

	return _self->rowHeight;
}

void UIsetRowHeight(void *self, int rowHeight)
{
	struct UITableView	*_self = self;

	_self->rowHeight = rowHeight;
}

UITableViewCellSeperatorStyle UIseperatorStyle(void *self)
{
	struct UITableView	*_self = self;

	return (UITableViewCellSeperatorStyle)_self->seperatorStyle;
}

void UIsetSeperatorStyle(void *self, UITableViewCellSeperatorStyle seperatorStyle)
{
	struct UITableView	*_self = self;

	_self->seperatorStyle = seperatorStyle;
}

UIColor UIseperatorColor(void *self)
{
	struct UITableView *_self = self;

	return _self->seperatorColor;
}

void UIsetSeperatorColor(void *self, UIColor seperatorColor)
{
	struct UITableView *_self = self;

	_self->seperatorColor = seperatorColor;
}

void *UIcellForRowAtIndex(void *self, int index)
{
	struct UITableView *_self = self;
	void	*v;
	int		idx;

	idx = _self->contentIndex - 1;
	v = UIfirstChild(_self);
	while(v) {
		if(class(v) == UITableViewCell) {
			idx++;
			if(idx == index) break;
		}
		v = UInextChild(v);	
	}
	return v;
}

int UIindexForCell(void *self, void *cell)
{
	struct UITableView *_self = self;
	void	*v;
	int		idx;

	idx = _self->contentIndex - 1;
	v = UIfirstChild(_self);
	while(v) {
		if(class(v) == UITableViewCell) {
			idx++;
			if(v == cell) break;
		}
		v = UInextChild(v);	
	}
	if(!v) idx = -1;
	return idx;
}
	
int UIvisibleCells(void *self, void **cells)
{
	struct UITableView	*_self = self;
	void	*v, **p;
	int		count;

	p = cells; count = 0;
	v = UIfirstChild(_self);
	while(v) {
		if(class(v) == UITableViewCell) {
			*p++ = v; count++;
		}
		v = UInextChild(v);
	}
	return count;
}

void UIsetOnNumberOfRowsInTableView(void *self, int (*onNumberOfRowsInTableView)(void *, void *))
{
	struct UITableView *_self = self;

	_self->onNumberOfRowsInTableView = onNumberOfRowsInTableView;
}

void UIsetOnCellForRowAtIndex(void *self, void *(*onCellForRowAtIndex)(void *, void *, int))
{
	struct UITableView *_self = self;

	_self->onCellForRowAtIndex = onCellForRowAtIndex;
}

void UIsetOnPrefetchRowsAtIndexs(void *self, void *(*onPrefetchRowsAtIndexs)(void *, void *, int, int))
{
	struct UITableView *_self = self;

	_self->onPrefetchRowsAtIndexs = onPrefetchRowsAtIndexs;
}

void UIsetOnDidSelectRowAtIndex(void *self, void (*onDidSelectRowAtIndex)(void *, void *, int))
{
	struct UITableView *_self = self;

	_self->onDidSelectRowAtIndex = onDidSelectRowAtIndex;
}

void UIsetOnHeightForRowAtIndex(void *self, int (*onHeightForRowAtIndex)(void *, void *, int))
{
	struct UITableView *_self = self;

	_self->onHeightForRowAtIndex = onHeightForRowAtIndex;
}

