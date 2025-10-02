#ifndef _UISTEPPER__H
#define _UISTEPPER__H


#include "uicontrol_.h"


struct UIStepperClass {
	struct UIControlClass	_;
};

struct UIStepper {
	struct UIControl	_;
	int			value;
	int			minimumValue;
	int			maximumValue;
	unsigned char	flags;
	unsigned char	pad[3];
	int			stepValue;
};


#endif
