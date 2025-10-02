#ifndef _CGRECT_H
#define _CGRECT_H


typedef struct _CGPoint {
	int		x;
	int		y;
} CGPoint;

typedef struct _CGSize {
	int		width;
	int		height;
} CGSize;

typedef struct _CGRect {
	union {
		struct {
			int x, y;
		} origin;
		struct {
			int x, y;
		};
	};
	union {
		struct {
			int width, height;
		} size;
		struct {
			int width, height;
		};
	};
} CGRect;

typedef struct _CGEdgeInsets {
	int		top;
	int		left;
	int		bottom;
	int		right;
} CGEdgeInsets;


void pointInit(CGPoint *point, int x, int y);
void sizeInit(CGSize *size, int width, int height);
void rectInit(CGRect *rect, int x, int y, int width, int height);
int  rectIsNull(CGRect *rect);
void rectOffset(CGRect *rect, int offsetX, int offsetY);
void rectInset(CGRect *rect, int insetX, int insetY);
int  rectContainsPoint(CGRect *rect, CGPoint *pt);
int  rectContainsRect(CGRect *rect, CGRect *rectT);
int  rectIntersectsRect(CGRect *rect, CGRect *rectT);
void rectUnion(CGRect *rect1, CGRect *rect2, CGRect *rect);
void rectIntersection(CGRect *rect1, CGRect *rect2, CGRect *rect);
void edgeInsetsInit(CGEdgeInsets *edgeInsets, int top, int left, int bottom, int right);
void rectInsetBy(CGRect *rect, int dx, int dy);
void rectInsetEdgeInsets(CGRect *rect, CGEdgeInsets *edgeInsets);



static inline CGRect CGRectMake(int x, int y, int width, int height)
{
	CGRect rect;

	rect.x = x; rect.y = y; rect.width = width; rect.height = height;
	return rect;
}


#endif
