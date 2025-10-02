#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "ctfont.h"
#include "lcdc.h"
#include "msg.h"
#include "bmpc.h"
#include "lang.h"
#include "syscfg.h"
#include "pref.h"
#include "viewprim.h"


//static unsigned long _memBuffer[r84000];	// 32KBytes
static unsigned long _memBuffer[24000];		// 96Kbytes
static unsigned char *memBuffer;


void ViewInitBuffer(void)
{
	memBuffer = (unsigned char *)_memBuffer;
}

void *ViewAllocBuffer(void)
{
	return (void *)memBuffer;
}

void ViewFreeBuffer(void *buffer)
{
	memBuffer = buffer;
}

void PrintBuffer(char *msg)
{
	printf("%s: buffer=%x offset=%d\n", msg, memBuffer, memBuffer - (unsigned char *)_memBuffer);
}

void *ViewAddChild(void *view, int tag, int x, int y, int width, int height)
{
	void 	*v;
	int		size;
	
	v = (void *)memBuffer;
	size = alloc(UIView, v);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(v, x, y, width, height);
	UIsetTag(v, tag);
	UIsetOpaque(v, FALSE); 
	UIaddChild(view, v);
	return v;
}

void *ViewAddScrollView(void *view, int tag, int x, int y, int width, int height)
{
	void 	*v;
	int		size;
	
	v = (void *)memBuffer;
	size = alloc(UIScrollView, v);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(v, x, y, width, height);
	UIsetTag(v, tag);
	UIsetOpaque(v, FALSE); 
	UIaddChild(view, v);
	return v;
}

void *ViewAddLabel(void *view, int tag, char *text, int x, int y, int width, int height, UITextAlignment textAlignment)
{
	void 	*lbl;
	int		size;
	
	lbl = (void *)memBuffer;
	size = alloc(UILabel, lbl);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(lbl, x, y, width, height);
	UIsetTag(lbl, tag);
	UIsetOpaque(lbl, FALSE);
	UIsetText(lbl, text);
	UIsetTextAlignment(lbl, textAlignment);
	UIaddChild(view, (void *)lbl);
	return lbl;
}

void *ViewAddTextField(void *view, int tag, char *text, int x, int y, int width, int height)
{
	void 	*txtfld;
	int		size;
	
	txtfld = (void *)memBuffer;
	size = alloc(UITextField, txtfld);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(txtfld, x, y, width, height);
	UIsetTag(txtfld, tag);
	if(text && text[0]) UIsetText(txtfld, text);
	UIsetBackgroundColor(txtfld, TABLE_GREY_50);
	UIsetTextColor(txtfld, blackColor);
	UIsetOpaque(txtfld, TRUE);
	UIaddChild(view, txtfld);
	return txtfld;
}

void *ViewAddImageViewBitmap(void *view, int tag, char *fileName, int x, int y, int width, int height)
{
	void	*img, *imgv;
	int		size;

	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	imgv = (void *)memBuffer;
	size = alloc(UIImageView, imgv);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	UIinitWithFile(img, fileName, IMAGE_BITMAP);
	initWithFrame(imgv, x, y, width, height);
	UIsetTag(imgv, tag);
	UIsetImage(imgv, img);
	UIaddChild(view, imgv);
	return imgv;
}

void *ViewAddImageViewMemBitmap(void *view, int tag, void *bmp, int x, int y, int width, int height)
{
	void	*img, *imgv;
	int		size;

	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	imgv = (void *)memBuffer;
	size = alloc(UIImageView, imgv);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	UIinitWithMem(img, bmp, 0, IMAGE_BITMAP);
	initWithFrame(imgv, x, y, width, height);
	UIsetTag(imgv, tag);
	UIsetImage(imgv, img);
	UIaddChild(view, imgv);
	return imgv;
}

