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
#include <linux/fb.h>
#include "w55fa92_blt.h"
#include "w55fa92_fb.h"
#include "UIEnum.h"
#include "ctfont.h"
#include "lcdc.h"


int		_lcdNumerator, _lcdDenominator;
static int		fb_fd, blt_fd;
static int		_lcdWidth, _lcdHeight, _lcdLandscape;
static int		_colMul;
static unsigned long	_lcdBuffer[2];
static G2D_BUFFER		*g2dLcd, _g2dLcd;
static G2D_BUFFER		_g2dMems[3];
S_DRVBLT_MEMORY			_bmpBuffer[3];
static S_DRVBLT_FILL_OP				fop;
static S_DRVBLT_BLIT_OP				bop;
static S_DRVBLT_BLIT_TRANSFORMATION	xform;


int lcdOpen(void)
{
	struct fb_var_screeninfo var;
	G2D_BUFFER	*g2d;
	void		*p;
	int		rval, i, size, w, h, pagesz;

	fb_fd = open("/dev/fb0", O_RDWR, 0);
	if(fb_fd < 0) {
		printf( "lcdOpen: /dev/fb0 open error: %s\n", strerror(errno));
		return -1;  
	}
	if(ioctl(fb_fd, FBIOGET_VSCREENINFO, &var) < 0) {
		printf( "lcdOpen: ioctl(FBIOGET_VSCREENINFO): %s\n", strerror(errno));
		close(fb_fd); fb_fd = -1;
		return -1;
	}
	g2dLcd = &_g2dLcd;
	g2dLcd->width = w = var.xres; g2dLcd->height = h = var.yres; g2dLcd->bpp = var.bits_per_pixel;
printf("LCD: width=%d height=%d bits=%d\n", w, h, (int)g2dLcd->bpp);
	if(ioctl(fb_fd, IOCTL_LCD_GET_DMA_BASE, _lcdBuffer) < 0) {
		printf("lcdOpen: ioctl frame buffer physical address: %s\n", strerror(errno));
		close(fb_fd); fb_fd = -1;
		return -1;
	}
printf("Frame buffer: %08x-%08x\n", _lcdBuffer[0], _lcdBuffer[1]);
	blt_fd = open("/dev/blt", O_RDWR);
	if(blt_fd < 0) {
		printf( "lcdOpen: /dev/blt open error: %s\n", strerror(errno));
		close(fb_fd); fb_fd = -1;
		return -1;  
	}
	pagesz = getpagesize() - 1;
    size = (w * h * 4 + pagesz) & (~pagesz);
printf("BMP buffer: size=%d %d\n", size, w * h * 4);
	for(i = 0, g2d = _g2dMems;i < 3;i++, g2d++) {
		g2d->bpp = 32; g2d->size = size;
		_bmpBuffer[i].size = size;
		rval = ioctl(blt_fd, BLT_ALLOC_MEMORY, &_bmpBuffer[i]);
		if(rval < 0) {	
			printf("lcdOpen: ioctl alloc bmp buffer: %s\n", strerror(errno));
			break;
		}
		p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, blt_fd, _bmpBuffer[i].paddr);
		if(p == MAP_FAILED) {
			printf("lcdOpen: mmap error: %s\n", strerror(errno));
			break;
		}
		g2d->paddr = _bmpBuffer[i].paddr;
		g2d->addr  = _bmpBuffer[i].addr = p;
printf("BMP buffer[%d]: %08x-%08x %08x\n", i, g2d->addr, g2d->paddr, _bmpBuffer[i].vaddr);
	}
	if(i < 3) {
		size = i;
		for(i = 0;i < size;i++) { 
			rval = ioctl(blt_fd, BLT_FREE_MEMORY, &_bmpBuffer[i]);
		}
		close(fb_fd); fb_fd = -1;
		close(blt_fd); blt_fd = -1;
		return -1;  
	}
	g2dLcd->index = 0;	
	g2dLcd->paddr = _lcdBuffer[0];
	if(h > 500) {
		_lcdNumerator = 30; _lcdDenominator = 21;
	} else {
		_lcdNumerator = _lcdDenominator = 10;
	}
	_lcdWidth = w * _lcdDenominator / _lcdNumerator;
	_lcdHeight = h * _lcdDenominator / _lcdNumerator;
	_colMul = 50;
	lcdSetDirection(0);
	lcdSwapPage();
