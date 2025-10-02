#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#ifndef _HOST_EMU
#include <linux/fb.h>
#include "FB_IOCTL.h"
#include "w55fa92_blt.h"
#endif
#include "freetype.h"
#include "lcdc.h"


int		_lcdNumerator, _lcdDenominator;
static int		fb_fd, blt_fd;
static unsigned short *_lcdBuffer;	
static int		_lcdWidth, _lcdHeight, _lcdLandscape;
static int		_lcdPixelWidth, _lcdPixelHeight;
static CGRect	_lcdClipRect;
static unsigned int	*bmpBuffer, *wallPaperBuffer, *bmpBuffer2;
static S_DRVBLT_FILL_OP				fop;
static S_DRVBLT_BLIT_OP				bop;
static S_DRVBLT_BLIT_TRANSFORMATION	xform;


int lcdOpen(void)
{
	struct fb_var_screeninfo var;
	void	*p;
	int		rval, size, pagesz;

	fb_fd = open("/dev/fb0", O_RDWR);
	if(fb_fd < 0) {
		printf( "lcdOpen: cannot open LCD device\n");
		return -1;  
	}
	if(ioctl(fb_fd, FBIOGET_VSCREENINFO, &var) < 0) {
		printf( "lcdOpen: ioctl FBIOGET_VSCREENINFO: %s\n", strerror(errno));
		close(fb_fd); fb_fd = -1;
		return -1;
	}
printf("LCD: width=%d height=%d bits=%d\n", (int)var.xres, (int)var.yres, (int)var.bits_per_pixel);
    _lcdPixelWidth = var.xres; _lcdPixelHeight = var.yres;
	pagesz = getpagesize();
    size = (_lcdPixelWidth * _lcdPixelHeight * 2 + pagesz-1) / pagesz * pagesz;
printf("LCD buffer: size=%d %d pageSize=%d\n", size, _lcdPixelWidth * _lcdPixelHeight * 2, pagesz);
//	BufferSize = var.xres * var.yres * (var.bits_per_pixel >> 3);
	p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
	if(p == MAP_FAILED) {
		printf("lcdOpen: failed to map LCD device to memory: %s\n", strerror(errno));
		close(fb_fd); fb_fd = -1;
		return -1;
	}
printf("%x-%x\n", p, virt_to_phys(p));

	_lcdBuffer = (unsigned short *)p;
    size = (_lcdPixelWidth * _lcdPixelHeight * 4 + pagesz-1) / pagesz * pagesz;
printf("BMP buffer: size=%d %d\n", size, _lcdPixelWidth * _lcdPixelHeight * 4);
	rval = posix_memalign(&bmpBuffer, pagesz, size);
	if(rval) {
		printf("lcdOpen: failed alloc memory: %s\n", strerror(errno));
		close(fb_fd); fb_fd = -1;
		return -1;
	}
	rval = posix_memalign(&wallPaperBuffer, pagesz, size);
	if(rval) {
		printf("lcdOpen: failed alloc memory: %s\n", strerror(errno));
		close(fb_fd); fb_fd = -1;
		return -1;
	}
	rval = posix_memalign(&bmpBuffer2, pagesz, size);
	if(rval) {
		printf("lcdOpen: failed alloc memory: %s\n", strerror(errno));
		close(fb_fd); fb_fd = -1;
		return -1;
	}
	blt_fd = open("/dev/blt", O_RDWR);
	if(blt_fd < 0) {
		printf( "lcdOpen: cannot open BLT device\n");
		close(fb_fd); fb_fd = -1;
		return -1;  
	}
	if(ioctl(blt_fd, BLT_IOCTMMU, 1) < -1) {
		printf("lcdOpen: failed to configure BLT device to MMU mode: %s\n", strerror(errno));
		close(fb_fd); fb_fd = -1;
		close(blt_fd); blt_fd = -1;
		return -1;
	}
	if(_lcdPixelWidth > 400) {
		_lcdNumerator = 30; _lcdDenominator = 21;
	} else {
		_lcdNumerator = _lcdDenominator = 10;
	}
	_lcdWidth = _lcdPixelWidth * _lcdDenominator / _lcdNumerator;
	_lcdHeight = _lcdPixelHeight * _lcdDenominator / _lcdNumerator;
	lcdSetDirection(0);
printf("LCD size(point): width=%d height=%d\n", _lcdWidth, _lcdHeight);
	return 0;
}

