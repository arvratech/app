#ifndef _UIDATE_PICKER_H
#define _UIDATE_PICKER_H


#include "UIControl.h"


extern void *UIDatePickerClass;	// adds draw 
extern void *UIDatePicker;		// self = alloc(UILabel); init(self, x, y, width, height);


void initUIDatePicker(void);

int  UIyear(void *self);
void UIsetYear(void *self, int year);
int  UImonth(void *self);
void UIsetMonth(void *self, int month);
int  UIday(void *self);
void  UIsetDay(void *self, int day);
int  UIlocale(void *self);
void  UIsetLocale(void *self, int locale);


#endif
