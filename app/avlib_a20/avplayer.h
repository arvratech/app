#ifndef _AV_PLAYER_H_
#define _AV_PLAYER_H_

#include "avstream.h"

#define MAX_AV_PLAYER_TRACK_SZ		8


typedef struct _AV_PLAYER {
	AV_STREAM		_stream;
	AV_STREAM_TRACK	_track[MAX_AV_PLAYER_TRACK_SZ-MAX_AV_TRACK_SZ];
	int				fd;
	unsigned char	state;
	unsigned char	reserve[3];
	void			*bio;
	void			*priv_data;
	void			(*onStopped)(struct _AV_PLAYER *player);
} AV_PLAYER;


int  avPlayerOpen(AV_PLAYER *player, char *fileName);
void avPlayerClose(AV_PLAYER *player);
void avPlayerSetOnStopped(AV_PLAYER *player, void (*onStopped)(AV_PLAYER *));
int  avPlayerConnect(AV_PLAYER *player, void *sink, int mediaType);
int  avPlayerDisconnect(AV_PLAYER *player, void *sink, int mediaType);
int  avPlayerStart(AV_PLAYER *player);
void avPlayerStop(AV_PLAYER *player);
int  avPlayerState(AV_PLAYER *player);


#endif

