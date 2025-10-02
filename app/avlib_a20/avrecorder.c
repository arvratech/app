#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include "avio.h"
#include "movenc.h"
#include "avcodecs.h"
#include "avrecorder.h"


static int mp_write(void *opaque, uint8_t *buf, int size)
{
	AV_RECORDER	*recorder = opaque;
	int		rval;

	rval = write(recorder->fd, buf, size);
    return rval;
}

static int64_t mp_seek(void *opaque, int64_t pos, int whence)
{
	AV_RECORDER	*recorder = opaque;
    int64_t		rval;

	rval = lseek(recorder->fd, pos, whence);
    return rval;
}

#define BIO_BUFFER_SIZE		32768

static ByteIOContext	_bio;
static MOVMuxContext	_mov;
static unsigned char	 buffer[BIO_BUFFER_SIZE];

int avRecorderOpen(AV_RECORDER *recorder, char *fileName)
{
	int		i, fd;

	memset(recorder, 0, sizeof(AV_RECORDER));
	recorder->state		= AV_STATE_NULL; 
	recorder->fd		= -1;
	recorder->bio		= &_bio;
	recorder->priv_data	= &_mov;
	avStreamInit(recorder, MAX_AV_TRACK_SZ);
	fd = open(fileName, O_RDWR|O_CREAT|O_TRUNC, 0644);
	if(fd < 0) {
		printf("avRecorderOpen: open error: %s: %s\n", fileName, strerror(errno));
		return -1;
	}
	recorder->fd		= fd;
	init_put_byte(recorder->bio, buffer, BIO_BUFFER_SIZE, 1, recorder, NULL, mp_write, mp_seek);
	recorder->state		= AV_STATE_IDLE; 
	return 0;
}

void avRecorderClose(AV_RECORDER *recorder)
{
	if(recorder->state >= AV_STATE_IDLE) {
		close(recorder->fd);
		recorder->fd = -1;
		recorder->state = AV_STATE_NULL; 
	}		
}

void _AvRecorderAudioPushChain(void *self, AV_BUFFER *buffer);
void _AvRecorderVideoPushChain(void *self, AV_BUFFER *buffer);
int  mov_write_header(AV_RECORDER *recorder);
int  mov_write_trailer(AV_RECORDER *recorder);

int avRecorderStart(AV_RECORDER *recorder)
{
	AV_STREAM	*stream;
	AV_STREAM_TRACK	*trk;
	AV_CODEC	*codec;
	int		i, count;

	avStreamInit(recorder, MAX_AV_TRACK_SZ);
	trk = avStreamSrcTrack(recorder, AV_MEDIA_TYPE_AUDIO);
	if(trk) avStreamAddTrack(recorder, trk);
	trk = avStreamSrcTrack(recorder, AV_MEDIA_TYPE_VIDEO);
	if(trk) avStreamAddTrack(recorder, trk);
	count = avStreamTrackCount(recorder);
	if(count <= 0) {
		printf("avRecorderStart: not connected\n");
		return -1;
	}
	mov_write_header(recorder);
	stream = (AV_STREAM *)recorder;
	for(i = 0;i < count;i++) {
		trk = avStreamTrackAt(stream, i);
		codec = avCodecsFindDecoder(trk->codecId);
		switch(trk->mediaType) {
		case AV_MEDIA_TYPE_AUDIO:
//printf("audio: %d codec=%s ch=%d sr=%d bits=%d\n", i, codec->codecName, trk->channels, trk->sampleRate, trk->bitsPerSample);
			avStreamSetSinkChain(recorder, _AvRecorderAudioPushChain, AV_MEDIA_TYPE_AUDIO);
			break;
		case AV_MEDIA_TYPE_VIDEO:
//printf("video: %d codec=%s w=%d h=%d fr=%d/%d\n", i, codec->codecName, trk->width, trk->height, trk->frameRateNum, trk->frameRateDen);
			avStreamSetSinkChain(recorder, _AvRecorderVideoPushChain, AV_MEDIA_TYPE_VIDEO);
			break;
		default:
printf("Unknown: index=%d format=%d\n", i, trk->mediaType);
		}
	}
	if(avStreamTrack(recorder, AV_MEDIA_TYPE_VIDEO)) recorder->frameStarted = 0;
	else	recorder->frameStarted = 1;
	recorder->state = AV_STATE_BUSY; 
	return 0;
}

static void _AvRecorderStopped(AV_RECORDER *recorder)
{
	mov_write_trailer(recorder);
	recorder->state = AV_STATE_IDLE;
}

void avRecorderStop(AV_RECORDER *recorder)
{
	if(recorder->state >= AV_STATE_BUSY) {
		_AvRecorderStopped(recorder);
printf("avRecorderStop...\n");
	}
}

int avRecorderState(AV_RECORDER *recorder)
{
	return (int)recorder->state;
}

int mov_write_packet(AV_RECORDER *recorder, int formatIndex, AV_BUFFER *buffer);

void _AvRecorderAudioFrame(AV_RECORDER *recorder, AV_BUFFER *buffer)
{
	AV_STREAM_TRACK	*trk;

	trk = avStreamTrack(recorder, AV_MEDIA_TYPE_AUDIO);
	switch(trk->codecId) {
	case AV_CODEC_AAC:
		mov_write_packet(recorder, trk->index, buffer);
		break;
	default:
		buffer->status = 9;	
	}
}

void _AvRecorderVideoFrame(AV_RECORDER *recorder, AV_BUFFER *buffer)
{
	AV_STREAM_TRACK	*trk;

	trk = avStreamTrack(recorder, AV_MEDIA_TYPE_VIDEO);
	switch(trk->codecId) {
	case AV_CODEC_AVC:
		mov_write_packet(recorder, trk->index, buffer);
		break;
	default:
		buffer->status = 9;	
	}
}

void _AvRecorderAudioPushChain(void *self, AV_BUFFER *buffer)
{
	AV_RECORDER		*recorder = self;

	if(recorder->frameStarted) _AvRecorderAudioFrame(recorder, buffer);
}

void _AvRecorderVideoPushChain(void *self, AV_BUFFER *buffer)
{
	AV_RECORDER		*recorder = self;

	if(!recorder->frameStarted && (buffer->flags & AV_FLAG_KEY_FRAME)) {
		recorder->frameStarted = 1;
		recorder->startPts = buffer->pts; recorder->startDts = buffer->dts;
	}
	if(recorder->frameStarted) {
		buffer->pts -= recorder->startPts; buffer->dts -= recorder->startDts;
		_AvRecorderVideoFrame(recorder, buffer);
	}
}

