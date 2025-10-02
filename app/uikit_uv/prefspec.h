#ifndef _PREF_SPEC_H
#define _PREF_SPEC_H


#include "NSEnum.h"
#include <stdarg.h>


// type definitions
#define PS_TYPE_SCREEN			1
#define PS_TYPE_TITLE			2
#define PS_TYPE_BUTTON			3
#define PS_TYPE_CHECK			4
#define PS_TYPE_RADIO_GROUP		5
#define PS_TYPE_RADIO			6
#define	PS_TYPE_SWITCH			7
#define PS_TYPE_STEPPER			8
#define PS_TYPE_SLIDER			9
#define PS_TYPE_ALERT			10
#define PS_TYPE_TEXT_FIELD		11
#define PS_TYPE_SPINNER			12
#define PS_TYPE_CHECK_LIST		13
#define PS_TYPE_PICKER			14
#define PS_TYPE_TIME_RANGE		15
#define PS_TYPE_SLIDER_GROUP	16

// state flags definitions
#define PS_HIDDEN				0x01
#define PS_DISABLED				0x02
#define PS_INTERACTION_ENABLED	0x04
#define PS_ACCESSORY_DISABLED	0x08

#define PS_VIEW_STYLE_DEFAULT	0	
#define PS_VIEW_STYLE_READONLY	1	
#define PS_VIEW_STYLE_EDIT		2	

#define	PS_TEXT_SZ				32


typedef struct _PS_OBJECT {
	unsigned char	type;
	unsigned char	state;
	unsigned char	resultCode;
	unsigned char	viewStyle;
	unsigned char	button;
	unsigned char	backResultCode;
	unsigned char	userData;
	unsigned char	reserve[1];
	int				tag;
	char			title[PS_TEXT_SZ];
	char			subtitle[PS_TEXT_SZ];
	void			*activity;
	void			*context;
	void			(*onPrefChanged)(void *ps, int index, void *act);
	void			(*onValueChanged)(void *ps, int index, void *act);
	void			(*intAction)(void *ps);
	void			(*userAction)(int requestCode, void *ps);
	void			(*buttonIntAction)(void *ps);
	void			(*buttonUserAction)(int requestCode, void *ps);
	void			(*onUserButtonClick)(void *ps);
	void			(*onCreate)(void *ps);
	void			(*onStart)(void *ps);
	void			(*onActivityResult)(void *ps, int requestCode, int resultCode, char *intent);
	void			(*onDestroy)(void *ps);
	void			*children;
	void			*next;
	void			*bufAddress;
} PS_OBJECT;

typedef struct _PS_SCREEN {
	PS_OBJECT		_;
	char			imageFile[32];
} PS_SCREEN;

#define PS_TITLE_STYLE_DEFAULT		0x0	
#define PS_TITLE_STYLE_SUBTITLE		0x1	
#define PS_TITLE_STYLE_GROUP		0x2	
#define PS_TITLE_STYLE_COLUMN		0x4		

typedef struct _PS_TITLE {
	PS_OBJECT		_;
	int				style;
	unsigned short	column[4];
} PS_TITLE;

typedef struct _PS_BUTTON {
	PS_OBJECT		_;
	char			imageFile[32];
} PS_BUTTON;

typedef struct _PS_CHECK {
	PS_OBJECT		_;
	BOOL			_value;
	BOOL			(*value)(void *ctx);
	void			(*setValue)(void *ctx, BOOL value);
	short			interactionCount;
	short			interactionReverse;
	void			*onDialog;
	void			*offDialog;
} PS_CHECK;

typedef struct _PS_RADIO_GROUP {
	PS_OBJECT		_;
	int				_value;
	int				(*value)(void *ctx);
	void			(*setValue)(void *ctx, int value);
} PS_RADIO_GROUP;

typedef struct _PS_RADIO {
	PS_OBJECT		_;
	int				_value;
	void			*radioGroup;
} PS_RADIO;

