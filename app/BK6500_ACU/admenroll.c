#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "iodev.h"
#include "key.h"
#include "lcdc.h"
#include "gfont.h"
#include "syscfg.h"
#include "unit.h"
#include "acad.h"
#include "acap.h"
#include "fsuser.h"
#include "cr.h"
#include "sclib.h"
#include "msg.h"
#include "evt.h"
#include "sche.h"
#include "sysdep.h"
#ifdef STAND_ALONE
#ifdef BIO_FP
#include "fpapi.h"
#else
#include "fpapi_new.h"
#endif
#include "fpimage.h"
#endif
#include "menuprim.h"
#include "wnds.h"
#include "devprim.h"
#include "topprim.h"
#include "desktop.h"

extern FP_USER	*gUser;
extern CREDENTIAL_READER _gCredentialReaders[];
#ifdef STAND_ALONE
#ifdef BIO_FP
static BMPC		*bmpFP[6], _bmpFP[6];
static unsigned char FPImages[6][12480];		// 104*120
static unsigned char FPTmplStreams[6][TMPL_BODY_SZ];
static int		FPFlag;
#else
static BMPC		*bmpFP[4], _bmpFP[4];
static unsigned char *ProcessedTmpls[4];
static unsigned char *OldStoredTmpls;
static unsigned char *NewStoredTmpls;
static int		FPFlag;
#endif
#endif

#include "admenroll_prim.c"
#ifdef STAND_ALONE
#ifdef BIO_FP
#include "admenroll_biofp.c"
#else
#include "admenroll_sgfp.c"
#endif
#endif

static int _EnrollUserStage1(GWND *WizardWnd, int *Wizard, FP_USER *user)
{
	int		rval;

	rval = _EnrollAccessMode(WizardWnd, *Wizard, xmenu_user_enroll[1], user);
//cprintf("Stage1: rval=%d\n", rval);
	return rval;	
}

static int _EnrollUserStage2(GWND *WizardWnd, int *Wizard, FP_USER *user)
{
	int		rval;

	if(userIsCard(user)) {
		rval = _EnrollCardStage(WizardWnd, *Wizard, xmenu_user_enroll[4], user);
	} else if(userIsPIN(user)) { 
		rval = _EnrollPINStage(WizardWnd, *Wizard, xmenu_user_enroll[5], user);
	} else {
		*Wizard &= ~GWND_WIZARD_NEXT;
		rval = _EnrollFinalStage(WizardWnd, *Wizard, xmenu_user_enroll[0], user);
	}
//cprintf("Stage2: rval=%d\n", rval);
	return rval;
}

static int _EnrollUserStage3(GWND *WizardWnd, int *Wizard, FP_USER *user)
{
	int		rval;
	
	if(userIsCard(user) && userIsPIN(user)) {
		rval = _EnrollPINStage(WizardWnd, *Wizard, xmenu_user_enroll[5], user);
	} else {
		*Wizard &= ~GWND_WIZARD_NEXT;
		rval = _EnrollFinalStage(WizardWnd, *Wizard, xmenu_user_enroll[0], user);
	}
//cprintf("Stage3: rval=%d\n", rval);
	return rval;
}

static int _EnrollUserStage4(GWND *WizardWnd, int *Wizard, FP_USER *user)
{
	int		rval;

	*Wizard &= ~GWND_WIZARD_NEXT;
	rval = _EnrollFinalStage(WizardWnd, *Wizard, xmenu_user_enroll[0], user);
//cprintf("Stage6: rval=%d\n", rval);
	return rval;
}

void _MakeTitleFromID(char *DstTitle, char *OrgTitle, long nID)
{
	char	*p;

	p = DstTitle;
	strcpy(p, OrgTitle); p += strlen(p); strcpy(p, "  ID:"); p += strlen(p); sprintf(p, "%ld", nID);
}

void _EnrollSaveUser(GWND *WizardWnd, char *Title, FP_USER *user, FP_USER *OldUser)
{
	unsigned char	buf[64];
	int		rval, MsgIndex;

#ifdef STAND_ALONE
	rval = _AddFPTemplate(user, OldUser);
#else
	rval = 1;
#endif
	if(rval > 0 && (!OldUser || userCompare(OldUser, user))) {
		userEncode(user, buf);
		rval = userfsAddEncoded(user->ID, buf);
		if(rval < 0) {
#ifdef STAND_ALONE
			userfsRemoveFPTemplate(user);
#endif			
			MsgIndex = R_MSG_SYSTEM_ERROR;
		} else if(!rval) MsgIndex = R_USER_ARE_FULL;
		else	MsgIndex = R_DATA_SAVED;
		ResultMsg(WizardWnd, Title, MsgIndex);
//cprintf("[%02x", (int)user->CardData[0]); for(rval = 1;rval < 9;rval++) cprintf("-%02x", (int)user->CardData[rval]); cprintf("]\n");
//user->ID += 10000000;
//buf[0] = 0x11; buf[1] = 0x22; buf[2] = 0x33; buf[3] = 0x44; 
//buf[4] = 0x55; buf[5] = 0x66; buf[6] = 0x77; buf[7] = 0x88;
//bin2card(buf, 8, user->CardData);
//userEncode(user, buf);
//rval = userfsAddEncoded(user->ID, buf);
	}
}

int EnrollWizardUser(FP_USER *user, int FromStage, int NewUser, char *Title)
{
	GWND		*wnd, _wnd;
	FP_USER		*OldUser, _OldUser;
	char	title[32];
	int		rval, Stage, Wizard;

	wnd = &_wnd;
	_MakeTitleFromID(title, Title, user->ID);
	WizardWndInitialize(wnd, title);
	wndDrawWindow(wnd);
	if(NewUser) {
		OldUser = NULL;
	} else {
		OldUser = &_OldUser;
		userCopy(OldUser, user);	
	}
	_gCredentialReaders[0].CapturePurpose = CR_CAPTURE_ENROLL;
	_gCredentialReaders[1].CapturePurpose = CR_CAPTURE_ENROLL;
	Stage = FromStage;
	while(Stage) {
		Wizard = GWND_WIZARD;
		Wizard |= GWND_WIZARD_NEXT;
		if(Stage > FromStage) Wizard |= GWND_WIZARD_BACK;
		switch(Stage) {
		case 1:	rval = _EnrollUserStage1(wnd, &Wizard, user); break;
		case 2:	rval = _EnrollUserStage2(wnd, &Wizard, user); break;
		case 3: rval = _EnrollUserStage3(wnd, &Wizard, user); break;
		case 4: rval = _EnrollUserStage4(wnd, &Wizard, user); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else if(rval == 1) {
			Stage--;
		} else {
			if(Wizard & GWND_WIZARD_NEXT) Stage++;
			else {	// Final Stage
				_EnrollSaveUser(wnd, xmenu_user_enroll[0], user, OldUser);
				Stage = 0;
			}
		}
		taskYield();	// append for ACU comm.
	}
	_gCredentialReaders[0].CapturePurpose = CR_CAPTURE_IDENTIFY;
	_gCredentialReaders[1].CapturePurpose = CR_CAPTURE_IDENTIFY;
	return rval;
}
