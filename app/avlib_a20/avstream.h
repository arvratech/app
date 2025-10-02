#ifndef _AV_STREAM_H_
#define _AV_STREAM_H_

#include <time.h>
#include "cgrect.h"

// media type definitions
#define AV_MEDIA_TYPE_UNKNOWN		0
#define AV_MEDIA_TYPE_VIDEO			1
#define AV_MEDIA_TYPE_AUDIO			2
#define AV_MEDIA_TYPE_DATA			3
#define AV_MEDIA_TYPE_SUBTITLE		4

// video format definitions
#define AV_VIDEO_UNKNOWN			0
#define AV_VIDEO_YUV_PLANER_420		1
#define AV_VIDEO_YUV_PLANER_422		2
#define AV_VIDEO_YUV_PLANER_444		3
#define AV_VIDEO_YV12				4
#define AV_VIDEO_NV21				5
#define AV_VIDEO_NV12				6
#define AV_VIDEO_YUV_MB32_420		7
#define AV_VIDEO_YUV_MB32_422		8
#define AV_VIDEO_YUV_MB32_444		9

// audio format definitions
#define AV_AUDIO_UNKNOWN			0
#define AV_AUDIO_PCM				1

#define AV_NOPTS_VALUE				(int64_t)-1
#define AV_TIME_BASE				1000000
#define AV_TIME_BASE_Q				(AVRational){1, AV_TIME_BASE}

#define AVSEEK_FLAG_BACKWARD		1	///< seek backward
#define AVSEEK_FLAG_BYTE			2	///< seeking based on position in bytes
#define AVSEEK_FLAG_ANY				4	///< seek to any frame, even non-keyframes
#define AVSEEK_FLAG_FRAME			8	///< seeking based on frame number

// buffer flags definitions
#define AV_FLAG_KEY_FRAME			0x1
#define AV_FLAG_END_OF_STREAM		0x2

// play, record state definitions
#define AV_STATE_NULL				0
#define AV_STATE_IDLE				1
#define AV_STATE_BUSY				2
#define AV_STATE_DONE				3

#define FF_INPUT_BUFFER_PADDING_SIZE	8


typedef struct AVIndexEntry {
	int64_t			pos;
	int64_t			timestamp;
#define AVINDEX_KEYFRAME 0x0001
    int				flags:2;
    int				size:30;	// Yeah, trying to keep the size of this small to reduce memory requirements
								// (it is 24 vs. 32 bytes due to possible 8-byte alignment).
    int				min_distance; // Minimum distance between this and the previous keyframe, used to avoid unneeded searching.
} AVIndexEntry;

#define AV_DISPOSITION_DEFAULT	0x0001

typedef struct _AV_STREAM_TRACK {
	unsigned char	state;
	unsigned char	reserve[3];
	int				index;
	int				trackId;
	int				mediaType;
	int				mediaFormat;
	int				codecId;
	unsigned long	codecTag;
	int				codedWidth;
	int				codedHeight;
	int				width;
	int				height;
	int64_t			duration;
	int				frameRateNum;
	int				frameRateDen;
	int				channels;
	int				sampleRate;
	int				bitsPerSample;
	int				bitRate;
	int				blockAlign;
	time_t			timestamp;
	int				has_b_frames;
    int				nb_index_entries;
	int				index_entries_allocated_size;
    AVIndexEntry	*index_entries;		//< Only used if the format does not support seeking natively.
	void			*priv_data;
	int				currentSample;
	void			*bio;
	unsigned char	extradata[128];
	int				extradataLen;
	int				frameCount;
	int				frameSize;
} AV_STREAM_TRACK;


typedef struct _AV_BUFFER {
    unsigned char	*data;
	unsigned long	paddrY;	
	unsigned long	paddrC;	
    int				size;
    int				flags;
    int64_t			pts;
    int64_t			dts;
	int				duration;
	int				status;
	int				frameCount;
	int				frameSize;
} AV_BUFFER;

#define AV_PKT_FLAG_KEY			0x0001

#define MAX_AV_TRACK_SZ			2
#define MAX_AV_SRC_SZ			4
#define AV_AUDIO_INDEX			0
#define AV_VIDEO_INDEX			1


typedef struct _AV_STREAM {
	void			*sinkPeers[MAX_AV_TRACK_SZ];
	void			*srcPeers[MAX_AV_SRC_SZ][MAX_AV_TRACK_SZ];
	void			(*sinkChains[MAX_AV_TRACK_SZ])(void *self, AV_BUFFER *buffer);
	void			(*srcChains[MAX_AV_TRACK_SZ])(void *self, AV_BUFFER *buffer);
	void			(*onStopped[MAX_AV_TRACK_SZ])(void *self);
	int				maxTrackCount;
	int				trackCount;
	AV_STREAM_TRACK		_tracks[MAX_AV_TRACK_SZ];
} AV_STREAM;


void avStreamInit(void *self, int maxTrackCount);
int  avStreamTrackCount(void *self);
AV_STREAM_TRACK *avStreamTrackAt(void *self, int index);
AV_STREAM_TRACK *avStreamTrack(void *self, int mediaType);
AV_STREAM_TRACK *avStreamAddTrack(void *self, AV_STREAM_TRACK *track);
int  avStreamConnect(void *self, void *sink, int mediaType);
int  avStreamDisconnect(void *self, void *sink, int mediaType);
int  avStreamPush(void *self, AV_BUFFER *buffer, int mediaType);
int  avStreamPull(void *self, AV_BUFFER *buffer, int mediaType);
void avStreamOnStopped(void *self, int mediaType);
AV_STREAM_TRACK *avStreamSrcTrack(void *self, int mediaType);
void avStreamSetSrcChain(void *self, void (*srcChain)(void *self, AV_BUFFER *buffer), int mediaType);
void avStreamSetSinkChain(void *self, void (*sinkChain)(void *self, AV_BUFFER *buffer), int mediaType);


#endif