void *ViewAddImageViewJpeg(void *view, int tag, char *fileName, int x, int y, int width, int height)
{
	void	*img, *imgv;
	int		size;

	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	imgv = (void *)memBuffer;
	size = alloc(UIImageView, imgv);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	UIinitWithFile(img, fileName, IMAGE_JPEG);
	initWithFrame(imgv, x, y, width, height);
	UIsetImage(imgv, img);
	UIsetTag(imgv, tag);
	UIaddChild(view, imgv);
	return imgv;
}

void *ViewAddImageViewMemJpeg(void *view, int tag, void *jpegBuffer, int jpegSize, int x, int y, int width, int height)
{
	void	*img, *imgv;
	int		size;

	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	imgv = (void *)memBuffer;
	size = alloc(UIImageView, imgv);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	UIinitWithMem(img, jpegBuffer, jpegSize, IMAGE_JPEG);
	initWithFrame(imgv, x, y, width, height);
	UIsetTag(imgv, tag);
	UIsetImage(imgv, img);
	UIaddChild(view, imgv);
	return imgv;
}

void *ViewAddImageViewAlphaMask(void *view, int tag, char *fileName, int x, int y, int width, int height)
{
	void	*img, *imgv;
	int		size;

	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	imgv = (void *)memBuffer;
	size = alloc(UIImageView, imgv);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	UIinitWithFile(img, fileName, IMAGE_ALPHAMASK);
	initWithFrame(imgv, x, y, width, height);
	UIsetTag(imgv, tag);
	UIsetOpaque(imgv, FALSE);
	UIsetImage(imgv, img);
	UIaddChild(view, imgv);
	return imgv;
}

void *ViewChangeImageViewAlphaMask(void *view, int tag, char *fileName)
{
	void	*img, *imgv;
	
	imgv = UIviewWithTag(view, tag);
	if(imgv) {
		img = UIimage(imgv);
		UIinitWithFile(img, fileName, IMAGE_ALPHAMASK);
		UIsetNeedsDisplay(imgv);
	}
	return imgv;
}

void *ViewAddImageViewIcon(void *view, int tag, char *fileName, int x, int y, int iconSize)
{
	void	*img, *imgv;
	int		size;

	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	imgv = (void *)memBuffer;
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	size = alloc(UIImageView, imgv);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	UIinitWithFile(img, fileName, IMAGE_ICON);
	UIsetIconSize(img, iconSize);
	initWithFrame(imgv, x, y, iconSize, iconSize);
	UIsetTag(imgv, tag);
	UIsetOpaque(imgv, FALSE);
	UIsetImage(imgv, img);
	UIaddChild(view, imgv);
	return imgv;
}

void *ViewAddTextButton(void *view, int tag, char *text, int x, int y, int width, int height)
{
	void	*btn;
	int		size;

	btn = (void *)memBuffer;
	size = alloc(UIButton, btn);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(btn, x, y, width, height);
	UIsetTag(btn, tag);
	UIsetOpaque(btn, TRUE);
	UIsetBackgroundColor(btn, UIbackgroundColor(view));
	UIsetTitle(btn, text);
	UIaddChild(view, btn);
	return btn;
}

void *ViewAddTextButtonAlign(void *view, int tag, char *text, int x, int y, int height, int alignment)
{
	void	*btn, *v;
	int		w, size;

	btn = (void *)memBuffer;
	size = alloc(UIButton, btn);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	v = UItitleLabel(btn);
	ftSetSize(UIfont(v));
	w = ftTextWidth(text) + 32;
	if(alignment) x -= w;
	initWithFrame(btn, x, y, w, height);
	UIsetTag(btn, tag);
	UIsetTitle(btn, text);
	UIaddChild(view, btn);
	return btn;
}

