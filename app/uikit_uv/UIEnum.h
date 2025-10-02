#ifndef _UIENUM_H
#define _UIENUM_H


typedef unsigned long	UIColor;

#define clearColor			0x00000000		// totally transparent
#define blackColor			0xff000000
#define darkGrayColor		0xff555555
#define lightGrayColor		0xffaaaaaa
#define whiteColor			0xffffffff
#define grayColor			0xff808080
#define redColor			0xffff0000
#define greenColor			0xff00ff00
#define blueColor			0xff0000ff
#define cyanColor			0xff00ffff
#define yellowColor			0xffffff00
#define magentaColor		0xffff00ff
#define orangeColor			0xffff8000
#define purpleColor			0xff800080	
#define brownColor			0xff996633


#define systemRed			0xffff3b30		// 255, 59, 48
#define systemOrange		0xffff9500		// 255,149,  0
#define systemYellow		0xffffcc00		// 255,204,  0
#define systemGreen			0xff4cd964		//  76,217,100
#define systemBlue_			0xff34aadc		//  52,170,220
#define systemTealBlue		0xff5ac8fa		//  90,200,250
#define systemBlue			0xff007aff		//   0,122,255
#define systemPurple		0xff5856d6		//  88, 86,214
#define systemPink			0xffff2d55		// 255, 45, 85

#define systemGray			0xff8e8e93		// 142,142,147
#define systemGray1			0xffc7c7ce		// 199,199,206


#define enableWhiteColor	0xd4ffffff		// 212,255,255,255
#define disableWhiteColor	0x40ffffff		//  64,255,255,255


#define TABLE_GREY_50		0xfffafafa
#define TABLE_GREY_100		0xfff5f5f5
#define TABLE_GREY_200		0xffeeeeee
#define TABLE_GREY_300		0xffe0e0e0
#define TABLE_GREY_350		0xffd6d6d6
#define TABLE_GREY_400		0xffbdbdbd
#define TABLE_GREY			0xff9e9e9e
#define TABLE_GREY_600		0xff757575
#define TABLE_GREY_700		0xff616161
#define TABLE_GREY_800		0xff424242
#define TABLE_GREY_850		0xff303030
#define TABLE_GREY_900		0xff212121

#define TABLE_RED_50		0xffffebee
#define TABLE_RED_100		0xffffcdd2
#define TABLE_RED_200		0xffef9a9a
#define TABLE_RED_300		0xffe57373
#define TABLE_RED_400		0xffef5350
#define TABLE_RED			0xfff44336
#define TABLE_RED_600		0xffe53935
#define TABLE_RED_700		0xffd32f2f
#define TABLE_RED_800		0xffc62828
#define TABLE_RED_900		0xffb71c1c

#define TABLE_GREEN_50		0xffe8f5e9
#define TABLE_GREEN_100		0xffc8e6c9
#define TABLE_GREEN_200		0xffa5d6a7
#define TABLE_GREEN_300		0xff81c784
#define TABLE_GREEN_400		0xff66bb6a
#define TABLE_GREEN			0xff4caf50
#define TABLE_GREEN_600		0xff43a047
#define TABLE_GREEN_700		0xff388e3c
#define TABLE_GREEN_800		0xff2e7d32
#define TABLE_GREEN_900		0xff1b5e20

#define TABLE_BLUE_50		0xffe3f2fd
#define TABLE_BLUE_100		0xffbbdefb
#define TABLE_BLUE_200		0xff90caf9
#define TABLE_BLUE_300		0xff64b5f6
#define TABLE_BLUE_400		0xff42a5f5
#define TABLE_BLUE			0xff2196f3
#define TABLE_BLUE_600		0xff1e88e5
#define TABLE_BLUE_700		0xff1976d2
#define TABLE_BLUE_800		0xff1565c0
#define TABLE_BLUE_900		0xff0d47a1

