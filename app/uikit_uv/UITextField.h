#ifndef _UITEXT_FIELD_H
#define _UITEXT_FIELD_H


#include "UIControl.h"


extern void *UITextField;			// self = alloc(UILabel); init(self, x, y, width, height);
extern void *UITextFieldClass;		// adds draw 


void initUITextField(void);

char *UIplaceHolder(void *self);
void UIsetPlaceHolder(void *self, char *placeHolder);
BOOL UIisEditing(void *self);
UITextBorderStyle UIboderStyle(void *self);
void UIsetBoderStyle(void *self, UITextBorderStyle borderStyle);
UITextFieldViewMode textFieldClearButtonMode(void *self);
void UIsetClearButtonMode(void *self, UITextFieldViewMode clearButtonMode);
int  UItextPosition(void *self);
void UIsetTextPosition(void *self, int textPosition);


#endif
