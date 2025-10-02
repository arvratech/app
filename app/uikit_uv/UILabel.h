#ifndef _UILABEL_H
#define _UILABEL_H


#include "UIView.h"


extern void *UILabel;			// self = alloc(UILabel); init(self, x, y, width, height);
extern void *UILabelClass;		// adds draw 


void initUILabel(void);

char *UItext(void *self);
void UIsetText(void *self, char *text);
int  UIfont(void *self);
void UIsetFont(void *self, int font);
UIColor UItextColor(void *self);
void UIsetTextColor(void *self, UIColor textColor);
UITextAlignment UItextAlignment(void *self);
void UIsetTextAlignment(void *self, UITextAlignment textAlignment);
int  UIlineSpacing(void *self);
void UIsetLineSpacing(void *self, int lineSpacing);
UIColor UIhighlightedTextColor(void *self);
void UIsetHighlightedTextColor(void *self, UIColor highlightedTextColor);
UIColor UIshadowColor(void *self);
void UIsetShadowColor(void *self, UIColor shadowColor);
void UIshadowOffset(void *self, CGSize *shadowOffset);
void UIsetShadowOffset(void *self, CGSize *shadowOffset);


#endif
