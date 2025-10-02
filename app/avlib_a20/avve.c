#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "rtc.h"
#include "lcdc.h"
#include "uv.h"
#include "vencoder.h"
#include "av.h"


int avWriteVideo(AV_CONTEXT *av, void *buffer)
{
	VencOutputBuffer	*outBuffer;
	AVFormatContext		*avfc;
	AVPacket	*pkt, _pkt;
	unsigned char	*p;
	unsigned long	size;
	int		i;

	outBuffer = (VencOutputBuffer *)buffer;
	pkt = &_pkt;
	memset(pkt, 0, sizeof(AVPacket));
	pkt->data = outBuffer->pData0;
	p = pkt->data; 
	pkt->size = size = outBuffer->nSize0;
	pkt->pts = pkt->dts = outBuffer->nPts;
	pkt->duration = 1;
	size -= 4;
	p[0] = size >> 24; p[1] = size >> 16; p[2] = size >> 8; p[3] = size;
printf("%lld %d %02x", outBuffer->nPts, size, (int)pkt->data[0]); for(i = 1;i < 8;i++) printf("-%02x", (int)pkt->data[i]); printf("...\n");
	pkt->pts  = outBuffer->nPts;
	avfc = av_e->avfc;
	ff_mov_write_packet(avfc, pkt);
	return 0;
}

