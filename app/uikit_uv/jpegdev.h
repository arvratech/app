#ifndef _JPEG_DEV_H
#define _JPEG_DEV_H 


typedef enum {
	eJPEG_ENC_THB_NONE,
	eJPEG_ENC_THB_QQVGA,
	eJPEG_ENC_THB_QVGA,
} E_JPEG_ENC_THB_SUPP;


// YUV structire
typedef struct _YUV_INFO {
	int				width;
	int				height;
	int				planarFormat;
	unsigned int	addr;
} YUV_INFO;


// Bitmap structire
typedef struct _BMP_INFO {
	int				width;
	int				height;
	unsigned char	*buf;
	int				bufLength;
} BMP_INFO;


// JPEG structire
typedef struct _JPEG {
	int				width, height;
	unsigned char	bits;
	unsigned char	samplingType;	
	unsigned char	quantityLevel;
	unsigned char	reserve;
	char			*fileName;
	unsigned char	*buf;
	int				bufLength;
	E_JPEG_ENC_THB_SUPP thbSupp;
} JPEG;


int  jpegDevOpen(void);
void jpegDevClose(void);
int  jpegDevDecode(JPEG *jpeg, BMP_INFO	*bmp);
int  jpegDevEncode(YUV_INFO *yuv, JPEG *jpeg);


#endif