typedef struct _PS_SWITCH {
	PS_OBJECT		_;
	BOOL			_value;
	BOOL			(*value)(void *ctx);
	void			(*setValue)(void *ctx, BOOL value);
	void			*onDialog;
	void			*offDialog;
} PS_SWITCH;

typedef struct _PS_STEPPER {
	PS_OBJECT		_;
	int				_value;
	int				(*value)(void *ctx);
	void			(*setValue)(void *ctx, int value);
	int				minimumValue;
	int				maximumValue;
	int				stepValue;
	int				unitRatio;		// negative: divide  positive: multiply
	char			unitText[20];
	void			(*getValueText)(int value, char *text);
} PS_STEPPER;

typedef struct _PS_SLIDER {
	PS_OBJECT		_;
	int				_value;
	int				(*value)(void *ctx);
	void			(*setValue)(void *ctx, int value);
	int				minimumValue;
	int				maximumValue;
} PS_SLIDER;

typedef struct _PS_ALERT {
	PS_OBJECT		_;
	int				style;
	void			(*confirm)(void *ctx);
} PS_ALERT;

typedef struct _PS_TEXT_FIELD_ITEM {
	char			title[PS_TEXT_SZ];
	char			_value[PS_TEXT_SZ];
	unsigned char	inputType;
	unsigned char	limitLength;
	unsigned char	reserve[2];
	char			*(*value)(void *ctx);
	void			(*setValue)(void *ctx, char *value);
} PS_TEXT_FIELD_ITEM;

#define PS_TEXT_FIELD_SZ	2

typedef struct _PS_TEXT_FIELD {
	PS_OBJECT		_;
	BOOL			(*doneValidate)(char *intent, char *result);
	PS_TEXT_FIELD_ITEM	items[PS_TEXT_FIELD_SZ];
} PS_TEXT_FIELD;

#define PS_SPINNER_SZ		32

typedef struct _PS_SPINNER {
	PS_OBJECT		_;
	int				_value;
	int				(*value)(void *ctx);
	void			(*setValue)(void *ctx, int value);
	char			titles[PS_SPINNER_SZ][PS_TEXT_SZ];
	int				values[PS_SPINNER_SZ];
	unsigned long	enables;
} PS_SPINNER;

#define PS_CHECK_LIST_SZ	16

typedef struct _PS_CHECK_LIST {
	PS_OBJECT		_;
	BOOL			values[PS_CHECK_LIST_SZ];
	char			titles[PS_CHECK_LIST_SZ][PS_TEXT_SZ];
} PS_CHECK_LIST;

#define PS_PICKER_ITEM_SZ	32	

typedef struct PS_PICKER_ITEM {
	char			title[PS_TEXT_SZ];
	int				_value;
	char			titles[PS_PICKER_ITEM_SZ][PS_TEXT_SZ];
} PS_PICKER_ITEM;

#define PS_PICKER_SZ		4

typedef struct _PS_PICKER {
	PS_OBJECT		_;
	void			(*getValues)(void *ctx, int *values);
	void			(*setValues)(void *ctx, int *values);
	PS_PICKER_ITEM	items[PS_PICKER_SZ];
} PS_PICKER;

typedef struct _PS_TIME_RANGE {
	PS_OBJECT		_;
	unsigned char	value[4];
} PS_TIME_RANGE;

typedef struct _PS_SLIDER_ITEM {
	char			title[PS_TEXT_SZ];
	int				value;
	int				minimumValue;
	int				maximumValue;
} PS_SLIDER_ITEM;

#define PS_SLIDER_GROUP_SZ	4

typedef struct _PS_SLIDER_GROUP {
	PS_OBJECT		_;
	PS_SLIDER_ITEM	items[PS_SLIDER_GROUP_SZ];
} PS_SLIDER_GROUP;


