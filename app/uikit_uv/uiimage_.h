#ifndef _UIIMAGE__H
#define _UIIMAGE__H


#include "nsobject_.h"


struct UIImageClass {
	struct NSClass	_;
};

struct UIImage {
	struct NSObject	_;
	unsigned char	imageType;
	unsigned char	iconSize;
	unsigned char	renderingMode;
	char			fileName[65];
	void			*mem;
	int				memSize;
};


#endif
