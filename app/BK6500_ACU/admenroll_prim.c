int _EnrollAccessMode(GWND *WizardWnd, int Wizard, char *Title, FP_USER *user)
{
	char	*xmenuStr[3];
	unsigned char	cVal;
	int		rval;

	xmenuStr[0] = xmenu_access_mode[1];
	xmenuStr[1] = xmenu_access_mode[2];
	xmenuStr[2] = xmenu_access_mode[4];
	wndDrawClientArea(WizardWnd);
	cVal = 0;
	if(user->AccessMode & USER_CARD)	 cVal |= 0x01;
	if(user->AccessMode & USER_PIN)		 cVal |= 0x02;
	if(user->AccessMode & USER_CARD_PIN) cVal |= 0x04;
	rval = MenuCheckWizard(WizardWnd, Title, &cVal, xmenuStr, 3, Wizard);
	if(rval > 0) {
		user->AccessMode = 0;
		if(cVal & 0x01) user->AccessMode |= USER_CARD;
		if(cVal & 0x02) user->AccessMode |= USER_PIN;
		if(cVal & 0x04) user->AccessMode |= USER_CARD_PIN;
	}	
	return rval;
}

#define CHECK_ICON_SZ	20

// Return	-2=Error  -1=Timeout  0=Cancelled  1=Continue  2=Back Key  3=Next key
int _EnrollCardStage(GWND *WizardWnd, int Wizard, char *Title, FP_USER *user)
{
#ifdef STAND_ALONE
	CREDENTIAL_READER	*cr;
#endif
	FP_USER		*user2, _user2;
	unsigned char	CardData[40];
	unsigned long	timer;
	int		c, x, y, fh, rval, ment;

#ifdef STAND_ALONE
	cr = &_gCredentialReaders[0];
#endif
	if(Wizard & GWND_WIZARD_BACK) x = BTN_BACK; else x = 0;
	if(Wizard & GWND_WIZARD_NEXT) y = BTN_NEXT; else y = 0;
	_DrawLowerButtons(x, 0, 0, y, 0);
	wndDrawClientArea(WizardWnd);
	fh = GetFontHeight() + 4;
	x = WizardWnd->x+8;
	y = 27;
	x += lcdPutText(x, y, Title, wndGetBackGroundTextColor(WizardWnd));  
	user2 = &_user2;
#ifndef STAND_ALONE
	ConsoleCatptureCardStart();
#endif
	while(1) {
		if(user->CardData[0]) c = BTN_CHECK_ON; else c = BTN_CHECK_OFF;
		lcdFillRect(x, y, CHECK_ICON_SZ, CHECK_ICON_SZ, wndGetBackGroundColor(WizardWnd));
		lcdPutIconFile(x, y, CHECK_ICON_SZ, c);
		if(MenuGetAdminConsole()) {
			PromptMsg(WizardWnd, Title, M_TOUCH_YOUR_CARD); cPutText("\r\n"); if(menu_errno) return GWND_ERROR;
		} else {
			PromptMsg(WizardWnd, Title, M_TOUCH_YOUR_CARD);
		}
		ment = GetCapturePromptMent(USER_CARD);
		if(ment) MentUserNormal(ment);
		timer = DS_TIMER + GWND_TIMER;
		while(1) {
			if(DS_TIMER >= timer) rval = GWND_TIMEOUT;
			else {
#ifdef STAND_ALONE
				crOpenCloseCard(cr);
				cr->Data[0] = 0;
				rval = crPeekCard(cr);
				if(rval > 0) {
					rval = crReadCard(cr);
					if(rval > 1) memcpy(CardData, cr->Data, 9);
				}
#else
				rval = ConsoleCaptureCard(CardData);
#endif
				if(rval > 0) rval += 10;
				else {
					if(MenuGetAdminConsole()) {
						cUsbWriteNull();
						if(menu_errno) return GWND_ERROR;
						c = cGetc();
						if(menu_errno) return GWND_ERROR;
					} else {
						if(menuGetGlobalExit()) c = KEY_CANCEL;
						else	c = keyGet();
					}
					if(c == KEY_CANCEL) rval = GWND_QUIT;
					else if((Wizard & GWND_WIZARD_BACK) && c == KEY_F1) rval = 1;
					else if((Wizard & GWND_WIZARD_NEXT) && (c == KEY_F4 || c == KEY_OK)) rval = 2;
					else	rval = 99;
				}
			}
			if(rval < 99) break;
			taskYield();
		}
		if(rval < 10) break;
		if(rval < 11) {
			ResultMsg(WizardWnd, Title, R_FAILED);			
		} else if(rval < 19) {
			ResultMsg(WizardWnd, Title, R_INVALID_DATA);
		} else if(rval > 19) {
			ResultMsg(WizardWnd, Title, R_TOUCH_SINGLE_CARD);	
		} else {
			rval = 0;
			//userfsSeek(0L);
			rval = userfsGetCardData(user2, CardData);
			if(rval > 0 && user->ID == user2->ID) {
				rval = userfsGetCardData(user2, CardData);
				//rval = userfsGetCountCardData(CardData);
				if(rval == 1) rval = 0;
			}
			if(rval) ResultMsg(WizardWnd, Title, R_CARD_DATA_OVERLAPPED);
			else {
				memcpy(user->CardData, CardData, 9);
				user->CardStatus = 1;
			}
		}
	}
#ifndef STAND_ALONE
	ConsoleCatptureCardEnd();	
#endif
	return rval;
}

