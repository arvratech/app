#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include "../../jpeg-9d/jpeglib.h"


struct my_error_mgr {
  struct jpeg_error_mgr pub;	// "public" fields 
  jmp_buf setjmp_buffer;		// for return to caller
};

typedef struct my_error_mgr *my_error_ptr;

// Here's the routine that will replace the standard error_exit method:
METHODDEF(void)my_error_exit(j_common_ptr cinfo)
{
printf( "### Oops! error occured, here is the message from library. ###\n" );
	// cinfo->err really points to a my_error_mgr struct, so coerce pointer
	my_error_ptr myerr = (my_error_ptr)cinfo->err;
	// Always display the message.
	// We could postpone this until after returning, if we chose.
	(*cinfo->err->output_message) (cinfo);
	// Return control to the setjmp point 
	longjmp(myerr->setjmp_buffer, 1);
}

int jpegDecode(unsigned char *jpegBuffer, int jpegSize, unsigned char *dstBuffer, int *width, int *height)
{
	struct jpeg_decompress_struct *ji, _ji;
	struct my_error_mgr		_jerr;
	JSAMPARRAY		buffer;
	unsigned char	*d, *p, r, g, b;
	unsigned short	*s, u16RealWidth, u16RealHeight, u16YUVFormat;
	unsigned short	u16RealWidth2, u16RealHeight2;
	int		i, c, size, scale_flag, row_stride;

printf("jpegDecode: %d\n", jpegSize);
	ji = &_ji;
	ji->err = jpeg_std_error(&_jerr.pub);
	_jerr.pub.error_exit = my_error_exit;
	// Establish the setjmp return context for my_error_exit to use.
	if(setjmp(_jerr.setjmp_buffer)) {
		// If we get here, the JPEG code has signaled an error.
		// We need to clean up the JPEG object, close the input file, and return.
		jpeg_destroy_decompress(ji);
		return -1;
	}
/*
	if(enableScaleDown) {
		NvtJpeg_GetRealWidthRealHeightYUVFormat(jpegBuffer, &u16RealWidth, &u16RealHeight, &u16YUVFormat);
		jpeg_calc_output_dimensions
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
*/
printf("1...\n");
	jpeg_create_decompress(ji);
	jpeg_mem_src(ji, jpegBuffer, jpegSize);
	jpeg_read_header(ji, TRUE);
/*
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
*/
printf("1...\n");
	jpeg_start_decompress(ji);
printf("Width=%d Height=%d Components=%d\n", ji->output_width, ji->output_height, ji->output_components);
	row_stride = ji->output_width * ji->output_components;
	buffer = (*ji->mem->alloc_sarray)((j_common_ptr)ji, JPOOL_IMAGE, row_stride, 1);
printf("stride=%d Buffer=%x\n", row_stride, buffer[0]);
	d = dstBuffer;
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
			p = (unsigned char *)buffer[0];
			for(i = 0;i < ji->output_width;i++) {
				r = *p++; g = *p++; b = *p++;
				*d++ = b; *d++ = g; *d++ = r; *d++ = 0xff;
			}
		}
	} else {
		printf("Unsupported\n");
	}	
	jpeg_finish_decompress(ji);
	jpeg_destroy_decompress(ji);
	*width = ji->output_width; *height = ji->output_height;
	return 0;
}

int jpegDecodeFile(char *FileName, unsigned char *dstBuffer, int *width, int *height)
{
	struct jpeg_decompress_struct *ji, _ji;
	struct my_error_mgr		_jerr;
	JSAMPARRAY		buffer;
	FILE			*fp;
	unsigned char	*jpegBuffer, *d, *p, r, g, b;
	unsigned short	*s, u16RealWidth, u16RealHeight, u16YUVFormat;
	unsigned short	u16RealWidth2, u16RealHeight2;
	int		i, c, scale_num, scale_denom, size, scale_flag, row_stride;

	fp = fopen(FileName, "rb");
	if(!fp) {
		fprintf(stderr, "can't open %s\n", FileName);
		return -1;
	}
	ji = &_ji;
	ji->err = jpeg_std_error(&_jerr.pub);
	_jerr.pub.error_exit = my_error_exit;
	// Establish the setjmp return context for my_error_exit to use.
	if(setjmp(_jerr.setjmp_buffer)) {
		// If we get here, the JPEG code has signaled an error.
		// We need to clean up the JPEG object, close the input file, and return.
		jpeg_destroy_decompress(ji);
		fclose(fp);
		return -1;
	}
	// Now we can initialize the JPEG decompression object.
	jpeg_create_decompress(ji);
	// Step 2: specify data source (eg, a file)
	jpeg_stdio_src(ji, fp);
	// Step 3: read file parameters with jpeg_read_header()
	jpeg_read_header(ji, TRUE);
	// Step 4: set parameters for decompression
//	ji->dct_method			= JDCT_IFAST;		// don't care, hw not support
//	ji->desired_number_of_colors = 256;			// don't care, hw not support
//	ji->quantize_colors		= TRUE;				// don't care, hw not support
//	ji->dither_mode			= JDITHER_FS;		// don't care, hw not support
//	ji->dither_mode			= JDITHER_NONE;		// don't care, hw not support
//	ji->dither_mode			= JDITHER_ORDERED;	// don't care, hw not support
//	ji->two_pass_quantize	= FALSE;			// don't care, hw not support
//	ji->scale_num			= 1;				// hw just support decode scaling down
//	ji->scale_denom			= 1;				// and hw needs denom great than num
//	ji->do_fancy_upsampling	= FALSE;			// don't care, hw not support
//	ji->do_block_smoothing	= TRUE;				// don't care, hw not support
	// Step 5: Start decompressor
	jpeg_start_decompress(ji);
printf("width=%d height=%d components=%d\n", ji->output_width, ji->output_height, ji->output_components);
	// JSAMPLEs per row in output buffer
	row_stride = ji->output_width * ji->output_components;
	// Make a one-row-high sample array that will go away when done with image
	buffer = (*ji->mem->alloc_sarray)((j_common_ptr)ji, JPOOL_IMAGE, row_stride, 1);
printf("stride=%d Buffer=%x\n", row_stride, buffer[0]);
	d = dstBuffer;
	// Step 6: while (scan lines remain to be read)
	//            jpeg_read_scanlines(...);
	// Here we use the library's state variable ji->output_scanline as the
	// loop counter, so that we don't have to keep track ourselves.
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
			// jpeg_read_scanlines expects an array of pointers to scanlines.
			// Here the array is only one element long, but you could ask for
			// more than one scanline at a time if that's more convenient.
			jpeg_read_scanlines(ji, buffer, 1);
			p = (unsigned char *)buffer[0];
			for(i = 0;i < ji->output_width;i++) {
				r = *p++; g = *p++; b = *p++;
				*d++ = b; *d++ = g; *d++ = r; *d++ = 0xff;
			}
		}
	} else {
		printf("Unsupported\n");
	}	
	// Step 7: Finish decompression
	jpeg_finish_decompress(ji);
	// Step 8: Release JPEG decompression object
	jpeg_destroy_decompress(ji);
	*width = ji->output_width; *height = ji->output_height;
	fclose(fp);
	return 0;
}

