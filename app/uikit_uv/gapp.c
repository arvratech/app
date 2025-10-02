#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "rtc.h"
#include "cbuf.h"
#include "ctfont.h"
#include "lang.h"
#include "msg.h"
#include "lcdc.h"
#include "cgrect.h"
#include "UIKit.h"
#include "gactivity.h"
#include "gesture.h"
#include "actprim.h"
#include "uv.h"
#include "gapp.h"

int		_appDraw;

#define MAX_ACTIVITY_SZ		16

typedef struct _ACT_CMD {
	GACTIVITY		*act;
	unsigned char	method;
	unsigned char	flag;
	unsigned char	reserve[2];
} ACT_CMD;

static GACTIVITY		*gActs[MAX_ACTIVITY_SZ];
static void				*gWnds[MAX_ACTIVITY_SZ];
static int				gActCount;
static unsigned char	WndBuffer[56*MAX_ACTIVITY_SZ];	// must be update if sizeof(struct UIWindow) changed
static unsigned char	gSoundVolume, gSoundOption, gSoundOptionSave;
static ACT_CMD			gActCmds[4];
static int				gActCmdIdx;
static uv_poll_t		_poll_ts;
static uv_poll_t		_poll_actq;
static uv_poll_t		_poll_msgq;
static uv_timer_t		_timerApp;
static int				actq_fd;
static int				msgq_fd;
static uv_idle_t		_idle;
static uv_check_t		_check;
static int				_lcdDimmed;
	

static void _DrawActivity(GACTIVITY *act, CGRect *updateRect)
{
	CGRect		rt, rt2;

//	if(index == 0) wallpaperDraw(updateRect);
	if(UIisHidden(act->window)) return;
	UIframe(act->window, &rt);
	if(rt.height < lcdHeight()) {
		if(updateRect) {
			rt2.x = _scale(updateRect->x); rt2.y = _scale(updateRect->y);
			rt2.width = _scale(updateRect->x+updateRect->width) - rt2.x;
			rt2.height = _scale(updateRect->y+updateRect->height) - rt2.y;
		} else {
			rt2.x = rt2.y = 0; rt2.width = lcdPixelWidth(); rt2.height = lcdPixelHeight();
		}
		lcdClipToRect(&rt2);
		if(!_lcdDimmed) lcdDimm(rt2.x, rt2.y, rt2.width, rt2.height, -100); 
		_lcdDimmed = 1;
		UIsetNeedsDisplayInRect(act->window, updateRect);
		rt.x = rt.y = 0;
		UIdrawView(act->window, &rt);
	} else if(updateRect) {
		UIsetNeedsDisplayInRect(act->window, updateRect);
		rt.x = updateRect->x; rt.y = updateRect->y;
		rt.width = updateRect->width; rt.height = updateRect->height;
		UIdrawView(act->window, &rt);
		_lcdDimmed = 0;
	} else {
		UIsetNeedsDisplayInRect(act->window, updateRect);
		lcdChangePage();
		rt.x = rt.y = 0;
		UIdrawView(act->window, &rt);
		lcdSwapPage();
		_lcdDimmed = 0;
	}
	_appDraw = 0;
}

void appDrawView(void)
{
	GACTIVITY	*act;
	CGRect	rt;

	if(_appDraw) {
		act = gActs[gActCount-1];
		UIframe(act->window, &rt); rt.x = rt.y = 0;
		UIdrawView(act->window, &rt);
		_appDraw = 0;
	}
}

void appDrawCurrentActivity(void)
{
	GACTIVITY	*act;

	act = gActs[gActCount-1];
	_DrawActivity(act, NULL);
}

void appDrawActivity(CGRect *rect)
{
	GACTIVITY	*act, *a;
	CGRect	*rt, _rt;
	int		x, y, w, h, wh;

	act = gActs[gActCount-1];
	UIframe(act->window, &_rt); wh = _rt.height;
	if(rect) {
		x = rect->x; y = rect->y; w = rect->width; h = rect->height;
	} else	x = y = w = h = 0;
	if(w <= 0 || h <= 0) {
		rt = NULL;
		x = y = 0; w = lcdPixelWidth(); h = lcdPixelHeight();
	} else {
		rt = &_rt;
		rt->x = x; rt->y = y; rt->width = w; rt->height = h;
		x = _scale(x); y = _scale(y); w = _scale(rt->x+w) - x; h = _scale(rt->y+h) - y;
	}
	if(wh < lcdHeight() && gActCount > 1) {
		a = gActs[gActCount-2];
		_DrawActivity(a, rt);
		rt->x = x; rt->y = y; rt->width = w; rt->height = h;
		lcdClipToRect(rt);
		lcdDimm(x, y, w, h, -100); 
	}
}

