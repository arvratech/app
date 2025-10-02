#ifndef _UICONTROL_H
#define _UICONTROL_H


#include "UIView.h"


extern void *UIControlClass;	// adds draw 
extern void *UIControl;			// self = alloc(UIControl); init(self, x, y, width, height);


void initUIControl(void);

UIControlState UIstate(void *self);
BOOL UIisEnabled(void *self);
void UIsetEnabled(void *self, BOOL isEnabled);
BOOL UIisHighlighted(void *self);
void UIsetHighlighted(void *self, BOOL isHighlighted);
BOOL UIisSelected(void *self);
void UIsetSelected(void *self, BOOL isSelected);
UIContentVerticalAlignment UIcontentVerticalAlignment(void *self);
void UIsetContentVerticalAlignment(void *self, UIContentVerticalAlignment contentVerticalAlignment);
UIContentHorizontalAlignment UIcontentHorizontalAlignment(void *self);
void UIsetContentHorizontalAlignment(void *self, UIContentHorizontalAlignment contentHorizontalAlignment);


#endif
