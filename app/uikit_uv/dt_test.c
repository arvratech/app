

#include <errno.h>
#include "dt.h"


#define FB_WIDTH 800
#define FB_HEIGHT 480
#define DOUBLE_BUFFER 0 //0/1
#define SCREEN_A 0

int main(void)
{
	struct fb_var_screeninfo var;
	struct fb_fix_screeninfo fix;
	__disp_color_t bk_color;
	__disp_fb_create_para_t fb_para;
	__disp_rect_t scn_win;
	unsigned long args[4];
	unsigned long layer_hdl0;
	void	 *addr;
	unsigned long	*p, val;
	int		rval, fd, fbfd, vd_layer, i, j, w, h;

	fd = open("/dev/disp", O_RDWR);
	if(fd < 0) {
		printf("/dev/disp open error: %s\n", strerror(errno));
		return 0;
	}
	args[0] = 0;
	rval = ioctl(fd, DISP_CMD_GET_OUTPUT_TYPE, args);
printf("output type=%d\n", rval);
	args[0] = 0;
	rval = ioctl(fd, DISP_CMD_SCN_GET_WIDTH, args);
	w = rval;
	args[0] = 0;
	rval = ioctl(fd, DISP_CMD_SCN_GET_HEIGHT, args);
	h = rval;
printf("width=%d height=%d\n", w, h);
	args[0] = 0;
	rval = ioctl(fd, DISP_CMD_HWC_CLOSE, args);

    args[0] = SCREEN_A;
	ioctl(fd, DISP_CMD_LCD_ON, (unsigned long)args);

	fbfd = open("/dev/fb0", O_RDWR);
	if(fbfd < 0) {
		printf("/dev/fb0 open error: %s\n", strerror(errno));	
		close(fd);
		return 0;
	}
	ioctl(fbfd, FBIOGET_VSCREENINFO, &var);
printf("%d,%d %d %d bits=%d\n", var.xoffset, var.yoffset, var.xres, var.yres, var.bits_per_pixel);
	ioctl(fbfd, FBIOGET_FSCREENINFO, &fix);
	addr = mmap(NULL, fix.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
printf("addr=%08x %08x len=%d\n", addr, fix.smem_start, fix.smem_len);
	ioctl(fbfd, FBIOBLANK, FB_BLANK_POWERDOWN); 
printf("VESA_POWERDOWN, press any key\n"); getchar();
	ioctl(fbfd, FBIOBLANK, FB_BLANK_UNBLANK); 
printf("FB_BLANK_UNBLANK, press ant key\n"); getchar();

/*
	args[0] = 0;
	args[1] = DISP_LAYER_WORK_MODE_SCALER;
	args[2] = args[3] = 0;
	vd_layer = ioctl(fd, DISP_CMD_LAYER_REQUEST, args);
	if(vd_layer == 0) {
		printf("DISP_CMD_LAYER_REQUEST error\n");
	}
	args[1] = vd_layer;
	ioctl(fd, DISP_CMD_LAYER_BOTTOM, args);
*/

	bk_color.red = 0x00;
	bk_color.green = 0x00;
	bk_color.blue = 0x80;
	args[0] = SCREEN_A;
	args[1] = (unsigned long)&bk_color;
	ioctl(fd, DISP_CMD_SET_BKCOLOR, (unsigned long)args);
printf("set background color, press any key\n"); getchar();
	p = (unsigned long *)addr; val = 0xffffff00;
	for(i = 0;i < 180;i++)
		for(j = 0;j < w;j++)
			*p++ = val;
	val = 0xffff00ff;
	for( ;i < h;i++)
		for(j = 0;j < w;j++)
			*p++ = val;
printf("press any key\n"); getchar();
	 var.yoffset = 272;
	val = 0xffff0000;
	for(i = 0;i < 180;i++)
		for(j = 0;j < w;j++)
			*p++ = val;
	val = 0xff0000ff;
	for( ;i < h;i++)
		for(j = 0;j < w;j++)
			*p++ = val;
	 var.yoffset = 272;
    ioctl(fbfd, FBIOPAN_DISPLAY, &var);
printf("press any key\n"); getchar();
	 var.yoffset = 0;
    ioctl(fbfd, FBIOPAN_DISPLAY, &var);
printf("press any key\n"); getchar();
	

/*
	fb_para.mode = DISP_LAYER_WORK_MODE_SCALER;
	fb_para.smem_len = w * h * 2;
	fb_para.ch1_offset = w * h;
	fb_para.ch2_offset = 0;
	args[0] = SCREEN_A;
	args[1] = (unsigned long)&fb_para;
	layer_hdl0 = ioctl(fd, DISP_CMD_FB_REQUEST, args);
	if(layer_hdl0 <= 0) {
		printf("request fb fail\n");
	}
printf("addr=%08x\n", layer_hdl0);
	
    scn_win.x = scn_win.y = 0;
    scn_win.width = w;
    scn_win.height = h;
	args[0] = SCREEN_A;
	args[1] = layer_hdl0;
	args[2] = (unsigned long)&scn_win;
    ioctl(fd, DISP_CMD_LAYER_SET_SCN_WINDOW, args);
*/

printf("press any key\n"); getchar();
	args[0] = SCREEN_A;
//	ioctl(fd, DISP_CMD_LCD_OFF,(unsigned long)args);

	close(fbfd);
	close(fd);
	return 0;
}

