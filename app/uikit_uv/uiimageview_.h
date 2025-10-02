#ifndef _UIIMAGE_VIEW__H
#define _UIIMAGE_VIEW__H


#include "uiview_.h"


struct UIImageViewClass {
	struct UIViewClass	_;
};

struct UIImageView {
	struct UIView	_;
	unsigned char	state;
	unsigned char	rotate;
	unsigned char	reserve[2];
	void			*image;
	void			*highlightedImage;
};


#endif
