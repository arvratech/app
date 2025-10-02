#ifndef _BMPC_H_
#define _BMPC_H_

#include "UIEnum.h"
#include "cgrect.h"

#define TYPE_COLOR_BITMAP		0
#define TYPE_GRAY_BITMAP		1
#define TYPE_BW_BITMAP			2


typedef struct _BMPC {
	unsigned char	type;
	unsigned char	bitCount;
	unsigned short	status;
	short	width, height;
	unsigned long	*buffer;
} BMPC;

//The BITMAPFILEHEADER structure contains information about the type, size, and layout of a file
// that contains a DIB.
typedef struct tagBITMAPFILEHEADER {
	unsigned short	bfType;			// The file type; must be BM.
    unsigned long	bfSize;			// The size, in bytes, of the bitmap file.
    unsigned short	bfReserved1;	// Reserved; must be zero.
    unsigned short	bfReserved2;	// Reserved; must be zero.
    unsigned long	bfOffBits;		// The offset, in bytes, from the beginning of the BITMAPFILEHEADER structure to the bitmap bits.
} BITMAPFILEHEADER; 

//The BITMAPINFOHEADER structure contains information about the dimensions and color format of a DIB.
typedef struct tagBITMAPINFOHEADER {
	unsigned long	biSize;			// The number of bytes required by the structure.
	long			biWidth;		// The width of the bitmap, in pixels.
	long			biHeight;		// The height of the bitmap, in pixels.
	unsigned short	biPlanes;		// The number of planes for the target device. This value must be set to 1.
	unsigned short	biBitCount;		// The number of bits-per-pixel. 
	unsigned long	biCompression;	// The type of compression for a compressed bottom-up bitmap (top-down DIBs cannot be compressed). This member can be one of the following values
	unsigned long	biSizeImage;	// The size, in bytes, of the image. This may be set to zero for BI_RGB bitmaps.
	long			biXPelsPerMeter;// The horizontal resolution, in pixels-per-meter, of the target device for the bitmap. 
	long			biYPelsPerMeter;// The vertical resolution, in pixels-per-meter, of the target device for the bitmap.
	unsigned long	biClrUsed;		// The number of color indexes in the color table that are actually used by the bitmap.
	unsigned long	biClrImportant;	// The number of color indexes that are required for displaying the bitmap. If this value is zero, all colors are required.
} BITMAPINFOHEADER;

// The RGBQUAD structure describes a color consisting of relative intensities of red, green, and blue.
typedef struct tagRGBQUAD {
	unsigned char	rgbBlue;		// The intensity of blue in the color.
	unsigned char	rgbGreen;		// The intensity of green in the color.
	unsigned char	rgbRed;			// The intensity of red in the color.
	unsigned char	rgbReserved;	// This member is reserved and must be zero.
} RGBQUAD;


typedef struct _IconHeader {
	unsigned short	Reserved; 		// Reserved (always 0)
	unsigned short	ResourceType;	// Resource ID (always 1)
	unsigned short	IconCount;		// Number of icon bitmaps in file
} ICONHEADER;

typedef struct _IconEntry {
	unsigned char	Width;			// Width of icon in pixels
	unsigned char	Height;			// Height of icon in pixels
	unsigned char	NumColors;		// Maximum number of colors
	unsigned char	Reserved;		// Not used (always 0)
	unsigned short	NumPlanes;		// Not used (always 0)
	unsigned short	BitsPerPixel;	// Not used (always 0)
	unsigned long	DataSize;		// Length of icon bitmap in bytes
	unsigned long	DataOffset;		// Offset position of icon bitmap in file
} ICONENTRY;


void bmpcInit(BMPC *bmp, void *buffer, int width, int height);
void bmpcSetSize(BMPC *bmp, int width, int height);
int  bmpcWidth(BMPC *bmp);
int  bmpcHeight(BMPC *bmp);
unsigned long *bmpcBuffer(BMPC *bmp);
unsigned long *bmpcStartBuffer(BMPC *bmp, int x, int y);
void bmpcClipRect(BMPC *bmp, CGRect *rect);
void bmpcSetBitCount(BMPC *bmp, int bitCount);
int  bmpcPut(BMPC *bmp, int x, int y, BMPC *bmpS);
int  bmpcPutEx(BMPC *bmp, int x, int y, int Width, int Height, BMPC *bmpS);
void bmpcFillRect(BMPC *bmp, int x, int y, int width, int height, unsigned long color);
void bmpcDarkenRect(BMPC *bmp, int x, int y, int width, int height);
void bmpcInverse(BMPC *bmp, int x, int y, int width, int height);
void bmpcDarken(BMPC *bmp, int x, int y, int width, int height);
void bmpcRect(BMPC *bmp, int x, int y, int width, int height, unsigned long color);
void bmpcHorizontalLine(BMPC *bmp, int x, int y, int width, unsigned long color);
void bmpcVerticalLine(BMPC *bmp, int x, int y, int height, unsigned long color);
int  bmpcReadBmp(BMPC *bmp, char *fileName);
int  bmpcReadAlphaMask(BMPC *bmp, char *fileName, int color);
int  bmpcReadAlphaMaskEx(BMPC *bmp, char *fileName, int color);
void bmpcReadBitMask(BMPC *bmp, BMPC *bmpMask, int stride, int color);
int  bmpcPutText(BMPC *bmp, char *string, unsigned long color);
void bmpcPutYUV420(BMPC *bmp, unsigned char *YUVBuffer);


#endif