void PSinit(void *self, int type, char *title);
int  PStype(void *self);
char *PStitle(void *self);
void PSsetTitle(void *self, char *title);
char *PSsubtitle(void *self);
void PSsetSubtitle(void *self, char *subtitle);
BOOL PSisEnabled(void *self);
void PSsetEnabled(void *self, BOOL isEnabled);
BOOL PSisHidden(void *self);
void PSsetHidden(void *self, BOOL isHidden);
BOOL PSisInteractionEnabled(void *self);
void PSsetInteractionEnabled(void *self, BOOL isEnabled);
BOOL PSisAccessoryEnabled(void *self);
void PSsetAccessoryEnabled(void *self, BOOL isEnabled);
void *PSactivity(void *self);
void PSsetActivity(void *self, void *activity);
void *PScontext(void *self);
void PSsetContext(void *self, void *context);
int  PStag(void *self);
void PSsetTag(void *self, int tag);
int  PSresultCode(void *self);
void PSsetResultCode(void *self, int resultCode);
int  PSviewStyle(void *self);
void PSsetViewStyle(void *self, int viewStyle);
int  PSbutton(void *self);
void PSsetButton(void *self, int button, void (*action)(void *));
int  PSbackResultCode(void *self);
void PSsetBackResultCode(void *self, int backResultCode);
int  PSuserData(void *self);
void PSsetUserData(void *self, int userData);
void PSsetOnPrefChanged(void *self, void (*onPrefChanged)(void *, int, void *));
void PScallOnPrefChanged(void *self, int index, void *act);
void PSsetOnValueChanged(void *self, void (*onPrefChanged)(void *, int, void *));
void PScallOnValueChanged(void *self, int index, void *act);
void *PSuserAction(void *self);
void PSsetUserAction(void *self, void (*action)(int, void *));
void PScallUserAction(void *self, int requestCode);
void *PSintAction(void *self);
void PSsetIntAction(void *self, void (*action)(void *));
void PScallIntAction(void *self);
void *PSbuttonUserAction(void *self);
void PSsetButtonUserAction(void *self, void (*action)(int, void *));
void PScallButtonUserAction(void *self, int requestCode);
void *PSbuttonIntAction(void *self);
void PSsetButtonIntAction(void *self, void (*action)(void *));
void PScallButtonIntAction(void *self);
void PSsetOnUserButtonClick(void *self, void (*onUserButtonClick)(void *));
void PScallOnUserButtonClick(void *self);
void PSsetOnCreate(void *self, void (*onCreate)(void *));
void PScallOnCreate(void *self);
void PSsetOnStart(void *self, void (*onStart)(void *));
void PScallOnStart(void *self);
void PSsetOnActivityResult(void *self, void (*onActivityResult)(void *, int, int, char *));
void PScallOnActivityResult(void *self, int requestCode, int resultCode, char *intent);
void PSsetOnDestroy(void *self, void (*onDestroy)(void *));
void PScallOnDestroy(void *self);
void *PSfirstChild(void	*self);
void *PSnextChild(void *self);
void *PSobjectAtIndex(void *self, int index);
void *PSobjectWithContext(void *self, void *ctx);
int  PSindexForObject(void *self, void *psObj);
void *PSviewBuffer(void *self);
void PSsetViewBuffer(void *self, void *address);
int  PSchildrenCount(void *self);
void PSremoveChildren(void *self);
void PSaddChild(void *self, void *child);
int  PScompareChildrenContext(void *self, void *objs[], int count);
void *PSaddScreen(void *self, char *title, void (*action)(void *));
void *PSaddTitle(void *self, char *title, char *subtitle, int style);
void *PSaddButton(void *self, char *title, void (*action)(int, void *));
void *PSaddCheck(void *self, char *title, BOOL (*value)(void *), void (*setValue)(void *, BOOL));
void *PSaddRadioGroup(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int));
void *PSaddRadio(void *self, char *title, int value, void *radioGroup);
void *PSaddSwitch(void *self, char *title, BOOL (*value)(void *), void (*setValue)(void *, BOOL), void (*action)(void *));
void *PSaddStepper(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int), int minValue, int maxValue, int stepValue, char *unitText);
void *PSaddSlider(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int), int minValue, int maxValue);
void *PSaddAlert(void *self, char *title, char *value, void (*confirm)(void *));
void *PSaddTextField(void *self, char *title, BOOL (*doneValidate)(char *, char *));
void *PSaddSpinner(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int));
void *PSaddCheckList(void *self, char *title);
void *PSaddPicker(void *self, char *title, void (*getValues)(void *, int *), void (*setValues)(void *, int *));
void *PSaddTimeRange(void *self, char *title, unsigned char *value);
void *PSaddSliderGroup(void *self, char *title);

