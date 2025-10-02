#ifndef _UIWINDOW__H
#define _UIWINDOW__H


#include "uiview_.h"


struct UIWindowClass {
	struct UIViewClass	_;
};

struct UIWindow {
	struct UIView	_;
	unsigned char	reserve[4];
};


#endif
