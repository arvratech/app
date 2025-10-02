#ifndef _AV_PLAY_H_
#define _AV_PLAY_H_

#include "avdemux.h"
#include "avmux.h"
#include "avformat.h"
#include "avcodec.h"


typedef struct _AV_PLAY {
	AV_DEMUX		_demux;	
	int				audioIndex;
	int				videoIndex;
	void			(*onStopped)(struct _AV_PLAY *av);
	unsigned char	audioState;
	unsigned char	videoState;
	unsigned char	reserve[2];
	int64_t			videoStartTimer;
} AV_PLAY;


int  avPlayOpen(AV_PLAY *play);
void avPlayClose(AV_PLAY *play);
int  avPlayStart(AV_PLAY *play);
void avPlayStop(AV_PLAY *play);
void avPlaySetVolume(AV_PLAY *play, int volume);
void avPlaySetDataSource(AV_PLAY *play, char *url);
void avPlaySetOnStopped(AV_PLAY *play, void (*onStopped)(AV_PLAY *));


#endif

