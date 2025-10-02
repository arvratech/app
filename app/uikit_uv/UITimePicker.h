#ifndef _UITIME_PICKER_H
#define _UITIME_PICKER_H


#include "UIControl.h"


extern void *UITimePickerClass;	// adds draw 
extern void *UITimePicker;		// self = alloc(UILabel); init(self, x, y, width, height);


void initUITimePicker(void);

int  UIhour(void *self);
void UIsetHour(void *self, int year);
int  UIminute(void *self);
void UIsetMinute(void *self, int month);
int  UIminute(void *self);
void UIsetMinute(void *self, int month);
BOOL UI24HourView(void *self);
void UIset24HourView(void *self, BOOL hour24View);


#endif
