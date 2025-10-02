#ifndef _UIBUTTON_H
#define _UIBUTTON_H


#include "UIControl.h"


extern void *UIButtonClass;		// adds draw 
extern void *UIButton;			// self = alloc(UILabel); init(self, x, y, width, height);


void initUIButton(void);

void *UItitleLabel(void *self);
void *UIimageView(void *self);
UIColor UIhighlightedColor(void *self);
void UIsetHighlightedColor(void *self, UIColor highlightedColor);
char *UItitle(void *self);
void UIsetTitle(void *self, char *title);
UIColor UItitleColor(void *self);
void UIsetTitleColor(void *self, UIColor textColor);
UIColor UItitleShadowColor(void *self);
void UIsetTitleShadowColor(void *self, UIColor shadowColor);
void *UIbuttonImage(void *self);
void UIsetButtonImage(void *self, void *image);
void *UIbuttonHighlightedImage(void *self);
void UIsetButtonHighlightedImage(void *self, void *image);


#endif
