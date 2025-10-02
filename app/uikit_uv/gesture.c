#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtc.h"
#include "cgrect.h"
#include "UIKit.h"
#include "gactivity.h"
#include "gapp.h"
#include "uv.h"
#include "gesture.h"

#define MAX_TPOINT_SZ		16
#define TOUCH_ERR_TIMEOUT	3000


typedef struct _TPOINT {
	short	x, y;
	long	time;
} TPOINT;

static TPOINT tpts[MAX_TPOINT_SZ];
static unsigned char	tptCount, tptScroll, tptOver; 
static unsigned char	touchState, touchCount; 
static short			downX, downY, lastX, lastY;
static unsigned long	downTimer;
static uv_timer_t		_timer;


void gestureInit(void *loop)
{
	touchState = 0; touchCount = 0;
	uv_timer_init((uv_loop_t *)loop, &_timer);
}

static void _GestureAction(GESTURE *gst)
{
	appProcActivity();
	activityProcGesture(appCurrentActivity(), gst); 
//	appDrawView();
}

//  State  Event        State  Event        State  Event  
//    0  |                0  |                  0  |
//  -----| OnDown       -----| OnDown         -----| OnDown
//    1  |                1  |                  1  |
//  -----| OnShowPress  -----| OnShowPress    -----| OnShowPress  
//    2  |                2  |                  2  |
//  -----| OnTabUp      -----| OnLongPress    -----| OnScroll 
//    0  |                3  |                  4  |
//       |              -----| OnTabUp             | OnScroll
//       |                0  |                 ----| OnFling
//       |                   |                  0  |  

#define ABS(x)	(((x) < 0) ? -(x) : (x))

void _OnGestureTimer(uv_timer_t *timer)
{
	GESTURE		*gst, _gst;
	int		timeout;

	if(touchState == 1) {
		gst = &_gst;
		gst->type = ON_SHOW_PRESS; gst->x = lastX; gst->y = lastY; 
		touchState = 2;
		timeout = 600 - (rtcMiliTimer() - downTimer); if(timeout < 1) timeout = 1;
	} else if(touchState == 2) {
		gst = &_gst;
		gst->type = ON_LONG_PRESS; gst->x = lastX; gst->y = lastY; 
		touchState = 3;;
		timeout = TOUCH_ERR_TIMEOUT;
	} else {
printf("touch timeout: %d\n", (int)touchState);
		gst->type = 0;
		touchState = 0; touchCount = 0;
	}
	if(touchState) uv_timer_start(&_timer, _OnGestureTimer, timeout, 0);
	if(gst->type) _GestureAction(gst);
}

void gestureProc(int x, int y, int pres)
{
	GESTURE		*gst, _gst;
	unsigned long	curTimer;
	short	sval;
	long	diff;
	int		i, loop, dx, dy, sumt, timeout;
int		cnt;

	if(touchState) uv_timer_stop(&_timer);
	gst = &_gst;
	if(lcdDirection()) { dy = y; y = lcdHeight()-x; x = dy; }	// 270
	gst->type = 0;
	curTimer = rtcMiliTimer();
	if(touchState == 0) {
		if(pres) {
			lastX = downX = x; lastY = downY = y; downTimer = curTimer;
			gst->type = ON_DOWN; gst->x = x; gst->y = y;
			touchState = 1;
			timeout = 100;
		}
	} else if(touchState < 4) {
		if(pres) {
			diff = curTimer - downTimer;
			dx = x - downX; dy = y - downY;
			if(ABS(dx) + ABS(dy) > 8) {		// 5 => 8	2018.7.27
				gst->type = ON_SCROLL; gst->x = dx; gst->y = dy; 
				tpts[0].x = x; tpts[0].y = y; tpts[0].time = diff; 
				tptScroll = 0; tptCount = 1; tptOver = 0;
				touchState = 4;
			}
			if(touchState == 1) {
				lastX = x; lastY = y;
				timeout = 100 - diff; if(timeout < 1) timeout = 1;
			} else if(touchState == 2) {
				lastX = x; lastY = y;
				timeout = 600 - diff; if(timeout < 1) timeout = 1;
			} else {
				timeout = TOUCH_ERR_TIMEOUT;	
			}
		} else {
			gst->type = ON_TAB_UP; gst->x = x; gst->y = y; 
			touchState = 0; touchCount = 0;
		}
	} else {
		diff = curTimer - downTimer;
		if(pres) {
			i = tptScroll + 4; if(i >= MAX_TPOINT_SZ) i -= MAX_TPOINT_SZ;
			if(i == tptCount) {
				dx = x - tpts[tptScroll].x; dy = y - tpts[tptScroll].y;
				gst->type = ON_SCROLL; gst->x = dx; gst->y = dy; 
				tptScroll = tptCount;
			}
			tpts[tptCount].x = x; tpts[tptCount].y = y; tpts[tptCount].time = diff; 
			tptCount++; if(tptCount >= MAX_TPOINT_SZ) { tptOver = 1; tptCount = 0; }
			timeout = TOUCH_ERR_TIMEOUT;	
		} else {
//printf("[-] %d\n", diff); 
			loop = tptCount;
			if(tptOver) loop += MAX_TPOINT_SZ;
			i = tptCount;		
			i--; if(i < 0) i = MAX_TPOINT_SZ - 1;
			x = tpts[i].x; y = tpts[i].y; diff = tpts[i].time;
			loop--;
			while(loop > 0 && (diff - tpts[i].time) < 300) {
				i--; if(i < 0) i = MAX_TPOINT_SZ - 1;
				loop--;
			}
			dx = x - tpts[i].x; dy = y - tpts[i].y;
			sumt = diff - tpts[i].time;
//if(sumt < 10) printf("================ %d sumt=%d dx=%d dy=%d\n", cnt, sumt, dx, dy);
//else printf("=== %d sumt=%d dx=%d dy=%d\n", cnt, sumt, dx, dy);
			if(sumt < 5) sumt = 5;
			dx = dx * 1000 / sumt; dy = dy * 1000 / sumt;
			gst->type = ON_FLING; gst->x = dx; gst->y = dy;
			touchState = 0; touchCount = 0;
		}
	}
	if(touchState) uv_timer_start(&_timer, _OnGestureTimer, timeout, 0);
	if(gst->type) _GestureAction(gst);
}

