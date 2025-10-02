#ifndef _UICONTROL__H
#define _UICONTROL__H


#include "uiview_.h"


#define UIC_DISABLED		0x01
#define UIC_HIGHLIGHTED		0x02
#define UIC_SELECTED		0x04


struct UIControlClass {
	struct UIViewClass	_;
};

//                                       state
// |    7    |    6    |    5    |    4    |    3    |    2    |    1    |    0    |
//                                                    selected highlighted disabled

struct UIControl {
	struct UIView	_;
	unsigned char	state;
	unsigned char	contentAlignment;
	unsigned char	reserve[2];
};


#endif
