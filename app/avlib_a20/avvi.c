#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include <linux/videodev2.h>
#include "lcdc.h"
#include "v4l_a20.h"
#include "avdevice.h"


static int	v4l_fd;

int avVideoInputOpen(void)
{
	int		fd, val;

	v4l_fd = -1;
	fd = v4lDevOpen();
	if(fd < 0) {
		return -1;
	}
	val = fcntl(fd, F_GETFL, 0);
    val |= O_NONBLOCK;
    fcntl(fd, F_SETFL, val);
	v4l_fd = fd;
	return 0;
}

void avVideoInputClose(void)
{
	v4lDevClose(v4l_fd);
	v4l_fd = -1;
}

int avVideoInputStart(AV_STREAM_TRACK *track)
{
	int		rval;

	rval = v4lDevRun(v4l_fd, track->width, track->height);
	return rval;
}

void avVideoInputStop(void)
{
	v4lDevStop(v4l_fd);
}

int avVideoInputGet(void)
{
	return v4l_fd;
}

