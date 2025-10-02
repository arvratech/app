#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "avstream.h"


void avStreamInit(void *self, int maxTrackCount)
{
	AV_STREAM	*stream = self;
	AV_STREAM_TRACK	*trk;
	int		i;

	stream->maxTrackCount = maxTrackCount;
	for(i = 0, trk = stream->_tracks;i < maxTrackCount;i++, trk++) {
		trk->state	= 0;
		trk->index	= i;
	}
	stream->trackCount = 0;
}

int avStreamTrackCount(void *self)
{
	AV_STREAM	*stream = self;

	return stream->trackCount;
}

AV_STREAM_TRACK *avStreamTrackAt(void *self, int index)
{
	AV_STREAM	*stream = self;
	AV_STREAM_TRACK	*trk;

	if(index >= 0 && index < stream->trackCount) {
		trk = &stream->_tracks[index];
	} else	trk = NULL;
	return trk;
}

AV_STREAM_TRACK *avStreamTrack(void *self, int mediaType)
{
	AV_STREAM	*stream = self;
	AV_STREAM_TRACK	*trk;
	int		i, count;

	for(i = 0, trk = stream->_tracks;i < stream->trackCount;i++, trk++)
		if(trk->mediaType == mediaType) break;
	if(i >= stream->trackCount) trk = NULL;
	return trk;
}

AV_STREAM_TRACK *avStreamAddTrack(void *self, AV_STREAM_TRACK *track)
{
	AV_STREAM	*stream = self;
	AV_STREAM_TRACK	*trk;
	int		index;
	
	index = stream->trackCount;
//printf("avStreamAddTrack: %d %d\n", index, stream->maxTrackCount);
	if(index >= stream->maxTrackCount) trk = NULL;
	else {
		trk = &stream->_tracks[index];
		memcpy(trk, track, sizeof(AV_STREAM_TRACK));
		trk->state = 1;
		trk->index = index;
		stream->trackCount++;
	}
	return trk;
}

int avStreamConnect(void *self, void *sink, int mediaType)
{
	AV_STREAM	*stream = self;
	AV_STREAM	*sink_ = sink;
	AV_STREAM_TRACK	*trk;
	int		i, rval, index;

	rval = -1;
	trk = avStreamTrack(stream, mediaType);
	if(!trk) index = -1;
	else if(mediaType == AV_MEDIA_TYPE_AUDIO) index = AV_AUDIO_INDEX;
	else if(mediaType == AV_MEDIA_TYPE_VIDEO) index = AV_VIDEO_INDEX;
	else	index = -1;
	if(index >= 0) {
		if(!sink_->sinkPeers[index]) {
			for(i = 0;i < MAX_AV_SRC_SZ;i++)
				if(!stream->srcPeers[i][index]) break;
			if(i < MAX_AV_SRC_SZ) {
				stream->srcPeers[i][index] = sink;
				sink_->sinkPeers[index] = stream;
				rval = 0;
			}
		}
	}
	return rval;
}

int avStreamDisconnect(void *self, void *sink, int mediaType)
{
	AV_STREAM	*stream = self;
	AV_STREAM	*sink_ = sink;
	AV_STREAM_TRACK	*trk;
	int		i, rval, index;

	rval = -1;
	trk = avStreamTrack(stream, mediaType);
	if(!trk) index = -1;
	else if(mediaType == AV_MEDIA_TYPE_AUDIO) index = AV_AUDIO_INDEX;
	else if(mediaType == AV_MEDIA_TYPE_VIDEO) index = AV_VIDEO_INDEX;
	else	index = -1;
	if(index >= 0) {
		if(sink_->sinkPeers[index] == stream) {
			for(i = 0;i < MAX_AV_SRC_SZ;i++)
				if(stream->srcPeers[i][index] == sink) break;
			if(i < MAX_AV_SRC_SZ) {
				stream->srcPeers[i][index] = NULL;
				sink_->sinkPeers[index] = NULL;
				rval = 0;
			}
		}
	}
	return rval;
}

int avStreamPush(void *self, AV_BUFFER *buffer, int mediaType)
{
	AV_STREAM	*stream = self;
	AV_STREAM	*sink;
	int		i, rval, index;

	rval = -1;
	if(mediaType == AV_MEDIA_TYPE_AUDIO) index = AV_AUDIO_INDEX;
	else if(mediaType == AV_MEDIA_TYPE_VIDEO) index = AV_VIDEO_INDEX;
	else	index = -1;
	if(index >= 0) {
		for(i = 0;i < MAX_AV_SRC_SZ;i++) {
			sink = stream->srcPeers[i][index];
			if(sink && sink->sinkChains[index]) {
				(*sink->sinkChains[index])(sink, buffer);
				rval = 0;
			}
		}
	}
	return rval;
}

int avStreamPull(void *self, AV_BUFFER *buffer, int mediaType)
{
	AV_STREAM	*stream = self;
	AV_STREAM	*src;
	int		rval, index;

	rval = -1;
	if(mediaType == AV_MEDIA_TYPE_AUDIO) index = AV_AUDIO_INDEX;
	else if(mediaType == AV_MEDIA_TYPE_VIDEO) index = AV_VIDEO_INDEX;
	else	index = -1;
	if(index >= 0) {
		src = stream->sinkPeers[index];
		if(src && src->srcChains[index]) {
			(*src->srcChains[index])(src, buffer);
			rval = 0;
		}
	}
	return rval;
}

void avStreamOnStopped(void *self, int mediaType)
{
	AV_STREAM	*stream = self;
	AV_STREAM	*src;
	int		index;

	if(mediaType == AV_MEDIA_TYPE_AUDIO) index = AV_AUDIO_INDEX;
	else if(mediaType == AV_MEDIA_TYPE_VIDEO) index = AV_VIDEO_INDEX;
	else	index = -1;
	if(index >= 0) {
		src = stream->sinkPeers[index];
		if(src && src->onStopped[index]) {
			(*src->onStopped[index])(src);
		}
	}
}

AV_STREAM_TRACK *avStreamSrcTrack(void *self, int mediaType)
{
	AV_STREAM	*stream = self;
	AV_STREAM	*src;
	AV_STREAM_TRACK	*trk;
	int		index;

	trk = NULL;
	if(mediaType == AV_MEDIA_TYPE_AUDIO) index = AV_AUDIO_INDEX;
	else if(mediaType == AV_MEDIA_TYPE_VIDEO) index = AV_VIDEO_INDEX;
	else	index = -1;
	if(index >= 0) {
		src = stream->sinkPeers[index];
		if(src) trk = avStreamTrack(src, mediaType);
	}
	return  trk;
}

void avStreamSetSrcChain(void *self, void (*srcChain)(void *self, AV_BUFFER *buffer), int mediaType)
{
	AV_STREAM	*stream = self;
	int		index;

	if(mediaType == AV_MEDIA_TYPE_AUDIO) index = AV_AUDIO_INDEX;
	else if(mediaType == AV_MEDIA_TYPE_VIDEO) index = AV_VIDEO_INDEX;
	else	index = -1;
	if(index >= 0) stream->srcChains[index] = srcChain;
}

void avStreamSetSinkChain(void *self, void (*sinkChain)(void *self, AV_BUFFER *buffer), int mediaType)
{
	AV_STREAM	*stream = self;
	int		index;

	if(mediaType == AV_MEDIA_TYPE_AUDIO) index = AV_AUDIO_INDEX;
	else if(mediaType == AV_MEDIA_TYPE_VIDEO) index = AV_VIDEO_INDEX;
	else	index = -1;
	if(index >= 0) stream->sinkChains[index] = sinkChain;
}

