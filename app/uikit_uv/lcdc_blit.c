static void _bmpFill(int width, int height, unsigned long color)
{
	CGRect	rect;
	unsigned char	alpha;
	int		rval;

printf("_bmpFill: w=%d h=%d c=%08x\n", width, height, color);
	alpha = color >> 24;
	if(!alpha) return;
	rect.x = 0; rect.y = 0; rect.width = width; rect.height = height;
	rectIntersection(&rect, &lcdClipRect, &rect);
	if(rect.width <= 0 || rect.height <= 0) return;
	fop.color.u8Alpha		= alpha;
	fop.color.u8Red			= color >> 16;
	fop.color.u8Green		= color >> 8;
	fop.color.u8Blue		= color;
	fop.blend				= true;
	fop.u32FrameBufAddr		= (unsigned int)layerBuffer;
	fop.rowBytes			= lcdGetWidth() << 1; 
	fop.format				= eDRVBLT_DEST_ARGB8888;
	fop.rect.i16Xmin		= 0;
	fop.rect.i16Xmax		= rect.width;
	fop.rect.i16Ymin		= 0;
	fop.rect.i16Ymax		= rect.height;
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
printf("%008x-%08x-%08x-%08x\n", bmpBuffer[0], bmpBuffer[1], bmpBuffer[2],  bmpBuffer[3]);
}

static void _bmpBlit(int alpha)
{
	int		rval, w, h;

	
	w = lcdGetWidth(); h = lcdGetHeight();
bmpBuffer[0] = bmpBuffer[1] = bmpBuffer[2] = bmpBuffer[3] = 0x00969696;
	bmpBGSBuffer[0] = 0x8090a0f0;
	bmpBGSBuffer[1] = 0x8090a0f0;
	bmpBGSBuffer[2] = 0xff90a0f0;
	bmpBGSBuffer[3] = 0xff90a0f0;
printf("_bmpBlit: w=%d h=%d, alpha=%d\n", w, h, alpha);
	bop.dest.u32FrameBufAddr= (unsigned int)bmpBuffer;
	bop.dest.i32Stride		= w  << 2;
	bop.dest.i32XOffset		= 0;
	bop.dest.i32YOffset		= 0;
 	bop.dest.i16Width		= w;
	bop.dest.i16Height		= h;
	bop.src.pSARGB8			= NULL;
	bop.src.u32PaletteInx	= 0;
	bop.src.u32Num			= 0;
	bop.src.u32SrcImageAddr	= (unsigned int)bmpBGSBuffer;
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
	xform.destFormat		= eDRVBLT_DEST_ARGB8888;
	xform.flags				= 0x06;
	xform.fillStyle			= (E_DRVBLT_FILL_STYLE) (eDRVBLT_NOTSMOOTH | eDRVBLT_CLIP);
	xform.colorMultiplier.i16Blue	= 0;
	xform.colorMultiplier.i16Green	= 0;
	xform.colorMultiplier.i16Red	= 0;
	xform.colorMultiplier.i16Alpha	= alpha;
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
printf("%08x-%08x-%08x-%08x\n", bmpBuffer[0], bmpBuffer[1], bmpBuffer[2],  bmpBuffer[3]);
}

void lcdMove(int x, int y, int width, int height, int dy)
{
#ifndef _HOST_EMU
	int		rval, w, h, y2;

//printf("_lcdMove: %d,%d %d %d dx=%d dy=%d\n", x, y, Width, Height, dx, dy);
	w = width;
	if(dy > 0) {
		h = height - dy; 
		y2 = y;
	} else {
		h = height + dy; y -= dy;
		y2 = y + h;
	}
	bop.dest.u32FrameBufAddr= (unsigned int)lcdGetBuffer(x, y2);
	bop.dest.i32Stride		= lcdGetWidth() << 1;
	bop.dest.i32XOffset		= 0;
	bop.dest.i32YOffset		= 0;
 	bop.dest.i16Width		= w;
	bop.dest.i16Height		= h;
	bop.src.pSARGB8			= NULL;
	bop.src.u32PaletteInx	= 0;
	bop.src.u32Num			= 0;
	bop.src.u32SrcImageAddr	= (unsigned int)lcdGetBuffer(x, y);
	bop.src.i32Stride		= lcdGetWidth() << 1;
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
#endif
}