int _AppStartActivity(GACTIVITY *act, int flag);
int _AppStartActivityForResult(GACTIVITY *act, int flag);
int _AppDestroyActivity(GACTIVITY *act);
int _AppDestroyActivityAffinity(GACTIVITY *act);

#define START_ACTIVITY				1
#define START_ACTIVITY_FOR_RESULT	2
#define DESTROY_ACTIVITY			3
#define DESTROY_ACTIVITY_AFFINITY	4

static void _OnActMessage(uv_poll_t *handle, int status, int events)
{
	ACT_CMD		*actcmd;
	unsigned char	buf[12];
	int		fd, rval;

	fd = handle->io_watcher.fd;
	while(1) {
		rval = read(fd, buf, 10);
		if(rval < 10) break;
		if(buf[0] == GM_ACTIVITY) {
			actcmd = &gActCmds[buf[1]];	
			switch(actcmd->method) {
			case START_ACTIVITY:
				_AppStartActivity(actcmd->act, (int)actcmd->flag);
				break;
			case START_ACTIVITY_FOR_RESULT:
				_AppStartActivityForResult(actcmd->act, (int)actcmd->flag);
				break;
			case DESTROY_ACTIVITY:
				_AppDestroyActivity(actcmd->act);
				break;
			case DESTROY_ACTIVITY_AFFINITY:
				_AppDestroyActivityAffinity(actcmd->act);
				break;
			}
		}
	}
}

void appProcActivity(void)
{
	_OnActMessage(&_poll_actq, 0, 0);
}

static void _OnAppMessage(uv_poll_t *handle, int status, int events)
{
	GACTIVITY	*act;
	CGRect		rt;
	unsigned char	buf[12];
	int		fd, rval;

	_OnActMessage(&_poll_actq, 0, 0);
	fd = handle->io_watcher.fd;
	while(1) {
		rval = read(fd, buf, 10);
		if(rval < 10) break;
		act = appCurrentActivity();
		if(act->onAppMessage) {
			(*act->onAppMessage)(act, buf);
			UIframe(act->window, &rt); rt.x = rt.y = 0;
			UIdrawView(act->window, &rt);
//printf("### _OnAppMessage: msg=%d...2\n", (int)buf[0]);
			_appDraw = 0;
		}
	}
}

void _OnTouchScreen(uv_poll_t *handle, int status, int events)
{
	unsigned char	buf[12];
	int		fd, rval, x, y;

	fd = handle->io_watcher.fd;
	rval = tsRead(fd, buf);
	if(rval > 0) {
//x = (buf[1] << 8) | buf[2]; y = (buf[3] << 8) | buf[4];
//printf(" [%d,%d] [%d,%d]\n", x, y, _reverseScale(x), _reverseScale(y));
		x = (buf[1] << 8) | buf[2]; x = _reverseScale(x);
		y = (buf[3] << 8) | buf[4]; y = _reverseScale(y);
		gestureProc(x, y, (int)buf[0]); 
	}
}

void _OnIdle(uv_idle_t *handle)
{
	GACTIVITY	*act;
	CGRect	rt;

	if(_appDraw) {
//printf("%lu idle draw...\n", MS_TIMER);
		act = gActs[gActCount-1];
		UIframe(act->window, &rt); rt.x = rt.y = 0;
		UIdrawView(act->window, &rt);
		_appDraw = 0;
	}
}

void _OnCheck(uv_check_t *handle)
{
	GACTIVITY	*act;
	CGRect	rt;

	if(_appDraw) {
//printf("%lu check draw...\n", MS_TIMER);
		act = gActs[gActCount-1];
		UIframe(act->window, &rt); rt.x = rt.y = 0;
		UIdrawView(act->window, &rt);
		_appDraw = 0;
	}
}

