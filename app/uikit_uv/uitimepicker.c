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
#include "UITimePicker.h"
#include "uitimepicker_.h"


static void UITimePickerClass_izer(void *self, va_list *app)
{
	struct UITimePickerClass *cls = self;

//printf("UITimePickerClass_izer...\n");
	super_izer(UITimePickerClass, cls, app);
}

#define COMP_WIDTH			90
#define COMP_BTN_HEIGHT		60
#define COMP_TITLE_HEIGHT	80	

static void UITimePicker_izerwf(void * self, va_list *app)
{
	struct UITimePicker	*_self = self;
	void	*v, *img, *imgv;
	CGRect	rt;
	char	fileName[32];
	int		i, j;

	super_izerwf(UITimePicker, _self, app);
	UIframe(_self, &rt);
	rt.width = COMP_WIDTH + COMP_WIDTH + COMP_WIDTH;
	rt.height = COMP_BTN_HEIGHT + COMP_TITLE_HEIGHT + COMP_BTN_HEIGHT;
	UIsetFrame(_self, &rt);
	UIsetOpaque(self, TRUE);;	
	UIsetBackgroundColor(_self, whiteColor);	// append at 2022.6.3
	UIsetTintColor(self, systemGreen);
//	UIsetBackgroundColor(self, ARGB(255,0,0,0));
	_self->locale		= LANG_ENG;
	_self->hour			= 0;
	_self->minute		= 0;
	_self->option		= 0x00;
	_self->status		= 0;
	for(i = 0;i < 2;i++) {
		v = &_self->_titleLabels[i]; 
		alloc(UILabel, v);
		initWithFrame(v, 0, 0, 0, 0);
		UIsetFont(v, 34);
		UIsetTextAlignment(v, UITextAlignmentCenter);
		UIsetOpaque(v, TRUE);
		UIsetBackgroundColor(v, UIbackgroundColor(_self));  // append at 2022.6.3
		UIaddChild(_self, v);
		for(j = 0;j < 2;j++) {
			v = &_self->_buttons[i][j]; 
			alloc(UIButton, v);
			initWithFrame(v, 0, 0, 0, 0);
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
	}
	v = &_self->_noonButton; 
	alloc(UIButton, v);
	initWithFrame(v, 0, 0, 0, 0);
	UIsetTag(v, 4);
	UIsetOpaque(v, TRUE);
	imgv = UItitleLabel(v);
	UIsetFont(imgv, 24);
	UIaddChild(_self, v);

	UIset24HourView(_self, FALSE);
}

static void UITimePicker_draw(void *self, CGRect *rect)
{
	struct UITimePicker *_self = self;

	super_UIdraw(UITimePicker, _self, rect);
}

void _PlusTimeComp(struct UITimePicker *self, int comp, char *text)
{
	int		hour24;

	if(comp == 0) {
		self->hour++; if(self->hour >= 24) self->hour = 0;
		if(self->option & 0x01) hour24 = 1;
		else	hour24 = 0;
		dtGetHour(hour24, (int)self->hour, text);
	} else { 
		self->minute++; if(self->minute >= 60) self->minute = 0;
		dtGetMinute((int)self->minute, text);
	}
}

void _MinusTimeComp(struct UITimePicker *self, int comp, char *text)
{
	int		hour24;

	if(comp == 0) {
		if(self->hour <= 0) self->hour = 23; else self->hour--;
		if(self->option & 0x01) hour24 = 1;
		else	hour24 = 0;
		dtGetHour(hour24, (int)self->hour, text);
	} else { 
		if(self->minute <= 0) self->minute = 59; else self->minute--;
		dtGetMinute((int)self->minute, text);
	}
}

int _TimePickerClick(void *self, int btnTag)
{
	struct UITimePicker *_self = self;
	void	*v;
	char	temp[32];
	int		i;
	
	if(btnTag == 4) {
		if(_self->hour < 12) _self->hour += 12;
		else	_self->hour -= 12; 
		dtGetNoon((int)_self->locale, (int)_self->hour, temp);
		v = &_self->_noonButton;
		UIsetTitle(v, temp);
		i = 1;
	} else {
		i = btnTag >> 1;
		if(i < 2) {
			if(btnTag & 1) _MinusTimeComp(_self, i, temp);
			else	_PlusTimeComp(_self, i, temp);
			v = &_self->_titleLabels[i];
			UIsetText(v, temp);
			i = 1;
		} else {
			i = 0;
		}
	}
	return i;
}

struct NSClass		_UITimePickerClass;
struct UITimePickerClass	_UITimePicker;

void *UITimePickerClass, *UITimePicker;

void initUITimePicker(void)
{
	if(!UITimePickerClass) {
		UITimePickerClass = &_UITimePickerClass;
		alloc(NSClass, UITimePickerClass);
		init(UITimePickerClass, "UITimePickerClass", UIControlClass, sizeof(struct UITimePickerClass),
		izer, UITimePickerClass_izer,
		0);
	}
	if(!UITimePicker) {
		UITimePicker = &_UITimePicker;
		alloc(UITimePickerClass, UITimePicker);
		init(UITimePicker, "UITimePicker", UIControl, sizeof(struct UITimePicker),
		izerwf, UITimePicker_izerwf, 
		UIdraw, UITimePicker_draw,
		0);
	}
}

int UIhour(void *self)
{
	struct UITimePicker *_self = self;

	return (int)_self->hour;
}

void UIsetHour(void *self, int hour)
{
	struct UITimePicker *_self = self;
	void	*v;
	char	temp[32];
	int		i, j;

	_self->hour = (unsigned char)hour;
	if(_self->option & 0x01) sprintf(temp, "%02d", (int)_self->hour);
	else	sprintf(temp, "%d", convert12hour(_self->hour));
	UIsetText(&_self->_titleLabels[0], temp);
	for(i = 0;i < 2;i++)
		for(j = 0;j < 2;j++) {
			v = &_self->_buttons[i][j]; 
			UIsetBackgroundColor(v, UIbackgroundColor(_self));
		}
	v = &_self->_noonButton;
	UIsetBackgroundColor(v, UIbackgroundColor(_self));
}

int UIminute(void *self)
{
	struct UITimePicker *_self = self;

	return (int)_self->minute;
}

void UIsetMinute(void *self, int minute)
{
	struct UITimePicker *_self = self;
	char	temp[32];

	_self->minute = (unsigned char)minute;
	sprintf(temp, "%02d", (int)_self->minute);
	UIsetText(&_self->_titleLabels[1], temp);
}

BOOL UI24HourView(void *self)
{
	struct UITimePicker *_self = self;

	if(_self->option & 0x01) return TRUE;
	else	return FALSE;
}

void UIset24HourView(void *self, BOOL hour24View)
{
	struct UITimePicker *_self = self;
	CGRect	rt;
	void	*v, *v2;
	char	temp[32];
	int		i, x;

	if(hour24View) _self->option |= 0x01;
	else		   _self->option &= 0xfe;
	if(hour24View) x = COMP_WIDTH >> 1;
	else if(_self->locale == LANG_KOR || _self->locale == LANG_JPN)	x = COMP_WIDTH;
	else	x = 0;
	for(i = 0;i < 2;i++) {
		v = &_self->_titleLabels[i]; 
		rt.x = x; rt.y = COMP_BTN_HEIGHT; rt.width = COMP_WIDTH; rt.height = COMP_TITLE_HEIGHT;
		UIsetFrame(v, &rt);
		v = &_self->_buttons[i][0]; 
		rt.x = x; rt.y = 0; rt.width = COMP_WIDTH; rt.height = COMP_BTN_HEIGHT;
		UIsetFrame(v, &rt);
		v = &_self->_buttons[i][1]; 
		rt.y = COMP_BTN_HEIGHT + COMP_TITLE_HEIGHT;
		UIsetFrame(v, &rt);
		x += COMP_WIDTH;
	}
	v = &_self->_noonButton; 
	if(hour24View) UIsetHidden(v, TRUE);
	else {
		UIsetHidden(v, FALSE); 
		if(_self->locale == LANG_KOR || _self->locale == LANG_JPN) x = 0;
		else	x = COMP_WIDTH + COMP_WIDTH;
		rt.x = x; rt.y = COMP_BTN_HEIGHT + ((COMP_TITLE_HEIGHT-COMP_BTN_HEIGHT)>> 1);
		rt.width = COMP_WIDTH; rt.height = COMP_BTN_HEIGHT;
		UIsetFrame(v, &rt);	
		dtGetNoon((int)_self->locale, (int)_self->hour, temp);
		UIsetTitle(v, temp);
		v2 = UItitleLabel(v);
		rt.x = rt.y = 0;
		UIsetFrame(v2, &rt);	
		i = 1;
	}
	UIsetHour(_self, (int)_self->hour);
}

