#ifndef _UISWITCH_H
#define _UISWITCH_H


#include "UIControl.h"


extern void *UISwitchClass;		// adds draw 
extern void *UISwitch;			// self = alloc(UILabel); init(self, x, y, width, height);


void initUISwitch(void);

BOOL UIisOn(void *self);
void UIsetOn(void *self, BOOL on);


#endif
