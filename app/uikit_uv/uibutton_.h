#ifndef _UIBUTTON__H
#define _UIBUTTON__H


#include "uicontrol_.h"
#include "uilabel_.h"
#include "uiimageview_.h"


struct UIButtonClass {
	struct UIControlClass	_;
};

struct UIButton {
	struct UIControl	_;
	UIColor			highlightedColor;		// appended by KDK
	struct UIImageView	_imageView;
	struct UILabel		_titleLabel;
};


#endif
