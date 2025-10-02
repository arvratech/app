#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include "uv.h"
#include "avdevice.h"
#include "avcodecs.h"
#include "avrtpsend.h"

void *GetMainLoop(void);


int avRtpSendOpen(AV_RTP_SEND *rtpSend)
{
	memset(rtpSend, 0, sizeof(AV_RTP_SEND));
	rtpSend->state = AV_STATE_NULL;
	avStreamInit(rtpSend, MAX_AV_TRACK_SZ);
	rtpSend->state = AV_STATE_IDLE;
	return 0;
}

void  avRtpSendClose(AV_RTP_SEND *rtpSend)
{
	if(rtpSend->state >= AV_STATE_IDLE) {
		rtpSend->state = AV_STATE_NULL;
	}
}

void _AvRtpSendVideoPushChain(void *self, AV_BUFFER *buffer);

int avRtpSendStart(AV_RTP_SEND *rtpSend)
{
	AV_STREAM_TRACK	*audioTrk, *videoTrk;
	int		count;

	audioTrk = avStreamSrcTrack(rtpSend, AV_MEDIA_TYPE_VIDEO);
	videoTrk = avStreamSrcTrack(rtpSend, AV_MEDIA_TYPE_VIDEO);
	if(!audioTrk && !videoTrk) {
		printf("avRtpSendStart: no audio and video\n");
		return -1;
	}
	if(videoTrk) {
		rtpSend->frameStarted = 0;
		rtpSend->state = AV_STATE_BUSY;
		avStreamSetSinkChain(rtpSend, _AvRtpSendVideoPushChain, AV_MEDIA_TYPE_VIDEO);
	}
	return 0;
}

static void _AvRtpSendStopped(AV_RTP_SEND *rtpSend)
{
	rtpSend->state = AV_STATE_IDLE;
}

void avRtpSendStop(AV_RTP_SEND *rtpSend)
{
	if(rtpSend->state >= AV_STATE_BUSY) {
		_AvRtpSendStopped(rtpSend);
	}
}

int avRtpSendState(AV_RTP_SEND *rtpSend)
{
	return (int)rtpSend->state;
}

void _AvRtpSendVideoFrame(AV_RTP_SEND *rtpSend, AV_BUFFER *buffer)
{
	AV_STREAM_TRACK	*trk;

	trk = avStreamSrcTrack(rtpSend, AV_MEDIA_TYPE_VIDEO);
}

void _AvRtpSendVideoPushChain(void *self, AV_BUFFER *buffer)
{
	AV_RTP_SEND	*rtpSend = self;

	if(!rtpSend->frameStarted && (buffer->flags & AV_FLAG_KEY_FRAME)) {
		rtpSend->frameStarted = 1;
		rtpSend->startPts = buffer->pts; rtpSend->startDts = buffer->dts;
	}
	if(rtpSend->frameStarted) {
		buffer->pts -= rtpSend->startPts; buffer->dts -= rtpSend->startDts;
		_AvRtpSendVideoFrame(rtpSend, buffer);
	}
}