printf("LCD size: pixel=%d %d point=%d %d\n", w, h, _lcdWidth, _lcdHeight);
	return 0;
}

void lcdClose(void)
{
	int		i, rval;

	for(i = 0;i < 3;i++) {
		rval = ioctl(blt_fd, BLT_FREE_MEMORY, &_bmpBuffer[i]);
	}
	close(blt_fd);
	close(fb_fd);
	osdClose();
printf("lcdClosed...\n");
}

unsigned long lcdPhysFrameBuffer(void)
{
	return g2dLcd->paddr;
}

void lcdChangePage(void)
{
	if(g2dLcd->index) {
		g2dLcd->index = 0;
		g2dLcd->paddr = _lcdBuffer[0];
	} else {
		g2dLcd->index = 1;
		g2dLcd->paddr = _lcdBuffer[1];
	}
}

void lcdSwapPage(void)
{
	int		arg;

	arg = g2dLcd->index;
	if(ioctl(fb_fd, IOCTL_LCD_SWAP, arg) < 0) {
		printf("lcdSwapPage: swap page: %s\n", strerror(errno));
	}
}

void osdOpen(int x, int y, int width, int height)
{
	osd_cmd_t	cmd;
	int		rval;

	cmd.cmd = OSD_Open2;	
  	cmd.format = OSD_RGB565;
	cmd.x0 = x;	cmd.y0 = y;
	cmd.x0_size = width; cmd.y0_size = height;
	rval = ioctl(fb_fd, OSD_SEND_CMD, (unsigned long)&cmd);
	if(rval < 0) {
		printf("osdOpen: OSD_Open2: %s\n", strerror(errno));
	}
	cmd.cmd = OSD_Clear;
	ioctl(fb_fd, OSD_SEND_CMD, (unsigned long)&cmd);
	cmd.cmd = OSD_Show;
	ioctl(fb_fd, OSD_SEND_CMD, (unsigned long)&cmd);
	cmd.cmd = OSD_Fill;
	cmd.x0 = x;	cmd.y0 = y;
	cmd.x0_size = width; cmd.y0_size = height;
	cmd.color = 0;
	ioctl(fb_fd, OSD_SEND_CMD, (unsigned long)&cmd);
}

void osdClose(void)
{
	osd_cmd_t	cmd;
	int		rval;

	cmd.cmd = OSD_Hide;
	rval = ioctl(fb_fd, OSD_SEND_CMD, (unsigned long)&cmd);
}

void osdSetAlpha(int alpha)
{
	osd_cmd_t	cmd;

	if(alpha <= 0) { 
		cmd.cmd = OSD_ClrBlend;
//		ioctl(fb_fd, OSD_SEND_CMD, (unsigned long)&cmd);
		cmd.alpha = 0;
		cmd.cmd = OSD_SetBlend;
		ioctl(fb_fd, OSD_SEND_CMD, (unsigned long)&cmd);
	} else {
		cmd.alpha = 64;
		cmd.cmd = OSD_SetBlend;
		ioctl(fb_fd, OSD_SEND_CMD, (unsigned long)&cmd);
	}
}

void lcdOnBacklight(void)
{
	ioctl(fb_fd, VIDEO_DISPLAY_ON);
}

void lcdOffBacklight(void)
{
	ioctl(fb_fd, VIDEO_DISPLAY_OFF);
}

void lcdSetBrightness(int val)
{
	if(val < 0) val = 0; else if(val > 100) val = 100;
	ioctl(fb_fd, IOCTL_LCD_BRIGHTNESS, &val);
}

void lcdSetContrast(int val)
{
	if(val < 4) val = 4; else if(val > 100) val = 100;
//	ioctl(fb_fd, IOCTL_LCD_CONTRAST, &val);
	_colMul = val;
}

void lcdResetDisplay(void)
{
	int		regs[8];

	ioctl(fb_fd, DUMP_LCD_REG, regs);
}

G2D_BUFFER *g2dLcdBuffer(void)
{
	return g2dLcd;
}

G2D_BUFFER *g2dMemoryBuffer(int index)
{
	return &_g2dMems[index];
}

