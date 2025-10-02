#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "UILabel.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIPickerView.h"
#include "uipickerview_.h"


static void UIPickerViewClass_izer(void *self, va_list *app)
{
	struct UIPickerViewClass *cls = self;

//printf("UIPickerViewClass_izer...\n");
	super_izer(UIPickerViewClass, cls, app);
}

#define COMP_BTN_HEIGHT		60
#define COMP_TITLE_HEIGHT	80	

static void UIPickerView_izerwf(void * self, va_list *app)
{
	struct UIPickerView	*_self = self;
	struct PickerComponent	*comp;
	void	*v, *img;
	char	fileName[32];
	int		i, j;

	super_izerwf(UIPickerView, _self, app);
	_self->numberOfComp			= 0;
	UIsetBackgroundColor(_self, whiteColor);	// append at 2022.6.3
	for(i = 0, comp = _self->components;i < MAX_PICKERVIEW_COMP_SZ;i++, comp++) {
		comp->numberOfRow		= 0;
		comp->sizeOfRow.width	= 0;
		comp->sizeOfRow.height	= 80;
		comp->selectedRow		= 0;
		v = &comp->_titleLabel; 
		alloc(UILabel, v);
		initWithFrame(v, 0, 0, 0, 0);
		UIsetFont(v, 34);
		UIsetTextAlignment(v, UITextAlignmentCenter);
		UIsetOpaque(v, TRUE);
		UIsetBackgroundColor(v, UIbackgroundColor(_self));	// append at 2022.6.3
		UIaddChild(_self, v);
		for(j = 0;j < 2;j++) {
			v = &comp->_buttons[j]; 
			alloc(UIButton, v);
			initWithFrame(v, 0, 0, 0, 0);
			UIsetTag(v, (i<<1)+j);
			UIsetOpaque(v, TRUE);
			img = &comp->_images[j]; 
			alloc(UIImage, img);
			if(j) strcpy(fileName, "arrowh_down.bmp"); 
			else  strcpy(fileName, "arrowh_up.bmp"); 
			UIinitWithFile(img, fileName, IMAGE_ALPHAMASK);
			UIsetButtonImage(v, img);
			UIaddChild(_self, v);
		}
		comp->rowView			= NULL;
	}
	_self->rowHeightForComponent	= NULL;
	_self->widthForComponent		= NULL;
	_self->titleForRow_forComponent	= NULL;
	_self->viewForRow_forComponent_reusingView = NULL;
	_self->didSelectRow_inComponent	= NULL;
	_self->numberOfComponents		= NULL;
	_self->numberOfRowsInComponent	= NULL;
}

static void UIPickerView_draw(void *self, CGRect *rect)
{
	struct UIPickerView *_self = self;
	struct PickerComponent *comp;
	void	*v;
	int		i, num;

//printf("UIPickerView_draw...\n");
	super_UIdraw(UIPickerView, _self, rect);
/*
	num = UInumberOfComponent(_self);
	for(i = 0, comp = _self->components;i < num;i++, comp++) {
		if(_self->viewForRow_forComponent_reusingView) v = comp->rowView;
		else	v = &comp->_titleLabel;
		UIsetNeedsDisplay(v);
		UIdrawChild(v, _self, rect);
		v = &comp->_buttons[0];
		UIsetNeedsDisplay(v);
		UIdrawChild(v, _self, rect);
		v = &comp->_buttons[1];
		UIsetNeedsDisplay(v);
		UIdrawChild(v, _self, rect);
	}
*/
}

int _PickerViewClick(void *self, int btnTag)
{
	struct UIPickerView *_self = self;
	struct PickerComponent	*comp;
	int		i, row;

	i = btnTag >> 1;
	if(i < UInumberOfComponent(_self)) {
		comp = &_self->components[i];
		row = comp->selectedRow;
		if(btnTag & 1) {	// down
			row--;
			if(row < 0) row = comp->numberOfRow - 1;
		} else {			// up
			row++;
			if(row >= comp->numberOfRow) row = 0;
		}
		UIselectRow_inComponent(_self, row, i);
	}
	return 1;
}

struct NSClass		_UIPickerViewClass;
struct UIPickerViewClass	_UIPickerView;

void *UIPickerViewClass, *UIPickerView;

void initUIPickerView(void)
{
	if(!UIPickerViewClass) {
		UIPickerViewClass = &_UIPickerViewClass;
		alloc(NSClass, UIPickerViewClass);
		init(UIPickerViewClass, "UIPickerViewClass", UIViewClass, sizeof(struct UIPickerViewClass),
		izer, UIPickerViewClass_izer,
		0);
	}
	if(!UIPickerView) {
		UIPickerView = &_UIPickerView;
		alloc(UIPickerViewClass, UIPickerView);
		init(UIPickerView, "UIPickerView", UIView, sizeof(struct UIPickerView),
		izerwf, UIPickerView_izerwf, 
		UIdraw, UIPickerView_draw,
		0);
	}
}

// Gets the number of components for the picker view.
int UInumberOfComponent(void *self)
{
	struct UIPickerView *_self = self;

	return (int)_self->numberOfComp;
}

// Returns the number of rows for a component.
int UInumberOfRowsInComponent(void *self, int component)
{
	struct UIPickerView *_self = self;

	return _self->components[component].numberOfRow;
}

// Returns the size of a row for a component.
CGSize UIrowSizeForComponent(void *self, int component)
{
	struct UIPickerView *_self = self;

	return _self->components[component].sizeOfRow;
}

