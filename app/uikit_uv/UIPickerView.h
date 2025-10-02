#ifndef _UIPICKER_VIEW_H
#define _UIPICKER_VIEW_H


#include "UIView.h"


extern void *UIPickerView;			// self = alloc(UILabel); init(self, x, y, width, height);
extern void *UIPickerViewClass;		// adds draw 


void initUIPickerView(void);


int  UInumberOfComponent(void *self);
int  UInumberOfRowsInComponent(void *self, int component);
CGSize UIrowSizeForComponent(void *self, int component);
void UIreloadAllComponents(void *self);
void UIreloadComponent(void *self, int component);
int  UIselectRow_inComponent(void *self, int row, int component);
int  UIselectedRowInComponent(void *self, int component);
//void *UIviewForRow_forComponent(void *self, int row, int component);

void UIsetPickerView_rowHeightForComponent(void *self, int (*pickerView_rowHeightForComponent)(void *self, void *pickerView, int component));
void UIsetPickerView_widthForComponent(void *self,  int (*pickerView_widthForComponent)(void *self, void *pickerView, int component));
void UIsetPickerView_titleForRow_forComponent(void *self, char *(*pickerView_titleForRow_forComponent)(void *self, void *pickerView, int row, int component));
void UIsetPickerView_viewForRow_forComponent_reusingView(void *self, void *(*pickerView_viewForRow_forComponent_reusingView)(void *self, void *pickerView, int row, int component, void *view));
void UIsetPickerView_didSelectRow_inComponent(void *self, void *(*pickerView_didSelectRow_inComponent)(void *self, void *pickerView, int row, int component));
void UIsetNumberOfComponentsInPickerView(void *self, int (*numberOfComponentsInPickerView)(void *self, void *pickerView));
void UIsetPickerView_numberOfRowsInComponent(void *self, int (*pickerView_numberOfRowsInComponent)(void *self, void *pickerView, int component));


#endif
