#ifndef _UIDATE_PICKER__H
#define _UIDATE_PICKER__H


#include "uicontrol_.h"
#include "uilabel_.h"
#include "uiimage_.h"
#include "uibutton_.h"


struct UIDatePickerClass {
	struct UIControlClass	_;
};

struct UIDatePicker {
	struct UIControl	_;
	unsigned char	locale;
	unsigned char	status;
	unsigned short	year;
	unsigned char	month;
	unsigned char	day;
	unsigned char	reserve[2];
	struct UILabel	_titleLabels[3];
	struct UIButton	_buttons[3][2];
	struct UIImage	_images[3][2];
};


#endif

