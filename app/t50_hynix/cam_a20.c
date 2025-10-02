#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "uv.h"
#include "v4l_a20.h"
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

/*
//extern AV_STREAM_INPUT	*streamIn;
extern AV_RECORDER	*recorder;

void CamEncodeFrame(V4L_PICT *pict)
{
	AV_FORMAT	*fmt;
	AV_CODEC	*codec;
	AV_BUFFER	*buffer, _buffer;
	int		rval;

	fmt = avStreamInputGetTrackFormat(streamIn, 0);
	codec = fmt->codec;
	buffer = &_buffer;
	buffer->paddrY	= pict->v4lbuf.m.offset;
	buffer->data	= pict->addr;
	buffer->size	= 1; 
	buffer->flags	= 0;
	rval = avCodecPutInputBuffer(codec, buffer);
	if(rval == 0) {
		while(1) {
			rval = avCodecDequeueOutputBuffer(codec);
			if(rval < 0) break;
			rval = avCodecGetOutputBuffer(codec, rval, buffer);
			buffer->pts = codec->pFrameCount - 1; buffer->dts = codec->dFrameCount - 1;
			buffer->duration = 1;
			avMuxWriteSampleData(&record->_mux, record->videoIndex, buffer);
			avCodecEnqueueOutputBuffer(codec, rval);
		}
	}
}
*/

static void _PostCamChanged(void)
{
    unsigned char   msg[12];

	msg[0] = GM_CAM_CHANGED; memset(msg+1, 9, 0);
	appPostMessage(msg);
}

static int		camread;

void camRunFrameRate(int frameRate)
{
//	v4lDevSetFrameRate(v4l_fd, frameRate);
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
//	*bufLength = jpegBufLength;
//	return  jpegBuf;
return NULL;
}

