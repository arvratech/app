#ifndef _UITABLE_VIEW_CELL_H
#define _UITABLE_VIEW_CELL_H


#include "UIView.h"


extern void *UITableViewCell;			// self = alloc(UILabel); init(self, x, y, width, height);
extern void *UITableViewCellClass;		// adds draw 


void initUITableViewCell(void);


void initWithStyle(void *self, UITableViewCellStyle style, int reuseIdentifier);
UITableViewCellStyle UIcellStyle(void *self);
int  UIreuseIdentifier(void *self);
void *UItextLabel(void *self);
void *UIdetailTextLabel(void *self);
UITableViewCellAccessoryType UIaccessoryType(void *self);
void UIsetAccessoryType(void *self, UITableViewCellAccessoryType accessoryType);
void *UIaccessoryView(void *self);
void UIsetAccessoryView(void *self, void *accessoryView);
UITableViewCellSelectionStyle UIselectionStyle(void *self);
void UIsetSelectionStyle(void *self, UITableViewCellSelectionStyle selectionStyle);


#endif
