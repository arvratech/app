#ifndef _UITEXT_FIELD__H
#define _UITEXT_FIELD__H


#include "uicontrol_.h"


struct UITextFieldClass {
	struct UIControlClass	_;
};

struct UITextField {
	struct UIControl	_;
	unsigned char	textAlignment;
	unsigned char	font;
	unsigned char	lineSpacing;
	unsigned char	text[165];
	UIColor			textColor;
	unsigned char	placeHolder[64];
	unsigned char	borderStyle;
	unsigned char	viewMode;
	unsigned char	reserve[2];
	int				textPosition;
};


#endif
