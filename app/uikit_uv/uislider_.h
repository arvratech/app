#ifndef _UISLIDER__H
#define _UISLIDER__H


#include "uicontrol_.h"


struct UISliderClass {
	struct UIControlClass	_;
};

struct UISlider {
	struct UIControl	_;
	int			value;
	int			minimumValue;
	int			maximumValue;
	UIColor		minimumTrackTintColor;
	UIColor		maximumTrackTintColor;
	UIColor		thumbTintColor;
};


#endif
