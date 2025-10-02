#ifndef _GMENUPRIM_H_
#define _GMENUPRIM_H_

#include "gwnd.h"
#include "glabel.h"
#include "gedit.h"
#include "gdate.h"
#include "gcheck.h"
#include "gradio.h"
#include "gmenu.h"
#include "gmenuc.h"


void MenuSetAdminConsole(int Enable);
int  MenuGetAdminConsole(void);
void MenuInitialize(GMENU *menu, char *Title, char *menuStr[], int ItemSize);
void MenuTerminate(GMENU *menu);
char *MenuGetItemText(GMENU *menu, int Item);
void MenuClearItems(GMENU *menu);
void MenuAddItem(GMENU *menu, char *Text, int IconFile, int Status);
void MenuAddItemCallBack(GMENU *menu, int Status, void (*PutCallBack)());
void MenuShow(GMENU *menu);
int  MenuPopup(GMENU *menu);
int  MenuPopupEx(GMENU *menu, int Wizard, int Timeout);
int  MenuPopupGlobalExitOnly(GMENU *menu);
int  MenuUsers(GWND *ParentWnd, char *Title, unsigned char *dIDs, int UserSize, int curItem, int ViewOnly);
int  MenuPhones(GWND *ParentWnd, char *Title, unsigned char *dPhoneNos, int PhoneSize, int curItem, int ViewOnly);
int  WndMessageBoxYesNo(GWND *ParentWnd, char *Title, char *Text);
int  MenuRadio(GWND *ParentWnd, char *Title, unsigned char *pVal, char *menuStr[], int menuSize, int FromZero, int Section);
int  MenuNoYes(GWND *ParentWnd, char *Title, unsigned char *pVal, int Mask, int Section);
int  MenuYesNo(GWND *ParentWnd, char *Title, unsigned char *pVal, int Mask, int Section);
int  MenuCheck(GWND *ParentWnd, char *Title, unsigned char *pVal, char *menuStr[], int menuSize, int Section);
int  MenuCheck2(GWND *ParentWnd, char *Title, unsigned long *pVal, char *menuStr[], int menuSize, int Section);
int  MenuInputSchedule(GWND *ParentWnd, char *Title, unsigned char *pSchedule, int Section);
int  MenuRadioWizard(GWND *WizardWnd, char *Title, unsigned char *pVal, char *menuStr[], int menuSize, int FromZero, int Wizard);
int  MenuRadioWizardEx(GWND *WizardWnd, RECT *rectWnd, char *Title, unsigned char *pVal, char *menuStr[], int menuSize, int FromZero, int Wizard);
int  MenuNoYesWizard(GWND *WizardWnd, char *Title, unsigned char *pVal, int Mask, int Wizard);
int  MenuCheckWizard(GWND *WizardWnd, char *Title, unsigned char *pVal, char *menuStr[], int menuSize, int Wizard);
int  WndWizard(GWND *wnd, int Wizard);
int  EditWizard(GEDIT *edt, int Wizard);
int  DateWizard(GDATE *date, int Wizard);
int  RadioGroupWizard(GRADIO_GROUP *rdGroup, int Wizard);
int  RadioGroupWizardEx(GRADIO_GROUP *rdGroup, int Wizard, void (*ChangedCallBack)(), void *Context);


#endif	/* _GMENUPRIM_H_ */