void g2dSetSize(G2D_BUFFER *g2d, int width, int height)
{
	g2d->width = g2d->clipRect.width = width;
	g2d->height = g2d->clipRect.height = height;
	g2d->clipRect.x = 0; g2d->clipRect.y = 0;
}

void g2dClipToRect(G2D_BUFFER *g2d, CGRect *rect)
{
	g2d->clipRect.x = rect->x; g2d->clipRect.y = rect->y;
	g2d->clipRect.width = rect->width; g2d->clipRect.height = rect->height;
}

void g2dSetDefaultClipRect(G2D_BUFFER *g2d)
{
	g2d->clipRect.x = 0; g2d->clipRect.y = 0;
	g2d->clipRect.width = g2d->width; g2d->clipRect.height = g2d->height;
}

void g2dFill(G2D_BUFFER *g2d, int lx, int ly, int width, int height, unsigned long color)
{
	CGRect	rt;
	int		rval, x, y, w, h, co;

//printf("g2dFill: %d,%d %d %d %08x\n", lx, ly, width, height, color);
	rt.x = lx; rt.y = ly; rt.width = width; rt.height = height;
	rectIntersection(&rt, &g2d->clipRect, &rt);
	x = rt.x; y = rt.y; w = rt.width; h = rt.height;
	if(w <= 0 || h <= 0) return;
	fop.color.u8Alpha	= color >> 24;
	fop.blend			= true;	// true if alpha blending
	if(g2d->bpp == 16) {
		co = (color >> 16) & 0xff; co = co * _colMul / 50; if(co > 255) co = 255;
		fop.color.u8Red		= co;
		co = (color >> 8) & 0xff; co = co * _colMul / 50; if(co > 255) co = 255;
		fop.color.u8Green	= co;
		co = color & 0xff; co = co * _colMul / 50; if(co > 255) co = 255;
		fop.color.u8Blue	= co;
		fop.u32FrameBufAddr	= g2d->paddr + ((y * g2d->width + x) << 1);
		fop.rowBytes		= g2d->width << 1;
		fop.format			= eDRVBLT_DEST_RGB565;
	} else {
		fop.color.u8Red		= color >> 16;
		fop.color.u8Green	= color >> 8;
		fop.color.u8Blue	= color;
		fop.u32FrameBufAddr	= g2d->paddr + ((y * g2d->width + x) << 2);
		fop.rowBytes		= g2d->width << 2;
		fop.format			= eDRVBLT_DEST_ARGB8888;
	}
	fop.rect.i16Xmin		= 0;	// none
	fop.rect.i16Xmax		= w;
	fop.rect.i16Ymin		= 0;	// none
	fop.rect.i16Ymax		= h;
	ioctl(fb_fd, IOCTL_LCD_DISABLE_INT);
	if((ioctl(blt_fd, BLT_IOCSFILL, &fop))  < 0) {
		printf("g2dFill: ioctl BLT_IOCSFILL failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	if((ioctl(blt_fd, BLT_IOCTRIGGER, NULL))  < 0) {
		printf("g2dFill: ioctl BLT_IOCTRIGGER failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	while((rval = ioctl(blt_fd, BLT_IOCFLUSH)) < 0 && x < 4) {
		if(errno == EINTR) continue;
		printf("g2dFill: ioctl BLT_IOCFLUSH failed: %s\n", strerror(errno));
		break;
	}
	ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
}

// TRANS_FLAG
//  0:SrcCopy 1:PixelAlpha 2:ColorTransform 3:PixelAlpha+ColorTransform 6:GlobalAlpha 7:GlobalAlpha+PixelAlpha
void g2dBlit(G2D_BUFFER *g2d, int lx, int ly, int width, int height, G2D_BUFFER *g2dS, int alpha)
{
	CGRect	rt;
	long long	lv;
	int		rval, x, y, w, h, temp;
int		v;

/printf("g2dBlit: %d,%d %d %d\n", lx, ly, width, height);
	rt.x = lx; rt.y = ly; rt.width = width; rt.height = height;
	rectIntersection(&rt, &g2d->clipRect, &rt);
	x = rt.x; y = rt.y; w = rt.width; h = rt.height;
	if(w <= 0 || h <= 0) return;
	if(g2d->bpp == 16) {
		bop.dest.u32FrameBufAddr= g2d->paddr + ((y * g2d->width + x) << 1);
		bop.dest.i32Stride		= g2d->width << 1;
	} else {
		bop.dest.u32FrameBufAddr= g2d->paddr + ((y * g2d->width + x) << 2);
		bop.dest.i32Stride		= g2d->width << 2;
	}
	//bop.dest.i32XOffset = bop.dest.i32YOffset = 0;
 	bop.dest.i16Width		= w;
	bop.dest.i16Height		= h;
	x -= lx; y -= ly;
	bop.src.pSARGB8			= NULL;
	bop.src.u32PaletteInx	= 0;
	bop.src.u32Num			= 0;
	bop.src.u32SrcImageAddr	= g2dS->paddr;
	bop.src.i32Stride		= g2dS->width << 2;
	lv = x * g2dS->width; lv <<= 16;
	bop.src.i32XOffset		= lv / width;
	lv = y * g2dS->height; lv <<= 16;
	bop.src.i32YOffset		= lv / height;
	bop.src.i16Width		= g2dS->width;
	bop.src.i16Height		= g2dS->height;
	xform.rotationDx		= 1;	// KDK Reveal Alpha(S_ALPHA)
	xform.rotationDy		= 0;	// KDK	
	xform.matrix.a			= (g2dS->width << 16) / width;		// x-axis scale
	xform.matrix.b			= 0x00000000;
	xform.matrix.c			= 0x00000000;
	xform.matrix.d			= (g2dS->height << 16) / height;	// y-axis scale
//printf("%08x %08x\n", xform.matrix.a, xform.matrix.d);
	xform.srcFormat			= eDRVBLT_SRC_ARGB8888;
	if(g2d->bpp == 16) {
		xform.destFormat	= eDRVBLT_DEST_RGB565;
		xform.flags			= 7;	// 3
	} else {
		xform.destFormat	= eDRVBLT_DEST_ARGB8888;
		xform.flags			= 7;
	}
//	xform.fillStyle			= (E_DRVBLT_FILL_STYLE)(eDRVBLT_NOTSMOOTH | eDRVBLT_CLIP);
	xform.fillStyle			= 0;
	if(g2d->bpp == 16) {
		xform.colorMultiplier.i16Blue	= (_colMul << 8) / 50;
		xform.colorMultiplier.i16Green	= (_colMul << 8) / 50;
		xform.colorMultiplier.i16Red	= (_colMul << 8) / 50;
	} else {
		xform.colorMultiplier.i16Blue	= 0;
		xform.colorMultiplier.i16Green	= 0;
		xform.colorMultiplier.i16Red	= 0;
	}
	xform.colorMultiplier.i16Alpha	= alpha;
	xform.colorOffset.i16Blue		= 0;
	xform.colorOffset.i16Green		= 0;
	xform.colorOffset.i16Red		= 0;
	xform.colorOffset.i16Alpha		= 0;
	xform.userData			= NULL;
    bop.transformation		= &xform;

	ioctl(fb_fd, IOCTL_LCD_DISABLE_INT);
	if((ioctl(blt_fd, BLT_IOCSBLIT, &bop)) < 0) {
		printf("g2dBlit: ioctl BLT_IOCSBLIT failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	if((ioctl(blt_fd, BLT_IOCTRIGGER, NULL))  < 0) {
		printf("g2dBlit: ioctl BLT_IOCTRIGGER failed: %s\n", strerror(errno));
		ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
		return;
	}
	while((rval = ioctl(blt_fd, BLT_IOCFLUSH)) < 0) {
		if(errno == EINTR) continue;
		printf("g2dBlit: octl BLT_IOCFLUSH failed: %s\n", strerror(errno));
		break;
	}
	ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
}

void g2dMove(G2D_BUFFER *g2d, CGRect *rect, int dy)
{
	int		rval,  x, y, w, h;

//printf("_lcdMove: %d,%d %d %d dy=%d\n", rect->x, rect->y, rect->width, rect->height, dy);
	x = rect->x; w = rect->width;
	if(dy > 0) {
		h = rect->height - dy; y = rect->y;
	} else {
		h = rect->height + dy; y = rect->y - dy;
	}
	bop.dest.u32FrameBufAddr= _g2dMems[2].paddr;
	bop.dest.i32Stride		= w << 2;
	bop.dest.i32XOffset		= 0;
	bop.dest.i32YOffset		= 0;
 	bop.dest.i16Width		= w;
	bop.dest.i16Height		= h;
	bop.src.pSARGB8			= NULL;
	bop.src.u32PaletteInx	= 0;
	bop.src.u32Num			= 0;
	if(g2d->bpp == 16) {
		bop.src.u32SrcImageAddr	= g2d->paddr + ((y * g2d->width + x) << 1);
		bop.src.i32Stride		= g2d->width << 1;
	} else {
		bop.src.u32SrcImageAddr	= g2d->paddr + ((y * g2d->width + x) << 2);
		bop.src.i32Stride		= g2d->width << 2;
	}
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
		printf("_lcdBlit: octl BLT_IOCFLUSH failed: %s\n", strerror(errno));
		break;
	}
	ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);

	y += dy;
	if(g2d->bpp == 16) {
		bop.dest.u32FrameBufAddr= g2d->paddr + ((y * g2d->width + x) << 1);
		bop.dest.i32Stride		= g2d->width << 1;
	} else {
		bop.dest.u32FrameBufAddr= g2d->paddr + ((y * g2d->width + x) << 2);
		bop.dest.i32Stride		= g2d->width << 2;
	}
	bop.dest.i32XOffset		= 0;
	bop.dest.i32YOffset		= 0;
 	bop.dest.i16Width		= w;
	bop.dest.i16Height		= h;
	bop.src.pSARGB8			= NULL;
	bop.src.u32PaletteInx	= 0;
	bop.src.u32Num			= 0;
	bop.src.u32SrcImageAddr	= _g2dMems[2].paddr;
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
	xform.userData		= NULL;
    bop.transformation	= &xform;

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
		printf("_lcdBlit: octl BLT_IOCFLUSH failed: %s\n", strerror(errno));
		break;
	}
	ioctl(fb_fd, IOCTL_LCD_ENABLE_INT);
}

