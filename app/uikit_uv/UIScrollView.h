#ifndef _UISCROLL_VIEW_H
#define _UISCROLL_VIEW_H


#include "UIView.h"


extern void *UIScrollView;			// self = alloc(UILabel); init(self, x, y, width, height);
extern void *UIScrollViewClass;		// adds draw 


void initUIScrollView(void);


void UIcontentOffset(void *self, CGPoint *contentOffset);
void UIsetContentOffset(void *self, CGPoint *contentOffset);
void UIcontentSize(void *self, CGSize *contentSize);
void UIsetContentSize(void *self, CGSize *contentSize);


#endif