void *ViewAddButtonAlphaMask(void *view, int tag, char *fileName, int x, int y, int width, int height, int imgSize)
{
	void	*btn, *v, *img;
	CGRect	rt;
	int		size;

	btn = (void *)memBuffer;
	size = alloc(UIButton, btn);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(btn, x, y, width, height);
	UIsetTag(btn, tag);
	UIinitWithFile(img, fileName, IMAGE_ALPHAMASK);
	UIsetButtonImage(btn, img);
	v = UIimageView(btn);
	rt.x = (width - imgSize) >> 1; rt.y = (height - imgSize) >> 1;
	rt.width = rt.height = imgSize;
	UIsetFrame(v, &rt);
	UIaddChild(view, btn);
	return btn;
}

void *ViewAddButtonAlphaMask2(void *view, int tag, char *fileName, int x, int y, int width, int height)
{
	void	*btn, *v, *img;
	CGRect	rt;
	int		size;

	btn = (void *)memBuffer;
	size = alloc(UIButton, btn);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(btn, x, y, width, height);
	UIsetTag(btn, tag);
	UIinitWithFile(img, fileName, IMAGE_ALPHAMASK);
	UIsetButtonImage(btn, img);
	v = UIimageView(btn);
	rt.x = 0; rt.y = 0; rt.width = width; rt.height = height;
	UIsetFrame(v, &rt);
	UIaddChild(view, btn);
	return btn;
}

void *ViewAddButtonAlphaMaskText(void *view, int tag, int x, int y, char *fileName, char *text)
{
	void	*btn, *img, *lbl, *imgv;
	CGRect	rect;
	int		w, h, iw, id, size;

	btn = (void *)memBuffer;
	size = alloc(UIButton, btn);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	ftSetSize(22);
	iw = 40; id = 8;
	w = ftTextWidth(text);
	initWithFrame(btn, x, y, id+iw+id+w+id+id, iw+id);
	UIsetTag(btn, tag);
	UIsetOpaque(btn, TRUE);
	UIsetBackgroundColor(btn, UIbackgroundColor(view));
	UIsetTitle(btn, text);
	UIsetTitleColor(btn, enableWhiteColor);
	lbl = UItitleLabel(btn);
	//UIsetFont(lbl, FONT_TYPE_28);
	rect.x = id+iw+id; rect.y = 0; rect.width = w + id+id; rect.height = iw+id;
	UIsetFrame(lbl, &rect);
	UIinitWithFile(img, fileName, IMAGE_ALPHAMASK);
	UIsetButtonImage(btn, img);
	imgv = UIimageView(btn);
	rect.x = id; rect.y = id>>1; rect.width = rect.height = iw;
	UIsetFrame(imgv, &rect);
	UIsetTintColor(imgv, enableWhiteColor);

	UIaddChild(view, btn);
	return btn;
}

void *ViewAddButtonIcon(void *view, int tag, char *fileName, int x, int y, int iconSize)
{
	void	*btn, *img;
	int		size;

	btn = (void *)memBuffer;
	size = alloc(UIButton, btn);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(btn, x, y, iconSize, iconSize);
	UIsetTag(btn, tag);
	UIinitWithFile(img, fileName, IMAGE_ICON);
	UIsetIconSize(img, iconSize);
	UIsetButtonImage(btn, img);
	UIaddChild(view, btn);
	return btn;
}

void *ViewAddButtonIconMask(void *view, int tag, char *fileName, int x, int y, int iconSize, int delta)
{
	void	*btn, *img;
	int		size;

	btn = (void *)memBuffer;
	size = alloc(UIButton, btn);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(btn, x, y, iconSize+delta+delta, iconSize+delta+delta);
	UIsetTag(btn, tag);
	UIinitWithFile(img, fileName, IMAGE_ALPHAMASK);
	UIsetIconSize(img, iconSize);
	UIsetButtonImage(btn, img);
	UIaddChild(view, btn);
	return btn;
}

