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
#include "/root/a20-olimex/linux-sunxi/include/linux/fb.h"
#include "/root/a20-olimex/linux-sunxi/include/linux/g2d_driver.h"
#include "/root/a20-olimex/linux-sunxi/include/video/sunxi_disp_ioctl.h"
//#include "drv_display_sun4i.h"
#include "UIEnum.h"
#include "ctfont.h"
#include "cbuf.h"
#include "lcdc.h"

int		_lcdNumerator, _lcdDenominator;
static int		disp_fd, fb_fd, blt_fd;
static int		_lcdWidth, _lcdHeight, _lcdLandscape;
static unsigned long	_lcdBuffer[2];
static G2D_BUFFER		*g2dLcd, _g2dLcd;
static G2D_BUFFER		_g2dMems[3];
static g2d_fillrect		fop;
static g2d_blt			bop;
static g2d_stretchblt	sop;
static struct fb_var_screeninfo var;
unsigned char	cbufLcd[8+32];


int lcdOpen(void)
{
	struct fb_fix_screeninfo fix;
	G2D_BUFFER	*g2d;
	void		*p;
	unsigned long	args[4];
	int		rval, i, size, w, h, pagesz;

	disp_fd = open("/dev/disp", O_RDWR, 0);
	if(disp_fd < 0) {
		printf( "lcdOpen: /dev/disp open error: %s\n", strerror(errno));
		return -1;  
	}
    args[0] = 0;
	ioctl(disp_fd, DISP_CMD_LCD_ON, args);

	fb_fd = open("/dev/fb0", O_RDWR);
	if(fb_fd < 0) {
		printf( "lcdOpen: /dev/fb0 open error\n");
		close(disp_fd); disp_fd = -1;
		return -1;  
	}
	if(ioctl(fb_fd, FBIOGET_VSCREENINFO, &var) < 0) {
		printf( "lcdOpen: ioctl(FBIOGET_VSCREENINFO): %s\n", strerror(errno));
		close(disp_fd); disp_fd = -1; close(fb_fd); fb_fd = -1;
		return -1;
	}
	g2dLcd = &_g2dLcd;
	g2dLcd->width = w = var.xres; g2dLcd->height = h = var.yres; g2dLcd->bpp = var.bits_per_pixel;
printf("LCD: width=%d height=%d bits=%d\n", w, h, (int)g2dLcd->bpp);
	if(ioctl(fb_fd, FBIOGET_FSCREENINFO, &fix) < 0) {
		printf( "lcdOpen: ioctl(FBIOGET_FSCREENINFO): %s\n", strerror(errno));
		close(disp_fd); disp_fd = -1; close(fb_fd); fb_fd = -1;
		return -1;
	}
	_lcdBuffer[0] = fix.smem_start;
	_lcdBuffer[1] = fix.smem_start + w * h * 4;
printf("Framebuffer: %08x-%08x size=%d\n", _lcdBuffer[0], _lcdBuffer[1], fix.smem_len);
	blt_fd = open("/dev/g2d", O_RDWR);
	if(blt_fd < 0) {
		printf( "lcdOpen: /dev/g2d open error: %s\n", strerror(errno));
		close(disp_fd); disp_fd = -1; close(fb_fd); fb_fd = -1;
		return -1;  
	}
	pagesz = getpagesize();
    size = (w * h * 4 + pagesz-1) / pagesz * pagesz;
printf("BMP buffer: size=%d %d\n", size, w * h * 4);
	for(i = 0, g2d = _g2dMems;i < 3;i++, g2d++) {
		g2d->bpp = 32; g2d->size = size;
		g2d->index = ioctl(blt_fd, G2D_CMD_MEM_REQUEST, size);
		if(g2d->index < 0) {
			printf("lcdOpen: ioctl(G2D_CMD_MEM_REQUEST) error: %s\n", strerror(errno));
			break;
		}
		g2d->paddr = ioctl(blt_fd, G2D_CMD_MEM_GETADR, g2d->index);
		ioctl(blt_fd, G2D_CMD_MEM_SELIDX, g2d->index);
		//p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, blt_fd, g2d->paddr);
		p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, blt_fd, 0);
		if(p == MAP_FAILED) {
			printf("lcdOpen: mmap error: %s\n", strerror(errno));
			break;
		}
		g2d->addr = p;
printf("BMP buffer[%d]: index=%d %08x-%08x\n", i, g2d->index, g2d->addr, g2d->paddr);
	}
	if(i < 3) {
		size = i;
		for(i = 0, g2d = _g2dMems;i < size;i++, g2d++) {
			rval = ioctl(blt_fd, G2D_CMD_MEM_RELEASE, g2d->index);
		}
		close(disp_fd); disp_fd = -1; close(fb_fd); fb_fd = -1; close(blt_fd); blt_fd = -1;
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
	lcdSetDirection(0);
	lcdSwapPage();
printf("LCD size: pixel=%d %d point=%d %d\n", w, h, _lcdWidth, _lcdHeight);
args[0] = 0;
rval = ioctl(disp_fd, DISP_CMD_HWC_CLOSE, args);
//ioctl(fb_fd, FBIOBLANK, FB_BLANK_POWERDOWN); 
//printf("VESA_POWERDOWN, press any key\n"); getchar();
//ioctl(fb_fd, FBIOBLANK, FB_BLANK_UNBLANK); 
//printf("FB_BLANK_UNBLANK, press ant key\n"); getchar();
	cbufInit(cbufLcd, 31);
	return 0;
}

