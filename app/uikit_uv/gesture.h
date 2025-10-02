#ifndef _GESTURE_H_
#define _GESTURE_H_


#define ON_DOWN				1
#define ON_SHOW_PRESS		2
#define ON_TAB_UP			3
#define ON_LONG_PRESS		4
#define ON_SCROLL			5
#define ON_FLING			6


typedef struct _GESTURE {
	int		type;
	int		x, y;
} GESTURE;


void gestureInit(void *loop);
void gestureProc(int x, int y, int pres);


#endif

