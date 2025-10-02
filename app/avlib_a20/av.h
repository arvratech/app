#ifndef _AV_H_
#define _AV_H_


#include "avstream.h"
#include "avcodec.h"
#include "avcodecs.h"
#include "avsink.h"
#include "avsrc.h"
#include "avrtpsend.h"
#include "avplayer.h"
#include "avrecorder.h"


void AvOpen(void);
void AvClose(void);
void AvStartSource(void);
void AvStopSource(void);
void AvStartSourcePreview(int x, int y, int width, int height);
void AvStopSourcePreview(void);
void AvPlayWave(char *fileName, int volume, void (*onStopped)(AV_PLAYER *));
void AvStopPlayWave(void);
void AvStoppedPlayWave(void);
void AvPlayMpeg4(char *fileName, int volume, CGRect *screen, void (*onStopped)(AV_PLAYER *));
void AvStopPlayMpeg4(void);
void AvStoppedPlayMpeg4(void);
int  AvPlayState(void);
int  AvIsPlayingWave(void);
void AvRecordMpeg4(char *fileName);
void AvStopRecordMpeg4(void);
void AvStartSendSip(void);
void AvStopSendSip(void);
void AvStartSendSvr(void);
void AvStopSendSvr(void);


#endif