void *PSscreenAlloc(void);
void PSscreenInit(void *self, char *title, void (*action)(void *));
char *PSscreenImageFile(void *self);
void PSscreenSetImageFile(void *self, char *imageFile);

void *PStitleAlloc(void);
void PStitleInit(void *self, char *title, char *subtitle, int style);
int  PStitleStyle(void *self);
void PStitleGetColumn(void *self, int *column);
void PStitleSetColumn(void *self, int *column);

void *PSbuttonAlloc(void);
void PSbuttonInit(void *self, char *title, void (*action)(int, void *));
char *PSbuttonImageFile(void *self);
void PSbuttonSetImageFile(void *self, char *imageFile);

void *PScheckAlloc(void);
void PScheckInit(void *self, char *title, BOOL (*value)(void *), void (*setValue)(void *, BOOL));
BOOL PScheckValue(void *self);
void PScheckSetValue(void *self, BOOL value);
void PScheckRestoreValue(void *self);
void *PScheckOnDialog(void *self);
void PScheckSetOnDialog(void *self, void *dialog);
void *PScheckOffDialog(void *self);
void PScheckSetOffDialog(void *self, void *dialog);
int  PScheckInteractionCount(void *self);
BOOL PScheckInteractionReverse(void *self);
void PScheckSetInteraction(void *self, int count, BOOL reverse);

void *PSradioGroupAlloc(void);
void PSradioGroupInit(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int));
int  PSradioGroupValue(void *self);
void PSradioGroupSetValue(void *self, int value);
void PSradioGroupRestoreValue(void *self);

void *PSradioAlloc(void);
void PSradioInit(void *self, char *title, int value, void *radioGroup);
void *PSradioRadioGroup(void *self);
BOOL PSradioIsOn(void *self);
void PSradioSetOn(void *self);

void *PSswitchAlloc(void);
void PSswitchInit(void *self, char *title, BOOL (*value)(void *), void (*setValue)(void *, BOOL), void (*action)(void *));
BOOL PSswitchValue(void *self);
void PSswitchSetValue(void *self, BOOL value);
void PSswitchRestoreValue(void *self);
void *PSswitchOnDialog(void *self);
void PSswitchSetOnDialog(void *self, void *dialog);
void *PSswitchOffDialog(void *self);
void PSswitchSetOffDialog(void *self, void *dialog);

void *PSstepperAlloc(void);
void PSstepperInit(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int), int minValue, int maxValue, int stepvalue, char *unitText);
int  PSstepperValue(void *self);
void PSstepperSetValue(void *self, int value);
void PSstepperRestoreValue(void *self);
int  PSstepperMinimumValue(void *self);
int  PSstepperMaximumValue(void *self);
void PSstepperSetMaximumValue(void *self, int maximumValue);
int  PSstepperStepValue(void *self);
char *PSstepperUnitText(void *self);
void PSstepperSetUnitText(void *self, char *unitText);
int  PSstepperUnitRatio(void *self);
void PSstepperSetUnitRatio(void *self, int unitRatio);
void PSstepperSetGetValueText(void *self, void (*getValueText)(int, char *));
int PSstepperCallGetValueText(void *self, char *text);

void *PSsliderAlloc(void);
void PSsliderInit(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int), int minValue, int maxValue);
int  PSsliderValue(void *self);
void PSsliderSetValue(void *self, int value);
void PSsliderRestoreValue(void *self);
int  PSsliderMinimumValue(void *self);
int  PSsliderMaximumValue(void *self);