// Return	-2=Error  -1=Timeout  0=Cancelled  1=Back Key  2=Next key
int _EnrollPINStage(GWND *WizardWnd, int Wizard, char *Title, FP_USER *user)
{
	GWND		*wndInput, _wndInput;
	GLABEL		*lblInput1, _lblInput1, *lblInput2, _lblInput2;
	GEDIT		*edtInput1, _edtInput1, *edtInput2, _edtInput2;
	char	temp[12];
	int		rval, c, x, y, fh, len;

	if(Wizard & GWND_WIZARD_BACK) x = BTN_BACK; else x = 0;
	if(Wizard & GWND_WIZARD_NEXT) y = BTN_NEXT; else y = BTN_OK;	// ?
	_DrawLowerButtons(x, 0, 0, y, 0);
	wndDrawClientArea(WizardWnd);
	fh = GetFontHeight() + 4;
	x = WizardWnd->x+8;
	y = 27;
	x += lcdPutText(x, y, Title, wndGetBackGroundTextColor(WizardWnd)); 
	wndInput = &_wndInput;
	lblInput1 = &_lblInput1; lblInput2 = &_lblInput2;
	edtInput1 = &_edtInput1; edtInput2 = &_edtInput2;
	wndInitialize(wndInput, NULL, 64, 52, 192, 148); 
	wndSetParentWnd(wndInput, WizardWnd);
	wndSetWindowText(wndInput, Title);		// Title
	labelInitialize(lblInput1, wndInput, 10,  6, 150, fh); 
	editInitialize(edtInput1,  wndInput, 40, 31, 128, fh+2);
	editSetFont(edtInput1, GWND_NORMAL_FONT);
	editLimitText(edtInput1, 8);
	editModifyStyle(edtInput1, ES_READONLY|ES_WILDDIGIT, ES_PASSWORD|ES_NUMBER);
	labelSetWindowText(lblInput1, xmsgs[M_ENTER_PASSWORD]);
	bcd2string(user->PIN, 8, temp);
	strcpy(edtInput1->Text, temp);
	labelInitialize(lblInput2, wndInput, 10, 62, 150, fh); 
	editInitialize(edtInput2,  wndInput, 40, 87, 128, fh+2);
	editSetFont(edtInput2, GWND_NORMAL_FONT);
	editLimitText(edtInput2, 8);
	editModifyStyle(edtInput2, ES_READONLY|ES_WILDDIGIT, ES_PASSWORD|ES_NUMBER);
	labelSetWindowText(lblInput2, xmsgs[M_VERIFY_PASSWORD]); 
	bcd2string(user->PIN, 8, temp);
	strcpy(edtInput2->Text, temp);

	wndDrawWindow(wndInput);
	labelDrawWindow(lblInput1);
	editDrawWindow(edtInput1);
	labelDrawWindow(lblInput2);
	editDrawWindow(edtInput2);

	while(1) {
		if(user->PIN[0]) rval = BTN_CHECK_ON; else rval = BTN_CHECK_OFF;
		lcdFillRect(x, y, CHECK_ICON_SZ, CHECK_ICON_SZ, wndGetBackGroundColor(WizardWnd));
		lcdPutIconFile(x, y, CHECK_ICON_SZ, rval);
		c = 0;	
		rval = editProcessWindow(edtInput1, Wizard);
		if(rval > 0) c = edtInput1->LastKey;
		if(rval > 0 && edtInput1->LastKey == KEY_OK) {
			rval = editProcessWindow(edtInput2, Wizard);
			if(rval > 0) c = edtInput2->LastKey;
		}
		if(rval < 1) break;
		rval = 0;
		if(c == KEY_F1 || c == KEY_F4) {
			len = strlen(edtInput1->Text);
			if(!len && strlen(edtInput2->Text) || len && len < 4) ResultMsg(WizardWnd, Title, R_INVALID_DATA);
			else if(len && strcmp(edtInput1->Text, edtInput2->Text)) ResultMsg(WizardWnd, Title, R_DATA_MISMATCHED);
			else {
				string2bcd(edtInput1->Text, 8, user->PIN);
				if(c == KEY_F1) rval = 1; else rval = 2;
			}
			if(!rval) {
				wndDrawWindow(wndInput);
				labelDrawWindow(lblInput1);
				editDrawWindow(edtInput1);
				labelDrawWindow(lblInput2);
				editDrawWindow(edtInput2);
			}
		}
		if(rval) break;
		taskYield();
	}
	return rval;
}

