#include "cgrect.h"


void pointInit(CGPoint *point, int x, int y)
{
	point->x = x; point->y = y;
}

void sizeInit(CGSize *size, int width, int height)
{
	size->width = width; size->height = height;
}

void rectInit(CGRect *rect, int x, int y, int width, int height)
{
	rect->x = x; rect->y = y; rect->width = width; rect->height = height;
}

int rectIsNull(CGRect *rect)
{
	if(rect->width > 0 && rect->height > 0) return 0;
	else	return 1;
}

void rectOffset(CGRect *rect, int offsetX, int offsetY)
{
	rect->x += offsetX; rect->y += offsetY;
}

void rectInset(CGRect *rect, int insetX, int insetY)
{
	rect->x += insetX; rect->y += insetY;
	rect->width -= insetX + insetX; rect->height -= insetY + insetY;
}

int rectContainsPoint(CGRect *rect, CGPoint *pt)
{
	if(pt->x >= rect->x && pt->x < rect->x+rect->width && pt->y >= rect->y && pt->y < rect->y+rect->height) return 1;
	else	return 0;
}

int rectContainsRect(CGRect *rect, CGRect *rectT)
{
	int		x, y;

	x = rectT->x; y = rectT->y;
	if(x < rect->x || x >= rect->x+rect->width || y < rect->y || y >= rect->y+rect->height) return 0;
	x += rect->width - 1;
	if(x < rect->x || x >= rect->x+rect->width || y < rect->y || y >= rect->y+rect->height) return 0;
	y += rect->height - 1;
	if(x < rect->x || x >= rect->x+rect->width || y < rect->y || y >= rect->y+rect->height) return 0;
	x -= rect->width - 1;
	if(x < rect->x || x >= rect->x+rect->width || y < rect->y || y >= rect->y+rect->height) return 0;
	else	return 1;
}

int rectIntersectsRect(CGRect *rect, CGRect *rectT)
{
	int		x, y;

	x = rectT->x; y = rectT->y;
	if(x > rect->x && x < rect->x+rect->width && y >= rect->y && y < rect->y+rect->height) return 1;
	x += rect->width - 1;
	if(x > rect->x && x < rect->x+rect->width && y >= rect->y && y < rect->y+rect->height) return 1;
	y += rect->height - 1;
	if(x > rect->x && x < rect->x+rect->width && y >= rect->y && y < rect->y+rect->height) return 1;
	x -= rect->width - 1;
	if(x > rect->x && x < rect->x+rect->width && y >= rect->y && y < rect->y+rect->height) return 1;
	else	return 0;
}

void rectUnion(CGRect *rect1, CGRect *rect2, CGRect *rect)
{
	int	 	x1, y1, x2, y2;

	x1 = rect1->x + rect1->width - 1; y1 = rect1->y + rect1->height - 1;
	x2 = rect2->x + rect2->width - 1; y2 = rect2->y + rect2->height - 1;
	if(x2 < x1) x2 = x1;
	if(y2 < y1) y2 = y1;
	if(rect1->x > rect2->x) x1 = rect2->x; else x1 = rect1->x;
	if(rect1->y > rect2->y) y1 = rect2->y; else y1 = rect1->y;
	rect->x = x1; rect->y = y1;
	rect->width = x2 - x1 + 1; rect->height = y2 - y1 + 1;
}

void rectIntersection(CGRect *rect1, CGRect *rect2, CGRect *rect)
{
	int	 	x1, y1, x2, y2;

	x1 = rect1->x + rect1->width - 1; y1 = rect1->y + rect1->height - 1;
	x2 = rect2->x + rect2->width - 1; y2 = rect2->y + rect2->height - 1;
	if(x2 > x1) x2 = x1;
	if(y2 > y1) y2 = y1;
	if(rect1->x < rect2->x) x1 = rect2->x; else x1 = rect1->x;
	if(rect1->y < rect2->y) y1 = rect2->y; else y1 = rect1->y;
	rect->x = x1; rect->y = y1;
	rect->width = x2 - x1 + 1; rect->height = y2 - y1 + 1;
}

void edgeInsetsInit(CGEdgeInsets *edgeInsets, int top, int left, int bottom, int right)
{
	edgeInsets->top = top; edgeInsets->left = left; edgeInsets->bottom = bottom; edgeInsets->right = right;
}

void rectInsetBy(CGRect *rect, int dx, int dy)
{
	rect->x += dx; rect->y += dy;
	rect->width -= dx + dx; rect->height -= dy + dy;
}

void rectInsetEdgeInsets(CGRect *rect, CGEdgeInsets *edgeInsets)
{
	rect->x += edgeInsets->left; rect->y += edgeInsets->top;
	rect->width -= edgeInsets->left + edgeInsets->right;
	rect->height -= edgeInsets->top + edgeInsets->bottom;
}
