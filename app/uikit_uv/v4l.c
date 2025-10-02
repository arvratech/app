/* V4L.c
 *
 *
 * Copyright (c)2008 Nuvoton technology corporation
 * http://www.nuvoton.com
 *
 * video for linux function
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include <unistd.h>  
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <inttypes.h>
#include <linux/videodev.h>
#include "videodev_ex.h"
#include "lcdc.h"
#include "v4l.h"

#define VID_DEVICE "/dev/video0"

unsigned char	codecState, previewState;


int v4lDevOpen(void)
{
	struct video_capability	cap;
	int		fd;

	fd = open(VID_DEVICE, O_RDWR, 0);
	if(fd < 0) {
		printf("v4lDevOpen: open error: %s\n", strerror(errno));
		return ERR_V4L_OPEN_DEV;
	}
	if(ioctl(fd, VIDIOCGCAP, &cap) < 0) {
		printf("v4lDevOpen: GCAP error: %s\n", strerror(errno));
		close(fd);
		return ERR_V4L_VID_GCAP;
	}
	if(!(cap.type & VID_TYPE_CAPTURE)) {
		printf("v4lDevOpen: not support capture\n");
		close(fd);
		return ERR_V4L_NOT_SUPPOTRT_CAP;
	}
	codecState = previewState = 0;
	return fd;
}

void v4lDevClose(int fd)
{
	if(previewState) v4lDevStopPreview(fd);
	if(codecState) v4lDevStopCodec(fd);
	close(fd);
	codecState = previewState = 0;
}

int v4lDevRunCodec(int fd, int x, int y, int width, int height)
{
	uint32_t	cmd;
	int		rval;

	cmd = 1;
	if(ioctl(fd, VIDIOCCAPTURE, &cmd) < 0) {
printf("v4lDevRunCodec: CAPTURE: %s\n", strerror(errno));
		return ERR_V4L_VID_CAPTURE;
	}
	codecState = 1;
	return 0;
}

int v4lDevStopCodec(int fd)
{
	uint32_t	cmd;

	cmd = 0;
	if(ioctl(fd, VIDIOCCAPTURE, &cmd) < 0) {
printf("v4lDevStopCodec: CAPTURE: %s\n", strerror(errno));
		return ERR_V4L_VID_CAPTURE;
	}
	codecState = 0;
	return 0;
}

int v4lDevCapture(int fd, S_PIPE_INFO *planarInfo)
{

	if(ioctl(fd, VIDIOCSYNC, planarInfo) < 0) {
printf("v4lDevCapture: SYNC: %s\n", strerror(errno));
		return ERR_V4L_VID_SYNC;
	}
	return 0;
}

int v4lDevCaptureEnd(int fd)
{
	uint32_t	arg;

	if(ioctl(fd, VIDIOC_G_PLANAR_INFO, &arg)) {
		printf("v4lDevCaptureEnd: _G_PLANAR_INFO fail\n");
		return -1; 
	}
	return 0;
}

int v4lDevRunPreview(int fd, int x, int y, int width, int height)
{
    struct video_window	videowin;
	uint32_t	cmd;
	int		rval;

	rval = ioctl(fd, VIDIOCGWIN, &videowin);
	if(rval < 0) {
printf("v4lDevRunPreview: GWIN: %s\n", strerror(errno));
		return ERR_V4L_VID_GWIN;
	}
	videowin.x = x;
	videowin.y = y;
	videowin.width = width;
	videowin.height = height;
	videowin.chromakey = -1;
	videowin.flags = 0;
	rval = ioctl(fd, VIDIOCSWIN, &videowin);
	if(rval < 0) {
printf("v4lDevRunPreview: SWIN: %s\n", strerror(errno));
		return ERR_V4L_VID_SWIN;
	}
	cmd = 1;
	rval = ioctl(fd, VIDIOCSPREVIEW, &cmd);
	if(rval < 0) {
printf("v4lDevRunPreview: SPREVIEW: %s\n", strerror(errno));
		return ERR_V4L_VID_SPREVIEW;
	}
	previewState = 1;
	return 0;
}

int v4lDevStopPreview(int fd)
{
	uint32_t	cmd;

	cmd = 0;
	if(ioctl(fd, VIDIOCSPREVIEW, &cmd) < 0) {
printf("v4lDevStopPreview: SPREVIEW: %s\n", strerror(errno));
		return ERR_V4L_VID_SPREVIEW;
	}
	previewState = 0;
	return 0;
}

int v4lDevSetFrameRate(int fd, int frameRate)
{
	uint32_t	cmd;

	cmd = frameRate;
	return ioctl(fd, VIDIOC_S_FRAME_RATE, &cmd);
}

int v4lDevCodecState(int fd)
{
	return (int)codecState;
}

int v4lDevPreviewState(int fd)
{
	return (int)previewState;
}