// Return	-2=Error  -1=Timeout  0=Cancelled  1=Continue  2=Back Key  3=Next key
int _EnrollFinalStage(GWND *WizardWnd, int Wizard, char *Title, FP_USER *user)
{
	RECT	rect;
	unsigned long	timer;
	int		c, x, y, fh, BtnFile, Color, timeout, contflg;

	wndDrawClientArea(WizardWnd);
	Color = wndGetBackGroundTextColor(WizardWnd);
	wndGetClientArea(WizardWnd, &rect);
	fh = GetFontHeight() + 4;
	y = rect.y + 10;
	x = rect.x + 16;
	lcdPutText(x, y, xmenu_user_enroll[1], Color); y += fh;
	x = rect.x + 32;
	if(user->AccessMode & USER_CARD) c = BTN_CHECK_ON; else c = BTN_CHECK_OFF;
	lcdPutIconFile(x, y, CHECK_ICON_SZ, c); x += fh-2;
	x += lcdPutText(x, y, xmenu_access_mode[1], Color);
	if(user->AccessMode & USER_PIN) c = BTN_CHECK_ON; else c = BTN_CHECK_OFF;
	x += 16;
	lcdPutIconFile(x, y, CHECK_ICON_SZ, c); x += fh-2;
	x += lcdPutText(x, y, xmenu_access_mode[2], Color); y += fh;

	x = rect.x + 32;
	if(user->AccessMode & USER_CARD_PIN) c = BTN_CHECK_ON; else c = BTN_CHECK_OFF;
	lcdPutIconFile(x, y, CHECK_ICON_SZ, c); x += fh-2;
	x += lcdPutText(x, y, xmenu_access_mode[4], Color); y += fh;
	
	BtnFile = BTN_OK;
	if(user->AccessMode == 0) BtnFile = 0;
	if(userIsCard(user)) {
		x = rect.x + 16;
		lcdPutText(x, y, xmenu_user_enroll[4], Color); x += 80;
		if(user->CardData[0]) c = BTN_CHECK_ON; else { c = BTN_CHECK_OFF; BtnFile = 0; }
		lcdPutIconFile(x, y, CHECK_ICON_SZ, c); y += fh+2;
	}
	if(userIsPIN(user)) {
		x = rect.x + 16;
		lcdPutText(x, y, xmenu_user_enroll[5], Color); x += 80;
		if(user->PIN[0]) c = BTN_CHECK_ON; else { c = BTN_CHECK_OFF; BtnFile = 0; }
		lcdPutIconFile(x, y, CHECK_ICON_SZ, c); y += fh+2;
	}
	if(Wizard & GWND_WIZARD_BACK) c = BTN_BACK; else c = 0;
	_DrawLowerButtons(c, 0, 0, BtnFile, 0);
	timer = DS_TIMER; timeout = GWND_TIMER;
	contflg = 1;
	while(contflg) {
		//if(menuGetGlobalExit()) c = KEY_CANCEL;
		//else	c = keyGet();
		c = keyGet();
		if(!c) {
			if((DS_TIMER-timer) >= timeout) c = -1;
		} else {
			timer = DS_TIMER;
		}
		switch(c) {
		case -1:
			c = GWND_TIMEOUT; contflg = 0;
			break;
		case KEY_CANCEL:
			c = GWND_QUIT; contflg = 0;
			break;
		case KEY_F1:
			if(Wizard & GWND_WIZARD_BACK) {
				c = 1; contflg = 0;
			} else if(!(Wizard & GWND_WIZARD) && menuGetGlobalExit()) {
				c = GWND_QUIT; contflg = 0;
			}
			break;
		case KEY_OK:
		case KEY_F4:
			if(BtnFile) {
				c = 2; contflg = 0;
			} 
			break;
		}
		taskYield();
	}
	return c;
}
