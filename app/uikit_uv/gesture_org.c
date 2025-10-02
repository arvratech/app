#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtc.h"
#include "gwnd.h"
#include "gview.h"
#include "gos.h"
#include "gesture.h"

#define MAX_TPOINT_SZ	16

typedef struct _TPOINT {
	short	x, y;
	long	time;
} TPOINT;

TPOINT Tpts[MAX_TPOINT_SZ];
unsigned char	TptCount, TptOver; 
unsigned char	TouchState; 
struct timeval	*DownTm, _DownTm;


void gestureInit(void)
{
	TouchState = 0;
	TptCount = TptOver = 0;
	DownTm = &_DownTm;
}

//  State  Event        State  Event        State  Event  
//    0  |                0  |                  0  |
//  -----| OnDown       -----| OnDown         -----| OnDown
//    1  |                1  |                  1  |
//  -----| OnShowPress  -----| OnShowPress    -----| OnShowPress  
//    2  |                2  |                  2  |
// ------| OnTabUp      -----| OnLongPress    -----| OnScroll 
//    0  |                3  |                  4  |
//       |              -----|                     | OnScroll
//       |                0  |                 ----| OnFling
//       |                   |                  0  |  

#define ABS(x)	(((x) <0) ? -(x) : (x))

void gestureProc(int x, int y, int pressure)
{
	struct timeval	_curtm;
	GMSG	*msg, _msg;
	short	sval;
	long	diff;
	int		i, dx, dy, sumt;

//printf("[%d,%d %d]\n", x, y, pressure);
	msg = &_msg;
	msg->message = GM_GESTURE;
	if(TouchState == 0) {
		if(pressure) {
			Tpts[0].x = x; Tpts[0].y = y; Tpts[0].time = 0L;
			TptCount = 1; TptOver = 0;
			gettimeofday(DownTm, NULL);
			TouchState = 1;
			msg->code = ON_DOWN;
			msg->wParam = (x << 16) | y;  msg->lParam = 0;
			PostMessage(msg);
		}
	} else if(TouchState < 3) {
		if(pressure) {
			gettimeofday(&_curtm, NULL);
			diff = GetTimeDiff(&_curtm, DownTm);
			dx = x - Tpts[0].x; dy = y - Tpts[0].y;
			if(ABS(dx) + ABS(dy) > 5) {
				msg->code = ON_SCROLL;
				msg->wParam = (dx << 16) | dy; msg->lParam = 0;
				PostMessage(msg);
				TouchState = 4;
			} else if(TouchState == 1 && diff > 100) {
				msg->code = ON_SHOW_PRESS;
				msg->wParam = (x << 16) | y;  msg->lParam = 0;
				PostMessage(msg);
				TouchState = 2;;
			} else if(TouchState == 2 && diff > 600) {
				msg->code = ON_LONG_PRESS;
				msg->wParam = (x << 16) | y;  msg->lParam = 0;
				PostMessage(msg);
				TouchState = 3;;
			}
			Tpts[TptCount].x = x; Tpts[TptCount].y = y; Tpts[TptCount].time = diff; 
			TptCount++; if(TptCount >= MAX_TPOINT_SZ) { TptOver = 1; TptCount = 0; }
		} else {
			msg->code = ON_TAB_UP;
			msg->wParam = (x << 16) | y; msg->lParam = 0;
			PostMessage(msg);
			TouchState = 0;
		}
	} else if(TouchState > 3) {
		if(pressure) {
			gettimeofday(&_curtm, NULL);
			diff = GetTimeDiff(&_curtm, DownTm);
			i = TptCount - 1; if(i < 0) i = MAX_TPOINT_SZ - 1;
			dx = x - Tpts[i].x; dy = y - Tpts[i].y; 
			msg->code = ON_SCROLL;
			msg->wParam = (dx << 16) | dy; msg->lParam = 0;
			PostMessage(msg);
			Tpts[TptCount].x = x; Tpts[TptCount].y = y; Tpts[TptCount].time = diff; 
			TptCount++; if(TptCount >= MAX_TPOINT_SZ) { TptOver = 1; TptCount = 0; }
		} else {
			if(TptOver) {
				i = TptCount; sumt = MAX_TPOINT_SZ;
			} else {
				i = 0; sumt = TptCount;
			}
			x = Tpts[i].x; y = Tpts[i].y; diff = Tpts[i].time;
			dx = dy = 0;
			if(sumt > 2) {
				sumt = 0;
				i++; if(i >= MAX_TPOINT_SZ) i = 0;
				while(i != TptCount) {
					dx += Tpts[i].x - x;  dy += Tpts[i].y - y; sumt += Tpts[i].time - diff; 
					i++; if(i >= MAX_TPOINT_SZ) i = 0;
				}
				dx = dx * 1000 / sumt; dy = dy * 1000 / sumt;
			}
			msg->code = ON_FLING;
			msg->wParam = dx; msg->lParam = dy;
			PostMessage(msg);
			TouchState = 0;
		}
	} else {
		if(pressure == 0) {
			TouchState = 0;
		}
	}
}
