#ifndef _UIVIEW__H
#define _UIVIEW__H


#include "nsobject_.h"


// definition for internal use
#define UIV_HIDDEN				0x01
#define UIV_FOCUSED				0x02
#define UIV_CAN_BECOME_FOCUSED	0x04
#define UIV_OPAQUE				0x08
#define UIV_CLIP_TO_BOUNDS		0x10
#define UIV_USER_INTERACTION_ENABLED  0x20
#define UIV_NEEDS_DISPLAY		0x80

//                                       state
// |    7    |    6    |    5    |    4    |    3    |    2    |    1    |    0    |
//    needs             userInter  cliopTo   opaque   canBecome  focused   hidden  
//   Display         actionEnabled Bounds              Focused
//
//                                       mode
// |    7    |    6    |    5    |    4    |    3    |    2    |    1    |    0    |
//                     |              ContentMode              |  TintAdjustMode   |

struct UIViewClass {
	struct NSClass	_;
	void (*izerwf)(void *self, va_list *app);
	void (*draw)(void *self, CGRect *rect);
};

struct UIView {
	struct NSObject	_;
	unsigned char	state;
	unsigned char	mode;
	unsigned char	alpha;
	unsigned char	reserve[1];
	int				tag;
	short			x, y, w, h;
	short			ix, iy, iw, ih;		// invalidRect
	void			*parent;
	void			*children;
	void			*next;
	UIColor			backgroundColor;
	UIColor			tintColor;
	void (*drawRect)(void *self, CGRect *rect);
};


void izerwf(void *self, va_list *app);
void super_izerwf(void *class, void *self, va_list *app);
void super_UIdraw(void *class, void *self, CGRect *rect);

void _ViewSetParent(void *self, void *parent);
BOOL _ViewIsLastChild(void *self);
void _ViewInvalidRect(void *self, CGRect *rect);


#endif

