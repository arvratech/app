#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/videodev.h>
#include <fcntl.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "uv.h"
#include "jpegdev.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "lcdc.h"
#include "av.h"
#include "cam.h"


void camSetDefault(CAM_CFG *cam_cfg)
{
	cam_cfg->mode		= 0;	// 0:Disable 1:Off 2:On
	cam_cfg->runState	= 0;
	cam_cfg->channel	= 0;
	cam_cfg->option		= 0x00;
	cam_cfg->frameRate	= 2;	// 1..15
}

int camEncode(CAM_CFG *cam_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cam_cfg->mode;
	*p++ = cam_cfg->runState;
	*p++ = cam_cfg->channel;
	*p++ = cam_cfg->option;
	*p++ = cam_cfg->frameRate;
	return p - (unsigned char *)buf;
}

int camDecode(CAM_CFG *cam_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cam_cfg->mode		= *p++;
	cam_cfg->runState	= *p++;
	cam_cfg->channel	= *p++;
	cam_cfg->option		= *p++;
	cam_cfg->frameRate = *p++;
	return p - (unsigned char *)buf;
}

int camValidate(CAM_CFG *cam_cfg)
{
	if(cam_cfg->mode > 2) return 0;		
	return 1;
}

int camEncode2(CAM_CFG *cam_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cam_cfg->mode;
	*p++ = cam_cfg->runState;
	*p++ = cam_cfg->channel;
	*p++ = cam_cfg->option;
	memset(p, 0xff, 4); p += 4;
	return p - (unsigned char *)buf;
}

int camDecode2(CAM_CFG *cam_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cam_cfg->mode		= *p++;
	cam_cfg->runState	= *p++;
	cam_cfg->channel	= *p++;
	cam_cfg->option		= *p++;
	p += 4;
	return p - (unsigned char *)buf;
}

int camMode(void *self)
{
	return (int)sys_cfg->camera.mode;
}

void camSetMode(void *self, int mode)
{
	sys_cfg->camera.mode = mode;
}

int camFrameRate(void *self)
{
	return (int)sys_cfg->camera.frameRate;
}
	
void camSetFrameRate(void *self, int frameRate)
{
	sys_cfg->camera.frameRate = frameRate;
}

BOOL camEventWithCamImage(void *self)
{
	BOOL	bVal;

	if(sys_cfg->camera.option && EVENT_WITH_CAM_IMAGE) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void camSetEventWithCamImage(void *self, BOOL enable)
{
	if(enable) sys_cfg->camera.option |= EVENT_WITH_CAM_IMAGE;
	else	   sys_cfg->camera.option &= ~EVENT_WITH_CAM_IMAGE;
}

static int			v4l_fd = -1;
static uv_poll_t	_poll_cam;
static unsigned char  camread; 	
static unsigned char  *jpegBuf;
static int		jpegBufLength;
static long		camCount;
unsigned long	camTimer;


void _OnCam(uv_poll_t *handle, int status, int events)
{
	YUV_INFO	*yuv, _yuv;
	JPEG		*jpeg, _jpeg;
	S_PIPE_INFO	planarInfo;
	int		fd, rval, x, y;

	fd = handle->io_watcher.fd;
	rval = v4lDevCapture(fd, &planarInfo);
	if(rval) {
		return;
	}
//printf("%d frame=%d/%d\n", MS_TIMER-camTimer, planarInfo.i32CurrBufIndex, planarInfo.i32PipeBufNo);
camTimer = MS_TIMER;
	yuv = &_yuv; jpeg = &_jpeg;
	yuv->width = 640; yuv->height = 480;
	yuv->planarFormat = VIDEO_PALETTE_YUV420P;	
	yuv->addr = planarInfo.i32CurrPipePhyAddr;
//if(planarInfo.i32CurrBufIndex == 0) paddr = planarInfo.i32CurrPipePhyAddr;
	jpeg->width = 640; jpeg->height = 480;
	jpeg->thbSupp = eJPEG_ENC_THB_NONE;
	rval = jpegDevEncode(yuv, jpeg);
	v4lDevCaptureEnd(fd);
	if(rval < 0) {
		printf("jpegDevEncode fail\n");	
		return;
	}
	jpegBuf = jpeg->buf; jpegBufLength = jpeg->bufLength;
	if(camread) SvrnetSendCamServer(jpegBuf, jpegBufLength);
}

static void _PostCamChanged(void)
{
    unsigned char   msg[12];

	msg[0] = GM_CAM_CHANGED; memset(msg+1, 9, 0);
	appPostMessage(msg);
}

int camRun(void *loop)
{
	int		fd, rval, val, w, h;

	if(v4l_fd > 0) return 0;
	fd = v4lDevOpen();
	if(fd < 0) {
		v4l_fd = -1;
		return -1;
	}
	val = fcntl(fd, F_GETFL, 0);
    val |= O_NONBLOCK;
    fcntl(fd, F_SETFL, val);
	rval = v4lDevRunCodec(fd, 0, 0, 640, 480);
	if(rval == 0) {
		v4l_fd = fd;
		uv_poll_init(loop, &_poll_cam, fd);
		uv_poll_start(&_poll_cam, UV_READABLE, _OnCam);
		camCount = 0;
	} else {
		v4lDevClose(v4l_fd);
		v4l_fd = -1;
		rval = -1;
	}
	camread = 0;
	_PostCamChanged();
	return rval;
}

void camStop(void)
{
	int		rval;

	if(v4l_fd >= 0) {
		uv_poll_stop(&_poll_cam);
		rval = v4lDevStopCodec(v4l_fd);
		v4lDevClose(v4l_fd);
		v4l_fd = -1;
		_PostCamChanged();
	}
}

int camCodecState(void)
{
	return v4lDevCodecState(v4l_fd);
}

int camPreviewState(void)
{
	return v4lDevPreviewState(v4l_fd);
}

int camRunPreview(int width, int height)
{
	int		rval;

	rval = v4lDevRunPreview(v4l_fd, 0, 0, width, height);
	if(rval) rval = -1;
	return rval;
}

int camStopPreview(void)
{
	int		rval;

	rval = v4lDevStopPreview(v4l_fd);
	if(rval) rval = -1;
	return rval;
}

void camRunFrameRate(int frameRate)
{
	v4lDevSetFrameRate(v4l_fd, frameRate);
}

void camjpgOpenServer(void)
{
if(!camread) printf("=== cam send start...\n");
	camread = 1;
}

void camjpgCloseServer(void)
{
if(camread) printf("=== cam send stop...\n");
	camread = 0;
}

unsigned char *camjpgBuffer(int *bufLength)
{
	*bufLength = jpegBufLength;
	return  jpegBuf;
}

