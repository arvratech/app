#include "cr_fp.h"
#include "sysdep.h"

extern FP_READER	_gFPReaders[];

static void _DrawEnrollFPImageBorder(void)
{
	int		x, y, w, h, i, Color;

	Color = RGB(0,255,0);
	x = 2; y = 52;
	w = lcdGetWidth()-4; h = 2;
	lcdFillRect(x, y,     w, h, Color);
	lcdFillRect(x, y+122, w, h, Color);
	y += 2;
	w = 2; h = 120;
	x = 0;
	for(i = 0;i < 2;i++) {
		x += 106;
		lcdFillRect(x,  y,  w, h, Color);
	}
}

static void _DrawEnrollOldFPImage(BMPC *bmpFP, int position)
{
	int		x, y;
	
	x = 2 + position * 106; y = 54;
	lcdPutGray(x, y, bmpFP);
}

static void _DrawEnrollNewFPImage(BMPC *bmpFP, int position)
{
	IMAGE	*imgS;
	int		x, y;
	
	x = 2 + position * 106; y = 54;
	imgS = fprGetImage(&_gFPReaders[0]);
	imgResizeInverseRaw((unsigned char *)bmpFP->Buffer, (int)bmpFP->Width, (int)bmpFP->Height, imgS);
	lcdPutGray(x, y, bmpFP);
}

static void _DrawEnrollFPPrompt(int position, int Color)
{
	int		i, x, y;
/*
	x = 2 + 36; y = 180;
	for(i = 0;i < 3;i++) {
		if(i == position) c = RGB(0,0,255);
		else	c = Color;
		lcdFillRect(x, y, 32, 8, Color);
		x += 106;
	}
*/
	x = 2 + 40; y = 178;
	for(i = 0;i < 3;i++) {
		lcdFillRect(x, y, 24, 24, Color);
		if(i == position) lcdPutIconFile(x, y, 24, FPCAPTURE_ICON);
		x += 106;
	}
}

static void _DrawEnrollProcessing(int position, int Color)
{
	int		x, y;

	x = 2 + 40 + 106 * position; y = 178;
	lcdFillRect(x, y, 24, 24, Color);
	lcdPutIconFile(x, y, 24, WAITING_ICON);
}


static int _FPTmplCanGenerate(int Index, int FPFlag)
{
	
	if(Index && ((FPFlag & 0x38) == 0x38) || !Index && ((FPFlag & 0x07) == 0x7)) return 1;
	else	return 0;
}	

