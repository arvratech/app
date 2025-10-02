#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/soundcard.h>
#include <sys/poll.h>
#include <pthread.h>
#include <math.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <linux/fb.h>
#include <jpeglib.h>


METHODDEF(void)
my_error_exit(j_common_ptr ji)
{
	printf( "### Oops! error occured, here is the message from library. ###\n" );
	(*ji->err->output_message)(ji);
	jpeg_destroy(ji);
	printf( "### Here, you can exit program or not. ###\n" );
//	exit(0);	//FIX ME: if you want to return to console while error occured.
}

unsigned long	_jpegBuffer[204800];		// 819200 Bytes

int  jpegDecode(char *FileName, int enableScaleDown, unsigned char *DestBuffer, int *Width, int *Height)
{
	struct jpeg_decompress_struct *ji, _ji;
	struct jpeg_error_mgr _jerr;
	JSAMPARRAY	buffer;
	FILE	*fp;
	float	scale_ratio;
	unsigned char	*jpegBuffer, *d, r, g, b;
	unsigned short	*s, sr, sg, sb, u16RealWidth, u16RealHeight, u16YUVFormat;
	unsigned short	u16RealWidth2, u16RealHeight2;
	int		i, c, scale_num, scale_denom, size, scale_flag, row_stride;

printf( "jpegDecode: %s\n", FileName);
	fp = fopen(FileName, "rb");
	if(!fp) {
		fprintf(stderr, "can't open %s\n", FileName);
		return -1;
	}
	jpegBuffer = (unsigned char *)_jpegBuffer;
	ji = &_ji;
	ji->err = jpeg_std_error(&_jerr);
	_jerr.error_exit = my_error_exit;
	jpeg_create_decompress(ji);
	scale_num = scale_denom = 1;
	if(enableScaleDown) {
		fseek(fp, 0L, SEEK_END );
		size = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		fread(jpegBuffer, 1, size, fp);
		fseek(fp, 0L, SEEK_SET);
		NvtJpeg_GetRealWidthRealHeightYUVFormat(jpegBuffer, &u16RealWidth, &u16RealHeight, &u16YUVFormat);
		u16RealWidth2 = u16RealWidth;
		u16RealHeight2 = u16RealHeight;
		printf("\tbefore scale width = %d, before scale height = %d\n", u16RealWidth, u16RealHeight );
		scale_flag = 0;
		scale_num = 4;
		scale_denom = 8;
		while(1) {
			if(u16RealWidth2 <= 240 && u16RealHeight2 <= 320)  break;
			scale_flag = 1;
			scale_ratio = (float)scale_num / (float)scale_denom;
			u16RealWidth2 = u16RealWidth * scale_ratio;
			u16RealHeight2 = u16RealHeight * scale_ratio;
			printf( "\tafter scaled width = %d, after scaled height = %d\n", u16RealWidth2, u16RealHeight2);
			scale_denom++;
		}
		if(scale_flag) scale_denom--;
		else {
			scale_num = 1; scale_denom = 1;
		}
	}
	jpeg_stdio_src(ji, fp);
	jpeg_read_header(ji, TRUE);
	ji->dct_method = JDCT_IFAST;				// don't care, hw not support
//	ji->desired_number_of_colors = 256;		// don't care, hw not support
//	ji->quantize_colors = TRUE;				// don't care, hw not support
//	ji->dither_mode = JDITHER_FS;				// don't care, hw not support
//	ji->dither_mode = JDITHER_NONE;			// don't care, hw not support
//	ji->dither_mode = JDITHER_ORDERED;		// don't care, hw not support
//	ji->two_pass_quantize = FALSE;			// don't care, hw not support
	ji->scale_num = scale_num;				// hw just support decode scaling down
	ji->scale_denom = scale_denom;			// and hw needs denom great than num
//	ji->do_fancy_upsampling = FALSE;			// don't care, hw not support
//	ji->do_block_smoothing = TRUE;			// don't care, hw not support
	jpeg_start_decompress(ji);
printf("Width=%d Height=%d Components=%d\n", ji->output_width, ji->output_height, ji->output_components);
	row_stride = ji->output_width * ji->output_components;
	buffer = (*ji->mem->alloc_sarray)((j_common_ptr)ji, JPOOL_IMAGE, row_stride, 1);
printf("stride=%d Buffer=%x\n", row_stride, buffer[0]);
	d = DestBuffer;
	if(ji->output_components == 2) {
		while(ji->output_scanline < ji->output_height) {
			jpeg_read_scanlines(ji, buffer, 1);
			s = (unsigned short *)buffer[0];
			for(i = 0;i < ji->output_width;i++) {
				c = *s++;	
				r = ((c & 0xf800) >> 11) << 3;
				g = ((c & 0x07e0) >> 5) << 2;
				b = (c & 0x001f) << 3;
				*d++ = b; *d++ = g; *d++ = r; *d++ = 0xff;
			}
		}
	} else if(ji->output_components == 3) {
		while(ji->output_scanline < ji->output_height) {
			jpeg_read_scanlines(ji, buffer, 1);
			s = (unsigned short *)buffer[0];
			for(i = 0;i < ji->output_width;i++) {
				sr = *s++; sg = *s++; sb = *s++;
				*d++ = sb; *d++ = sg; *d++ = sr; *d++ = 0xff;
			}
		}
	} else {
		printf("Unsupported\n");
	}	
	jpeg_finish_decompress(ji);
	jpeg_destroy_decompress(ji);
	fclose(fp);
	*Width = ji->output_width; *Height = ji->output_height;
	return 0;
}
