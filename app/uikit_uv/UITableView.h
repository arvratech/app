#ifndef _UITABLE_VIEW_H
#define _UITABLE_VIEW_H


#include "UIView.h"


extern void *UITableView;			// self = alloc(UILabel); init(self, x, y, width, height);
extern void *UITableViewClass;		// adds draw 


void initUITableView(void);


void UIreloadData(void *self);
void UIreloadRowAtIndex(void *self, int index);
int  UIloadLastRow(void *self);
void *UIdequeueReusableCellWithIdentifier(void *self, int identifier);
void UIdequeueReusableCellAll(void *self);
void *UIdelegate(void *self);
void UIsetDelegate(void *self, void *delegate);
void *UIdataSource(void *self);
void UIsetDataSource(void *self, void *dataSource);
BOOL UIallowsMultipleSelection(void *self);
void UIsetAllowsMultipleSelection(void *self, BOOL allowsMultipleSelection);
int  UIrowHeight(void *self);
void UIsetRowHeight(void *self, int rowHeight);
UITableViewCellSeperatorStyle UIseperatorStyle(void *self);
void UIsetSeperatorStyle(void *self, UITableViewCellSeperatorStyle seperatorStyle);
UIColor UIseperatorColor(void *self);
void UIsetSeperatorColor(void *self, UIColor seperatorColor);
void *UIcellForRowAtIndex(void *self, int index);
int  UIindexForCell(void *self, void *cell);
int  UIvisibleCells(void *self, void **cells);

void UIsetOnNumberOfRowsInTableView(void *self, int (*onNumberOfRowsInTableView)(void *, void *));
void UIsetOnCellForRowAtIndex(void *self, void *(*onCellForRowAtIndex)(void *, void *, int));
void UIsetOnPrefetchRowsAtIndexs(void *self, void *(*onPrefetchRowsAtIndexs)(void *, void *, int, int));
void UIsetOnDidSelectRowAtIndex(void *self, void (*onDidSelectRowAtIndex)(void *, void *, int));
void UIsetOnHeightForRowAtIndex(void *self, int (*onHeightForRowAtIndex)(void *, void *, int));


#endif
