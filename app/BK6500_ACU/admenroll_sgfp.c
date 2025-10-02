static void _DrawEnrollOldFPImage(BMPC *bmpFP, int position)
{
	int		x, y;
	
	if(position) x = 168; else x = 18;
	y = 52;
	lcdPutGray(x+2, y+2, bmpFP);
}

static void _DrawEnrollNewFPImage(BMPC *bmpFP, int position)
{
	unsigned char	*s, *d;
	int		i, j, w, h, sw;
	
	s = sfpGetFPImage();
	d = (unsigned char *)bmpFP->Buffer;
	w = bmpFP->Width; h = bmpFP->Height;
	sw = w + w;
	for(i = 0;i < h;i++) {
		for(j = 0;j < w;j++) {
			*d++ = (*s + *(s+1) + *(s+sw) + *(s+sw+1)) >> 2;
			s += 2;
		}
		s += sw;
	}
	if(position) j = 168; else j = 18;
	i = 52;
	lcdPutGray(j+2, i+2, bmpFP);
}

static void _DrawEnrollBoundary(int position)
{
	int		y, c1, c2;
	
	y = 52;
	if(position) {
		c1 = RGB(0,255,0); c2 = RGB(0,0,255);
	} else {
		c1 = RGB(0,0,255); c2 = RGB(0,255,0);
	}
	FPImageDrawBoundary( 18, y, c1);
	FPImageDrawBoundary(168, y, c2);
}

static int _FPTmplCanGenerate(int Index, int FPFlag)
{
	
	if(Index && ((FPFlag & 0x0c) == 0x0c) || !Index && ((FPFlag & 0x03) == 0x3)) return 1;
	else	return 0;
}	

// Return	-2=Error  -1=Timeout  0=Cancelled  1=Back Key  2=Next key
int _EnrollFPStage(GWND *WizardWnd, int Wizard, char *Title, FP_USER *user, int Index)
{
	unsigned long	timer;
	int		c, rval, x, y, x1, fh, touch, updated, position, init, backFile, nextFile;

	wndDrawClientArea(WizardWnd);
	fh = GetFontHeight() + 4;
	y = 52;
	x = WizardWnd->x+8;
	x1 = lcdPutText(x, y-25, Title, wndGetBackGroundTextColor(WizardWnd));  
	x1 += x + 8;
	position = 0; updated = 0; init = 1;
	x = 18;
	while(1) {
		if(init) {
			_DrawEnrollBoundary(position);
			for(c = 0;c < 2;c++) _DrawEnrollOldFPImage(bmpFP[Index*2+c], c);
			init = 0;
		}
		if((Wizard & GWND_WIZARD_BACK) && (!updated || _FPTmplCanGenerate(Index, FPFlag))) backFile = BTN_BACK; else backFile = 0;
		if((Wizard & GWND_WIZARD_NEXT) && (!updated || _FPTmplCanGenerate(Index, FPFlag))) nextFile = BTN_NEXT; else nextFile = 0;
		_DrawLowerButtons(backFile, BTN_LEFT_ARROW, BTN_RIGHT_ARROW, nextFile, 0);
		if(!Index && userFPData1IsValid(user) || Index && userFPData2IsValid(user)) c = BOX_CHECKED;
		else	c = BOX_UNCHECKED;
		lcdFillRect(x1, y-25, fh, fh, wndGetBackGroundColor(WizardWnd));
		lcdPutIconFile(x1, y-25, fh, c);
		timer = DS_TIMER + 150;
		touch = ioGetFingerTouch();
		while(1) {
			if(DS_TIMER >= timer) rval = GWND_TIMEOUT;
			else {
				if(menuGetGlobalExit()) c = KEY_CANCEL;
				else	c = keyGet();
				if(c == KEY_CANCEL) rval = GWND_QUIT;
				else if(backFile && c == KEY_F1) rval = 1;
				else if(nextFile && (c == KEY_F4 || c == KEY_OK)) rval = 2;
				else if(c == KEY_F2) rval = 3;
				else if(c == KEY_F3) rval = 4;
				else {
					c = touch;
					touch = ioGetFingerTouch();
					if(!c && touch) rval = 5;
					else	rval = 9;
				}
			}
			if(rval < 9) break;
			taskYield();
		}
		if((rval == 1 || rval == 2) && updated) {
			c = user->FPSecurityLevel;
			if(c <= 0) c = sys_cfg->FPSecurityLevel;
			c = sfpCreateTemplate(ProcessedTmpls[Index*2], ProcessedTmpls[Index*2+1], c, &NewStoredTmpls[Index*800]);
			if(c > 0) {
				if(Index) userSetFPData2(user); else userSetFPData1(user);
			} else	rval = 5;	
		} else if(rval == 5) {
			if(position) x = 168; else x = 18;
			sfpSetPrompt(x, y);
			rval = sfpCapture(0);
			sfpSetPrompt(0, 0);
			if(rval > 0) {
				FPCaptureBeep();
				c = (Index << 1) + position;
				_DrawEnrollNewFPImage(bmpFP[c], position);
				if(position) backFile = 219; else backFile = 69;
				_DrawLowerIcon(backFile, WAITING_ICON, 0);
				updated = 1;
				rval = sfpProcess(ProcessedTmpls[c]);
				if(rval > 0) {
					rval = 4;
					FPFlag |= 0x01 << c;
				} else {
					rval = 6;
					memset(ProcessedTmpls[c], 0xff, 400);
					FPFlag &= ~(0x01 << c);
 				}
 			} else	rval = 6;
  		}
 		if(rval < 3) {
 			if(rval == GWND_ERROR) ResultMsg(WizardWnd, Title, R_MSG_SYSTEM_ERROR);
			break;
		} else if(rval == 3) {
			position--; if(position < 0) position = 1;
			_DrawEnrollBoundary(position);
		} else if(rval == 4) {
			position++; if(position > 1) position = 0;
			_DrawEnrollBoundary(position);
		} else if(rval == 5) {
			ResultMsg(WizardWnd, Title, R_DATA_MISMATCHED);
			init = 1;
		} else if(rval == 6) {
			ResultMsg(WizardWnd, Title, R_FAILED);
			init = 1;
		}
	}
	return rval;
}

