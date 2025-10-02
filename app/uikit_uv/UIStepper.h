#ifndef _UISTEPPER_H
#define _UISTEPPER_H


#include "UIControl.h"


extern void *UIStepperClass;	// adds draw 
extern void *UIStepper;			// self = alloc(UILabel); init(self, x, y, width, height);


void initUIStepper(void);

int  UIstepValue(void *self);
void UIsetStepValue(void *self, int value);
BOOL UIisContinuous(void *self);
void UIsetContinuous(void *self, BOOL continuous);
BOOL UIautorepeat(void *self);
void UIsetAutorepeat(void *self, BOOL autorepeat);
BOOL UIwraps(void *self);
void UIsetWraps(void *self, BOOL wraps);


#endif