void *PSalertAlloc(void);
void PSalertInit(void *self, char *title, char *value, void (*confirm)(void *));
void PSalertCallConfirm(void *self);
void PSalertEncode(void *self, char *buf);

void *PStextFieldAlloc(void);
void PStextFieldInit(void *self, char *title, BOOL (*doneValidate)(char *, char *));
int  PStextFieldAddItem(void *self, char *title, char *(*value)(void *), void (*setValue)(void *, char *), int inputType, int limitLength);
char *PStextFieldValueAtIndex(void *self, int index);
void PStextFieldSetValueAtIndex(void *self, int index, char *value);
void PStextFieldAllValue(void *self, char *buf);
BOOL PStextFieldIsPassword(void *self);
void PStextFieldSetInputTypeAtIndex(void *self, int index, int inputType);
BOOL (*PStextFieldDoneValidate(void *self))(char *, char *);
void PStextFieldEncode(void *self, char *buf);
void PStextFieldDecode(void *self, char *buf);

void *PSspinnerAlloc(void);
void PSspinnerInit(void *self, char *title, int (*value)(void *), void (*setValue)(void *, int));
int  PSspinnerItemCount(void *self);
int  PSspinnerAddTitle(void *self, char *title);
int  PSspinnerAddTitleValue(void *self, char *title, int value);
int  PSspinnerValue(void *self);
void PSspinnerSetValue(void *self, int value);
void PSspinnerRestoreValue(void *self);
char *PSspinnerTitleAtIndex(void *self, int index);
int  PSspinnerValueAtIndex(void *self, int index);
void PSspinnerSetTitleValueAtIndex(void *self, char *title, int value, int index);
BOOL PSspinnerEnableAtIndex(void *self, int index);
void PSspinnerSetEnableAtIndex(void *self, BOOL enable, int index);
int  PSspinnerIndexWithValue(void *self, int value);
int  PSspinnerEnableIndexWithValue(void *self, int value);
void PSspinnerEncode(void *self, char *buf);
BOOL PSspinnerDecode(void *self, char *buf);
void PSspinnerDecode2(void *self, char *buf);

void *PScheckListAlloc(void);
void PScheckListInit(void *self, char *title);
int  PScheckListAddTitle(void *self, char *title, BOOL value);
char *PScheckListTitleAtIndex(void *self, int index);
BOOL PScheckListValueAtIndex(void *self, int index);
void PScheckListSetValueAtIndex(void *self, BOOL value, int index);
int  PScheckListCount(void *self);
void PScheckListEncode(void *self, char *buf);
BOOL PScheckListDecode(void *self, char *buf);

void *PSpickerAlloc(void);
void PSpickerInit(void *self, char *title, void (*getValues)(void *, int *), void (*setValues)(void *, int *));
int  PSpickerAddItem(void *self, char *title);
int  PSpickerAtIndexAddTitle(void *self, int index, char *title);
void PSpickerGetValues(void *self, int *values);
void PSpickerSetValues(void *self, int *values);
void PSpickerValuesTitle(void *self, char *buf);
void PSpickerEncode(void *self, char *buf);
BOOL PSpickerDecode(void *self, char *buf);

void *PStimeRangeAlloc(void);
void PStimeRangeInit(void *self, char *title, unsigned char *value);
void PStimeRangeValue(void *self, unsigned char *value);
void PStimeRangeAllValueTitle(void *self, char *buf);
void PStimeRangeEncode(void *self, char *buf);
BOOL PStimeRangeDecode(void *self, char *buf);

void *PSsliderGroupAlloc(void);
void PSsliderGroupInit(void *self, char *title);
int  PSsliderGroupAddItem(void *self, char *title, int value, int minValue, int maxValue);
int  PSsliderGroupValueAtIndex(void *self, int index);
void PSsliderGroupEncode(void *self, char *buf);
BOOL PSsliderGroupDecode(void *self, char *buf);


#endif
