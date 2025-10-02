#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/mman.h>
#include <asm/ioctl.h>
//#include <linux/vt.h>
//#include <linux/kd.h>
#include <unistd.h>
#include <dirent.h>
#include "jpegdev.h"


int jpegDevOpen(void)
{
	return 0;
}

void jpegDevClose(void)
{
}

int jpegDecode(unsigned char *jpegBuffer, int jpegSize, unsigned char *dstBuffer, int *width, int *height);
int jpegDecodeFile(char *FileName, unsigned char *dstBuffer, int *width, int *height);

int jpegDevDecode(JPEG *jpeg, BMP_INFO *bmp)
{
	int		rval;

	if(jpeg->fileName && jpeg->fileName[0]) {
		rval = jpegDecodeFile(jpeg->fileName, bmp->buf, &bmp->width, &bmp->height);
	} else {
		rval = jpegDecode(jpeg->buf, jpeg->bufLength, bmp->buf, &bmp->width, &bmp->height);
	}
	return rval;
}

int jpegDevEncode(YUV_INFO *yuv, JPEG *jpeg)
{
	return 0;
}