void lcdClose(void)
{
	int		i, rval;

	for(i = 0;i < 3;i++) {
		rval = ioctl(blt_fd, G2D_CMD_MEM_RELEASE, _g2dMems[i].index);
	}
	close(blt_fd);
	close(fb_fd);
	close(disp_fd);
printf("lcdClose...\n");
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

#include "rtc.h"

void lcdSwapPage(void)
{
	if(g2dLcd->index) var.yoffset = g2dLcd->height;
	else	var.yoffset = 0;
    if(ioctl(fb_fd, FBIOPAN_DISPLAY, &var) < 0) {
		printf("framebuffer: FBIOPAN_DISPLAY error: %s\n", strerror(errno));
	}
}

int lcdOpenVideo(void)
{
	unsigned long	args[4];
	int		handle;

	args[0] = 0;
	args[1] = DISP_LAYER_WORK_MODE_SCALER;
	args[2] = args[3] = 0;
	handle = ioctl(disp_fd, DISP_CMD_LAYER_REQUEST, args);
	if(!handle) {
		printf("disp layer request error: %s\n", strerror(errno));
		return -1;
	}
	//width = ioctl(disp_fd, DISP_CMD_SCN_GET_WIDTH, args);
	//height = ioctl(disp_fd,DISP_CMD_SCN_GET_HEIGHT, args);
//printf("lcdOpenVideo: %x\n", handle);
	return handle;
}

void lcdCloseVideo(int handle)
{
	unsigned long	args[4];
	int		rval;

	args[0]	= 0;
	args[1] = handle;
	args[2] = args[3] = 0;
	ioctl(disp_fd, DISP_CMD_VIDEO_STOP, args);
	args[0]	= 0;
	args[1] = handle;
	args[2] = args[3] = 0;
	ioctl(disp_fd, DISP_CMD_LAYER_CLOSE, args);
	args[0]	= 0;
	args[1] = handle;
	args[2] = args[3] = 0;
	ioctl(disp_fd, DISP_CMD_LAYER_RELEASE, args);
printf("lcdCloseVideo: %d\n", handle);
//	args[0]	= 0;
//	rval = ioctl(disp_fd, DISP_CMD_GET_OUTPUT_TYPE, args);
//printf("lcd outType=%d %d %d\n", rval, DISP_OUTPUUT_TYPE_LCD, DISP_OUTPUUT_TYPE_HDMI);
}

#ifndef _NOVIDEO

#include "../avlib_a20/vdecoder.h"

void lcdStartVideo(int handle, LCD_VIDEO *video)
{
	unsigned long	args[4];
	__disp_layer_info_t		*layer_info, _layer_info;
	int		rval;

	layer_info = &_layer_info;
	args[0]	= 0;
	args[1] = handle;
	args[2] = (unsigned long)layer_info;
	args[3] = 0;
	rval = ioctl(disp_fd, DISP_CMD_LAYER_GET_PARA, args);
	layer_info->b_from_screen	= 0;
	//if(pict->nWidth < 720) layer_info->fb.cs_mode = DISP_BT601;
	//else	layer_info->fb.cs_mode = DISP_BT709;
	layer_info->fb.cs_mode		= DISP_BT601;
	layer_info->fb.format		= DISP_FORMAT_YUV420;
	if(video->mode) layer_info->fb.mode	= DISP_MOD_MB_UV_COMBINED;		// YUV420SP-NV12 Tiled - H.264 decoder
	else			layer_info->fb.mode	= DISP_MOD_NON_MB_UV_COMBINED;	// YUV420SP-NV12 - camera preview
	layer_info->fb.seq			= DISP_SEQ_UVUV;
	layer_info->fb.br_swap		= 0;
	layer_info->fb.size.width	= video->size.width;
	layer_info->fb.size.height	= video->size.height;
	layer_info->fb.addr[0]		= video->paddrY;
	layer_info->fb.addr[1]		= video->paddrC;
	layer_info->mode			= DISP_LAYER_WORK_MODE_SCALER;
	layer_info->alpha_en		= 1;
	layer_info->alpha_val		= 255;
	layer_info->ck_enable		= 0;
	layer_info->pipe			= 0;
	layer_info->prio			= 255;
	layer_info->src_win.x		= video->src.x;
	layer_info->src_win.y		= video->src.y;
	layer_info->src_win.width	= video->src.width;
	layer_info->src_win.height	= video->src.height;
	layer_info->scn_win.x		= video->dst.x;
	layer_info->scn_win.y		= video->dst.y;
	layer_info->scn_win.width	= video->dst.width;
	layer_info->scn_win.height	= video->dst.height;

	args[0]	= 0;
	args[1] = handle;
	args[2] = (unsigned long)layer_info;
	args[3] = 0;
	rval = ioctl(disp_fd, DISP_CMD_LAYER_SET_PARA, args);
	args[0]	= 0;
	args[1] = handle;
	args[2] = args[3] = 0;
	ioctl(disp_fd, DISP_CMD_LAYER_TOP, args);
	args[0]	= 0;
	args[1] = handle;
	args[2] = args[3] = 0;
	rval = ioctl(disp_fd, DISP_CMD_LAYER_OPEN, args);
	args[0]	= 0;
	args[1] = handle;
	args[2] = args[3] = 0;
	rval = ioctl(disp_fd, DISP_CMD_VIDEO_START, args);
/*
{
	__disp_rect_t	rt;
	args[0]	= 0;
	args[1] = handle;
	args[2] = (unsigned long)&rt;
	args[3] = 0;
	rval = ioctl(disp_fd, DISP_CMD_LAYER_GET_SCN_WINDOW, args);
printf("lcd: %d,%d %d %d\n", rt.x, rt.y, rt.width, rt.height);
}
*/
printf("lcdStartVideo: %d\n", (int)video->mode);
}

void lcdUpdateVideo(int handle, unsigned long paddrY, unsigned long paddrC)
{
	unsigned long	args[4];
	__disp_video_fb_t	*video_fb, _video_fb;
	int		rval;

	video_fb = &_video_fb;
	video_fb->id				= 0;
	video_fb->addr[0]			= paddrY;
	video_fb->addr[1]			= paddrC;
	video_fb->interlace			= false;
	video_fb->top_field_first	= false;
	video_fb->frame_rate		= 0;
	video_fb->flag_addr			= 0;
	video_fb->flag_stride		= 0;
	video_fb->maf_valid			= false;
	video_fb->pre_frame_valid	= false;
	args[0]	= 0;
	args[1] = handle;
	args[2] = (unsigned long)video_fb;
	args[3] = 0;
	ioctl(disp_fd, DISP_CMD_VIDEO_SET_FB, args);
}

#endif

void lcdSetPwm(int on)
{
	unsigned char	buf[4];

	buf[0] = 1; buf[1] = on;
	cbufPut(cbufLcd, buf, 2);
}

void lcdOnBacklight(void)
{
	unsigned char	buf[4];

	buf[0] = 2; buf[1] = 0;
	cbufPut(cbufLcd, buf, 2);
}

void lcdOffBacklight(void)
{
	unsigned char	buf[4];

	buf[0] = 3; buf[1] = 0;
	cbufPut(cbufLcd, buf, 2);
}

void lcdSetBrightness(int val)
{
	unsigned char	buf[4];

	if(val < 0) val = 0; else if(val > 100) val = 100;
	buf[0] = 4; buf[1] = val;
	cbufPut(cbufLcd, buf, 2);
}

void lcdSetContrast(int val)
{
	unsigned char	buf[4];

	if(val < 4) val = 4; else if(val > 100) val = 100;
	buf[0] = 5; buf[1] = val;
	cbufPut(cbufLcd, buf, 2);
}

void lcdDoOperation(unsigned char *buf)
{
	unsigned long	args[4];

	args[0] = 0;
	args[1] = buf[1];
	args[2] = args[3] = 0;
	switch(buf[0]) {
	case 2:
		ioctl(disp_fd, DISP_CMD_LCD_ON, args);
		break;
	case 3:
		ioctl(disp_fd, DISP_CMD_LCD_OFF, args);
		break;
	case 4:
		ioctl(disp_fd, DISP_CMD_LCD_SET_BRIGHTNESS, args);
		break;
	case 5:
		//ioctl(disp_fd, IOCTL_LCD_CONTRAST, &val);
		break;
	}
}

void lcdResetDisplay(void)
{
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
	int		x, y, w, h;

//printf("g2dFill: %d,%d %d %d <= %d,%d\n", lx, ly, width, height, g2d->width, g2d->height);
	rt.x = lx; rt.y = ly; rt.width = width; rt.height = height;
	rectIntersection(&rt, &g2d->clipRect, &rt);
	x = rt.x; y = rt.y; w = rt.width; h = rt.height;
	if(w <= 0 || h <= 0) return;
	fop.dst_image.addr[0]	= g2d->paddr;
	fop.dst_image.w			= g2d->width;
	fop.dst_image.h			= g2d->height;
	fop.dst_image.format	= G2D_FMT_ARGB_AYUV8888;
	fop.dst_image.pixel_seq	= G2D_SEQ_NORMAL;
	fop.dst_rect.x			= x;
	fop.dst_rect.y			= y;
	fop.dst_rect.w			= w;
	fop.dst_rect.h			= h;
//	fop.flag				= G2D_FIL_PIXEL_ALPHA;
	fop.flag				= G2D_FIL_NONE;
	fop.alpha				= 0;
	fop.color				= color;
	if((ioctl(blt_fd, G2D_CMD_FILLRECT, &fop))  < 0) {
		printf("g2dFill: G2D_CMD_FILLRECT error: %s\n", strerror(errno));
	}
}

void g2dBlit(G2D_BUFFER *g2d, int lx, int ly, int width, int height, G2D_BUFFER *g2dS, int alpha)
{
	CGRect	rt;
	int		rval, x, y, w, h, flag;

//printf("g2dBlit: %d,%d %d %d <= %d,%d\n", lx, ly, width, height, g2d->width, g2d->height);
	rt.x = lx; rt.y = ly; rt.width = width; rt.height = height;
	rectIntersection(&rt, &g2d->clipRect, &rt);
	x = rt.x; y = rt.y; w = rt.width; h = rt.height;
	if(w <= 0 || h <= 0) return;
	if(width == g2dS->width && height == g2dS->height) {
//printf("blit:\n");
		bop.src_image.addr[0]	= g2dS->paddr;
		bop.src_image.w			= g2dS->width;
		bop.src_image.h			= g2dS->height;
		bop.src_image.format	= G2D_FMT_ARGB_AYUV8888;
		bop.src_image.pixel_seq	= G2D_SEQ_NORMAL;
		bop.src_rect.x			= x - lx;
		bop.src_rect.y			= y - ly;
		bop.src_rect.w			= w;
		bop.src_rect.h			= h;

		bop.dst_image.addr[0]	= g2d->paddr;
		bop.dst_image.w			= g2d->width;
		bop.dst_image.h			= g2d->height;
		bop.dst_image.format	= G2D_FMT_ARGB_AYUV8888;
		bop.dst_image.pixel_seq	= G2D_SEQ_NORMAL;
		bop.dst_x				= x;
		bop.dst_y				= y;

		if(alpha < 255) flag = G2D_BLT_MULTI_ALPHA;
		else			flag = G2D_BLT_PIXEL_ALPHA;
		bop.flag				= flag;
		bop.alpha				= alpha;
		bop.color				= 0;
		if((ioctl(blt_fd, G2D_CMD_BITBLT, &bop))  < 0) {
			printf("g2dBlit: G2D_CMD_BITBLT error: %s\n", strerror(errno));
		}
	} else if(w == width && h == height) { 
//printf("stretch: %d %d => %d %d\n", g2dS->width, g2dS->height, w, h); 
		sop.src_image.addr[0]	= g2dS->paddr;
		sop.src_image.w			= g2dS->width;
		sop.src_image.h			= g2dS->height;
		sop.src_image.format	= G2D_FMT_ARGB_AYUV8888;
		sop.src_image.pixel_seq	= G2D_SEQ_NORMAL;
		sop.src_rect.x			= 0;
		sop.src_rect.y			= 0;
		sop.src_rect.w			= g2dS->width;
		sop.src_rect.h			= g2dS->height;

		sop.dst_image.addr[0]	= g2d->paddr;
		sop.dst_image.w			= g2d->width;
		sop.dst_image.h			= g2d->height;
		sop.dst_image.format	= G2D_FMT_ARGB_AYUV8888;
		sop.dst_image.pixel_seq	= G2D_SEQ_NORMAL;
		sop.dst_rect.x			= x;
		sop.dst_rect.y			= y;
		sop.dst_rect.w			= w;
		sop.dst_rect.h			= h;

		if(alpha < 255) flag = G2D_BLT_MULTI_ALPHA;
		else			flag = G2D_BLT_PIXEL_ALPHA;
		sop.flag				= flag;
		sop.alpha				= alpha;
		sop.color				= 0;
		if((ioctl(blt_fd, G2D_CMD_STRETCHBLT, &sop))  < 0) {
			printf("g2dBlit: G2D_CMD_STRETCHBLT error: %s\n", strerror(errno));
		}
	} else {
//printf("stretch, blit: %d %d => %d %d\n", g2dS->width, g2dS->height, width, height); 
		sop.src_image.addr[0]	= g2dS->paddr;
		sop.src_image.w			= g2dS->width;
		sop.src_image.h			= g2dS->height;
		sop.src_image.format	= G2D_FMT_ARGB_AYUV8888;
		sop.src_image.pixel_seq	= G2D_SEQ_NORMAL;
		sop.src_rect.x			= 0;
		sop.src_rect.y			= 0;
		sop.src_rect.w			= g2dS->width;
		sop.src_rect.h			= g2dS->height;

		sop.dst_image.addr[0]	= _g2dMems[2].paddr;
		sop.dst_image.w			= width;
		sop.dst_image.h			= height;
		sop.dst_image.format	= G2D_FMT_ARGB_AYUV8888;
		sop.dst_image.pixel_seq	= G2D_SEQ_NORMAL;
		sop.dst_rect.x			= 0;
		sop.dst_rect.y			= 0;
		sop.dst_rect.w			= width;
		sop.dst_rect.h			= height;

		sop.flag				= G2D_BLT_NONE;
		sop.alpha				= 0;
		sop.color				= 0;
		if((ioctl(blt_fd, G2D_CMD_STRETCHBLT, &sop))  < 0) {
			printf("g2dBlit: G2D_CMD_STRETCHBLT error: %s\n", strerror(errno));
			return;
		}

		bop.src_image.addr[0]	= _g2dMems[2].paddr;
		bop.src_image.w			= width;
		bop.src_image.h			= height;
		bop.src_image.format	= G2D_FMT_ARGB_AYUV8888;
		bop.src_image.pixel_seq	= G2D_SEQ_NORMAL;
		bop.src_rect.x			= x - lx;
		bop.src_rect.y			= y - ly;
		bop.src_rect.w			= w;
		bop.src_rect.h			= h;

		bop.dst_image.addr[0]	= g2d->paddr;
		bop.dst_image.w			= g2d->width;
		bop.dst_image.h			= g2d->height;
		bop.dst_image.format	= G2D_FMT_ARGB_AYUV8888;
		bop.dst_image.pixel_seq	= G2D_SEQ_NORMAL;
		bop.dst_x				= x;
		bop.dst_y				= y;

		if(alpha < 255) flag = G2D_BLT_MULTI_ALPHA;
		else			flag = G2D_BLT_PIXEL_ALPHA;
		bop.flag				= flag;
		bop.alpha				= alpha;
		bop.color				= 0;
		if((ioctl(blt_fd, G2D_CMD_BITBLT, &bop))  < 0) {
			printf("g2dBlit: G2D_CMD_BITBLT error: %s\n", strerror(errno));
		}
	}
}

void g2dMove(G2D_BUFFER *g2d, CGRect *rect, int dy)
{
	int		x, y, w, h;

	x = rect->x; w = rect->width;
	if(dy > 0) {
		h = rect->height - dy; y = rect->y;
	} else {
		h = rect->height + dy; y = rect->y - dy;
	}
	bop.src_image.addr[0]	= g2d->paddr;
	bop.src_image.w			= g2d->width;
	bop.src_image.h			= g2d->height;
	bop.src_image.format	= G2D_FMT_ARGB_AYUV8888;
	bop.src_image.pixel_seq	= G2D_SEQ_NORMAL;
	bop.src_rect.x			= x;
	bop.src_rect.y			= y;
	bop.src_rect.w			= w;
	bop.src_rect.h			= h;

	bop.dst_image.addr[0]	= g2d->paddr;
	bop.dst_image.w			= g2d->width;
	bop.dst_image.h			= g2d->height;
	bop.dst_image.format	= G2D_FMT_ARGB_AYUV8888;
	bop.dst_image.pixel_seq	= G2D_SEQ_NORMAL;
	bop.dst_x				= x;
	bop.dst_y				= y + dy;

	bop.flag				= G2D_BLT_NONE;
	sop.alpha				= 0;
	bop.color				= 0;
	if((ioctl(blt_fd, G2D_CMD_BITBLT, &bop))  < 0) {
		printf("g2dMove: G2D_CMD_BITBLT error: %s\n", strerror(errno));
	}
}

#include "lcdc_com.c"
