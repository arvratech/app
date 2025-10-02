#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "UIButton.h"
#include "uiview_.h"
#include "UIImage.h"
#include "UIImageView.h"
#include "uiimage_.h"
#include "uiimageview_.h"


static void UIImageViewClass_izer(void *self, va_list *app)
{
	struct UIImageViewClass *cls = self;

//printf("UIImageViewClass_izer...\n");
	super_izer(UIImageViewClass, cls, app);
}

static void UIImageView_izerwf(void * self, va_list *app)
{
	struct UIImageView	*_self = self;

	super_izerwf(UIImageView, _self, app);
	UIsetUserInteractionEnabled(_self, FALSE);
	_self->state			= 0;
	_self->image			= NULL;
	_self->highlightedImage	= NULL;
	_self->rotate			= 0;
}

static void UIImageView_draw(void *self, CGRect *rect)
{
	struct UIImageView *_self = self;
	struct UIImage		*img;
	BMPC		*bmp, _bmp;
	CGRect		rt;
	CGPoint		pt;
	UIViewContentMode contentMode;
	UIColor		color;
	void		*v;
	int		x, y, w, h, imgType;

//printf("ImageView Draw: rect=%d,%d %d %d\n", rect->x, rect->y, rect->width, rect->height);
	super_UIdraw(UIImageView, self, rect);
	UIframe(_self, &rt);
	pt.x = pt.y = 0;
	UIconvertPointToScreen(_self, &pt, (CGPoint *)&rt);
	lcdPixelClipToRect(&rt, rect);
	x = rt.x; y = rt.y; w = rt.width; h = rt.height;
//lcdSetDefaultClipRect();
	v = UIparent(_self);
	if(v && class(v) == UIButton && UIisHighlighted(v) && _self->highlightedImage) {
		img = _self->highlightedImage;
	} else {
		img = _self->image;
	}
	imgType = UIimageType(img);
	contentMode = UIcontentMode(self);
	if(contentMode != UIViewContentModeScaleToFill) w = h = 0;
	if(img->fileName[0]) {
//printf("ImageView Draw: imageType=%d [%s] %d,%d %d %d\n", imgType, img->fileName, x, y, w, h);
		if(imgType == IMAGE_BITMAP) lcdPutBmpFile(x, y, w, h, img->fileName);
		else if(imgType == IMAGE_JPEG) lcdPutJpegFile(x, y, w, h, img->fileName);
		else if(imgType == IMAGE_ALPHAMASK) {
			v = UIparent(self);
			if(v && class(v) == UIButton && !UIisEnabled(v)) color = 0xffa3a3a3;
			else	color = UItintColor(self);	
			lcdPutAlphaMaskFile(x, y, w, h, color, img->fileName);
		}
	} else if(img->mem) {
		if(imgType == IMAGE_BITMAP) {
			//if(_self->rotate) lcdPutRotate(x, y, (BMPC *)img->mem);
			//else	lcdPut(x, y, w, h, (BMPC *)img->mem);
		} else if(imgType == IMAGE_JPEG) lcdPutJpeg(x, y, w, h, img->mem, img->memSize);
	}
}

struct NSClass		_UIImageViewClass;
struct UIImageViewClass	_UIImageView;

void *UIImageViewClass, *UIImageView;

void initUIImageView(void)
{
	if(!UIImageViewClass) {
		UIImageViewClass = &_UIImageViewClass;
		alloc(NSClass, UIImageViewClass);
		init(UIImageViewClass, "UIImageViewClass", UIViewClass, sizeof(struct UIImageViewClass),
		izer, UIImageViewClass_izer,
		0);
	}
	if(!UIImageView) {
		UIImageView = &_UIImageView;
		alloc(UIImageViewClass, UIImageView);
		init(UIImageView, "UIImageView", UIView, sizeof(struct UIImageView),
		izerwf, UIImageView_izerwf, 
		UIdraw, UIImageView_draw,
		0);
	}
}

void *UIimage(void *self)
{
	struct UIImageView	*_self = self;

	return _self->image;
}

void UIsetImage(void *self, void *image)
{
	struct UIImageView	*_self = self;

	_self->image = image;
	UIsetNeedsDisplay(_self);
}

void *UIhighlightedImage(void *self)
{
	struct UIImageView	*_self = self;

	return _self->highlightedImage;
}

void UIsetHighlightedImage(void *self, void *highlightedImage)
{
	struct UIImageView	*_self = self;

	_self->highlightedImage = highlightedImage;
	UIsetNeedsDisplay(_self);
}

void UIsetRotateImage(void *self)
{
	struct UIImageView	*_self = self;

	_self->rotate = 1;
}

