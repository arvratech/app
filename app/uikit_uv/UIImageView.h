#ifndef _UIIMAGE_VIEW_H
#define _UIIMAGE_VIEW_H


#include "UIView.h"


extern void *UIImageViewClass;	// adds draw 
extern void *UIImageView;		// self = alloc(UILabel); init(self, x, y, width, height);


void initUIImageView(void);

void *UIimage(void *self);
void UIsetImage(void *self, void *image);
void *UIhighlightedImage(void *self);
void UIsetHighlightedImage(void *self, void *highlightedImage);
void UIsetRotateImage(void *self);

#endif
