#ifndef _UITIME_RANGE_H
#define _UITIME_RANGE_H


#include "UIControl.h"


extern void *UITimeRangeClass;	// adds draw 
extern void *UITimeRange;		// self = alloc(UILabel); init(self, x, y, width, height);


void initUITimeRange(void);

void UItimeRange(void *self, unsigned char *timeRange);
void UIsetTimeRange(void *self, unsigned char *timeRange);


#endif