int appInit(void *loop, void (*onCreate)(GACTIVITY *))
{
	GACTIVITY	*act;
	void	*wnd;
	CGRect	rt;
	unsigned char	*p;
	int		i, fd, rval, fds[2];

	fd = tsOpen();
	if(fd < 0) {
		printf("tsOpen error\n");
		return -1;
	}
	uv_poll_init(loop, &_poll_ts, fd);
printf("uv_poll_start _OnTouchScreen ... \n");
	uv_poll_start(&_poll_ts, UV_READABLE, _OnTouchScreen);

	rval = pipe(fds);
	if(rval < 0) {
		printf("pipe() error\n");
	}
	fcntl(fds[0], F_SETFL, O_NONBLOCK);
	uv_poll_init(loop, &_poll_actq, fds[0]);
	actq_fd = fds[1];
	uv_poll_start(&_poll_actq, UV_READABLE, _OnActMessage);

	rval = pipe(fds);
	if(rval < 0) {
		printf("pipe() error\n");
	}
	fcntl(fds[0], F_SETFL, O_NONBLOCK);
	uv_poll_init(loop, &_poll_msgq, fds[0]);
	msgq_fd = fds[1];
	uv_poll_start(&_poll_msgq, UV_READABLE, _OnAppMessage);

	uv_idle_init(loop, &_idle);
	uv_idle_start(&_idle, _OnIdle);
	uv_check_init(loop, &_check);
	uv_check_start(&_check, _OnCheck);
	uv_timer_init((uv_loop_t *)loop, &_timerApp);
	langOpen();
	msgInitialize();
	p = WndBuffer;
	for(i = 0;i < MAX_ACTIVITY_SZ;i++) {
		wnd = p;
		p += alloc(UIWindow, wnd);
		initWithFrame(wnd, 0, 0, lcdWidth(), lcdHeight());
		gWnds[i] = wnd;
	}
	wallpaperInit(0);
	gActCmdIdx = 0;
	act = CreateActivity(onCreate);
	wnd = gWnds[0];
	//init(wnd);
	UIsetOpaque(wnd, FALSE);
	act->window = wnd;
	gActs[0] = act; gActCount = 1;
	if(act->onCreate) (*act->onCreate)(act);
	wallpaperDraw(NULL);
	UIsetNeedsDisplay(act->window);
	UIframe(act->window, &rt); rt.x = rt.y = 0;
	UIdrawView(act->window, &rt);
	_appDraw = 0;
	if(act->onStart) (*act->onStart)(act);
	return 0;
}

void appExit(void)
{
	int		fd;

printf("appExit...\n");
//	pthread_attr_destroy(&pt_attr);
//	pthread_join(pt_tinfo.thread_id, &res);
	langClose();
	uv_timer_stop(&_timerApp);
	fd = _poll_msgq.io_watcher.fd;
	uv_poll_stop(&_poll_msgq);
	close(fd);
	close(msgq_fd);
	fd = _poll_actq.io_watcher.fd;
	uv_poll_stop(&_poll_actq);
	close(fd);
	close(actq_fd);
	fd = _poll_ts.io_watcher.fd;
	uv_poll_stop(&_poll_ts);
	tsClose(fd);
printf("appExit end: %d\n", fd);
}

void appPostMessage(unsigned char *msg)
{
	write(msgq_fd, msg, 10);
}

void _OnTouchScreen(uv_poll_t *handle, int status, int events);

void _OnTouchScreen(uv_poll_t *handle, int status, int events);

static void _OnAppTimer(uv_timer_t *timer)
{
	GACTIVITY	*act;

	act = (GACTIVITY *)timer->data;
	if(act == gActs[gActCount-1]) {
printf("UIApp: act(%d) timeout...\n", gActCount);
		activitySetResult(act, RESULT_CANCELLED, NULL);
		if(act->onDismiss) (*act->onDismiss)(act);
		else	appDestroyActivity(act);
	}
//	appDrawView();
}

int appStartActivity(GACTIVITY *act, char *intent, int flag)
{
	ACT_CMD		*actcmd;
	unsigned char	msg[12];

	msg[0] = GM_ACTIVITY;
	msg[1] = gActCmdIdx;
	actcmd = &gActCmds[gActCmdIdx];
	gActCmdIdx++; if(gActCmdIdx > 3) gActCmdIdx = 0;
	actcmd->method		= START_ACTIVITY;
	actcmd->act			= act;
	actcmd->flag		= flag;
	if(intent && intent[0]) strcpy(act->intent, intent);
	else	act->intent[0] = 0;
	write(actq_fd, msg, 10);
}