void _lcdDimm(int lx, int ly, int width, int height, int alpha)
{
	int		rval, x, y, w, h, temp;

	x = lx; y = ly; w = width; h = height;
	if(w <= 0 || h <= 0) return;
	bop.dest.u32FrameBufAddr= g2dLcd->paddr + ((y * g2dLcd->width + x) << 1);
	bop.dest.i32Stride		= g2dLcd->width << 1;
	bop.dest.i32XOffset		= 0;
	bop.dest.i32YOffset		= 0;
 	bop.dest.i16Width		= w;
	bop.dest.i16Height		= h;
	bop.src.pSARGB8			= NULL;
	bop.src.u32PaletteInx	= 0;
	bop.src.u32Num			= 0;
	bop.dest.u32FrameBufAddr= g2dLcd->paddr + ((y * g2dLcd->width + x) << 1);
	bop.src.i32Stride		= g2dLcd->width << 1;
	bop.src.i32XOffset		= 0;
	bop.src.i32YOffset		= 0;
	bop.src.i16Width		= w;
	bop.src.i16Height		= h;
	xform.rotationDx		= eDRVBLT_NO_EFFECTIVE;	// KDK	
	xform.rotationDy		= eDRVBLT_NO_EFFECTIVE;	// KDK	
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
	xform.colorMultiplier.i16Alpha	= alpha;
	xform.colorOffset.i16Blue		= 0;
	xform.colorOffset.i16Green		= 0;
	xform.colorOffset.i16Red		= 0;
	xform.colorOffset.i16Alpha		= 0;
	xform.userData			= NULL;
    bop.transformation = &xform;

	if((ioctl(blt_fd, BLT_IOCSBLIT, &bop)) < 0) {
		printf("_lcdBlit: ioctl BLT_IOCSBLIT failed: %s\n", strerror(errno));
		return;
	}
	if((ioctl(blt_fd, BLT_IOCTRIGGER, NULL))  < 0) {
		printf("_lcdBlit: ioctl BLT_IOCTRIGGER failed: %s\n", strerror(errno));
		return;
	}
	while((rval = ioctl(blt_fd, BLT_IOCFLUSH)) < 0) {
		if(errno == EINTR) continue;
		printf("_lcdBlit: octl BLT_IOCFLUSH failed: %s\n", strerror(errno));
		break;
	}
}


#include "lcdc_com.c"

