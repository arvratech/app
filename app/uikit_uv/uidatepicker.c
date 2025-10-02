#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "cgrect.h"
#include "UIView.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIDatePicker.h"
#include "uidatepicker_.h"


static void UIDatePickerClass_izer(void *self, va_list *app)
{
	struct UIDatePickerClass *cls = self;

//printf("UIDatePickerClass_izer...\n");
	super_izer(UIDatePickerClass, cls, app);
}

#define COMP_WIDTH			90
#define COMP_BTN_HEIGHT		60
#define COMP_TITLE_HEIGHT	80	

static void UIDatePicker_izerwf(void * self, va_list *app)
{
	struct UIDatePicker	*_self = self;
	void	*v, *img, *imgv;
	CGRect	rt;
	char	fileName[32];
	int		i, j, x, y;

	super_izerwf(UIDatePicker, _self, app);
	UIframe(_self, &rt);
	rt.width = COMP_WIDTH + COMP_WIDTH + COMP_WIDTH;
	rt.height = COMP_BTN_HEIGHT + COMP_TITLE_HEIGHT + COMP_BTN_HEIGHT;
	UIsetFrame(_self, &rt);
	UIsetOpaque(_self, TRUE);;	
	UIsetBackgroundColor(_self, whiteColor);	// append at 2022.6.3
	UIsetTintColor(self, systemGreen);
//	UIsetBackgroundColor(self, ARGB(255,0,0,0));
	_self->locale		= LANG_ENG;
	_self->year			= 2000;
	_self->month		= 1;
	_self->day			= 1;
	_self->status		= 0;
	x = 0;
	for(i = 0;i < 3;i++) {
		v = &_self->_titleLabels[i]; 
		alloc(UILabel, v);
		initWithFrame(v, x, COMP_BTN_HEIGHT, COMP_WIDTH, COMP_TITLE_HEIGHT);
		UIsetFont(v, 34);
		UIsetTextAlignment(v, UITextAlignmentCenter);
		UIsetOpaque(v, TRUE);
		UIsetBackgroundColor(v, UIbackgroundColor(_self));  // append at 2022.6.3
		UIaddChild(_self, v);
		for(j = 0;j < 2;j++) {
			v = &_self->_buttons[i][j]; 
			alloc(UIButton, v);
			if(j) y = COMP_BTN_HEIGHT + COMP_TITLE_HEIGHT;
			else  y = 0;
			initWithFrame(v, x, y, COMP_WIDTH, COMP_BTN_HEIGHT);
			UIsetTag(v, (i<<1)+j);
			UIsetOpaque(v, TRUE);
			img = &_self->_images[i][j]; 
			alloc(UIImage, img);
			if(j) strcpy(fileName, "arrowh_down.bmp"); 
			else  strcpy(fileName, "arrowh_up.bmp"); 
			UIinitWithFile(img, fileName, IMAGE_ALPHAMASK);
			UIsetButtonImage(v, img);
			imgv = UIimageView(v);
			rt.width = rt.height = 36;
			rt.x = (COMP_WIDTH - rt.width) >> 1; rt.y = (COMP_BTN_HEIGHT - rt.height) >> 1;
			UIsetFrame(imgv, &rt);
			UIaddChild(_self, v);
		}
		x += COMP_WIDTH;
	}
}

static void UIDatePicker_draw(void *self, CGRect *rect)
{
	struct UIDatePicker *_self = self;

	super_UIdraw(UIDatePicker, _self, rect);
}

void _PlusDateComp(struct UIDatePicker *self, int comp, char *text)
{
	if(self->locale == LANG_ENG) {
		if(comp == 0) {
			self->month++; if(self->month > 12) self->month = 1;
			dtGetShortMonth((int)self->locale, (int)self->month, text);
		} else if(comp == 1) { 
			self->day++; if(self->day > get_month_days(self->year, self->month)) self->day = 1;
			sprintf(text, "%02d", (int)self->day);
		} else {
			self->year++;
			sprintf(text, "%04d", (int)self->year);
		}
	} else {
		if(comp == 0) {
			self->year++;
			sprintf(text, "%04d", (int)self->year);
		} else if(comp == 1) {
			self->month++; if(self->month > 12) self->month = 1;
			dtGetShortMonth((int)self->locale, (int)self->month, text);
		} else {
			self->day++; if(self->day > get_month_days(self->year, self->month)) self->day = 1;
			sprintf(text, "%02d", (int)self->day);
		}
	}
}

