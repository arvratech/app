#ifndef _UISWITCH__H
#define _UISWITCH__H


#include "uicontrol_.h"


struct UISwitchClass {
	struct UIControlClass	_;
};

struct UISwitch {
	struct UIControl	_;
	unsigned char	onOff;
	unsigned char	reserve[3];
};


#endif
