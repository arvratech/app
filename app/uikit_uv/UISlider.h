#ifndef _UISLIDER_H
#define _UISLIDER_H


#include "UIControl.h"


extern void *UISliderClass;		// adds draw 
extern void *UISlider;			// self = alloc(UILabel); init(self, x, y, width, height);


void initUISlider(void);

int  UIvalue(void *self);
void UIsetValue(void *self, int value);
int  UIminumumValue(void *self);
void UIsetMinimumValue(void *self, int mninimumValue);
int  UImaximumValue(void *self);
void UIsetMaximumValue(void *self, int maximumValue);
UIColor UIminimumTrackTintColor(void *self);
void UIsetMinimumTrackTintColor(void *self, UIColor minimumTrackTintColor);
UIColor UImaximumTrackTintColor(void *self);
void UIsetMaximumTrackTintColor(void *self, UIColor maximumTrackTintColor);
UIColor UIthumbTintColor(void *self);
void UIsetThumbTintColor(void *self, UIColor thumbTintColor);


#endif
