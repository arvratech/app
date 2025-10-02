#ifndef _AV_RTP_SEND_H
#define _AV_RTP_SEND_H

#include "cgrect.h"
#include "avstream.h"
#include "avcodec.h"


typedef struct _AV_RTP_SEND {
	AV_STREAM		_stream;
	unsigned char	state;
	unsigned char	frameStarted;
	unsigned char	reserve[2];
	int64_t			startTimer;
	int64_t			startPts;
	int64_t			startDts;
} AV_RTP_SEND;


int  avRtpSendOpen(AV_RTP_SEND *rtpSend);
void avRtpSendClose(AV_RTP_SEND *rtpSend);
int  avRtpSendStart(AV_RTP_SEND *rtpSend);
void avRtpSendStop(AV_RTP_SEND *rtpSend);
int  avRtpSendState(AV_RTP_SEND *rtpSend);


#endif

