#ifndef _AV_DEVICE_H_
#define _AV_DEVICE_H_

#include "cgrect.h"
#include "avstream.h"
#include <alsa/asoundlib.h>


int  avAudioMixerOpen(void);
void avAudioMixerClose(void);
void avAudioMixerSetVolume(int volume);

int  avAudioOutputOpen(void);
void avAudioOutputClose(void);
void avAudioOutputSet(void);
int  avAudioOutputStart(AV_STREAM_TRACK *track);
void avAudioOutputStop(void);
int  avAudioOutputPeriodSize(void);
int  avAudioOutputBufferSize(void);
int  avAudioOutputFreeSize(void);
int  avAudioOutputUsedSize(void);
int  avAudioOutputWrite(void *buf);
void avAudioOutputRecover(void);
void avAudioOutputStopped(void);
int  avAudioOutputIsStopped(void);

int  avAudioInputOpen(void);
void avAudioInputClose(void);
void avAudioInputSet(void);
int  avAudioInputStart(AV_STREAM_TRACK *track);
void avAudioInputStop(void);

int  avVideoOutputOpen(void);
void avVideoOutputClose(void);
int  avVideoOutputStart(AV_STREAM_TRACK *track);
void avVideoOutputStop(int layerHandle);
void adVideoOutputFirstFrame(int layerHandle, AV_STREAM_TRACK *track, AV_BUFFER *buffer, CGRect *wnd);
void adVideoOutputFrame(int layerHandle, AV_BUFFER *buffer);

int  avVideoInputOpen(void);
void avVideoInputClose(void);
int  avVideoInputStart(AV_STREAM_TRACK *track);
void avVideoInputStop(void);


#endif

