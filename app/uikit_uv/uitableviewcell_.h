#ifndef _UITABLE_VIEW_CELL__H
#define _UITABLE_VIEW_CELL__H


#include "uiview_.h"
#include "uilabel_.h"
#include "uiimageview_.h"


struct UITableViewCellClass {
	struct UIViewClass	_;
};

struct UITableViewCell {
	struct UIView	_;
	unsigned char	state;
	unsigned char	cellStyle;
	unsigned char	accessoryType;
	unsigned char	selectionStyle;
	UIColor			highlightedColor;		// appended by KDK
	struct UIImageView	_imageView;
	struct UILabel	_textLabel;
	struct UILabel	_detailTextLabel;
	int				reuseIdentifier;
	void			*accessoryView;
};


#endif
