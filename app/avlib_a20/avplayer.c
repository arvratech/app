#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include "avio.h"
#include "wav.h"
#include "mov.h"
#include "avcodecs.h"
#include "avplayer.h"


static int mp_read(void *opaque, uint8_t *buf, int size)
{
	AV_PLAYER	*player = opaque;
	int		rval;

	rval = read(player->fd, buf, size);
    return rval;
}

static int64_t mp_seek(void *opaque, int64_t pos, int whence)
{
	AV_PLAYER	*player = opaque;
    int64_t		rval;

	rval = lseek(player->fd, pos, whence);
    return rval;
}

#define BIO_BUFFER_SIZE		32768

static ByteIOContext	_bio;
static MOVContext		_mov;
static unsigned char	buffer[BIO_BUFFER_SIZE];

void _AvPlayerAudioOnStopped(void *self);
void _AvPlayerVideoOnStopped(void *self);
void _AvPlayerAudioPullChain(void *self, AV_BUFFER *buffer);
void _AvPlayerVideoPullChain(void *self, AV_BUFFER *buffer);

int avPlayerOpen(AV_PLAYER *player, char *fileName)
{
	AV_STREAM	*stream;
	AV_STREAM_TRACK	*trk;
	AV_CODEC	*codec;
	unsigned char	buf[2048];
	int		i, fd, rval, count;

	memset(player, 0, sizeof(AV_PLAYER));
	player->state		= AV_STATE_NULL;
	player->fd			= -1;
	player->bio			= &_bio;
	player->priv_data	= &_mov;
	fd = open(fileName, O_RDONLY);
	if(fd < 0) {
		printf("avPlayerOpen: open error: %s: %s\n", fileName, strerror(errno));
		return -1;
	}
	player->fd = fd;
	count = mp_read(player, buf, 2048);
	if(count <= 0) {
		printf("avPlayerOpen: read error=%d\n", count);
		close(fd);
		return -1;
	}
	rval = wavProbe(buf);
	avStreamInit(player, MAX_AV_PLAYER_TRACK_SZ);
	if(rval > 50) {
		mp_seek(player, 0L, SEEK_SET);
		init_put_byte(player->bio, buffer, BIO_BUFFER_SIZE, 0, player, mp_read, NULL, mp_seek);
		rval = wavReadHeader(player);
		if(rval < 0) printf("avPlayerOpen: waveReadHeader error\n");
	} else {
		rval = mov_probe(buf, count);
		if(rval < 50) printf("avPlayerOpen: mov_probe error\n");
		else {
//printf("avPlayerOpen: score=%d\n", rval);
			mp_seek(player, 0L, SEEK_SET);
			init_put_byte(player->bio, buffer, BIO_BUFFER_SIZE, 0, player, mp_read, NULL, mp_seek);
			//player->duration = AV_NOPTS_VALUE;
			//player->start_time = AV_NOPTS_VALUE;
			rval = mov_read_header(player);
			if(rval < 0) printf("avPlayerOpen: mov_read_header error\n");
		}
	}
	if(rval < 0) {
		close(fd);
		return rval;
	}
	stream = (AV_STREAM *)player;
	for(i = count = 0;i < avStreamTrackCount(stream);i++) {
		trk = avStreamTrackAt(stream, i);
		codec = avCodecsFindDecoder(trk->codecId);
		switch(trk->mediaType) {
		case AV_MEDIA_TYPE_AUDIO:
//printf("audio: %d codec=%s ch=%d sr=%d bits=%d\n", i, codec->codecName, trk->channels, trk->sampleRate, trk->bitsPerSample);
			stream->srcChains[AV_AUDIO_INDEX] = _AvPlayerAudioPullChain;
			stream->onStopped[AV_AUDIO_INDEX] = _AvPlayerAudioOnStopped;
			count++;
			break;
		case AV_MEDIA_TYPE_VIDEO:
//printf("video: %d codec=%s w=%d h=%d fr=%d/%d\n", i, codec->codecName, trk->width, trk->height, trk->frameRateNum, trk->frameRateDen);
			stream->srcChains[AV_VIDEO_INDEX] = _AvPlayerVideoPullChain;
			stream->onStopped[AV_VIDEO_INDEX] = _AvPlayerVideoOnStopped;
			count++;
			break;
		default:
printf("Unknown: index=%d format=%d\n", i, trk->mediaType);
		}
	}
	player->state = AV_STATE_IDLE;
	if(count > 0) rval = 0;
	else {
		avPlayerClose(player);
		rval = -1;
	}
//if(!rval) printf("avPlayerOpen: %d [%s]\n", player->fd, fileName);
	return rval;
}