int appStartActivityForResult(GACTIVITY *act, int requestCode, char *intent, int flag)
{
	ACT_CMD		*actcmd;
	unsigned char	msg[12];

	msg[0] = GM_ACTIVITY;
	msg[1] = gActCmdIdx;
	actcmd = &gActCmds[gActCmdIdx];
	gActCmdIdx++; if(gActCmdIdx > 3) gActCmdIdx = 0;
	actcmd->method		= START_ACTIVITY_FOR_RESULT;
	actcmd->act			= act;
	actcmd->flag		= flag;
	act->requestCode	= requestCode;
	if(intent && intent[0]) strcpy(act->intent, intent);
	else	act->intent[0] = 0;
	write(actq_fd, msg, 10);
}

int appDestroyActivity(GACTIVITY *act)
{
	ACT_CMD		*actcmd;
	unsigned char	msg[12];

	act->isFinishing = 1;
	msg[0] = GM_ACTIVITY;
	msg[1] = gActCmdIdx;
	actcmd = &gActCmds[gActCmdIdx];
	gActCmdIdx++; if(gActCmdIdx > 3) gActCmdIdx = 0;
	actcmd->method		= DESTROY_ACTIVITY;
	actcmd->act			= act;
	actcmd->flag		= 0;
	write(actq_fd, msg, 10);
}

int appDestroyActivityAffinity(GACTIVITY *act)
{
	ACT_CMD		*actcmd;
	unsigned char	msg[12];

	act->isFinishing = 1;
	msg[0] = GM_ACTIVITY;
	msg[1] = gActCmdIdx;
	actcmd = &gActCmds[gActCmdIdx];
	gActCmdIdx++; if(gActCmdIdx > 3) gActCmdIdx = 0;
	actcmd->method		= DESTROY_ACTIVITY_AFFINITY;
	actcmd->act			= act;
	actcmd->flag		= 0;
	write(actq_fd, msg, 10);
}

int _AppStartActivity(GACTIVITY *act, int flag)
{
	GACTIVITY	*a;
	void		*wnd;
	CGRect		rt;
	void (*onCreate)(struct _GACTIVITY *self);

	if(!flag && gActCount >= MAX_ACTIVITY_SZ) {
		printf("UIApp: can't create activity for full\n");
		return -1;
	}
	if(gActCount > 0) {
		a = gActs[gActCount-1];
		if(a->onStop) (*a->onStop)(a);
		if(flag) {
			if(a->onDestroy) (*a->onDestroy)(a);
			activityRelease(a);
			gActCount--;
		}
	}
	uv_timer_stop(&_timerApp);
	wnd = gWnds[gActCount];
	act->window = wnd;
	gActs[gActCount] = act;
	gActCount++;
	initWithFrame(wnd, 0, 0, lcdWidth(), lcdHeight());
	UIsetOpaque(wnd, FALSE);
	act->startForResult = 0;
	if(act->onCreate) (*act->onCreate)(act);
	_DrawActivity(act, NULL);
	if(act->onStart) (*act->onStart)(act);
	if(act->timeout) {
		_timerApp.data = act;
		uv_timer_start(&_timerApp, _OnAppTimer, act->timeout*1000, 0);
	}
	UIframe(wnd, &rt); rt.x = rt.y = 0;
	if(!UIisHidden(wnd)) UIdrawView(wnd, &rt);
	_appDraw = 0;
	return 0;
}

