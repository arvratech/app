#ifndef _VIEW_PRIM_H_
#define _VIEW_PRIM_H_


#include "UIKit.h"


#define BLACK_BGCOLOR		ARGB(255,22,22,24)
#define WHITE_BGCOLOR		ARGB(255,255,255,255)	
#define WHITE_TEXT_COLOR	ARGB(255,88,89,90)
#define BLACK_TEXT_COLOR	ARGB(255,220,221,223)	// Gray Fill
#define GREEN_FILL			ARGB(255,191,214,48)
#define GREEN_FRAME			ARGB(255,77,183,72)
#define BLUE_FILL			ARGB(255,108,207,246)
#define BLUE_FRAME			ARGB(255,0,173,239)
#define BLUE_FILL2			ARGB(255,0,154,218)
#define YELLOW_FILL			ARGB(255,254,242,0)
#define YELLOW_FRAME		ARGB(255,253,184,19)
#define ORANGE_FILL			ARGB(255,246,129,33)
#define ORANGE_FRAME		ARGB(255,237,27,36)
#define ORANGE_FRAME2		ARGB(255,238,63,36)
#define GRAY_FILL			ARGB(255,220,221,223)
#define GRAY_FRAME			ARGB(255,148,149,153)
#define GRAY_BRIGHT			ARGB(255,200,201,202)
#define GRAY_MIDDLE			ARGB(255,129,130,134)
#define GRAY_DARK			ARGB(255,92,93,95)

#define HOR_MARGIN			40
#define TOP_MARGIN			20
#define BOTTOM_MARGIN		20
#define LOWER_BTN_SZ		64


// Keypad data 
typedef struct _KEYPAD_CONTEXT {
	void	*view;
	void	*label;
	void	*memBuffer;
	int		size;
} KEYPAD_CONTEXT;


void ViewInitBuffer(void);
void *ViewAllocBuffer(void);
void ViewFreeBuffer(void *buffer);
void PrintBuffer(char *msg);
void *ViewAddChild(void *view, int tag, int x, int y, int width, int height);
void *ViewAddScrollView(void *view, int tag, int x, int y, int width, int height);
void *ViewAddLabel(void *view, int tag, char *text, int x, int y, int width, int height, UITextAlignment textAlignment);
void *ViewAddTextField(void *view, int tag, char *text, int x, int y, int width, int height);
void *ViewAddImageViewBitmap(void *view, int tag, char *fileName, int x, int y, int width, int height);
void *ViewAddImageViewMemBitmap(void *view, int tag, void *bmp, int x, int y, int width, int height);
void *ViewAddImageViewJpeg(void *view, int tag, char *fileName, int x, int y, int width, int height);
void *ViewAddImageViewMemJpeg(void *view, int tag, void *jpegBuffer, int jpegSize, int x, int y, int width, int height);
void *ViewAddImageViewAlphaMask(void *view, int tag, char *fileName, int x, int y, int width, int height);
void *ViewChangeImageViewAlphaMask(void *view, int tag, char *fileName);
void *ViewAddImageViewIcon(void *view, int tag, char *fileName, int x, int y, int iconSize);
void *ViewAddTextButton(void *view, int tag, char *text, int x, int y, int width, int height);
void *ViewAddTextButtonAlign(void *view, int tag, char *text, int x, int y, int height, int alignment);
void *ViewAddButtonIcon(void *view, int tag, char *fileName, int x, int y, int iconSize);
void *ViewAddButtonAlphaMask(void *view, int tag, char *fileName, int x, int y, int width, int height, int imgSize);
void *ViewAddButtonAlphaMask2(void *view, int tag, char *fileName, int x, int y, int width, int height);
void *ViewAddButtonIconMask(void *view, int tag, char *fileName, int x, int y, int iconSize, int delta);
void *ViewAddButtonIconText(void *view, int tag, char *fileName, char *text, int x, int y, int width, int height);
void *ViewAddButtonBitmap(void *view, int tag, int x, int y, int width, int height, char *fileName, char *highlightedFileName);
void *ViewAddSwitch(void *view, int x, int y);
void *CreateCellAccessorySwitch(BOOL on);
void *ViewAddSlider(void *view, int x, int y, int width, int value, int minValue, int maxValue);
void *CreateCellSlider(int value, int minValue, int maxValue);
void *CreateStepper(int x, int y);
void *ViewAddStepper(void *view, int x, int y);
void *CellCreateAccessoryStepper(void *cell, int tag, int value, int minValue, int maxValue, int stepValue);
void *CreateCellAccessoryStepper(int value, int minValue, int maxValue, int stepValue);
void *ViewAddPickerView(void *view, int x, int y, int width, int height);
void *ViewAddDatePicker(void *view, int x, int y);
void *ViewAddTimePicker(void *view, int x, int y);
void *ViewAddTimeRange(void *view, int y);

void *ViewAddTableView(void *view, int x, int y, int width, int height);
void *ViewAddTableViewCell(UITableViewCellStyle cellStyle, int reuseIdentifier);
void *ImageCreate(char *fileName, int imageType);
void *ImageIconCreate(char *fileName, int iconSize, int imageType);
char *CellTitle(void *cell);
void CellSetTitle(void *cell, char *text);
char *CellSubtitle(void *cell);
void CellSetSubtitle(void *cell, char *text);
void CellSetTitleSubtitle(void *cell, char *title, char *subtitle);
void CellSetImageAlphaMask(void *cell, char *imageFile);
int  ViewConfirmButtonHeight(void);
void *ViewAddOkButton(void *view, int y);
void *ViewAddCancelButton(void *view, int y);
void ViewAddCancelDoneButton(void *view, int y);
void ViewAddNoYesButton(void *view, int y);
void *ViewAddTitleImageTextButton(void *view, int tag, int x, int y, char *fileName, char *text);
void *ViewAddTitleCancelOkButton(void *view, char *okTitle);
void LabelSetChar(void *view, int ch);
void LabelSetHiddenText(void *view, char *text);
void LabelSetHiddenTextExceptLast(void *view, char *text);

void *ViewAddClock(void *view, int viewTag, int baseTag, int y);
void ClockViewResize(void *view, int baseTag);
void ClockViewUpdate(void *view, int baseTag);
void ClockViewUpdateColon(void *view, int baseTag, int flag);
void ViewAddKeypad(KEYPAD_CONTEXT *kpd, void *view, int type); 
void ViewAddImageKeypad(KEYPAD_CONTEXT *kpd, void *view);
void ViewRemoveKeypad(KEYPAD_CONTEXT *kpd);
void KeypadChangeQwerty(KEYPAD_CONTEXT *kpd, int mode); 
void KeypadChangeNumeric(KEYPAD_CONTEXT *kpd);
void KeypadChangeHexaNumeric(KEYPAD_CONTEXT *kpd);
void KeypadChangeDigit(KEYPAD_CONTEXT *kpd);
void KeypadChangeImageDigit(KEYPAD_CONTEXT *kpd);
void KeypadChangeImageDigit2(KEYPAD_CONTEXT *kpd);
int  KeypadQwertyCode(void *view, int mode);
int  KeypadNumericCode(void *view);
int  KeypadHexaNumericCode(void *view);
void KeypadSetNumericCode(void *view, int c);


#endif

