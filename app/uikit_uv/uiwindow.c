#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lcdc.h"
#include "cgrect.h"
#include "UIView.h"
#include "UIWindow.h"
#include "uiwindow_.h"


static void UIWindowClass_izer(void *self, va_list *app)
{
	struct UIWindowClass *cls = self;

	super_izer(UIWindowClass, cls, app);
}

static void UIWindow_izerwf(void * self, va_list *app)
{
	struct UIWindow	*_self = self;

	super_izerwf(UIWindow, _self, app);
}

struct NSClass			_UIWindowClass;
struct UIWindowClass	_UIWindow;

void *UIWindowClass, *UIWindow;

void initUIWindow(void)
{
	if(!UIWindowClass) {
		UIWindowClass = &_UIWindowClass;
		alloc(NSClass, UIWindowClass);
		init(UIWindowClass, "UIWindowClass", UIViewClass, sizeof(struct UIWindowClass),
		izer, UIWindowClass_izer,
		0);
	}
	if(!UIWindow) {
		UIWindow = &_UIWindow;
		alloc(UIWindowClass, UIWindow);
		init(UIWindow, "UIWindow", UIView, sizeof(struct UIWindow),
		izerwf, UIWindow_izerwf, 
		0);
	}
}