int _AppStartActivityForResult(GACTIVITY *act, int flag)
{
	GACTIVITY	*a;
	void		*wnd;
	CGRect		rt;
	void (*onCreate)(struct _GACTIVITY *self);

	if(!flag && gActCount >= MAX_ACTIVITY_SZ) {
		printf("UIApp: can't create activity for full\n");
		return -1;
	}
	if(gActCount > 0) {
		a = gActs[gActCount-1];
		if(a->onStop) (*a->onStop)(a);
		if(flag) {
			if(a->onDestroy) (*a->onDestroy)(a);
			activityRelease(a);
			gActCount--;
		}
	}
	uv_timer_stop(&_timerApp);
	wnd = gWnds[gActCount];
	act->window = wnd;
	gActs[gActCount] = act; gActCount++;
	initWithFrame(wnd, 0, 0, lcdWidth(), lcdHeight());
	UIsetOpaque(wnd, FALSE);
	act->startForResult = 1;
	if(act->onCreate) (*act->onCreate)(act);
	_DrawActivity(act, NULL);
	if(act->onStart) (*act->onStart)(act);
	if(act->timeout) {
		_timerApp.data = act;
		uv_timer_start(&_timerApp, _OnAppTimer, act->timeout*1000, 0);
	}
	UIframe(wnd, &rt); rt.x = rt.y = 0;
	if(!UIisHidden(wnd)) UIdrawView(wnd, &rt);
	_appDraw = 0;
	return 0;
}

int _AppDestroyActivity(GACTIVITY *act)
{
	GACTIVITY	*a;
	CGRect		rt;

	if(gActCount < 1 || gActs[gActCount-1] != act) {
		printf("UIApp: can't destroy activity for non-exist\n");
		return -1;
	}
	uv_timer_stop(&_timerApp);
	if(act->onStop) (*act->onStop)(act);
	if(act->onDestroy) (*act->onDestroy)(act);
	activityRelease(act);
	gActCount--;
	if(gActCount > 0) {
		a = gActs[gActCount-1];
		if(act->startForResult && a->onActivityResult) (*a->onActivityResult)(a, act->requestCode, act->resultCode, act->intent);
		_DrawActivity(a, NULL);
		if(a->onStart) (*a->onStart)(a);
		if(a->timeout) {
			_timerApp.data = a;
			uv_timer_start(&_timerApp, _OnAppTimer, a->timeout*1000, 0);
		}
		UIframe(a->window, &rt); rt.x = rt.y = 0;
		if(!UIisHidden(a->window)) UIdrawView(a->window, &rt);
		_appDraw = 0;
	}
	return 0;
}

int _AppDestroyActivityAffinity(GACTIVITY *act)
{
	GACTIVITY	*a;
	CGRect		rt;

	if(gActCount < 1 || gActs[gActCount-1] != act) {
		printf("UIApp: can't destroy activity for non-exist\n");
		return -1;
	}
	while(gActCount > 1) {
		if(act->onDestroy) (*act->onDestroy)(act);
		activityRelease(act);
		gActCount--;
		act = gActs[gActCount-1];
	}
	uv_timer_stop(&_timerApp);
	_DrawActivity(act, NULL);
	if(act->onStart) (*act->onStart)(act);
	UIframe(act->window, &rt); rt.x = rt.y = 0;
	UIdrawView(act->window, &rt);
	_appDraw = 0;
	return 0;
}

void appStopActivityTimer(GACTIVITY *act)
{
	uv_timer_stop(&_timerApp);
}

void appStartActivityTimer(GACTIVITY *act)
{
	if(act->timeout) {
		_timerApp.data = act;
		uv_timer_start(&_timerApp, _OnAppTimer, act->timeout*1000, 0);
	}
}

void appSetSoundSetting(int volume, int option)
{
	gSoundVolume = volume;
	gSoundOption = option;
}

int appSoundVolume(void)
{
	return (int)gSoundVolume;
}

int appKeypadTone(void)
{
	if(gSoundOption & 0x80) return 1; else return 0;
}

int appTouchSounds(void)
{
	if(gSoundOption & 0x40) return 1; else return 0;
}

void appMuteSounds(void)
{
	gSoundOptionSave = gSoundOption;
	gSoundOption = 0;
}

void appRecoverSounds(void)
{
	gSoundOption = gSoundOptionSave;
}

GACTIVITY *appCurrentActivity(void)
{
	GACTIVITY	*act;
	
	act = gActs[gActCount-1];
	return act;
}

void *_AppParentActivity(GACTIVITY *act)
{
	GACTIVITY	*a;
	int		i;
	
	for(i = gActCount-1;i > 0;i--) 
		if(gActs[i] == act) break;
	if(i > 0) a = gActs[i-1];
	else	a = NULL;
	return a;
}