void *ViewAddButtonIconText(void *view, int tag, char *fileName, char *text, int x, int y, int width, int height)
{
	void	*btn, *img;
	int		size;

	btn = (void *)memBuffer;
	size = alloc(UIButton, btn);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(btn, x, y, width, height);
	UIsetTag(btn, tag);
	UIinitWithFile(img, fileName, IMAGE_ICON);
	UIsetIconSize(img, height);
	UIsetButtonImage(btn, img);
	UIsetTitle(btn, text);
	UIaddChild(view, btn);
	return btn;
}

void *ViewAddButtonBitmap(void *view, int tag, int x, int y, int width, int height, char *fileName, char *highlightedFileName)
{
	void	*btn, *img, *img1;
	int		size;

	btn = (void *)memBuffer;
	size = alloc(UIButton, btn);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	img1 = (void *)memBuffer;
	size = alloc(UIImage, img1);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;		// bug fix: inserted 2022.4.11
	initWithFrame(btn, x, y, width, height);
	UIsetTag(btn, tag);
	UIinitWithFile(img, fileName, IMAGE_BITMAP);
	UIsetButtonImage(btn, img);
	UIinitWithFile(img1, highlightedFileName, IMAGE_BITMAP);
	UIsetButtonHighlightedImage(btn, img1);
	UIaddChild(view, btn);
	return btn;
}

void *ViewAddSwitch(void *view, int x, int y)
{
	void	*sw;
	int		size;

	sw = (void *)memBuffer;
	size = alloc(UISwitch, sw);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(sw, x, y, 0, 0);
	UIaddChild(view, sw);
	return sw;
}

void *CreateCellAccessorySwitch(BOOL on)
{
	void	*v;
	int		size;

	v = (void *)memBuffer;
	size = alloc(UISwitch, v);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(v, 0, 0, 0, 0);
	UIsetOn(v, on);
	return v;
}

void *ViewAddSlider(void *view, int x, int y, int width, int value, int minValue, int maxValue)
{
	void	*v;
	int		size;

	v = (void *)memBuffer;
	size = alloc(UISlider, v);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
//	initWithFrame(v, 40, y, rect.width-56, 32);
printf("ViewAddSlider\n");
	initWithFrame(v, x, y, width, 0);
	UIsetValue(v, value);
	UIsetMinimumValue(v, minValue);
	UIsetMaximumValue(v, maxValue);
	UIsetMinimumTrackTintColor(v, blueColor);
	UIsetMaximumTrackTintColor(v, grayColor);
	UIsetThumbTintColor(v, blueColor);
	UIaddChild(view, v);
	return v;
}

void *CreateCellSlider(int value, int minValue, int maxValue)
{
	void	*v;
	int		size;

	v = (void *)memBuffer;
	size = alloc(UISlider, v);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(v, 0, 0, 0, 0);
	UIsetValue(v, value);
	UIsetMinimumValue(v, minValue);
	UIsetMaximumValue(v, maxValue);
	UIsetMinimumTrackTintColor(v, blueColor);
	UIsetMaximumTrackTintColor(v, grayColor);
	UIsetThumbTintColor(v, blueColor);
	return v;
}

void *CreateStepper(int x, int y)
{
	void	*v;
	int		size;

	v = (void *)memBuffer;
	size = alloc(UIStepper, v);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(v, x, y, 0, 0);
	UIsetWraps(v, TRUE);
	UIsetTintColor(v, blueColor);
	return v;
}

void *ViewAddStepper(void *view, int x, int y)
{
	void	*v;

	v = CreateStepper(x, y);
	UIaddChild(view, v);
	return v;
}

void *CellCreateAccessoryStepper(void *cell, int tag, int value, int minValue, int maxValue, int stepValue)
{
	void	*v;

	v = CreateStepper(0, 0);
	UIsetValue(v, value);
	UIsetMinimumValue(v, minValue);
	UIsetMaximumValue(v, maxValue);
	UIsetStepValue(v, stepValue);
	UIsetTag(v, tag);
	UIsetAccessoryView(cell, v);
	return v;
}

