#ifndef _UILABEL__H
#define _UILABEL__H


#include "uiview_.h"


#define UIL_DISABLED		0x01
#define UIL_HIGHLIGHTED		0x02


struct UILabelClass {
	struct UIViewClass	_;
};

//                                       state
// |    7    |    6    |    5    |    4    |    3    |    2    |    1    |    0    |
//                                                             highlighted disabled

struct UILabel {
	struct UIView	_;
	unsigned char	state;
	unsigned char	reserve[3];
	unsigned char	textAlignment;
	unsigned char	font;
	unsigned char	lineSpacing;	
	unsigned char	text[165];
	UIColor			textColor;
	UIColor			highlightedTextColor;
	UIColor			shadowColor;
	short			shadowOffsetX, shadowOffsetY;
};


#endif
