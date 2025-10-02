#ifndef _UIVIEW_H
#define _UIVIEW_H


#include "NSObject.h"
#include "UIEnum.h"


extern void *UIViewClass;		// adds draw 
extern void *UIView;			// self = alloc(UIView); init(self, x, y, width, height);

#define MB_ICONNONE				0
#define MB_ICONINFORMATION		1
#define MB_ICONWARNING			2
#define MB_ICONERROR			3
#define MB_ICONWAITING			4


void initUIView(void);

// Dynamic link method
void initWithFrame(void *self, ...);
void UIdraw(void *self, CGRect *rect);
// Property
int  UItag(void *self);
void UIsetTag(void *self, int tag);
void UIframe(void *self, CGRect *rect);
void UIsetFrame(void *self, CGRect *rect);
void *UIparent(void *self);
int  UIchildren(void *self, void **children);
int  UIchildrenCount(void *self);
void *UIfirstChild(void *self);
void *UInextChild(void *self);
void *UIwindow(void *self);
UIColor UIbackgroundColor(void *self);
void UIsetBackgroundColor(void *self, UIColor backgroundColor);
BOOL UIisHidden(void *self);
void UIsetHidden(void *self, BOOL isHidden);
int  UIalpha(void *self);
void UIsetAlpha(void *self, int alpha);
BOOL UIisOpaque(void *self);
void UIsetOpaque(void *self, BOOL isOpaque);
UIColor UItintColor(void *self);
void UIsetTintColor(void *self, UIColor tintColor);
UIViewTintAdjustmentMode UItintAdjustmentMode(void *self);
void UIsetTintAdjustmentMode(void *self, UIViewTintAdjustmentMode tintAdjustmentMode);
BOOL UIclipToBounds(void *self);
void UIsetClipToBounds(void *self, BOOL clipToBounds);
UIViewContentMode UIcontentMode(void *self);
void UIsetContentMode(void *self, UIViewContentMode contentMode);
BOOL UIisFocused(void *self);
void UIsetFocused(void *self, BOOL isFocused);
BOOL UIcanBecomeFocused(void *self);
void UIsetCanBecomeFocused(void *self, BOOL canBecomeFocused);
BOOL UIisUserInteractionEnabled(void *self);
void UIsetUserInteractionEnabled(void *self, BOOL isUserInteractionEnabled);
void UIsetDrawRect(void *self, void (*drawRect)(void *, CGRect *));
// Method
void UIdrawView(void *self, CGRect *rect);
void UIdrawChild(void *self, void *parentView, CGRect *rect);
void *UIhitTestView(void *self, CGPoint *pt);
void UIaddChild(void *self, void *childView);
void UIaddChildFirst(void *self, void *childView);
void UIinsertChildAboveChild(void *self, void *childView, void *siblingView);
//void UIinsertChildBelowChild(void *self, void *childView, void *siblingView);
void UIremoveFromChild(void *self);
void UIremoveChildAll(void *self);
void *UIviewWithTag(void *self, int tag);
BOOL UIneedsDisplay(void *self);
void UIsetNeedsDisplay(void *self);
void UIsetNeedsDisplayInRect(void *self, CGRect *rect);
void UIconvertPointToView(void *self, CGPoint *pt, void *view, CGPoint *ptView);
void UIconvertPointToWindow(void *self, CGPoint *pt, CGPoint *ptWindow);
void UIconvertPointFromWindow(void *self, CGPoint *pt, CGPoint *ptWindow);
void UIconvertPointToScreen(void *self, CGPoint *pt, CGPoint *ptScreen);
void UIconvertPointFromScreen(void *self, CGPoint *pt, CGPoint *ptScreen);


#endif