// Return	-2=Error  -1=Timeout  0=Cancelled  1=Back Key  2=Next key
int _EnrollFPStage(GWND *WizardWnd, int Wizard, char *Title, FP_USER *user, int Index)
{
	ACCESS_POINT	*ap;	
	FP_READER	*cr_fp;	
	unsigned char	*p;
	unsigned long	timer;
	int		c, rval, pos, x, y, x1, fh, init, updated;

	ap = &_gAccessPoints[0];
	cr_fp = &_gFPReaders[0];	
	cr_fp->OSExposure = sys_cfg->OSExposure;
	wndDrawClientArea(WizardWnd);
	fh = GetFontHeight() + 4;
	y = 54;
	x = WizardWnd->x+8;
	x1 = lcdPutText(x, y-25, Title, wndGetBackGroundTextColor(WizardWnd));  
	x1 += x + 8;
	pos = 0; updated = 0; init = 1;
	while(1) {
		if(init) {
			 _DrawEnrollFPImageBorder();
			for(c = 0;c < 3;c++) _DrawEnrollOldFPImage(bmpFP[Index*3+c], c);
			init = 0;
		}
		if((Wizard & GWND_WIZARD_BACK) && (!updated || _FPTmplCanGenerate(Index, FPFlag))) x = BTN_BACK; else x = 0;
		if((Wizard & GWND_WIZARD_NEXT) && (!updated || _FPTmplCanGenerate(Index, FPFlag))) y = BTN_NEXT; else y = 0;
		DrawLowerButtons(x, BTN_LEFT_ARROW, BTN_RIGHT_ARROW, y, 0);
		x = 18; y = 52;
		if(!Index && userFPData1IsValid(user) || Index && userFPData2IsValid(user)) c = BOX_CHECKED;
		else	c = BOX_UNCHECKED;
		lcdFillRect(x1, y-25, fh, fh, wndGetBackGroundColor(WizardWnd));
		lcdPutIconFile(x1, y-25, fh, c);
		_DrawEnrollFPPrompt(pos, wndGetBackGroundColor(WizardWnd));
		y = 54;
		timer = DS_TIMER + 150;
		while(1) {
			if(DS_TIMER >= timer) rval = GWND_TIMEOUT;
			else {
				if(menuGetGlobalExit()) c = KEY_CANCEL;
				else	c = keyGet();
				if(c == KEY_CANCEL) rval = GWND_QUIT;
				else if((Wizard & GWND_WIZARD_BACK) && (!updated || _FPTmplCanGenerate(Index, FPFlag)) && c == KEY_F1) rval = 1;
				else if((Wizard & GWND_WIZARD_NEXT) && (!updated || _FPTmplCanGenerate(Index, FPFlag)) && (c == KEY_F4 || c == KEY_OK)) rval = 2;
				else if(c == KEY_F2) rval = 3;
				else if(c == KEY_F3) rval = 4;
				else {
					rval = fprDetectTouch(cr_fp);
					if(rval > 0) rval = 5;
					else if(!rval)	rval = 9;
				}
			}
			if(rval < 9) break;
			taskYield();
		}
		if((rval == 1 || rval == 2) && updated) {
			p = user->FPTemplate; if(Index) p += STORED_TMPL_SZ;
			c = bfpGenerate((unsigned char *)FPTmplStreams[Index*3], p+TMPL_HEAD_SZ);
//cprintf("%ld bfpGenerate=%d\n", DS_TIMER, c);
			if(c >= 0) {
				_ExportHeader(fprGetImage(cr_fp), p);
			} else {
				rval = 5;
			}
		} else if(rval == 5) {
			rval = fprScan(cr_fp);
			fprClose(cr_fp);
//cprintf("%ld fprScan=%d\n", DS_TIMER, rval);
			if(rval > 1) {
				FPCaptureBeep();
				DrawLowerButtons(0, 0, 0, 0, 0);
				c = Index * 3 + pos;
				_DrawEnrollNewFPImage(bmpFP[c], pos);
				_DrawEnrollProcessing(pos, wndGetBackGroundColor(WizardWnd));	
				updated = 1;
				rval = bfpCreateProcess(fprGetImage(cr_fp), FPTmplStreams[c]);
				if(rval >= 0) {
					rval = 4;
					FPFlag |= 0x01 << c;
				} else {
					rval = 5;
					memset(FPTmplStreams[c], 0xff, TMPL_BODY_SZ);
					FPFlag &= ~(0x01 << c);
 				}
 			} else if(rval == 1) rval = 5;
  		}
 		if(rval < 3) {
 			if(rval == GWND_ERROR) ResultMsg(WizardWnd, Title, R_MSG_SYSTEM_ERROR);
			break;
		} else if(rval == 3) {
			pos--; if(pos < 0) pos = 2;
		} else if(rval == 4) {
			pos++; if(pos > 2) pos = 0;
		} else if(rval == 5) {
			ResultMsg(WizardWnd, Title, R_FAILED);
			init = 1;
		}
	}
	return rval;
}

void _UndoEnrollFPTemplate(FP_USER *user)
{
}

void _GetUserFPTemplates(FP_USER *user)
{
	int		rval;

	if(user) {
		rval = fsGetFPTemplate(user);
	}
}

void _EnrollSaveUser(GWND *WizardWnd, char *Title, FP_USER *user)
{
	unsigned char	*s, *d, buf[1200];
	int		rval, i;

/*
	mode = user->AccessMode;
	switch(mode) {
	case 1:		user->CardData[0] = 0; user->PIN[0] = 0; break;
	case 2:		user->FPTemplate[0] = user->FPTemplate[STORED_TMPL_SZ] = 0; user->PIN[0] = 0; break;
	case 3:		user->FPTemplate[0] = user->FPTemplate[STORED_TMPL_SZ] = 0; user->CardData[0] = 0; break;
	case 4:
	case 7:		user->PIN[0] = 0; break;
	case 5:
	case 8:		user->CardData[0] = 0; break;
	case 6:
	case 9:		user->FPTemplate[0] = user->FPTemplate[STORED_TMPL_SZ] = 0; break;
	}
*/
	d = buf;
	*d = 0x31;
	userEncode(user, d);
	d += USER_RECORD_SZ;
	s = user->FPTemplate;
	for(i = 0;i < 2;i++, s += STORED_TMPL_SZ) {
		if(*s) {
			user->FPIndex[i] = i;
			memcpy(d, s, STORED_TMPL_SZ);
			d += STORED_TMPL_SZ;
		} else	user->FPIndex[i] = -1;
	}
	userEncodeFPIndex(user->FPIndex, buf);
	if(sys_cfg->TermOption & FP_IDENTIFY) i = GetMaxFPIdentifySize();
	else	i = GetMaxFPTemplateSize();
	rval = d - buf;
	if(rval > USER_RECORD_SZ && fsGetFPTemplateCount() >= i) ResultMsg(WizardWnd, Title, R_USER_ARE_FULL);
	else {
		rval = fsAddEncodedUser(buf, i);
		if(rval < 0) ResultMsg(WizardWnd, Title, R_MSG_SYSTEM_ERROR);
		else if(rval == 0) ResultMsg(WizardWnd, Title, R_USER_ARE_FULL);
		else {
			if(rval & 0x0f) UpdateTrpls();
			if(rval & 0xff) ResultMsg(WizardWnd, Title, R_DATA_SAVED);
		}
	}
}