void _MinusDateComp(struct UIDatePicker *self, int comp, char *text)
{
	if(self->locale == LANG_ENG) {
		if(comp == 0) {
			self->month--; if(self->month < 1) self->month = 12;
			dtGetShortMonth((int)self->locale, (int)self->month, text);
		} else if(comp == 1) { 
			self->day--; if(self->day < 1) self->day = get_month_days(self->year, self->month);
			sprintf(text, "%02d", (int)self->day);
		} else {
			if(self->year > 2000) self->year--;
			sprintf(text, "%04d", (int)self->year);
		}
	} else {
		if(comp == 0) {
			if(self->year > 2000) self->year--;
			sprintf(text, "%04d", (int)self->year);
		} else if(comp == 1) {
			self->month--; if(self->month < 1) self->month = 12;
			dtGetShortMonth((int)self->locale, (int)self->month, text);
		} else{
			self->day--; if(self->day < 1) self->day = get_month_days(self->year, self->month);
			sprintf(text, "%02d", (int)self->day);
		}
	}
}

int _DatePickerClick(void *self, int btnTag)
{
	struct UIDatePicker *_self = self;
	void	*v;
	char	temp[32];
	int		i;
	
	i = btnTag >> 1;
	if(i < 3) {
		if(btnTag & 1) _MinusDateComp(_self, i, temp);
		else	_PlusDateComp(_self, i, temp);
		v = &_self->_titleLabels[i];
		UIsetText(v, temp);
		i = 1;
	} else {
		i = 0;
	}
	return i;
}

struct NSClass		_UIDatePickerClass;
struct UIDatePickerClass	_UIDatePicker;

void *UIDatePickerClass, *UIDatePicker;

void initUIDatePicker(void)
{
	if(!UIDatePickerClass) {
		UIDatePickerClass = &_UIDatePickerClass;
		alloc(NSClass, UIDatePickerClass);
		init(UIDatePickerClass, "UIDatePickerClass", UIControlClass, sizeof(struct UIDatePickerClass),
		izer, UIDatePickerClass_izer,
		0);
	}
	if(!UIDatePicker) {
		UIDatePicker = &_UIDatePicker;
		alloc(UIDatePickerClass, UIDatePicker);
		init(UIDatePicker, "UIDatePicker", UIControl, sizeof(struct UIDatePicker),
		izerwf, UIDatePicker_izerwf, 
		UIdraw, UIDatePicker_draw,
		0);
	}
}

int UIyear(void *self)
{
	struct UIDatePicker *_self = self;

	return (int)_self->year;
}

void UIsetYear(void *self, int year)
{
	struct UIDatePicker *_self = self;
	void	*v;
	char	temp[32];
	int		i, j;

	_self->year = (unsigned short)year;
	sprintf(temp, "%04d", (int)_self->year);
	if(_self->locale == LANG_ENG) i = 2;
	else	i = 0;
	UIsetText(&_self->_titleLabels[i], temp);
	for(i = 0;i < 3;i++)
		for(j = 0;j < 2;j++) {
			v = &_self->_buttons[i][j]; 
			UIsetBackgroundColor(v, UIbackgroundColor(_self));
		}
}

int UImonth(void *self)
{
	struct UIDatePicker *_self = self;

	return (int)_self->month;
}

void UIsetMonth(void *self, int month)
{
	struct UIDatePicker *_self = self;
	char	temp[32];
	int		i;

	_self->month = (unsigned char)month;
	dtGetShortMonth((int)_self->locale, (int)_self->month, temp);
	if(_self->locale == LANG_ENG) i = 0;
	else	i = 1;
	UIsetText(&_self->_titleLabels[i], temp);
}

int UIday(void *self)
{
	struct UIDatePicker *_self = self;

	return (int)_self->day;
}

void UIsetDay(void *self, int day)
{
	struct UIDatePicker *_self = self;
	char	temp[32];
	int		i;

	_self->day = (unsigned char)day;
	sprintf(temp, "%02d", (int)_self->day);
	if(_self->locale == LANG_ENG) i = 1;
	else	i = 2;
	UIsetText(&_self->_titleLabels[i], temp);
}

int UIlocale(void *self)
{
	struct UIDatePicker *_self = self;

	return (int)_self->locale;
}

void  UIsetLocale(void *self, int locale)
{
	struct UIDatePicker *_self = self;

	_self->locale = locale;
}

