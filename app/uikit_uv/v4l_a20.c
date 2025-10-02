#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include "log.h"
#include "v4l_a20.h"

#define VID_DEVICE	 "/dev/video0"

struct buffer {
	void	*addr;
	int		length;
};

static struct buffer	buffers[4];
static int				buffer_size;
static unsigned char	codecState;


static int xioctl(int fd, int request, void *arg)
{
	int		rval;

	do {
		rval = ioctl(fd, request, arg);
	} while (rval < 0 && errno == EINTR) ;
	return rval;
}

int v4lDevOpen(void)
{
	struct v4l2_capability		cap;
	struct stat		st;
	char	devName[32];
	int		fd, rval, i;

	strcpy(devName,  VID_DEVICE);
	rval = stat(devName, &st);
	if(rval < 0) {
		liblog(__FUNCTION__, errno, "can't identify %s", devName);
		return -1;
	}
	if(!S_ISCHR(st.st_mode)) {
		liblog(__FUNCTION__, errno, "%s is no device", devName);
		return -1;
	}
	fd = open(VID_DEVICE, O_RDWR | O_NONBLOCK, 0);
	if(fd < 0) {
		liblog(__FUNCTION__, errno, "open error");
		return -1;
	}
	rval = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if(rval < 0) {
		liblog(__FUNCTION__, errno, "QUERYCAP error");
		close(fd);
		return -1;
	}
	if(!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
		liblog(__FUNCTION__, 0, "not support capture");
		close(fd);
		return -1;
	}
	if(!(cap.capabilities & V4L2_CAP_STREAMING)) {
		liblog(__FUNCTION__, 0, "not support streaming i/o");
		close(fd);
		return -1;
	}
	codecState = 0;
printf("v4lDevOpen: OK\n");
	return fd;
}

void v4lDevClose(int fd)
{
	if(codecState) v4lDevStop(fd);
	close(fd);
	codecState = 0;
printf("v4lDevClose\n");
}

int v4lDevRun(int fd, int width, int height)
{
	struct v4l2_requestbuffers	req;
	struct v4l2_buffer	buf;
	struct v4l2_format	fmt;
	unsigned int	type;
	int		i, rval;

	memset(&fmt, 0, sizeof(struct v4l2_format));
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width       = width;
	fmt.fmt.pix.height      = height;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_NV12;	// input=YUV422 output=YUV420SP-NV12
//	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_HM12;	// input=YUV422 output=YUV420SP-NV12 Tiled
	fmt.fmt.pix.field       = V4L2_FIELD_NONE;
	rval = xioctl(fd, VIDIOC_S_FMT, &fmt);
	if(rval < 0) {
		liblog(__FUNCTION__, errno, "S_FMT error");
		return -1;
	}
	if(fmt.fmt.pix.width != width || fmt.fmt.pix.height != height) {
		liblog(__FUNCTION__, 0, "warning: driver is sending image at %dx%d", fmt.fmt.pix.width, fmt.fmt.pix.height);
	}
	memset(&req, 0, sizeof(struct v4l2_requestbuffers));
	req.count = 4;		// max=5
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;
	rval = xioctl(fd, VIDIOC_REQBUFS, &req);
	if(rval < 0) {
		if(errno == EINVAL) liblog(__FUNCTION__, 0, "not support memory mapping");
		else	liblog(__FUNCTION__, errno, "REQBUFS error");
		close(fd);
		return -1;
	}
	if(req.count < 2) {
		liblog(__FUNCTION__, 0, "insufficient buffer memory on");
		close(fd);
		return -1;
	}
	memset(buffers, 0, sizeof(struct buffer)*req.count);
	for(i = 0;i < req.count;i++) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;
		rval = xioctl(fd, VIDIOC_QUERYBUF, &buf);
		if(rval < 0) {
			liblog(__FUNCTION__, errno, "RQUERYBUF error");
			close(fd);
			return -1;
		}
		// return buf.m.offset = offset
		buffers[i].length = buf.length;
		buffers[i].addr = mmap(NULL, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
		if(buffers[i].addr == MAP_FAILED) {
			liblog(__FUNCTION__, errno, "mmap error");
			close(fd);
			return -1;
        }
	}
	buffer_size = req.count;
	codecState = 0;
	for(i = 0;i < buffer_size;i++) {
		memset(&buf, 0, sizeof(struct v4l2_buffer));
		buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory      = V4L2_MEMORY_MMAP;
		buf.index       = i;
		rval = xioctl(fd, VIDIOC_QUERYBUF, &buf);
		if(rval < 0) {
			liblog(__FUNCTION__, errno, "RQUERYBUF error");
			close(fd);
			return -1;
		}
		rval = xioctl(fd, VIDIOC_QBUF, &buf);
		if(rval < 0) {
			liblog(__FUNCTION__, errno, "QBUF error");
			return -1;
		}
	}
	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rval = xioctl(fd, VIDIOC_STREAMON, &type);
	if(rval < 0) {
		liblog(__FUNCTION__, errno, "STREAMON error");
		return -1;
	}
	codecState = 1;
printf("v4lDevRunCodec OK\n");
	return 0;
}

int v4lDevStop(int fd)
{
	enum v4l2_buf_type type;
	int		i, rval;

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	rval = xioctl(fd, VIDIOC_STREAMOFF, &type);
	if(rval < 0) {
		liblog(__FUNCTION__, errno, "STREAMOFF error");
		return -1;
	}
	for(i = 0; i < buffer_size;i++) {
		rval = munmap(buffers[i].addr, buffers[i].length);
		if(rval < 0) {
			liblog(__FUNCTION__, errno, "munmap error");
		}
	}
	codecState = 0;
	return 0;
}

#include "rtc.h"
#include <assert.h>

int v4lDevCapture(int fd, V4L_PICT *pict)
{
	struct v4l2_buffer	*buf;

	int		rval;

	buf = &pict->v4lbuf;
	if(buf->type) {
		rval = xioctl(fd, VIDIOC_QBUF, buf);
		if(rval < 0) {
			liblog(__FUNCTION__, errno, "QBUF error");
			return -1;
		}
	}
	memset(buf, 0, sizeof(struct v4l2_buffer));
	buf->type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	buf->memory = V4L2_MEMORY_MMAP;
	rval = xioctl(fd, VIDIOC_DQBUF, buf);
	if(rval < 0) {
		if(errno == EAGAIN) return 0;
		else {
			liblog(__FUNCTION__, errno, "DQBUF error");
			return -1;
		}
	}
	// return buf.m.offset = physical address
	assert(buf->index < buffer_size);
	pict->addr = buffers[buf->index].addr;
	return 0;
}

int v4lDevState(void)
{
	return (int)codecState;
}

int v4lDevSetFrameRate(int fd, int frameRate)
{
	return 0;
}

