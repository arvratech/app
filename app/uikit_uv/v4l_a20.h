#ifndef __V4L_H__
#define __V4L_H__

#include <stdint.h>
#include <linux/videodev2.h>

// Defined Error Code in here
#ifndef ERRCODE
#define ERRCODE int32_t
#endif

#define V4L_ERR						0x80400000							
#define ERR_V4L_SUCCESS				0
#define ERR_V4L_OPEN_DEV			(V4L_ERR | 0x1)
#define ERR_V4L_VID_GCAP			(V4L_ERR | 0x2)
#define ERR_V4L_NOT_SUPPOTRT_CAP	(V4L_ERR | 0x3)
#define ERR_V4L_VID_CAPTURE			(V4L_ERR | 0x4)
#define ERR_V4L_VID_GWIN			(V4L_ERR | 0x5)
#define ERR_V4L_VID_SWIN			(V4L_ERR | 0x6)
#define ERR_V4L_VID_SPREVIEW		(V4L_ERR | 0x7)
#define ERR_V4L_VID_SYNC			(V4L_ERR | 0x8)


typedef struct {
	unsigned int u32RemainBufSize;
	unsigned int u32RemainBufPhyAdr;
} S_BUF_INFO;

typedef struct {
	int32_t i32PipeBufNo;
	int32_t i32PipeBufSize;
	int32_t i32CurrPipePhyAddr;
	int32_t i32CurrBufIndex;
} S_PIPE_INFO;

typedef struct {
	int32_t i32SharedSensor;
	int32_t i32DependentPacketBuf;
	int32_t i32DependentPlanarBuf;
} S_KERNEL_CONFIG;

typedef struct {
	int32_t i32planar_pos_x;
	int32_t i32planar_pos_y;
} S_PLANAR_POSITION;

typedef struct _V4L_PICT {
	void	*addr;
	struct v4l2_buffer	v4lbuf;
} V4L_PICT;


int  v4lDevOpen(void);
void v4lDevClose(int fd);
int  v4lDevRun(int fd, int width, int height);
int  v4lDevStop(int fd);
int  v4lDevCapture(int fd, V4L_PICT *pict);
int  v4lDevState(void);
int  v4lDevSetFrameRate(int fd, int frameRate);


#endif

