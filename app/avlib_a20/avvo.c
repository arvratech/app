#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include "lcdc.h"
#include "avdevice.h"


int avVideoOutputOpen(void)
{
	return 0;
}

void avVideoOutputClose(void)
{
}

int avVideoOutputStart(AV_STREAM_TRACK *track)
{
	int		layerHandle;

	layerHandle = lcdOpenVideo();
	return layerHandle;
}

void avVideoOutputStop(int layerHandle)
{
	lcdCloseVideo(layerHandle);
}

void adVideoOutputFirstFrame(int layerHandle, AV_STREAM_TRACK *track, AV_BUFFER *buffer, CGRect *wnd)
{
	LCD_VIDEO	*video, _video;
	int		x, y, w, h, width, height;

	width = track->width; height = track->height;
	video = &_video;
	if(track->mediaFormat == AV_VIDEO_YUV_MB32_420) video->mode = 1;
	else	video->mode = 0;		// AV_VIDEO_NV12
	video->size.width	= track->codedWidth;
	video->size.height	= track->codedHeight;
	video->paddrY		= buffer->paddrY;
	video->paddrC		= buffer->paddrC;
	video->src.x		= 0;
	video->src.y		= 0;
	video->src.width	= width;
	video->src.height	= height;
	if(wnd && wnd->width > 0) {
		x = wnd->x; y = wnd->y;
		w = wnd->width; h = wnd->height;
	} else {
		w = lcdPixelWidth(); h = lcdPixelHeight();
		if(w * height <= h * width) {
			h = w * height / width;
		} else {
			w = h * width / height;
		}
		x = (lcdPixelWidth() - w) >> 1;
		y = (lcdPixelHeight() - h) >> 1;
	}
	video->dst.x		= x;
	video->dst.y		= y;
	video->dst.width	= w;
	video->dst.height	= h;
	lcdStartVideo(layerHandle, video);
printf("vo: %d %d => %d %d\n", width, height, w, h);
}

void adVideoOutputFrame(int layerHandle, AV_BUFFER *buffer)
{
	lcdUpdateVideo(layerHandle, buffer->paddrY, buffer->paddrC);
}