#define TABLE_CYAN_50		0xffe0f7fa
#define TABLE_CYAN_100		0xffb2ebf2
#define TABLE_CYAN_200		0xff80deea
#define TABLE_CYAN_300		0xff4dd0e1
#define TABLE_CYAN_400		0xff26c6da
#define TABLE_CYAN			0xff00bcd4
#define TABLE_CYAN_600		0xff00acc1
#define TABLE_CYAN_700		0xff0097a7
#define TABLE_CYAN_800		0xff00838f
#define TABLE_CYAN_900		0xff006064

#define RGB(r, g, b)		(unsigned long)((r << 16) | (g << 8) | b)
#define ARGB(a, r, g, b)	(unsigned long)((a << 24) | (r << 16) | (g << 8) | b)


typedef enum {
	UIViewTintAdjustmentModeAutomatic, UIViewTintAdjustmentModeNormal, UIViewTintAdjustmentModeDimmed 
} UIViewTintAdjustmentMode;

typedef enum {
	UIViewContentModeScaleToFill, UIViewContentModeScaleAspectFit, UIViewContentModeScaleAspectFill, UIViewContentModeRedraw,
	UIViewContentModeCenter, UIViewContentModeTop, UIViewContentModeBottom, UIViewContentModeLeft, UIViewContentModeRight,
	UIViewContentModeTopLeft, UIViewContentModeTopRight, UIViewContentModeBottomLeft, UIViewContentModeBottomRight
} UIViewContentMode;

typedef enum {
	UIControlStateNormal=0, UIControlStateDisabled=0x01, UIControlStateHighlighted=0x02, UIControlStateSelected=0x04,
	GcontrolStateFocused=0x08
} UIControlState;

typedef enum {
	UIContentVerticalAlignmentCenter, UIContentVerticalAlignmentTop, UIContentVerticalAlignmentBottom,
	UIContentVerticalAlignmentFill
} UIContentVerticalAlignment;

typedef enum {
	UIContentHorizontalAlignmentCenter, UIContentHorizontalAlignmentLeft, UIContentHorizontalAlignmentRight,
	UIContentHorizontalAlignmentFill
} UIContentHorizontalAlignment;

typedef enum {
	UITextAlignmentNatural, UITextAlignmentLeft, UITextAlignmentRight, UITextAlignmentCenter 
} UITextAlignment;

typedef enum {
	UITextBorderStyleNone, UITextBorderLine, UITextBorderBezel, UITextBorderRoundedRect 
} UITextBorderStyle;

typedef enum {
	UITextFieldViewModeNever, UITextFieldViewModeWhileEditing, UITextFieldViewModeUnlessEditing, UITextFieldViewModeAlways 
} UITextFieldViewMode;

typedef enum {
	UIImageRenderingModeAutomatic, UIImageRenderingModeAlwaysOriginal, UIImageRenderingModeAlwaysTemplate
} UIImageRenderingMode;

typedef enum {
	UITableViewCellStyleDefault, UITableViewCellStyleValue1, UITableViewCellStyleValue2, UITableViewCellStyleSubtitle
} UITableViewCellStyle;

typedef enum {
	UITableViewCellSelectionStyleNone, UITableViewCellSelectionStyleBlue, UITableViewCellSelectionStyleGray, 
	UITableViewCellSelectionStyleDefault
} UITableViewCellSelectionStyle;

typedef enum {
	UITableViewCellAccessoryTypeNone, UITableViewCellAccessoryTypeDisclosureIndicator,
	UITableViewCellAccessoryTypeDetailDisclosureButton, UITableViewCellAccessoryTypeDetailButton,
	UITableViewCellAccessoryTypeCheckmark, UITableViewCellAccessoryTypeUncheckmark,
	UITableViewCellAccessoryTypeRadiomark, UITableViewCellAccessoryTypeUnradiomark
} UITableViewCellAccessoryType;

typedef enum {
	UITableViewCellSeperatorStyleNone, UITableViewCellSeperatorStyleSingleLine, UITableViewCellSeperatorStyleSingleLineEtched
} UITableViewCellSeperatorStyle;


#endif