static void _ReloadComponent(struct UIPickerView *self, int component, void *act)
{
	struct PickerComponent	*comp;

//printf("reloadComponent...%d\n", component);
	comp = &self->components[component];
	comp->numberOfRow = (*self->numberOfRowsInComponent)(act, self, component);
//printf("rows=%d\n", comp->numberOfRow);
	comp->sizeOfRow.height = (*self->rowHeightForComponent)(act, self, component);
	comp->sizeOfRow.width = (*self->widthForComponent)(act, self, component);
//printf("row size=%d %d\n", comp->sizeOfRow.width, comp->sizeOfRow.height);
}

// Reloads all components of the picker view.
void UIreloadAllComponents(void *self)
{
	struct UIPickerView *_self = self;
	struct PickerComponent	*comp;
	void	*v, *imgv, *act;
	CGRect	rt, rt2;
	int		i, x, num;

	act = NULL;
	if(!_self->numberOfComponents || !_self->numberOfRowsInComponent) return;
	num = _self->numberOfComp = (*_self->numberOfComponents)(act, _self);
	for(i = 0;i < num;i++) _ReloadComponent(_self, i, act);
	x = 0;
	for(i = 0, comp = _self->components;i < num;i++, comp++) {
		rt.x = x; rt.y = 0; rt.width = comp->sizeOfRow.width; rt.height = COMP_BTN_HEIGHT;
		v = &comp->_buttons[0];
		UIsetFrame(v, &rt);
		UIsetBackgroundColor(v, UIbackgroundColor(_self));
		imgv = UIimageView(v);
		rt2.width = rt2.height = 36;
		rt2.x = (rt.width - rt2.width) >> 1; rt2.y = (rt.height - rt2.height) >> 1;
		UIsetFrame(imgv, &rt2);
		rt.y = COMP_BTN_HEIGHT + comp->sizeOfRow.height;
		v = &comp->_buttons[1];
		UIsetFrame(v, &rt);
		UIsetBackgroundColor(v, UIbackgroundColor(_self));
		imgv = UIimageView(v);
		UIsetFrame(imgv, &rt2);
		rt.y = COMP_BTN_HEIGHT; rt.height = comp->sizeOfRow.height;
		if(_self->viewForRow_forComponent_reusingView) v = comp->rowView;
		else	v = &comp->_titleLabel;
		UIsetFrame(v, &rt);
		x += rt.width;
		// appended at 2022.6.3
		v = &comp->_titleLabel; 
		UIsetBackgroundColor(v, UIbackgroundColor(_self));	// append at 2022.6.3
	}
}

// Reloads a particular component of the picker view.
void UIreloadComponent(void *self, int component)
{
	struct UIPickerView *_self = self;
	void	*act;

	act = NULL;
	_ReloadComponent(_self, component, act);
	UIsetNeedsDisplay(_self);
}

// Selects a row in a specified component of the picker view.
int UIselectRow_inComponent(void *self, int row, int component)
{
	struct UIPickerView *_self = self;
	struct PickerComponent	*comp;
	void	*act, *v;
	char	*title;
	int		i, x;

//printf("UIseectRow_inComponent: %d %d\n", row, component);
	comp = &_self->components[component];
	comp->selectedRow = row;
	act = NULL;
	if(_self->viewForRow_forComponent_reusingView) {
		v = _self->viewForRow_forComponent_reusingView(act, _self, row, component, comp->rowView);
		comp->rowView = v;
	} else {
		if(_self->titleForRow_forComponent) title = (*_self->titleForRow_forComponent)(act, _self, row, component);
		else	title = NULL;
		v = &comp->_titleLabel;
		UIsetText(v, title);
	}
	if(_self->didSelectRow_inComponent) (*_self->didSelectRow_inComponent)(act, _self, row, component);
}

// Returns the index of the selected row in a given component.
int UIselectedRowInComponent(void *self, int component)
{
	struct UIPickerView *_self = self;

	return _self->components[component].selectedRow;
}

void UIsetPickerView_rowHeightForComponent(void *self, int (*pickerView_rowHeightForComponent)(void *self, void *pickerView, int component))
{
	struct UIPickerView *_self = self;

	_self->rowHeightForComponent = pickerView_rowHeightForComponent;
}

void UIsetPickerView_widthForComponent(void *self,  int (*pickerView_widthForComponent)(void *self, void *pickerView, int component))
{
	struct UIPickerView *_self = self;

	_self->widthForComponent = pickerView_widthForComponent;
}

void UIsetPickerView_titleForRow_forComponent(void *self, char *(*pickerView_titleForRow_forComponent)(void *self, void *pickerView, int row, int component))
{
	struct UIPickerView *_self = self;

	_self->titleForRow_forComponent = pickerView_titleForRow_forComponent;
}

void UIsetPickerView_viewForRow_forComponent_reusingView(void *self, void *(*pickerView_viewForRow_forComponent_reusingView)(void *self, void *pickerView, int row, int component, void *view))
{
	struct UIPickerView *_self = self;

	_self->viewForRow_forComponent_reusingView = pickerView_viewForRow_forComponent_reusingView;
}

void UIsetPickerView_didSelectRow_inComponent(void *self, void *(*pickerView_didSelectRow_inComponent)(void *self, void *pickerView, int row, int component))
{
	struct UIPickerView *_self = self;

	_self->didSelectRow_inComponent = pickerView_didSelectRow_inComponent;
}

void UIsetNumberOfComponentsInPickerView(void *self, int (*numberOfComponentsInPickerView)(void *self, void *pickerView))
{
	struct UIPickerView *_self = self;

	_self->numberOfComponents = numberOfComponentsInPickerView;
}

void UIsetPickerView_numberOfRowsInComponent(void *self, int (*pickerView_numberOfRowsInComponent)(void *self, void *pickerView, int component))
{
	struct UIPickerView *_self = self;

	_self->numberOfRowsInComponent = pickerView_numberOfRowsInComponent;
}

