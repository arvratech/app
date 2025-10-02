#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "UIImage.h"
#include "uiimage_.h"


static void UIImageClass_izer(void *self, va_list *app)
{
	struct UIImageClass *cls = self;

//printf("UIImageClass_izer...\n");
	super_izer(UIImageClass, cls, app);
}

static void UIImage_izer(void *self, va_list *app)
{
	struct UIImage	*_self = self;
	char	*fileName;

//printf("UIImage_izer...\n");
	super_izer(UIImage, _self, app);
	_self->imageType	= va_arg(*app, int);
	fileName = va_arg(*app, char *);
	if(fileName && fileName[0]) strcpy(_self->fileName, fileName);
	else	_self->fileName[0] = 0;
	_self->mem			= NULL;
	_self->iconSize		= 0;
	_self->memSize		= 0;
}

struct NSClass			_UIImageClass;
struct UIImageClass		_UIImage;

void *UIImageClass, *UIImage;

void initUIImage(void)
{
	struct	NSObject	*obj;

	if(!UIImageClass) {
		UIImageClass = &_UIImageClass;
		alloc(NSClass, UIImageClass);
		init(UIImageClass, "UIImageClass", NSClass, sizeof(struct UIImageClass),
		izer, UIImageClass_izer,
		0);
	}
	if(!UIImage) {
		UIImage= &_UIImage;
		alloc(UIImageClass, UIImage);
		init(UIImage, "UIImage", NSObject, sizeof(struct UIImage),
		izer, UIImage_izer,
		0);
	}
}

void UIinitWithFile(void *self, char *fileName, int imageType)
{
	struct UIImage	*_self = self;

	_self->imageType	= imageType;
	if(fileName && fileName[0]) strcpy(_self->fileName, fileName);
	else	_self->fileName[0] = 0;
	_self->mem			= NULL;
	_self->iconSize		= 0;
	_self->memSize		= 0;
}

void UIinitWithMem(void *self, void *mem, int size, int imageType)
{
	struct UIImage	*_self = self;

	_self->imageType	= imageType;
	_self->fileName[0]	= 0;;
	_self->mem			= mem;
	_self->memSize		= size;
}

int UIimageType(void *self)
{
	struct UIImage	*_self = self;

	return _self->imageType;
}

void UIimageSize(void *self, CGSize *size)
{
}

UIImageRenderingMode UIrendeingMode(void *self)
{
	struct UIImage	*_self = self;

	return (UIImageRenderingMode)_self->renderingMode;
}

void UIsetRenderingMode(void *self, UIImageRenderingMode renderingMode)
{
	struct UIImage	*_self = self;

	_self->renderingMode = renderingMode;
}

int UIiconSize(void *self)
{
	struct UIImage	*_self = self;

	return (int)_self->iconSize;
}

void UIsetIconSize(void *self, int iconSize)
{
	struct UIImage	*_self = self;

	_self->iconSize = iconSize;
}