void *CreateCellAccessoryStepper(int value, int minValue, int maxValue, int stepValue)
{
	void	*v;

	v = CreateStepper(0, 0);
	UIsetValue(v, value);
	UIsetMinimumValue(v, minValue);
	UIsetMaximumValue(v, maxValue);
	UIsetStepValue(v, stepValue);
	return v;
}

void *ViewAddPickerView(void *view, int x, int y, int width, int height)
{
	void	*v;
	int		size;

	v = (void *)memBuffer;
	size = alloc(UIPickerView, v);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(v, x, y, width, height);
//	UIsetTintColor(v, grayColor);
	UIsetTintColor(v, blackColor);
	UIaddChild(view, v);
	return v;
}

void *ViewAddDatePicker(void *view, int x, int y)
{
	void	*v;
	int		size;

	v = (void *)memBuffer;
	size = alloc(UIDatePicker, v);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(v, x, y, 0, 0);
	UIsetTintColor(v, blackColor);
	UIaddChild(view, v);
	UIsetLocale(v, GetLanguage());
//UIsetLocale(v, LANG_ENG);
	return v;
}

void *ViewAddTimePicker(void *view, int x, int y)
{
	void	*v;
	int		size;

	v = (void *)memBuffer;
	size = alloc(UITimePicker, v);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(v, x, y, 0, 0);
	UIsetTintColor(v, blackColor);
	UIaddChild(view, v);
	UIsetLocale(v, GetLanguage());
	return v;
}

/*
void *ViewAddTimeRange(void *view, int y)
{
	void	*v;
	int		size;

	v = (void *)memBuffer;
	size = alloc(UITimeRange, v);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(v, 0, y, 0, 0);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, whiteColor);
	UIsetTintColor(v, blackColor);
	UIaddChild(view, v);
	return v;
}
*/

void *ViewAddTableView(void *view, int x, int y, int width, int height)
{
	void	*tblv;
	int		size;

	tblv = (void *)memBuffer;
	size = alloc(UITableView, tblv);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithFrame(tblv, x, y, width, height);
	UIsetAllowsMultipleSelection(tblv, TRUE);
	UIaddChild(view, tblv);
	return tblv;
}

void *ViewAddTableViewCell(UITableViewCellStyle cellStyle, int reuseIdentifier)
{
	void	*tvc;
	int		size;

	tvc = (void *)memBuffer;
	size = alloc(UITableViewCell, tvc);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	initWithStyle(tvc, cellStyle, reuseIdentifier);
	return tvc;
}

void *ImageCreate(char *fileName, int imageType)
{
	void	*img;
	int		size;

	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	UIinitWithFile(img, fileName, imageType);
	return img;
}

void *ImageIconCreate(char *fileName, int iconSize, int imageType)
{
	void	*img;
	int		size;

	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	UIinitWithFile(img, fileName, imageType);
	UIsetIconSize(img, iconSize);
	return img;
}

char *CellTitle(void *cell)
{
	void	*lbl;

	lbl = UItextLabel(cell);
	return UItext(lbl);
}

void CellSetTitle(void *cell, char *text)
{
	void	*lbl;

	lbl = UItextLabel(cell);
	UIsetText(lbl, text);
	UIsetNeedsDisplay(cell);
}

char *CellSubtitle(void *cell)
{
	void	*lbl;

	lbl = UIdetailTextLabel(cell);
	return UItext(lbl);
}

void CellSetSubtitle(void *cell, char *text)
{
	void	*lbl;
	CGRect	rt;

	lbl = UIdetailTextLabel(cell);
	UIframe(lbl, &rt);
	UIsetText(lbl, text);
	//UIsetNeedsDisplayInRect(cell, &rt);
	UIsetNeedsDisplay(cell);
}