void _GetUserFPTemplates(FP_USER *user)
{
	if(user) userfsGetFPTemplate(user, OldStoredTmpls);
}

void _RestoreFPTemplate(FP_USER *user, FP_USER *OldUser)
{
	unsigned long	FPID;
	int		NewFP, OldFP;
	
	NewFP = 0;
	if(userFPData1IsValid(user)) NewFP |= 1;
	if(userFPData2IsValid(user)) NewFP |= 2;
	OldFP = 0;
	if(OldUser && userFPData1IsValid(OldUser)) OldFP |= 1;
	if(OldUser && userFPData2IsValid(OldUser)) OldFP |= 2;
	userID2FPID(user->ID, &FPID);
	if(OldFP & 1) {
		if(!(NewFP & 1) || n_memcmp(OldStoredTmpls, NewStoredTmpls, 800)) userfsAddFPTemplate(FPID, OldStoredTmpls);
	} else {
		sfpRemoveTemplate(FPID);
	}
	FPID++;
	if(OldFP & 2) {
		if(!(NewFP & 2) || n_memcmp(OldStoredTmpls+800, NewStoredTmpls+800, 800)) userfsAddFPTemplate(FPID, OldStoredTmpls+800);
	} else {
		sfpRemoveTemplate(FPID);
	}
}

int _AddFPTemplate(FP_USER *user, FP_USER *OldUser)
{
	unsigned long	FPID;
	int		rval, NewFP, OldFP, count, max;
	
	NewFP = 0;
	if(userFPData1IsValid(user)) NewFP |= 1;
	if(userFPData2IsValid(user)) NewFP |= 2;
	OldFP = 0;
	if(OldUser && userFPData1IsValid(OldUser)) OldFP |= 1;
	if(OldUser && userFPData2IsValid(OldUser)) OldFP |= 2;
	if(NewFP) {
		if(sys_cfg->FPIdentify) max = GetMaxFPIdentifySize();
		else	max = GetMaxFPTemplateSize();
		count = userfsGetFPTemplateCount();
		if(!(OldFP & 1) && (NewFP & 1)) count++;
		if(!(OldFP & 2) && (NewFP & 2)) count++;
		if(count > max) return 0;
	}
	rval = 10;
	userID2FPID(user->ID, &FPID);
	if(NewFP & 1) {
		if(!(OldFP & 1) || n_memcmp(OldStoredTmpls, NewStoredTmpls, 800)) rval = userfsAddFPTemplate(FPID, NewStoredTmpls);
	} else {
		sfpRemoveTemplate(FPID);
		if(OldFP & 1) rval = 1;
	}
	FPID++;
	if(NewFP & 2) {
		if(!(OldFP & 2) || n_memcmp(OldStoredTmpls+800, NewStoredTmpls+800, 800)) rval = userfsAddFPTemplate(FPID, NewStoredTmpls+800);
	} else {
		sfpRemoveTemplate(FPID);
		if(OldFP & 2) rval = 1;
	}
	if(rval < 0) {
		//memset(NewStoredTmpls, 0xff, 1600);
		//fsGetFPTemplate(user, NewStoredTmpls);
		//_RestoreFPTemplate(user, OldUser);
		userfsRemoveFPTemplate(user);
	}
	return rval;
}
