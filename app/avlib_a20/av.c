#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "av.h"

static AV_SINK		*sink, _sink;
static AV_SOURCE	*src, _src;
static AV_RTP_SEND	*rtpSendSip, _rtpSendSip;
static AV_RTP_SEND	*rtpSendSvr, _rtpSendSvr;
static AV_PLAYER	*player, _player;
static AV_RECORDER	*recorder, _recorder;


void AvOpen(void)
{
	avCodecsInit();
	avAudioMixerOpen();
	sink = &_sink;
	avSinkOpen(sink);
	src = &_src;
	avSourceOpen(src);
	rtpSendSip = &_rtpSendSip;
	avRtpSendOpen(rtpSendSip);
	rtpSendSvr = &_rtpSendSvr;
	avRtpSendOpen(rtpSendSvr);
	player = &_player;
	recorder = &_recorder;
	avAudioOutputSet();
	avAudioInputSet();
}

void AvClose(void)
{
	avRtpSendClose(rtpSendSvr);
	avRtpSendClose(rtpSendSip);
	avSourceClose(src);
	avSinkClose(sink);
	avAudioMixerClose();
	avCodecsExit();
}

void AvStartSource(void)
{
	avSourceStart(src);
}

void AvStopSource(void)
{
	avSourceStop(src);
}

void AvStartSourcePreview(int x, int y, int width, int height)
{
	CGRect	rt;

	rt.x = x; rt.y = y; rt.width = width; rt.height = height;
	avSourceStartPreview(src, &rt);
}

void AvStopSourcePreview(void)
{
	avSourceStopPreview(src);
}

void AvPlayWave(char *fileName, int volume, void (*onStopped)(AV_PLAYER *))
{
	int		rval;

	rval = avPlayerOpen(player, fileName);
	if(!rval) {
		avPlayerSetOnStopped(player, onStopped);
		avPlayerConnect(player, sink, AV_MEDIA_TYPE_AUDIO);
		avSinkSetVolume(sink, volume);
		avPlayerStart(player);
		avSinkStart(sink);
	}
}

void AvStopPlayWave(void)
{
	avSinkStop(sink);
	avPlayerDisconnect(player, sink, AV_MEDIA_TYPE_AUDIO);
	avPlayerStop(player);
	avPlayerClose(player);
}

void AvStoppedPlayWave(void)
{
	avPlayerDisconnect(player, sink, AV_MEDIA_TYPE_AUDIO);
	avPlayerClose(player);
}

void AvPlayMpeg4(char *fileName, int volume, CGRect *screen, void (*onStopped)(AV_PLAYER *))
{
	int		rval;

	rval = avPlayerOpen(player, fileName);
	if(!rval) {
		avPlayerSetOnStopped(player, onStopped);
		avPlayerConnect(player, sink, AV_MEDIA_TYPE_AUDIO);
		avPlayerConnect(player, sink, AV_MEDIA_TYPE_VIDEO);
		avSinkSetVolume(sink, volume);
		avSinkSetScreen(sink, screen);
		avPlayerStart(player);
		avSinkStart(sink);
	}
}

void AvStopPlayMpeg4(void)
{
	avSinkStop(sink);
	avPlayerDisconnect(player, sink, AV_MEDIA_TYPE_AUDIO);
	avPlayerDisconnect(player, sink, AV_MEDIA_TYPE_VIDEO);
	avPlayerStop(player);
	avPlayerClose(player);
}

void AvStoppedPlayMpeg4(void)
{
	avPlayerDisconnect(player, sink, AV_MEDIA_TYPE_AUDIO);
	avPlayerDisconnect(player, sink, AV_MEDIA_TYPE_VIDEO);
	avPlayerClose(player);
}

int AvPlayState(void)
{
	return avSinkState(sink);
}

int AvIsPlayingWave(void)
{
	AV_STREAM_TRACK	*trk;

	trk = avStreamTrack(player, AV_MEDIA_TYPE_AUDIO);
	if(trk && trk->codecId == AV_CODEC_WAV) return 1;
	else	return 0;
}

void AvRecordMpeg4(char *fileName)
{
	int		rval;

	rval = avRecorderOpen(recorder, fileName);
	if(!rval) {
		avSourceConnect(src, recorder, AV_MEDIA_TYPE_VIDEO);
		avRecorderStart(recorder);
	}
}

void AvStopRecordMpeg4(void)
{
	avSourceDisconnect(src, recorder, AV_MEDIA_TYPE_VIDEO);
	avRecorderStop(recorder);
	avRecorderClose(recorder);
printf("##### AvStopRecordMpeg4...\n");
}


void AvStartSendSip(void)
{
	avSourceConnect(src, rtpSendSip, AV_MEDIA_TYPE_VIDEO);
	avRtpSendStart(rtpSendSip);
}

void AvStopSendSip(void)
{
	avSourceDisconnect(src, rtpSendSip, AV_MEDIA_TYPE_VIDEO);
	avRtpSendStop(rtpSendSip);
}

void AvStartSendSvr(void)
{
	avSourceConnect(src, rtpSendSvr, AV_MEDIA_TYPE_VIDEO);
	avRtpSendStart(rtpSendSvr);
}

void AvStopSendSvr(void)
{
	avSourceDisconnect(src, rtpSendSvr, AV_MEDIA_TYPE_VIDEO);
	avRtpSendStop(rtpSendSvr);
}

