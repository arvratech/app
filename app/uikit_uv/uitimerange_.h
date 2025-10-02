#ifndef _UITIME_RANGE__H
#define _UITIME_RANGE__H


#include "uicontrol_.h"


struct UITimeRangeClass {
	struct UIControlClass	_;
};

struct UITimeRange {
	struct UIControl	_;
	unsigned char	status;
	unsigned char	timeRange[4];
	unsigned char	reserve[3];
};


#endif
