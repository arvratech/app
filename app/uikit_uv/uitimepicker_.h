#ifndef _UITIME_PICKER__H
#define _UITIME_PICKER__H


#include "uicontrol_.h"
#include "uilabel_.h"
#include "uiimage_.h"
#include "uibutton_.h"


struct UITimePickerClass {
	struct UIControlClass	_;
};

struct UITimePicker {
	struct UIControl	_;
	unsigned char	locale;
	unsigned char	status;
	unsigned char	hour;
	unsigned char	minute;
	unsigned char	option;
	unsigned char	reserve[3];
	struct UILabel	_titleLabels[2];
	struct UIButton	_buttons[2][2];
	struct UIImage	_images[2][2];
	struct UIButton	_noonButton;
};


#endif

