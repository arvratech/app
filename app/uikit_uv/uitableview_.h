#ifndef _UITABLE_VIEW__H
#define _UITABLE_VIEW__H


#include "uiscrollview_.h"


struct UITableViewClass {
	struct UIViewClass	_;
};

struct UITableView {
	struct UIScrollView	_;
	unsigned char	style;
	unsigned char	allowsMultipleSelection;
	unsigned char	seperatorStyle;
	unsigned char	reserve[1];
	void			*delegate;
	void			*dataSource;
	int				contentIndex;
	int				contentSize;
	int				rowHeight;
	UIColor			seperatorColor;
	void			*tableHeaderView;
	void			*tableFooterView;
	void			*reuseCells;
	int				scrollDy;
	int  (*onNumberOfRowsInTableView)(void *self, void *tblv);
	void *(*onCellForRowAtIndex)(void *self, void *tblv, int index);
	void *(*onPrefetchRowsAtIndexs)(void *self, void *tblv, int startIndex, int endIndex);
	void (*onDidSelectRowAtIndex)(void *self, void *tblv, int index);
	int  (*onHeightForRowAtIndex)(void *self, void *tblv, int index);
};


#endif