void CellSetTitleSubtitle(void *cell, char *title, char *subtitle)
{
	void	*lbl;

	lbl = UItextLabel(cell);
	UIsetText(lbl, title);
	lbl = UIdetailTextLabel(cell);
	UIsetText(lbl, subtitle);
	UIsetNeedsDisplay(cell);
}

void CellSetImageAlphaMask(void *cell, char *imageFile)
{
	void	*img, *imgv;

	img	= ImageCreate(imageFile, IMAGE_ALPHAMASK);
	imgv = UIimageView(cell);
	UIsetImage(imgv, img);
	UIsetNeedsDisplay(cell);
}

int ViewConfirmButtonHeight(void)
{
	return 32;
}

void *ViewAddOkButton(void *view, int y)
{
	void	*btn;
	CGRect	rect;
	int		x, w, h, d;

	UIframe(view, &rect);
	ftSetSize(22);
//	h = 32; d = 8;
	h = 40; d = 8;
	w = ftTextWidth(xmenu_okcancel[0]);
	x = rect.width - w - h - d;
	btn = ViewAddTextButton(view, TAG_OK_BTN, xmenu_okcancel[0], x, y, w+h, h);
	UIsetTitleColor(btn, TABLE_CYAN_700);
	return btn;
}

void *ViewAddCancelButton(void *view, int y)
{
	void	*btn;
	CGRect	rect;
	int		x, w, h, d;

	UIframe(view, &rect);
	ftSetSize(22);
//	h = 32; d = 8;
	h = 40; d = 8;
	w = ftTextWidth(xmenu_okcancel[1]);
	x = rect.width - w - h - d;
	btn = ViewAddTextButton(view, TAG_CANCEL_BTN, xmenu_okcancel[1], x, y, w+h, h);
	UIsetTitleColor(btn, TABLE_CYAN_700);
	return btn;
}

void ViewAddCancelDoneButton(void *view, int y)
{
	void	*btn;
	CGRect	rect;
	int		x, w1, w2, h, d, fh;

	UIframe(view, &rect);
//	h = 32; d = 8;
	h = 40; d = 8;
	ftSetSize(22);
	w1 = ftTextWidth(xmenu_okcancel[1]);
	w2 = ftTextWidth(xmenu_okcancel[2]);
	x = rect.width - w1 - h - w2 - h - d;
	btn = ViewAddTextButton(view, TAG_CANCEL_BTN, xmenu_okcancel[1], x, y, w1+h, h);
	UIsetTitleColor(btn, TABLE_CYAN_700);
	x += w1 + h;
	btn = ViewAddTextButton(view, TAG_OK_BTN, xmenu_okcancel[2], x, y, w2+h, h);
	UIsetTitleColor(btn, TABLE_CYAN_700);
}

void ViewAddNoYesButton(void *view, int y)
{
	void	*btn;
	CGRect	rect;
	int		x, w1, w2, h, d;

	UIframe(view, &rect);
//	h = 32; d = 8;
	ftSetSize(22);
	h = 40; d = 8;
	w1 = ftTextWidth(xmenu_okcancel[1]);
	w2 = ftTextWidth(xmenu_okcancel[0]);
	x = rect.width - w1 - h - w2 - h - d;
	btn = ViewAddTextButton(view, TAG_CANCEL_BTN, xmenu_yesno[1], x, y, w1+h, h);
	UIsetTitleColor(btn, TABLE_CYAN_700);
	x += w1 + h;
	btn = ViewAddTextButton(view, TAG_OK_BTN, xmenu_yesno[0], x, y, w2+h, h);
	UIsetTitleColor(btn, TABLE_CYAN_700);
}

