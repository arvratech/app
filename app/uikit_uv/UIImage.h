#ifndef _UIIMAGE_H
#define _UIIMAGE_H


#include "NSObject.h"

// ImageType
#define IMAGE_BITMAP		1
#define IMAGE_ICON			2
#define IMAGE_ALPHAMASK		3
#define IMAGE_PNG			4	
#define IMAGE_JPEG			5


extern void *UIImageClass;	// adds draw 
extern void *UIImage;		// self = alloc(UIView); init(self, x, y, width, height);


void initUIImage(void);

void UIinitWithFile(void *self, char *fileName, int imageType);
void UIinitWithMem(void *self, void *mem, int size, int imageType);
int  UIimageType(void *self);
void UIimageSize(void *self, CGSize *size);
UIImageRenderingMode UIrendeingMode(void *self);
void UIsetRenderingMode(void *self, UIImageRenderingMode renderingMode);
int  UIiconSize(void *self);
void UIsetIconSize(void *self, int iconSize);


#endif