void avPlayerClose(AV_PLAYER *player)
{
	AV_STREAM_TRACK	*trk;
	int		i;

	if(player->state >= AV_STATE_IDLE) {
		for(i = 0;i < avStreamTrackCount(player);i++) {
			trk = avStreamTrackAt(player, i);
			if(trk->index_entries) av_free(trk->index_entries);
		}
		close(player->fd);
//printf("avPlayerClose: %d\n", player->fd);
		player->fd = -1;
		player->state = AV_STATE_NULL;
	}
}

void avPlayerSetOnStopped(AV_PLAYER *player, void (*onStopped)(AV_PLAYER *))
{
	player->onStopped = onStopped;
}

int avPlayerConnect(AV_PLAYER *player, void *sink, int mediaType)
{
	return avStreamConnect(player, sink, mediaType);
}

int avPlayerDisconnect(AV_PLAYER *player, void *sink, int mediaType)
{
	return avStreamDisconnect(player, sink, mediaType);
}

int avPlayerStart(AV_PLAYER *player)
{
	player->state = AV_STATE_BUSY;
	return 0;
}

static void _AvPlayerStopped(AV_PLAYER *player)
{
	player->state = AV_STATE_IDLE;
}

void avPlayerStop(AV_PLAYER *player)
{
	if(player->state >= AV_STATE_BUSY) {
		_AvPlayerStopped(player);
	}
}

int avPlayerState(AV_PLAYER *player)
{
	return (int)player->state;
}

void mov_read_packet2(AV_PLAYER *player, int formatIndex, AV_BUFFER *buffer);

void _AvPlayerAudioFrame(AV_PLAYER *player, AV_BUFFER *buffer)
{
	AV_STREAM_TRACK	*trk;
	int		rval;

	trk = avStreamTrack(player, AV_MEDIA_TYPE_AUDIO);
	switch(trk->codecId) {
	case AV_CODEC_WAV:
		rval = wavReadData(player, buffer->data, buffer->size);
		if(rval < 0) buffer->status = 1;
		else {
			buffer->size = rval; buffer->status = 0;
		}
		break;
	case AV_CODEC_AAC:
		mov_read_packet2(player, trk->index, buffer);
		break;
	default:
		buffer->status = 9;	
	}
}

void _AvPlayerVideoFrame(AV_PLAYER *player, AV_BUFFER *buffer)
{
	AV_STREAM_TRACK	*trk;

	trk = avStreamTrack(player, AV_MEDIA_TYPE_VIDEO);
	switch(trk->codecId) {
	case AV_CODEC_AVC:
		mov_read_packet2(player, trk->index, buffer);
		break;
	default:
		buffer->status = 9;	
	}
}

void _AvPlayerAudioPullChain(void *self, AV_BUFFER *buffer)
{
	_AvPlayerAudioFrame((AV_PLAYER *)self, buffer);
}

void _AvPlayerVideoPullChain(void *self, AV_BUFFER *buffer)
{
	_AvPlayerVideoFrame((AV_PLAYER *)self, buffer);
}

void _AvPlayerAudioOnStopped(void *self)
{
	AV_PLAYER	*player = self;
	AV_STREAM_TRACK	*trk;

//printf("_AvPlayerAudioOnStopped...\n");
	trk = avStreamTrack(player, AV_MEDIA_TYPE_VIDEO);
	if(trk && player->state == AV_STATE_BUSY) player->state = AV_STATE_DONE;
	else {
		_AvPlayerStopped(player);
		if(player->onStopped) (*player->onStopped)(player);
	}
}

void _AvPlayerVideoOnStopped(void *self)
{
	AV_PLAYER	*player = self;
	AV_STREAM_TRACK	*trk;

printf("_AvPlayerVideoOnStopped...\n");
	trk = avStreamTrack(player, AV_MEDIA_TYPE_AUDIO);
	if(trk && player->state == AV_STATE_BUSY) player->state = AV_STATE_DONE;
	else {
		_AvPlayerStopped(player);
		if(player->onStopped) (*player->onStopped)(player);
	}
}