void lcdClose(void)
{
	int		size, pagesz;

	pagesz = getpagesize();
    size = (_lcdPixelWidth * _lcdPixelHeight * 2 + pagesz-1) / pagesz * pagesz;
	munmap(_lcdBuffer, size);
	close(fb_fd);
	close(blt_fd);
}

#define lcdBuffer(x, y)		(_lcdBuffer + y * _lcdPixelWidth + x)

#define IOCTL_LCD_ENABLE_INT    _IO('v', 28)
#define IOCTL_LCD_DISABLE_INT   _IO('v', 29)

void _lcdFill(int lx, int ly, int width, int height, unsigned long color)
{
	CGRect	rect;
	unsigned char	alpha;
	unsigned short	*p, c;
	int		rval, x, y, w, h;

//printf("_lcdFill: %d,%d %d %d %06x\n", lx, ly, width, height, color);
//	alpha = color >> 24;
//	if(!alpha) return;
	rect.x = lx; rect.y = ly; rect.width = width; rect.height = height;
	rectIntersection(&rect, &_lcdClipRect, &rect);
	if(_lcdLandscape) {
//		x = _lcdPixelWidth-rect.y-rect.height; y = rect.x; w = rect.height; h = rect.width;	// 90
		y = _lcdPixelHeight-rect.x-rect.width; x = rect.y; w = rect.height; h = rect.width;	// 270
	} else {
		x = rect.x; y = rect.y; w = rect.width; h = rect.height;
	}
	if(w <= 0 || h <= 0) return;
	fop.color.u8Alpha		= 255;
	fop.color.u8Red			= color >> 16;
	fop.color.u8Green		= color >> 8;
	fop.color.u8Blue		= color;
	fop.blend				= false;	// true if alpha blending
	fop.u32FrameBufAddr		= (unsigned int)lcdBuffer(x, y);
	fop.rowBytes			= _lcdPixelWidth << 1;
	fop.format				= eDRVBLT_DEST_RGB565;
	fop.rect.i16Xmin		= 0;	// none
	fop.rect.i16Xmax		= w;
	fop.rect.i16Ymin		= 0;	// none
	fop.rect.i16Ymax		= h;
	ioctl(fb_fd, IOCTL_LCD_DISABLE_INT);
	if((ioctl(blt_fd, BLT_IOCSFILL, &fop))  < 0) {
		printf("_lcdFill: ioctl BLT_IOCSFILL failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	if((ioctl(blt_fd, BLT_IOCTRIGGER, NULL))  < 0) {
		printf("_lcdFill: ioctl BLT_IOCTRIGGER failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	while((rval = ioctl(blt_fd, BLT_IOCFLUSH)) < 0) {
		if(errno == EINTR) continue;
		printf("_lcdFill: ioctl BLT_IOCFLUSH failed: %s\n", strerror(errno));
		break;
	}
	ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
	if(x == 0 && y == 0) {
		p = _lcdBuffer;
		w = _lcdPixelWidth - rect.width;
		c = ((color>>16) & 0xf8) << 8;
		c |= ((color>>8) & 0xfc) << 3;
		c |= (color & 0xff) >> 3;
		for(rval = 0;rval < rect.width;rval++) *p++ = c; p += w;
		for(rval = 0;rval < rect.width;rval++) *p++ = c; p += w;
		for(rval = 0;rval < rect.width;rval++) *p++ = c; p += w;
		for(rval = 0;rval < rect.width;rval++) *p++ = c; p += w;
		for(rval = 0;rval < rect.width;rval++) *p++ = c; p += w;
		for(rval = 0;rval < rect.width;rval++) *p++ = c; p += w;
		for(rval = 0;rval < rect.width;rval++) *p++ = c;
	}
}

void _bmpFill(BMPC *bmp, int lx, int ly, int width, int height, unsigned long color)
{
	CGRect	rt;
	unsigned short	*p, c;
	int		rval, x, y, w, h;

//printf("_bmpFill(%x): %d %d : %d,%d %d %d\n", bmp->buffer, bmp->width, bmp->height, lx, ly, width, height);
	rt.x = lx; rt.y = ly; rt.width = width; rt.height = height;
	bmpcClipRect(bmp, &rt);
	x = rt.x; y = rt.y; w = rt.width; h = rt.height;
printf("_bmpFill(%x): %d,%d %d %d\n", bmp->buffer, x, y, w, h);
	if(w <= 0 || h <= 0) return;
if(w < 128) w = 128;
if(h < 128) h = 128;

	fop.color.u8Alpha		= color >> 24;
	fop.color.u8Red			= color >> 16;
	fop.color.u8Green		= color >> 8;
	fop.color.u8Blue		= color;
	fop.blend				= false;
	fop.u32FrameBufAddr		= (unsigned int)bmpcStartBuffer(bmp, x, y);
//	fop.rowBytes			= bmpcWidth(bmp) << 2;
	fop.rowBytes			= w << 2;
	fop.format				= eDRVBLT_DEST_ARGB8888;
	fop.rect.i16Xmin		= 0;	// none
	fop.rect.i16Xmax		= w;
	fop.rect.i16Ymin		= 0;
	fop.rect.i16Ymax		= h;	// none
	ioctl(fb_fd, IOCTL_LCD_DISABLE_INT);
	if((ioctl(blt_fd, BLT_IOCSFILL, &fop))  < 0) {
		printf("_bmpFill: ioctl BLT_IOCSFILL failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	if((ioctl(blt_fd, BLT_IOCTRIGGER, NULL))  < 0) {
		printf("_bmpFill: ioctl BLT_IOCTRIGGER failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
printf("...\n");
	while((rval = ioctl(blt_fd, BLT_IOCFLUSH)) < 0) {
		if(errno == EINTR) {
printf("eintr...\n");
			continue;
		}
		printf("_bmpFill: ioctl BLT_IOCFLUSH failed: %s\n", strerror(errno));
		break;
	}
	ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
printf("_bmpFill: rval=%d\n", rval);
}

/*
 *     |----x----|         
 *  -+-+---------+-----------------------------------+
 *   | |lcd                                          |
 *   y |                                             |
 *   | |                   Width                     |
 *  -+-|         +--------------------+--------------|---------+
 *     |         |                    |              |         |
 *     |         |                    |              |         |
 *     |   Height|                    |              |lcdPxielHeight|
 *     |         |                    |              |         |
 *     |         |                    |              |         |
 *     |         |                    |              |         |
 *     |         |                    |              |         |lcdPixelHeight 
 *     |         +--------------------+              |         | 
 *     |         |                                   |         |
 *     +---------+-----------------------------------+         |
 * 	             |     lcdPixelWidth                           |
 *               |                                             |
 *               |                                             |
 *               +---------------------------------------------+
 *                                lcdPixelWidth
*/


// TRANS_FLAG
//  0:SrcCopy 1:PixelAlpha 2:ColorTransform 3:PixelAlpha+ColorTransform 6:GlobalAlpha 7:GlobalAlpha+PixelAlpha
static void _lcdBlit(int lx, int ly, int width, int height, BMPC *bmpS)
{
	CGRect	rect;
	long long	lv;
	int		rval, x, y, w, h, temp;
int		v;

//printf("_lcdBlit: %d,%d %d %d\n", lx, ly, width, height);
//	if(!bmpS->Buffer) return;
	rect.x = lx; rect.y = ly; rect.width = width; rect.height = height;
	rectIntersection(&rect, &_lcdClipRect, &rect);
	if(_lcdLandscape) {
//		x = _lcdPixelWidth-rect.y-rect.height; y = rect.x; w = rect.height; h = rect.width;	// 90
		y = _lcdPixelHeight-rect.x-rect.width; x = rect.y; w = rect.height; h = rect.width;	// 270
	} else {
		x = rect.x; y = rect.y; w = rect.width; h = rect.height;
	}
//printf("_lcdBlit: %d,%d %d %d => %d,%d %d %d\n", lx, ly, width, height, x, y, w, h);
	if(w <= 0 || h <= 0) return;
	bop.dest.u32FrameBufAddr= (unsigned int)lcdBuffer(x, y);
	bop.dest.i32Stride		= _lcdPixelWidth << 1;
	//bop.dest.i32XOffset = bop.dest.i32YOffset = 0;
 	bop.dest.i16Width		= w;
	bop.dest.i16Height		= h;
	if(_lcdLandscape) {
		x = rect.x; y = rect.y; w = rect.width; h = rect.height;
	}
	x -= lx; y -= ly;
	bop.src.pSARGB8			= NULL;
	bop.src.u32PaletteInx	= 0;
	bop.src.u32Num			= 0;
	bop.src.u32SrcImageAddr	= (unsigned int)bmpS->buffer;
	bop.src.i32Stride		= bmpS->width << 2;
	lv = x * bmpS->width; lv <<= 16;
	bop.src.i32XOffset		= lv / width;
	lv = y * bmpS->height; lv <<= 16;
	bop.src.i32YOffset		= lv / height;
	bop.src.i16Width		= bmpS->width;
	bop.src.i16Height		= bmpS->height;
	xform.rotationDx		= eDRVBLT_NO_EFFECTIVE;	// KDK	
	xform.rotationDy		= eDRVBLT_NO_EFFECTIVE;	// KDK	
//	xform.rotationDx		= 0;	// KDK	
//	xform.rotationDy		= 0;	// KDK
	if(_lcdLandscape) {
		xform.matrix.a		= 0x00000000;
//		xform.matrix.b		= 0xffff0000;		// 90
//		xform.matrix.c		= 0x00010000;		// 90
		xform.matrix.b		= 0x00010000;		// 270
		xform.matrix.c		= 0xffff0000;		// 270
		xform.matrix.d		= 0x00000000;
	} else {
		xform.matrix.a		= (bmpS->width << 16) / width;		// x-axis scale
		xform.matrix.b		= 0x00000000;
		xform.matrix.c		= 0x00000000;
		xform.matrix.d		= (bmpS->height << 16) / height;	// y-axis scale
//printf("%08x %08x\n", xform.matrix.a, xform.matrix.d);
	}
	xform.srcFormat			= eDRVBLT_SRC_ARGB8888;
	xform.destFormat		= eDRVBLT_DEST_RGB565;
//	xform.flags				= eDRVBLT_HASTRANSPARENCY;	//  Bug: run like the eDRVBLT_HASTRANSPARENCY | eDRVBLT_HASCOLORTRANSFORM
	xform.flags				= 1;
//	xform.fillStyle			= (E_DRVBLT_FILL_STYLE)(eDRVBLT_NOTSMOOTH | eDRVBLT_CLIP);
	xform.fillStyle			= 0;
	xform.colorMultiplier.i16Blue	= 0;
	xform.colorMultiplier.i16Green	= 0;
	xform.colorMultiplier.i16Red	= 0;
	xform.colorMultiplier.i16Alpha	= 0;
	xform.colorOffset.i16Blue		= 0;
	xform.colorOffset.i16Green		= 0;
	xform.colorOffset.i16Red		= 0;
	xform.colorOffset.i16Alpha		= 0;
	xform.userData			= NULL;
    bop.transformation = &xform;

	ioctl(fb_fd, IOCTL_LCD_DISABLE_INT);
	if((ioctl(blt_fd, BLT_IOCSBLIT, &bop)) < 0) {
		printf("_lcdBlit: ioctl BLT_IOCSBLIT failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	if((ioctl(blt_fd, BLT_IOCTRIGGER, NULL))  < 0) {
		printf("_lcdBlit: ioctl BLT_IOCTRIGGER failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	while((rval = ioctl(blt_fd, BLT_IOCFLUSH)) < 0) {
		if(errno == EINTR) continue;
		printf("i_lcdBlit: octl BLT_IOCFLUSH failed: %s\n", strerror(errno));
		break;
	}
	ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
}

static void _lcdBlitRotate(int x, int y, int width, int height, BMPC *bmpS)
{
	CGRect	rect;
	int		rval, w, h;

//printf("_lcdBlitRotate: %d,%d %d %d\n", x, y, width, height);
//	if(!bmpS->Buffer) return;
	rect.x = x; rect.y = y; rect.width = width; rect.height = height;
	rectIntersection(&rect, &_lcdClipRect, &rect);
	w = rect.width; h = rect.height;
//if(gtag == 126) printf("_lcdBlit Rect: %d,%d %d %d\n", rect.x, rect.y, w, h);
	if(w <= 0 || h <= 0) return;
	bop.dest.u32FrameBufAddr= (unsigned int)lcdBuffer(rect.x, rect.y);
	bop.dest.i32Stride		= _lcdPixelWidth << 1;
	bop.dest.i32XOffset		= 0;
	bop.dest.i32YOffset		= 0;
 	bop.dest.i16Width		= w;
	bop.dest.i16Height		= h;
	x = rect.x - x; y = rect.y - y;
//if(gtag == 126) printf("_lcdBlit: %d,%d %d %d\n", x, y, bmpS->width, bmpS->height);
	bop.src.pSARGB8			= NULL;
	bop.src.u32PaletteInx	= 0;
	bop.src.u32Num			= 0;
	bop.src.u32SrcImageAddr	= (unsigned int)bmpcStartBuffer(bmpS, y, x);
	bop.src.i32Stride		= bmpS->width << 2;
	bop.src.i32XOffset		= 0;
	bop.src.i32YOffset		= 0;
	bop.src.i16Width		= h;
	bop.src.i16Height		= w;
	xform.rotationDx		= eDRVBLT_NO_EFFECTIVE;	// KDK	
	xform.rotationDy		= eDRVBLT_NO_EFFECTIVE;	// KDK	
	xform.matrix.a			= 0x00000000;
	xform.matrix.b			= 0xffff0000;
	xform.matrix.c			= 0x00010000;
	xform.matrix.d			= 0x00000000;

	xform.srcFormat			= eDRVBLT_SRC_ARGB8888;
	xform.destFormat		= eDRVBLT_DEST_RGB565;
//	xform.flags				= eDRVBLT_HASTRANSPARENCY;	//  Bug: run like the eDRVBLT_HASTRANSPARENCY | eDRVBLT_HASCOLORTRANSFORM
	xform.flags				= 1;
//	xform.fillStyle			= (E_DRVBLT_FILL_STYLE)(eDRVBLT_NOTSMOOTH | eDRVBLT_CLIP);
	xform.fillStyle			= 0;
	xform.colorMultiplier.i16Blue	= 0;
	xform.colorMultiplier.i16Green	= 0;
	xform.colorMultiplier.i16Red	= 0;
	xform.colorMultiplier.i16Alpha	= 0;
	xform.colorOffset.i16Blue		= 0;
	xform.colorOffset.i16Green		= 0;
	xform.colorOffset.i16Red		= 0;
	xform.colorOffset.i16Alpha		= 0;
	xform.userData			= NULL;
    bop.transformation = &xform;

	ioctl(fb_fd, IOCTL_LCD_DISABLE_INT);
	if((ioctl(blt_fd, BLT_IOCSBLIT, &bop)) < 0) {
		printf("_lcdBlit: ioctl BLT_IOCSBLIT failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	if((ioctl(blt_fd, BLT_IOCTRIGGER, NULL))  < 0) {
		printf("_lcdBlit: ioctl BLT_IOCTRIGGER failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	while((rval = ioctl(blt_fd, BLT_IOCFLUSH)) < 0) {
		if(errno == EINTR) continue;
		printf("i_lcdBlit: ioctl BLT_IOCFLUSH failed: %s\n", strerror(errno));
		break;
	}
	ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
}

/*
// TRANS_FLAG
//  0:SrcCopy 1:PixelAlpha 2:ColorTransform 3:PixelAlpha+ColorTransform 6:GlobalAlpha 7:HlobalAlpha+PixelAlpha
void _lcdDimm(int lx, int ly, int width, int height, int value)
{
	CGRect	rect;
	int		rval, x, y, w, h, temp;

	rect.x = lx; rect.y = ly; rect.width = width; rect.height = height;
	rectIntersection(&rect, &_lcdClipRect, &rect);
	if(_lcdLandscape) {
//		x = _lcdPixelWidth-rect.y-rect.height; y = rect.x; w = rect.height; h = rect.width;	// 90
		y = _lcdPixelHeight-rect.x-rect.width; x = rect.y; w = rect.height; h = rect.width;	// 270
	} else {
		x = rect.x; y = rect.y; w = rect.width; h = rect.height;
	}
	if(w <= 0 || h <= 0) return;
	bop.dest.u32FrameBufAddr= (unsigned int)lcdBuffer(x, y);
	bop.dest.i32Stride		= _lcdPixelWidth << 1;
	bop.dest.i32XOffset		= 0;
	bop.dest.i32YOffset		= 0;
 	bop.dest.i16Width		= w;
	bop.dest.i16Height		= h;
	bop.src.pSARGB8			= NULL;
	bop.src.u32PaletteInx	= 0;
	bop.src.u32Num			= 0;
	bop.src.u32SrcImageAddr	= (unsigned int)lcdBuffer(x, y);
	bop.src.i32Stride		= _lcdPixelWidth << 1;
	bop.src.i32XOffset		= 0;
	bop.src.i32YOffset		= 0;
	bop.src.i16Width		= w;
	bop.src.i16Height		= h;
	xform.rotationDx		= eDRVBLT_NO_EFFECTIVE;	// KDK	
	xform.rotationDy		= eDRVBLT_NO_EFFECTIVE;	// KDK	
//	xform.rotationDx		= 0;	// KDK	
//	xform.rotationDy		= 0;	// KDK	
	xform.matrix.a			= 0x00010000;
	xform.matrix.b			= 0x00000000;
	xform.matrix.c			= 0x00000000;
	xform.matrix.d			= 0x00010000;
	xform.srcFormat			= eDRVBLT_SRC_RGB565;
	xform.destFormat		= eDRVBLT_DEST_RGB565;
	xform.flags				= 2;
	xform.fillStyle			= (E_DRVBLT_FILL_STYLE) (eDRVBLT_NOTSMOOTH | eDRVBLT_CLIP);
	xform.colorMultiplier.i16Blue	= 1;
	xform.colorMultiplier.i16Green	= 1;
	xform.colorMultiplier.i16Red	= 1;
	xform.colorMultiplier.i16Alpha	= 0;
	xform.colorOffset.i16Blue		= value;
	xform.colorOffset.i16Green		= value;
	xform.colorOffset.i16Red		= value;
	xform.colorOffset.i16Alpha		= 0;
	xform.userData			= NULL;
    bop.transformation = &xform;

	ioctl(fb_fd, IOCTL_LCD_DISABLE_INT);
	if((ioctl(blt_fd, BLT_IOCSBLIT, &bop)) < 0) {
		printf("_lcdBlit: ioctl BLT_IOCSBLIT failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	if((ioctl(blt_fd, BLT_IOCTRIGGER, NULL))  < 0) {
		printf("_lcdBlit: ioctl BLT_IOCTRIGGER failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	while((rval = ioctl(blt_fd, BLT_IOCFLUSH)) < 0) {
		if(errno == EINTR) continue;
		printf("i_lcdBlit: octl BLT_IOCFLUSH failed: %s\n", strerror(errno));
		break;
	}
	ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
}
*/

void lcdMove(CGRect *rect, int dy)
{
	int		rval,  x, y, w, h;

//printf("_lcdMove: %d,%d %d %d dy=%d\n", rect->x, rect->y, rect->width, rect->height, dy);
	if(_lcdLandscape) {
		x = _lcdPixelWidth-rect->y-rect->height; w = rect->height;
		if(dy > 0) {
			h = rect->width - dy; y = rect->x;
		} else {
			h = rect->width + dy; y = rect->x - dy;
		}
	} else {
		x = rect->x; w = rect->width;
		if(dy > 0) {
			h = rect->height - dy; y = rect->y;
		} else {
			h = rect->height + dy; y = rect->y - dy;
		}
	}
	bop.dest.u32FrameBufAddr= (unsigned int)bmpBuffer;
	bop.dest.i32Stride		= w << 2;
	bop.dest.i32XOffset		= 0;
	bop.dest.i32YOffset		= 0;
 	bop.dest.i16Width		= w;
	bop.dest.i16Height		= h;
	bop.src.pSARGB8			= NULL;
	bop.src.u32PaletteInx	= 0;
	bop.src.u32Num			= 0;
	bop.src.u32SrcImageAddr	= (unsigned int)lcdBuffer(x, y);
	bop.src.i32Stride		= _lcdPixelWidth << 1;
	bop.src.i32XOffset		= 0;
	bop.src.i32YOffset		= 0;
	bop.src.i16Width		= w;
	bop.src.i16Height		= h;
	xform.rotationDx		= eDRVBLT_NO_EFFECTIVE;	// KDK	
	xform.rotationDy		= eDRVBLT_NO_EFFECTIVE;	// KDK	
//	xform.rotationDx		= 0;	// KDK	
//	xform.rotationDy		= 0;	// KDK	
	xform.matrix.a			= 0x00010000;
	xform.matrix.b			= 0x00000000;
	xform.matrix.c			= 0x00000000;
	xform.matrix.d			= 0x00010000;
	xform.srcFormat			= eDRVBLT_SRC_RGB565;
	xform.destFormat		= eDRVBLT_DEST_ARGB8888;
	xform.flags				= eDRVBLT_HASTRANSPARENCY;	//  Bug: run like the eDRVBLT_HASTRANSPARENCY | eDRVBLT_HASCOLORTRANSFORM
	xform.fillStyle			= (E_DRVBLT_FILL_STYLE) (eDRVBLT_NOTSMOOTH | eDRVBLT_CLIP);
	xform.colorMultiplier.i16Blue	= 0;
	xform.colorMultiplier.i16Green	= 0;
	xform.colorMultiplier.i16Red	= 0;
	xform.colorMultiplier.i16Alpha	= 0;
	xform.colorOffset.i16Blue		= 0;
	xform.colorOffset.i16Green		= 0;
	xform.colorOffset.i16Red		= 0;
	xform.colorOffset.i16Alpha		= 0;
	xform.userData			= NULL;
    bop.transformation = &xform;

	ioctl(fb_fd, IOCTL_LCD_DISABLE_INT);
	if((ioctl(blt_fd, BLT_IOCSBLIT, &bop)) < 0) {
		printf("_lcdBlit: ioctl BLT_IOCSBLIT failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	if((ioctl(blt_fd, BLT_IOCTRIGGER, NULL))  < 0) {
		printf("_lcdBlit: ioctl BLT_IOCTRIGGER failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	while((rval = ioctl(blt_fd, BLT_IOCFLUSH)) < 0) {
		if(errno == EINTR) continue;
		printf("i_lcdBlit: octl BLT_IOCFLUSH failed: %s\n", strerror(errno));
		break;
	}

	y += dy;
	bop.dest.u32FrameBufAddr= (unsigned int)lcdBuffer(x, y);
	bop.dest.i32Stride		= _lcdPixelWidth << 1;
	bop.dest.i32XOffset		= 0;
	bop.dest.i32YOffset		= 0;
 	bop.dest.i16Width		= w;
	bop.dest.i16Height		= h;
	bop.src.pSARGB8			= NULL;
	bop.src.u32PaletteInx	= 0;
	bop.src.u32Num			= 0;
	bop.src.u32SrcImageAddr	= (unsigned int)bmpBuffer;
	bop.src.i32Stride		= w << 2;
	bop.src.i32XOffset		= 0;
	bop.src.i32YOffset		= 0;
	bop.src.i16Width		= w;
	bop.src.i16Height		= h;
	xform.rotationDx		= eDRVBLT_NO_EFFECTIVE;	// KDK	
	xform.rotationDy		= eDRVBLT_NO_EFFECTIVE;	// KDK	
//	xform.rotationDx		= 0;	// KDK	
//	xform.rotationDy		= 0;	// KDK	
	xform.matrix.a			= 0x00010000;
	xform.matrix.b			= 0x00000000;
	xform.matrix.c			= 0x00000000;
	xform.matrix.d			= 0x00010000;
	xform.srcFormat			= eDRVBLT_SRC_ARGB8888;
	xform.destFormat		= eDRVBLT_DEST_RGB565;
	xform.flags				= eDRVBLT_HASTRANSPARENCY;	//  Bug: run like the eDRVBLT_HASTRANSPARENCY | eDRVBLT_HASCOLORTRANSFORM
	xform.fillStyle			= (E_DRVBLT_FILL_STYLE) (eDRVBLT_NOTSMOOTH | eDRVBLT_CLIP);
	xform.colorMultiplier.i16Blue	= 0;
	xform.colorMultiplier.i16Green	= 0;
	xform.colorMultiplier.i16Red	= 0;
	xform.colorMultiplier.i16Alpha	= 0;
	xform.colorOffset.i16Blue		= 0;
	xform.colorOffset.i16Green		= 0;
	xform.colorOffset.i16Red		= 0;
	xform.colorOffset.i16Alpha		= 0;
	xform.userData			= NULL;
    bop.transformation = &xform;

	ioctl(fb_fd, IOCTL_LCD_DISABLE_INT);
	if((ioctl(blt_fd, BLT_IOCSBLIT, &bop)) < 0) {
		printf("_lcdBlit: ioctl BLT_IOCSBLIT failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	if((ioctl(blt_fd, BLT_IOCTRIGGER, NULL))  < 0) {
		printf("_lcdBlit: ioctl BLT_IOCTRIGGER failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	while((rval = ioctl(blt_fd, BLT_IOCFLUSH)) < 0) {
		if(errno == EINTR) continue;
		printf("i_lcdBlit: octl BLT_IOCFLUSH failed: %s\n", strerror(errno));
		break;
	}
	ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
}

int lcdDirection(void)
{
	return (int)_lcdLandscape;
}

void lcdSetDirection(int landscape)
{
	_lcdLandscape = landscape;
	lcdSetDefaultClipRect();
}

#ifndef _OLDVER

void lcdOnBacklight(void)
{
	ioctl(fb_fd, VIDEO_DISPLAY_ON);
}

void lcdOffBacklight(void)
{
	ioctl(fb_fd, VIDEO_DISPLAY_OFF);
}

#endif

void lcdSetContrast(int val)
{
}

void lcdSetBrightness(int val)
{
	if(val < 0) val = 0; else if(val > 100) val = 100;
//	ioctl(fb_fd, IOCTL_LCD_BRIGHTNESS, &val);
}

int lcdRealWidth(void)
{
	return _lcdWidth;
}

int lcdRealHeight(void)
{
	return _lcdHeight;
}

int lcdWidth(void)
{
	int		val;

	if(_lcdLandscape) val = _lcdHeight;
	else	val = _lcdWidth;
	return val;
}

int lcdHeight(void)
{
	int		val;

	if(_lcdLandscape) val = _lcdWidth;
	else	val = _lcdHeight;
	return val;
}

int lcdPixelWidth(void)
{
	return _lcdPixelWidth;
}

int lcdPixelHeight(void)
{
	return _lcdPixelHeight;
}

// 24.8 fixed point format
int lcdScale(void)
{
	int		scale;
	//   model     resolution  pitch(mm/pixel) resolution(point)
	// ILI9488r     320x480      0.153a			  320x480
	// ILI9806E-2C	480x800      0.108            336x560
	if(_lcdPixelWidth > 400) scale = 0x016d;	// (30<<8) / 21;
	else	scale = 0x0100;					// 1.0
	return scale;
}

void lcdClipToRect(CGRect *rect)
{
	_lcdClipRect.x = rect->x; _lcdClipRect.y = rect->y;
	_lcdClipRect.width = rect->width; _lcdClipRect.height = rect->height;
}

//void lcdClipToMask(CGRRect *rect, BMPC *mask)

void lcdSetDefaultClipRect(void)
{
	_lcdClipRect.x = 0; _lcdClipRect.y = 0;
	_lcdClipRect.width = _lcdPixelWidth; _lcdClipRect.height = _lcdPixelHeight;
}

void lcdPointToPixelRect(CGRect *rect)
{
	rect->x = _scale(rect->x); rect->y = _scale(rect->y);
	rect->width = _scale(rect->width); rect->height = _scale(rect->height);
}

void lcdPixelClipToRect(CGRect *rect, CGRect *clipRect)
{
	_lcdClipRect.x = _scale(rect->x + clipRect->x);
	_lcdClipRect.y = _scale(rect->y + clipRect->y);
	_lcdClipRect.width  = _scale(clipRect->width);
	_lcdClipRect.height = _scale(clipRect->height);
	rect->x = _scale(rect->x); rect->y = _scale(rect->y);
	rect->width = _scale(rect->width); rect->height = _scale(rect->height);
}

void lcdClear(unsigned int Color)
{
	int		x, y, w, h;

	x = _lcdClipRect.x; y = _lcdClipRect.y; w = _lcdClipRect.width; h = _lcdClipRect.height;
	_lcdFill(x, y, w, h, Color);
}

void lcdPrint(void)
{
	unsigned short	*p;
	int		i, j;

	p = _lcdBuffer;
	for(i = 0;i < 16;i++) {
		for(j = 0;j < 32;j++) printf("%04x-", p[i]);
		printf("\n");
		p += _lcdPixelWidth;
	}
	printf("...\n");
}

#include "lcdc_com.c"
