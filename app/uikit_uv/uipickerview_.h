#ifndef _UIPICKER_VIEW__H
#define _UIPICKER_VIEW__H


#include "uiview_.h"
#include "uicontrol_.h"
#include "uilabel_.h"
#include "uiimage_.h"
#include "uibutton_.h"

#define MAX_PICKERVIEW_COMP_SZ	4


struct UIPickerViewClass {
	struct UIViewClass	_;
};

struct PickerComponent {
	int				numberOfRow;
	CGSize			sizeOfRow;
	int				selectedRow;
	struct UILabel	_titleLabel;
	struct UIButton	_buttons[2];
	struct UIImage	_images[2];
	void			*rowView;
};

struct UIPickerView {
	struct UIView	_;
	unsigned char	reserve[3];
	unsigned char	numberOfComp;
	struct PickerComponent components[MAX_PICKERVIEW_COMP_SZ];
	int  (*rowHeightForComponent)(void *self, void *pickerView, int component);
	int  (*widthForComponent)(void *self, void *pickerView, int component);
	char *(*titleForRow_forComponent)(void *self, void *pickerView, int row, int component);
	void *(*viewForRow_forComponent_reusingView)(void *self, void *pickerView, int row, int component, void *view);
	void *(*didSelectRow_inComponent)(void *self, void *pickerView, int row, int component);
	int  (*numberOfComponents)(void *self, void *pickerView);
	int  (*numberOfRowsInComponent)(void *self, void *pickerView, int component);
};


#endif
