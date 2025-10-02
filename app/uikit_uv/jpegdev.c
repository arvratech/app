#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/mman.h>
#include <asm/ioctl.h>
#include <asm/arch/hardware.h>
//#include <linux/vt.h>
//#include <linux/kd.h>
#include <unistd.h>
#include <dirent.h>
#include "jpegcodec.h"
#include "jpegdev.h"


static int		jpegFd;
static unsigned int	 jpegBufferSize;
static unsigned char *jpegBuffer;
static jpeg_param_t  jpegParam;
static jpeg_info_t	 jpegInfo;
static int		appOffset[10],appSize[10], appNum;

int jpegDevOpen(void)
{
	DIR		*dir;
	char	achDevice[16];
	unsigned int	jpegBufferSize;
	int		fd, rval;

	// Check device for jpegcodec "/dev/video0" or "/dev/video1"
	// Try to open folder "/sys/class/video4linux/video1/",
	// if the folder exists, jpegcodec is "/dev/video1", otherwises jpegcodec is "/dev/video0"
	strcpy(achDevice, "/dev/video0");
	dir = opendir("/sys/class/video4linux/video1/");
	if(dir) {
		closedir(dir);
		achDevice[10]++;
	}
	fd = open(achDevice, O_RDWR);
	if(fd < 0) {
		printf("jpegDev open error: %s\n", achDevice);
		jpegFd = -1;
		return -1;
	}
	// allocate memory for JPEG engine
	rval = ioctl(fd, JPEG_GET_JPEG_BUFFER, &jpegBufferSize);
	if(rval < 0) {
		close(fd);
		jpegFd = -1;
		return -1;
	}
	jpegBuffer = mmap(NULL, jpegBufferSize, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if(jpegBuffer == MAP_FAILED) {
		printf("jpegDev mmap failed\n");
		close(fd);
		return -1;
	}
printf("jpegDev opened: BufferSize=%d\n", jpegBufferSize);
	jpegFd = fd;
	return fd;
}

void jpegDevClose(void)
{
	munmap(jpegBuffer, jpegBufferSize);
	close(jpegFd);
}

int jpegParse(unsigned char *Buffer, int Length, int *Width, int *Height)
{
	//HByte,LByte:For JPEG Marker decode
	//MLength:Length of Marker (all data in the marker)
	unsigned char HByte,LByte;
	unsigned short int MLength;		
	int		index;		

	appNum = 0;
	index = 0;			
	while(index < Length) {
		HByte = Buffer[index++];
       	if(HByte == 0xff) {
			LByte = Buffer[index++];
			switch(LByte) {
			case 0xD8:		// SOI Marker(Start Of Image)
				break;
			case 0xC1:case 0xC2:case 0xC3:case 0xC5:
       		case 0xC6:case 0xC7:case 0xC8:case 0xC9:
       		case 0xCA:case 0xCB:case 0xCD:case 0xCE:
       		case 0xCF:		// SOF Marker(Not Baseline Mode)
				return -1;
				break;
       		case 0xdb:		// DQT Marker (Define Quantization Table)
				if(index + 2 > Length) return -1;
       			HByte = Buffer[index++];
           		LByte = Buffer[index++];
				MLength = (HByte << 8) + LByte - 2;
				if(MLength != 65) {	// Has more than 1 Quantization Table
					if(MLength % 65) return -1;
					index += MLength;	// Skip Data
				} else {	// Maybe has 1 Quantization Table
					index += 64;
				}
				if(index > Length) return -1;
                break;
             case 0xc0:		// SOF Marker(Baseline mode Start Of Frame)
				if(index + 7 > Length) return -1;
				HByte = Buffer[index++];
               	LByte = Buffer[index++];
				MLength = (HByte << 8) + LByte;						
				index++;	// Sample precision
               	HByte = Buffer[index++];
               	LByte = Buffer[index++];
				*Height = (HByte << 8) + LByte;											
               	HByte = Buffer[index++];
               	LByte = Buffer[index++];
				*Width = (HByte << 8) + LByte;	
				return 0;
				break;
			case 0xda:		// Scan Header
				if(index + 2 > Length) return -1;
              	HByte = Buffer[index++];
                LByte = Buffer[index++];
				MLength = (HByte << 8) + LByte;
	            index += MLength - 2;	// Skip Scan Header Data
				if(index > Length) return -1;
				break;
			case 0xc4:		// DHT Marker (Define Huffman Table)
				if(index + 2 > Length) return -1;
                HByte = Buffer[index++];
                LByte = Buffer[index++];
				MLength = (HByte << 8) + LByte;
	            index += MLength - 2;
				if(index > Length) return -1;
               	break;
			case 0xE0:case 0xE1:case 0xE2:case 0xE3:
			case 0xE4:case 0xE5:case 0xE6:case 0xE7:
			case 0xE8:case 0xE9:case 0xEA:case 0xEB:
			case 0xEC:case 0xED:case 0xEE:case 0xEF:
			case 0xFE:		// Application Marker && Comment
				if(index + 2 > Length) return -1;						
				HByte = Buffer[index++];
				LByte = Buffer[index++];
				MLength = (HByte << 8) + LByte;					
				appOffset[appNum] = index - 4;
				appSize[appNum] = MLength + 2;
				appNum++;
                index += MLength - 2;	// Skip Application or Comment Data
				if(index > Length) return -1;      
            	break;
            }
		}
	}
	return -1;		// Wrong file format
}

int jpegDevDecode(JPEG *jpeg, BMP_INFO *bmp)
{
	jpeg_info_t		*jpeg_info, _jpeg_info;	
	FILE	*fp;
	int		rval, width, height, size;

	if(jpeg->fileName && jpeg->fileName[0]) {
		fp = fopen(jpeg->fileName, "r");
		if(!fp) {
			printf("jpegDevDecode: fopen error\n");
			return -1;
		}
		size = 102400;
		rval = fread(jpegBuffer, 1, size+4, fp);
		if(rval < 0) {
			printf("jpegDevDecode: fread error\n");
			fclose(fp);
			return -1;
		}
		fclose(fp);
		if(rval > size) {
			printf("jpegDevDecode: jpeg file too big %d\n", rval);
			return -1;
		}
		size = rval;	
	} else if(jpeg->buf && jpeg->bufLength > 0) {
		size = jpeg->bufLength;
		memcpy(jpegBuffer, jpeg->buf, size);
	} else {
		printf("jpegDevDecode: parameter error\n");
		return -1;
	}
	rval = jpegParse(jpegBuffer, size, &width, &height);
	if(rval) {
		printf("jpegDevDecode: can't get image size in %d byte bistream\n", size);
		return -1;
	} 
printf("jpegDevDecode: size=%d w=%d h=%d\n", size, width, height);
	memset(&jpegParam, 0, sizeof(jpeg_param_t));
	jpegParam.encode = 0;			// Decode Operation
	jpegParam.decInWait_buffer_size = 0;	// Decode input Wait buffer size (Decode input wait function 
											// disable when decInWait_buffer_size is 0)
	jpegParam.decopw_en = 0;
	jpegParam.windec_en = 0;
	jpegParam.dec_stride = 0;	// Stride function is disabled when dec_stride is 0
	jpegParam.scale = 0;		// Scale function is disabled when scale is 0
	if(size % 4) size = (size & ~0x3) + 4; 
	jpegParam.src_bufsize = size;	// Src buffer size(Bitstream buffer size for JPEG engine)
	jpegParam.dst_bufsize = jpegBufferSize - size;	// Dst buffer size (Decoded Raw data buffer size for JPEG engine)
	jpegParam.buffersize = 0;		// only for continuous shot
	jpegParam.buffercount = 1;
	// Set decode output format: RGB555/RGB565/RGB888/YUV422/PLANAR_YUV
	jpegParam.decode_output_format = DRVJPEG_DEC_PRIMARY_PACKET_RGB888;		
	// Set operation property to JPEG engine
	if((ioctl(jpegFd, JPEG_S_PARAM, &jpegParam)) < 0) {
		fprintf(stderr, "set jpeg param failed:%d\n", errno);
		return -1;
	}		
	// Trigger JPEG engine
	if((ioctl(jpegFd, JPEG_TRIGGER, NULL)) < 0) {
		fprintf(stderr,"Jpeg trigger failed:%d\n", errno);
		return -1;
	}
	// Get JPEG decode information
	jpeg_info = &_jpeg_info;	
	rval = read(jpegFd, jpeg_info, sizeof(jpeg_info_t));
	if(rval < 0) {
		fprintf(stderr, "Jpeg read data error: %d\n", errno);
		return -1;
	}
printf("jpegDevDecode: decoded width=%d height=%d\n", jpeg_info->width, jpeg_info->height);
	bmp->width = jpeg_info->width; bmp->height = jpeg_info->height;
	if(jpeg_info->state == JPEG_DECODED_IMAGE) {
   		bmp->buf = jpegBuffer + jpegParam.src_bufsize;
		bmp->bufLength = jpeg_info->image_size[0];
printf("DecodeSize=%d\n", bmp->bufLength);
		rval = 0;
	} else if(jpeg_info->state == JPEG_DECODE_ERROR) {
		printf("Jpeg decode error\n");
		rval = -1;
	} else if(jpeg_info->state == JPEG_MEM_SHORTAGE) {
		printf("Jpeg memory shortage\n");	
		rval = -1;
	} else if(jpeg_info->state == JPEG_DECODE_PARAM_ERROR) {
		printf("Jpeg decode parameter error\n");			
		rval = -1;
	} else {
		printf("Jpeg decode other error: %d\n", (int)jpeg_info->state);			
		rval = -1;
	}
	return rval;
}

int jpegDevEncode(YUV_INFO *yuv, JPEG *jpeg)
{
	unsigned char	*reservedAddr;
	unsigned int	addr, val;
	int		rval, size, thumbnailSize, thumbnailOffset;

	memset(&jpegParam, 0, sizeof(jpeg_param_t));
	jpegParam.buffersize	= 0;		// only for continuous shot
	jpegParam.buffercount	= 1;		
	jpegParam.qscaling		= 7;
	jpegParam.qadjust		= 0xf;			
	if(yuv->planarFormat == VIDEO_PALETTE_YUV422P)
		jpegParam.encode_image_format = DRVJPEG_ENC_PRIMARY_YUV422;
	else
		jpegParam.encode_image_format = DRVJPEG_ENC_PRIMARY_YUV420; 
	jpegParam.encode_source_format = DRVJPEG_ENC_SRC_PLANAR;
	// Set encode source stride (Must calling after IOCTL - JPEG_S_PARAM)
	ioctl(jpegFd, JPEG_SET_ENC_STRIDE, yuv->width);
	if(jpeg->thbSupp) {
		// Set reserved size for exif
		size = 0x84;
		reservedAddr = jpegBuffer + 6;
		memset(reservedAddr, 0xff, size);
		ioctl(jpegFd, JPEG_SET_ENCOCDE_RESERVED, size); 
	}
	jpegParam.encode = 1;
	if(yuv->width != jpeg->width || yuv->height != jpeg->height) {
		jpegParam.scale = 1;
		jpegParam.encode_width	= yuv->width;	 
		jpegParam.encode_height	= yuv->height;
		jpegParam.scaled_width	= jpeg->width;
		jpegParam.scaled_height	= jpeg->height;					
	} else {
		jpegParam.scale	= 0; 
		jpegParam.encode_width	= yuv->width;
		jpegParam.encode_height	= yuv->height;
	}
//printf("JPEG encode source: %d %d\n", jpegParam.encode_width, jpegParam.encode_height);
//printf("JPEG encode target: %d %d\n", jpeg->width, jpeg->height);	
	size = jpegParam.encode_width * jpegParam.encode_height;
	addr = yuv->addr;
	rval = ioctl(jpegFd, JPEG_SET_ENC_USER_YADDRESS, addr);
	if(rval < 0) {
		printf("Jpeg set Y address failed:%d\n", errno);
		return -1;
	}
	addr += size;
	rval = ioctl(jpegFd, JPEG_SET_ENC_USER_UADDRESS, addr);
	if(rval < 0) {
		printf("Jpeg set U address failed: %d\n", errno);
		return -1;
	}
	if(yuv->planarFormat == VIDEO_PALETTE_YUV422P) addr += size >> 1;
	else	addr += size >> 2;
	rval = ioctl(jpegFd, JPEG_SET_ENC_USER_VADDRESS, addr);
	if(rval < 0) {
		printf("Jpeg set V address failed: %d\n", errno);
		return -1;
	}
	// Set operation property to JPEG engine
	rval = ioctl(jpegFd, JPEG_S_PARAM, &jpegParam);
	if(rval < 0){
		printf("Jpeg set param failed: %d\n", errno);
		return -1;
	}
	// Set Encode Source from VideoIn
	//ioctl(JpegFd,JPEG_SET_ENC_SRC_FROM_VIN,NULL);	
	if(jpeg->thbSupp){
		if(jpeg->thbSupp == eJPEG_ENC_THB_QQVGA) val = 1; else val = 0;
		ioctl(jpegFd, JPEG_SET_ENC_THUMBNAIL, val);	
	}
	rval = ioctl(jpegFd, JPEG_TRIGGER, NULL);
	if(rval < 0) {
		printf("Jpeg trigger failed: %d\n", errno);
		return -1;
	}
	rval = read(jpegFd, &jpegInfo, sizeof(jpeg_info_t));
	if(rval < 0) {
		printf("jpeg read failed: %d\n", errno);
		if(jpegInfo.state == JPEG_MEM_SHORTAGE) printf("Memory Stortage\n");	
		return -1;
	}
	if(jpeg->thbSupp) {
		ioctl(jpegFd, JPEG_GET_ENC_THUMBNAIL_SIZE, &thumbnailSize);
		ioctl(jpegFd, JPEG_GET_ENC_THUMBNAIL_OFFSET, &thumbnailOffset);
		create_simple_EXIF((char *)reservedAddr, thumbnailOffset, thumbnailSize);
	}
	jpeg->buf		= jpegBuffer;
	jpeg->bufLength	= jpegInfo.image_size[0];
	return 0;
}