void *ViewAddTitleImageTextButton(void *view, int tag, int x, int y, char *fileName, char *text)
{
	void	*btn, *img, *v;
	CGRect	rt;
	int		w, h, fh, size;

	btn = (void *)memBuffer;
	size = alloc(UIButton, btn);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	rt.x = 12; rt.y = 8; rt.width = rt.height = 32;
	ftSetSize(22);
	w = ftTextWidth(text);
	fh = ftHeight();
	h = rt.y + rt.height + rt.y;
	initWithFrame(btn, x, y, rt.x+rt.width+rt.x+w+rt.x, h);
	UIsetTag(btn, tag);
	UIsetOpaque(btn, TRUE);
	UIsetBackgroundColor(btn, UIbackgroundColor(view));

	UIinitWithFile(img, fileName, IMAGE_ALPHAMASK);
	UIsetButtonImage(btn, img);
	v = UIimageView(btn);
	UIsetFrame(v, &rt);
	UIsetTintColor(v, enableWhiteColor);
	UIsetTitle(btn, text);
	UIsetTitleColor(btn, enableWhiteColor);
	v = UItitleLabel(btn);
	rt.x = rt.x + rt.width + rt.x;
	rt.y = (h - fh) >> 1;
	rt.width = w; rt.height = fh;
	UIsetFrame(v, &rt);
	UIaddChild(view, btn);
	return btn;
}

void *ViewAddTitleImageTextButton2(void *view, int tag, int x, int y, char *fileName, char *text)
{
	void	*btn, *img, *v;
	CGRect	rt;
	int		w, h, fh, size;

	btn = (void *)memBuffer;
	size = alloc(UIButton, btn);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	img = (void *)memBuffer;
	size = alloc(UIImage, img);
	if(size & 3) size = ((size >> 2) + 1) << 2;	// 4's multiply
	memBuffer += size;
	rt.x = 12; rt.y = 8; rt.width = rt.height = 45;
	ftSetSize(31);
	w = ftTextWidth(text);
	fh = ftHeight();
	h = rt.y + rt.height + rt.y;
	initWithFrame(btn, x, y, rt.x+rt.width+rt.x+w+rt.x, h);
	UIsetTag(btn, tag);
	UIsetOpaque(btn, TRUE);
	UIsetBackgroundColor(btn, UIbackgroundColor(view));

	UIinitWithFile(img, fileName, IMAGE_ALPHAMASK);
	UIsetButtonImage(btn, img);
	v = UIimageView(btn);
	UIsetFrame(v, &rt);
	UIsetTintColor(v, enableWhiteColor);
	UIsetTitle(btn, text);
	UIsetTitleColor(btn, enableWhiteColor);
	v = UItitleLabel(btn);
	rt.x = rt.x + rt.width + rt.x;
	rt.y = (h - fh) >> 1;
	rt.width = w; rt.height = fh;
	UIsetFrame(v, &rt);
	UIaddChild(view, btn);
	return btn;
}

void *ViewAddTitleCancelOkButton(void *view, char *okTitle)
{
	void	*v;
	CGRect	rt;
	int		x, y, w, h;

	UIframe(view, &rt);
	x = y = 0; w = rt.width >> 1; h = 48;
	v = ViewAddTextButton(view, TAG_CANCEL_BTN, (char *)admCancelTitle(), x, y, w, h);
	UIsetTitleColor(v, enableWhiteColor);
	x += w;
	v = ViewAddTextButton(view, TAG_OK_BTN, okTitle, x, y, w, h);
	UIsetTitleColor(v, enableWhiteColor);
}

void LabelSetChar(void *view, int ch)
{
	char	temp[4];

	temp[0] = ch; temp[1] = 0;
	UIsetText(view, temp);
}

void LabelSetHiddenText(void *view, char *text)
{
	char	temp[64];

	plain2hidden(text, temp);
	UIsetText(view, temp);
}

void LabelSetHiddenTextExceptLast(void *view, char *text)
{
	char	temp[64];
	int		c, len;

	len = strlen(text);
	c = text[len-1];
	plain2hidden(text, temp);
	temp[len-1] = c;
	UIsetText(view, temp);
}

#include "view_clock.c"
#include "view_kpd.c"
#include "view_kpdimg.c"

