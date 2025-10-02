#ifndef _UISCROLL_VIEW__H
#define _UISCROLL_VIEW__H


#include "uiview_.h"


struct UIScrollViewClass {
	struct UIViewClass	_;
};

struct UIScrollView {
	struct UIView	_;
	CGPoint			contentOffset;
	CGSize			contentSize;
	unsigned char	state;
	unsigned char	reserve[3];
};


#endif
