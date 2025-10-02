void _CRCFG_Save(int crID, int offset, GMENU *menu, char *Title,  unsigned char NewVal);

#ifndef STAND_ALONE

void MenuSetupReader(GMENU *ParentMenu, char *Title, int apID);
void MenuAssignWiegandReader(GMENU *ParentMenu, char *Title, int apID);
void MenuAssignRS485Reader(GMENU *ParentMenu, char *Title, int apID);
void MenuSetupTimeAttend(GMENU *ParentMenu, char *Title);


void MenuAllReaders(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	CR_CFG	*cr_cfg;
	char	*p, *ItemText, temp[2][MAX_MSG_LEN];
	unsigned char	cVal;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
 	MenuAddItem(menu, xmenu_all_readers[0], 0, GMENU_ITEM_ENABLE);	
 	p = temp[0]; strcpy(p, xmenu_all_readers[1]); p += strlen(p); *p++ = '('; strcpy(p, xmenu_hw_config[2]); p += strlen(p); *p++ = ')'; *p = 0; 
 	MenuAddItem(menu, temp[0], 0, GMENU_ITEM_ENABLE);	
 	p = temp[1]; strcpy(p, xmenu_all_readers[1]); p += strlen(p); *p++ = '('; strcpy(p, xmenu_hw_config[3]); p += strlen(p); *p++ = ')'; *p = 0;
 	MenuAddItem(menu, temp[1], 0, GMENU_ITEM_ENABLE);
	MenuAddItem(menu, xmenu_all_readers[2], 0, GMENU_ITEM_ENABLE);	
	MenuAddItem(menu, xmenu_all_readers[3], 0, GMENU_ITEM_ENABLE);	
  	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:		MenuSetupReader(menu, ItemText, -1); break;
		case 2:		MenuAssignWiegandReader(menu, ItemText, -1); break;
		case 3:		MenuAssignRS485Reader(menu, ItemText, -1); break;
		case 4:
			cr_cfg = &sys_cfg->CredentialReaders[0];
			if(cr_cfg->TamperAlarmInhibit) cVal = 2; else cVal = 1;
			rval = MenuRadio((GWND *)ParentMenu, ItemText, &cVal, xmenu_yesno, 2, 0, 0);
			if(rval > 2) {
				if(cVal == 1) cVal = 0; else cVal = 1;
				_CRCFG_Save(-1, &cr_cfg->TamperAlarmInhibit-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			}
			break;
		case 5:		MenuSetupTimeAttend(menu, ItemText); break;
		}
	}
	MenuTerminate(menu);
}

#endif

void MenuFPReader(GMENU *ParentMenu, char *Title, int apID);
void MenuMifareReader(GMENU *ParentMenu, char *Title, int apID);
void MenuEMReader(GMENU *ParentMenu, char *Title, int apID);
void MenuWiegandReader(GMENU *ParentMenu, char *Title, int apID);
void MenuSerialReader(GMENU *ParentMenu, char *Title, int apID);
void MenuIPReader(GMENU *ParentMenu, char *Title, int apID);

void MenuSetupReader(GMENU *ParentMenu, char *Title, int apID)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval, i;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	MenuAddItem(menu, xmenu_readers[0], 0, GMENU_ITEM_ENABLE);	
	for(i = 0;i < 5;i++) MenuAddItem(menu, xmenu_card_readers[i], 0, GMENU_ITEM_ENABLE);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:		MenuFPReader(menu, ItemText, apID); break;
		case 2:		MenuMifareReader(menu, ItemText, apID); break;
		case 3:		MenuEMReader(menu, ItemText, apID); break;
		case 4:		MenuWiegandReader(menu, ItemText, apID); break;
		case 5:		MenuSerialReader(menu, ItemText, apID); break;
		case 6:		MenuIPReader(menu, ItemText, apID); break;
		}
	}
	MenuTerminate(menu);
}

void MenuFPReader(GMENU *ParentMenu, char *Title, int apID)
{
	GMENU	*menu, _menu;
	CR_CFG	*cr_cfg;
	char	*ItemText;
	unsigned char	cVal;	
	int		rval;

	if(apID < 0) cr_cfg = &sys_cfg->CredentialReaders[0];
	else	     cr_cfg = &sys_cfg->CredentialReaders[apID];
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_fp_reader, 3);		
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			cVal = cr_cfg->crFP.OSExposure;
			rval = WndInputAnalog((GWND *)menu, ItemText, &cVal, 0, 80, NULL, 0);
			if(rval > 0) _CRCFG_Save(apID, &cr_cfg->crFP.OSExposure-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			break;
		case 2:
			cVal = sys_cfg->FPSecurityLevel;
			rval = MenuRadio((GWND *)menu, ItemText, &cVal, &xmenu_security_level[1], 9, 0, 1);
			break;
		case 3:
			if(sys_cfg->FPIdentify) cVal = 1; else cVal = 2;
			rval = MenuRadio((GWND *)ParentMenu, Title, &cVal, xmenu_yesno, 2, 0, 0);
			if(rval > 0) {
				if(cVal == 1) cVal = 1; else cVal = 0;
			}
			break;
		}
	}
	MenuTerminate(menu);
}

//void MenuReadingCard(GMENU *ParentMenu, char *Title, CR_CARD *cr);
void MenuSpecialCardSetting(GMENU *ParentMenu, char *Title, int apID);
void WizardSpecialCardSetting(char *Title, int apID);

void MenuMifareReader(GMENU *ParentMenu, char *Title, int apID)
{
	GMENU	*menu, _menu;
	CR_CFG	*cr_cfg;
	char	*ItemText;
	unsigned char	cVal;
	int		rval;

	if(apID < 0) cr_cfg = &sys_cfg->CredentialReaders[0];
	else	     cr_cfg = &sys_cfg->CredentialReaders[apID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		MenuAddItem(menu, xmenu_mifare_reader[0], 0, GMENU_ITEM_ENABLE);
		cVal = cr_cfg->crMifare.Format;
		if(cVal == 1) MenuAddItem(menu, xmenu_mifare_reader[1], 0, GMENU_ITEM_ENABLE);
		MenuAddItem(menu, xmenu_mifare_reader[2], 0, GMENU_ITEM_ENABLE);
		MenuAddItem(menu, xmenu_mifare_reader[3], 0, GMENU_ITEM_ENABLE);
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);		
		if(rval > 1 && cVal != 1) rval++;
		switch(rval) {
		case 1:
			cVal = cr_cfg->crMifare.Format;
			rval = MenuRadio((GWND *)menu, ItemText, &cVal, xmenu_mifare_format, 3, 1, 0);
			if(rval > 0) _CRCFG_Save(apID, &cr_cfg->crMifare.Format-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			break;
		case 2:
			if(MenuGetAdminConsole()) MenuSpecialCardSetting(menu, ItemText, apID);
			else	WizardSpecialCardSetting(ItemText, apID);
			break;
		case 3:
			cVal = cr_cfg->crMifare.MifareType;
			rval = MenuRadio((GWND *)menu, ItemText, &cVal, xmenu_mifare_type, 3, 1, 0);
			if(rval > 0) _CRCFG_Save(apID, &cr_cfg->crMifare.MifareType-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			break;
		case 4:
//			MenuReadingCard(menu, ItemText, cr);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuSpecialCardSetting(GMENU *ParentMenu, char *Title, int apID)
{
	GMENU	*menu, _menu;
	CR_CFG	*cr_cfg;
	unsigned char	cVal, cVal2;
	char	*ItemText;
	int		rval, NewVal;

	if(apID < 0) cr_cfg = &sys_cfg->CredentialReaders[0];
	else	     cr_cfg = &sys_cfg->CredentialReaders[apID];
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_special_card, 5);		
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			cVal = cr_cfg->crMifare.DataPosition >> 2; cVal2 = cr_cfg->crMifare.DataPosition & 0x03;
			rval = WndInputNumber((GWND *)menu, ItemText, NULL, &cVal, 1, 0, 255, R_INVALID_DATA, 0);
			if(rval > 2) {
				cVal = (cVal << 2) + cVal2;		
				_CRCFG_Save(apID, &cr_cfg->crMifare.DataPosition-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			}
			break;
		case 2:
			cVal = cr_cfg->crMifare.DataPosition & 0x03; cVal2 = cr_cfg->crMifare.DataPosition >> 2; 
			rval = WndInputNumber((GWND *)menu, ItemText, NULL, &cVal, 1, 0, 3, R_INVALID_DATA, 0);
			if(rval > 2) {
				cVal = (cVal2 << 2) + cVal;
				_CRCFG_Save(apID, &cr_cfg->crMifare.DataPosition-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			}
			break;
		case 3:
			cVal = cr_cfg->crMifare.DataOffset; 
			rval = WndInputNumber((GWND *)menu, ItemText, NULL, &cVal, 1, 0, 15, R_INVALID_DATA, 0);
			if(rval > 2) _CRCFG_Save(apID, &cr_cfg->crMifare.DataOffset-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			break;
		case 4:
			if(!cr_cfg->crMifare.DataType) NewVal = 16; else NewVal = 8;
			cVal = cr_cfg->crMifare.DataLength;
			rval = WndInputNumber((GWND *)menu, ItemText, NULL, &cVal, 1, 1, NewVal, R_INVALID_DATA, 0);		
			if(rval > 2) _CRCFG_Save(apID, &cr_cfg->crMifare.DataLength-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			break;
		case 5:
			cVal = cr_cfg->crMifare.DataType;
			rval = MenuRadio((GWND *)menu, ItemText, &cVal, xmenu_data_type, 3, 1, 0);
			if(rval > 2) _CRCFG_Save(apID, &cr_cfg->crMifare.DataType-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			break;
		}
	}
	MenuTerminate(menu);
}

int  _SpecialCardDataPosition(GWND *WizardWnd, int *Wizard, unsigned char *NewOptions);
int  _SpecialCardDataType(GWND *WizardWnd, int *Wizard, unsigned char *NewOptions);
int  _SaveSpecialCardSetting(GWND *WizardWnd, char *Title, unsigned char *NewOptions, int apID);

void WizardSpecialCardSetting(char *Title, int apID)
{
	GWND	*wnd, _wnd;
	CR_MIFARE_CFG	*cr_cfg;
	unsigned char	NewOptions[8];
	int		rval, Stage, Wizard;

	cr_cfg = &sys_cfg->CredentialReaders[apID].crMifare;
	NewOptions[0] = cr_cfg->DataPosition >> 2;
	NewOptions[1] = cr_cfg->DataPosition & 0x03;
	NewOptions[2] = cr_cfg->DataOffset;
	NewOptions[3] = cr_cfg->DataLength;
	NewOptions[4] = cr_cfg->DataType;
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		if(Stage < 2) Wizard |= GWND_WIZARD_NEXT;
		if(Stage > 1) Wizard |= GWND_WIZARD_BACK;
		switch(Stage) {
		case 1:	rval = _SpecialCardDataType(wnd, &Wizard, NewOptions); break;
		case 2:	rval = _SpecialCardDataPosition(wnd, &Wizard, NewOptions); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else if(rval == 1) {
			Stage--;
		} else {
			if(Wizard & GWND_WIZARD_NEXT) Stage++;
			else {	// Final Stage
				_SaveSpecialCardSetting(wnd, Title, NewOptions, apID);
				Stage = 0;
			}
		}
		taskYield();
	}
}

int _SpecialCardDataType(GWND *WizardWnd, int *Wizard, unsigned char *NewOptions)
{
	int		rval;

	wndDrawClientArea(WizardWnd);
	rval = MenuRadioWizard(WizardWnd, xmenu_special_card[4], &NewOptions[4], xmenu_data_type, 3, 1, *Wizard);
	return rval;
}

int _SpecialCardDataPosition(GWND *WizardWnd, int *Wizard, unsigned char *NewOptions)
{
	GLABEL	*lblSectorNo, _lblSectorNo, *lblBlockNo, _lblBlockNo, *lblDataOffset, _lblDataOffset, *lblDataLength, _lblDataLength;
	GEDIT	*edtSectorNo, _edtSectorNo, *edtBlockNo, _edtBlockNo, *edtDataOffset, _edtDataOffset, *edtDataLength, _edtDataLength;
	GLABEL	*lblSectorNo2, _lblSectorNo2, *lblBlockNo2, _lblBlockNo2, *lblDataOffset2, _lblDataOffset2, *lblDataLength2, _lblDataLength2;
	RECT	rect;
	int		rval, c, x, y, w_lbl, h_lbl, w_edt, h_edt, w_lbl2, val[4];

	*Wizard &= ~GWND_WIZARD_NEXT;
	wndDrawClientArea(WizardWnd);
	lblSectorNo = &_lblSectorNo; lblBlockNo = &_lblBlockNo; lblDataOffset = &_lblDataOffset; lblDataLength = &_lblDataLength;
	edtSectorNo = &_edtSectorNo; edtBlockNo = &_edtBlockNo; edtDataOffset = &_edtDataOffset; edtDataLength = &_edtDataLength;
	lblSectorNo2 = &_lblSectorNo2; lblBlockNo2 = &_lblBlockNo2; lblDataOffset2 = &_lblDataOffset2; lblDataLength2 = &_lblDataLength2;
	w_lbl = 130; h_lbl = 24; w_edt = 42; h_edt = 26; w_lbl2 = 120;  
	x = 6; y = 20;
	labelInitialize(lblSectorNo, WizardWnd, x, y, w_lbl, h_lbl); x += w_lbl;
	editInitialize(edtSectorNo, WizardWnd,  x, y, w_edt, h_edt); x += w_edt+20;
	labelInitialize(lblSectorNo2, WizardWnd, x, y, w_lbl2, h_lbl);
	editSetFont(edtSectorNo, GWND_NORMAL_FONT);
	editLimitText(edtSectorNo, 3);
	editModifyStyle(edtSectorNo, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
	labelSetWindowText(lblSectorNo, xmenu_special_card[0]); 
	sprintf(edtSectorNo->Text, "%d", (int)NewOptions[0]);
	labelSetWindowText(lblSectorNo2, "(0-255)");

	x = 6; y += 30;
	labelInitialize(lblBlockNo, WizardWnd, x, y, w_lbl, h_lbl); x += w_lbl;
	editInitialize(edtBlockNo, WizardWnd, x, y, w_edt, h_edt); x += w_edt+20;
	labelInitialize(lblBlockNo2, WizardWnd, x, y, w_lbl2, h_lbl);
	editSetFont(edtBlockNo, GWND_NORMAL_FONT);
	editLimitText(edtBlockNo, 1);
	editModifyStyle(edtBlockNo, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
	labelSetWindowText(lblBlockNo, xmenu_special_card[1]); 
	sprintf(edtBlockNo->Text, "%d", (int)NewOptions[1]);
	labelSetWindowText(lblBlockNo2, "(0-3)"); 
	x = 6; y += 30;
	labelInitialize(lblDataOffset, WizardWnd, x, y, w_lbl, h_lbl); x += w_lbl;
	editInitialize(edtDataOffset, WizardWnd, x, y, w_edt, h_edt); x += w_edt+20;
	labelInitialize(lblDataOffset2, WizardWnd, x, y, w_lbl2, h_lbl);
	editSetFont(edtDataOffset, GWND_NORMAL_FONT);
	editLimitText(edtDataOffset, 2);
	editModifyStyle(edtDataOffset, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
	labelSetWindowText(lblDataOffset, xmenu_special_card[2]); 
	sprintf(edtDataOffset->Text, "%d", (int)NewOptions[2]);
	labelSetWindowText(lblDataOffset2, "(0-15)"); 
	x = 6; y += 30;
	labelInitialize(lblDataLength, WizardWnd, x, y, w_lbl, h_lbl); x += w_lbl;
	editInitialize(edtDataLength, WizardWnd, x, y, w_edt, h_edt); x += w_edt+20;
	labelInitialize(lblDataLength2, WizardWnd, x, y, w_lbl2, h_lbl);
	editSetFont(edtDataLength, GWND_NORMAL_FONT);
	editLimitText(edtDataLength, 2);
	editModifyStyle(edtDataLength, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
	labelSetWindowText(lblDataLength, xmenu_special_card[3]); 
	sprintf(edtDataLength->Text, "%d", (int)NewOptions[3]);
	if(!NewOptions[4]) labelSetWindowText(lblDataLength2, "(1-16)");
	else	 labelSetWindowText(lblDataLength2, "(1-8)");

	while(1) {
		labelDrawWindow(lblSectorNo); editDrawWindow(edtSectorNo); labelDrawWindow(lblSectorNo2);
		labelDrawWindow(lblBlockNo); editDrawWindow(edtBlockNo); labelDrawWindow(lblBlockNo2);
		labelDrawWindow(lblDataOffset); editDrawWindow(edtDataOffset); labelDrawWindow(lblDataOffset2); 
		labelDrawWindow(lblDataLength); editDrawWindow(edtDataLength); labelDrawWindow(lblDataLength2);
		wndGetClientArea(WizardWnd, &rect);
		x = rect.x + 130; y = rect.y + 142;
		lcdPutIconFile(x, y, 24, BTN_BACKSPACE); x += 26;
		lcdPutIconFile(x, y, 24, BTN_SHARP);
		if(NewOptions[4] != 2) {
			x += 35; lcdPutText(x, y-10, xmenu_other[7], wndGetBackGroundTextColor(WizardWnd));
		}
		rval = editProcessWindow(edtSectorNo, *Wizard);
		if(rval > 0) {
			editDrawWindow(edtSectorNo); c = edtSectorNo->LastKey;
		}
		if(rval > 0 && c == KEY_OK) {
			rval = editProcessWindow(edtBlockNo, *Wizard);
			if(rval > 0) {
				editDrawWindow(edtBlockNo); c = edtBlockNo->LastKey;
			}
		}
		if(rval > 0 && c == KEY_OK) {
			rval = editProcessWindow(edtDataOffset, *Wizard);
			if(rval > 0) {
				editDrawWindow(edtDataOffset); c = edtDataOffset->LastKey;
			}
		}
		if(rval > 0 && c == KEY_OK) {
			rval = editProcessWindow(edtDataLength, *Wizard);
			if(rval > 0) {
				editDrawWindow(edtDataLength); c = edtDataLength->LastKey;
			}
		}
		if(rval < 1) break;
		rval = 0;
		if(c == KEY_F1 || c == KEY_F4) {
			val[0] = n_atol(edtSectorNo->Text);
			val[1] = n_atol(edtBlockNo->Text);
			val[2] = n_atol(edtDataOffset->Text);
			val[3] = n_atol(edtDataLength->Text);
			if(val[0] < 0 || val[0] > 255) {
				ResultMsg(WizardWnd, xmenu_special_card[0], R_INVALID_DATA);
			} else if(val[1] < 0 || val[1] > 3) {
				ResultMsg(WizardWnd, xmenu_special_card[1], R_INVALID_DATA);
			} else if(val[2] < 0 || val[2] > 15) {
				ResultMsg(WizardWnd, xmenu_special_card[2], R_INVALID_DATA);
			} else if(val[3] < 1 || !NewOptions[4] && val[3] > 16 || NewOptions[4] && val[3] > 8) {				
				ResultMsg(WizardWnd, xmenu_special_card[3], R_INVALID_DATA);
			} else {
				NewOptions[0] = val[0];
				NewOptions[1] = val[1];
				NewOptions[2] = val[2];
				NewOptions[3] = val[3];
				if(c == KEY_F1) rval = 1; else rval = 2;
			}
		}
		if(rval) break;
		taskYield();
	}
	return rval;
}

int _SaveSpecialCardSetting(GWND *WizardWnd, char *Title, unsigned char *NewOptions, int apID)
{
	CR_MIFARE_CFG	*cr_cfg;
	unsigned char	OldOptions[MAX_AP_SZ][8];
	int		rval, i;

	if(apID < 0) {
		for(i = 0;i < MAX_AP_SZ;i++) {
			cr_cfg = &sys_cfg->CredentialReaders[i].crMifare;
			OldOptions[i][0] = cr_cfg->DataPosition >> 2;
			OldOptions[i][1] = cr_cfg->DataPosition & 0x03;
			OldOptions[i][2] = cr_cfg->DataOffset;
			OldOptions[i][3] = cr_cfg->DataLength;
			OldOptions[i][4] = cr_cfg->DataType;
		}
		for(i = 0;i < MAX_AP_SZ;i++) {
			cr_cfg = &sys_cfg->CredentialReaders[i].crMifare;
			cr_cfg->DataPosition	= (NewOptions[0] << 2) + NewOptions[1];
			cr_cfg->DataOffset		= NewOptions[2];
			cr_cfg->DataLength		= NewOptions[3];
			cr_cfg->DataType		= NewOptions[4];
		}
		for(i = 0;i < MAX_AP_SZ;i++)
			if(n_memcmp(NewOptions, OldOptions[apID], 5)) break;
		if(i < MAX_AP_SZ) {
			rval = WndSaveConfig(WizardWnd, Title, 1);
			if(rval < 0) {
				for(i = 0;i < MAX_AP_SZ;i++) {
					cr_cfg = &sys_cfg->CredentialReaders[i].crMifare;
					cr_cfg->DataPosition	= (OldOptions[i][0] << 2) + OldOptions[apID][1];
					cr_cfg->DataOffset		= OldOptions[i][2];
					cr_cfg->DataLength		= OldOptions[i][3];
					cr_cfg->DataType		= OldOptions[i][4];
				}
			}
		}		
	} else {
		cr_cfg = &sys_cfg->CredentialReaders[apID].crMifare;
		OldOptions[apID][0] = cr_cfg->DataPosition >> 2;
		OldOptions[apID][1] = cr_cfg->DataPosition & 0x03;
		OldOptions[apID][2] = cr_cfg->DataOffset;
		OldOptions[apID][3] = cr_cfg->DataLength;
		OldOptions[apID][4] = cr_cfg->DataType;
		if(n_memcmp(NewOptions, OldOptions[apID], 5)) {
			cr_cfg->DataPosition	= (NewOptions[0] << 2) + NewOptions[1];
			cr_cfg->DataOffset		= NewOptions[2];
			cr_cfg->DataLength		= NewOptions[3];
			cr_cfg->DataType		= NewOptions[4];
			rval = WndSaveConfig(WizardWnd, Title, 1);
			if(rval < 0) {
				cr_cfg->DataPosition	= (OldOptions[apID][0] << 2) + OldOptions[apID][1];
				cr_cfg->DataOffset		= OldOptions[apID][2];
				cr_cfg->DataLength		= OldOptions[apID][3];
				cr_cfg->DataType		= OldOptions[apID][4];
			} else {
#ifndef STAND_ALONE
				if(unitGetType(i) == 2) CliCRConfigChanged(apID);
				else	unitSetCruVersion(apID);
#endif
			}
		} else	rval = 0;
	}
	return rval;
}

void MenuEMReader(GMENU *ParentMenu, char *Title, int apID)
{
	GMENU	*menu, _menu;
	CR_CFG	*cr_cfg;
	char	*ItemText;
	unsigned char	cVal;
	int		rval;

	if(apID < 0) cr_cfg = &sys_cfg->CredentialReaders[0];
	else	     cr_cfg = &sys_cfg->CredentialReaders[apID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	MenuAddItem(menu, xmenu_special_card[4], 0, GMENU_ITEM_ENABLE);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			cVal = cr_cfg->crEM.DataBitLength;
			rval = MenuRadio((GWND *)menu, ItemText, &cVal, xmenu_em_format, 3, 1, 0);
			if(rval > 0) _CRCFG_Save(apID, &cr_cfg->crEM.DataBitLength-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			break;
		}
	}
	MenuTerminate(menu);	
}

void MenuWiegandReader(GMENU *ParentMenu, char *Title, int apID)
{
	GMENU	*menu, _menu;
	CR_CFG	*cr_cfg;
	char	*ItemText;
	unsigned char	cVal;
	int		rval;

	if(apID < 0) cr_cfg = &sys_cfg->CredentialReaders[0];
	else	     cr_cfg = &sys_cfg->CredentialReaders[apID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	MenuAddItem(menu, xmenu_special_card[5], 0, GMENU_ITEM_ENABLE);
	MenuAddItem(menu, xmenu_mifare_reader[4], 0, GMENU_ITEM_ENABLE);	
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1); 	
		switch(rval) {
		case 1:
			cVal = cr_cfg->crWiegand.ByteOrder;
			rval = MenuRadio((GWND *)menu, ItemText, &cVal, xmenu_byte_order, 2, 1, 0);
			if(rval > 0) _CRCFG_Save(apID, &cr_cfg->crWiegand.ByteOrder-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			break;
		case 2:
			cVal = cr_cfg->crWiegand.Format26Bit;
			rval = MenuRadio((GWND *)menu, ItemText, &cVal, xmenu_wiegand_26bit, 3, 1, 0);
			if(rval > 0) _CRCFG_Save(apID, &cr_cfg->crWiegand.Format26Bit-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuSerialReader(GMENU *ParentMenu, char *Title, int apID)
{
	GMENU	*menu, _menu;
	CR_CFG	*cr_cfg;
	char	*ItemText;
	unsigned char	cVal;
	int		rval;

	if(apID < 0) cr_cfg = &sys_cfg->CredentialReaders[0];
	else	     cr_cfg = &sys_cfg->CredentialReaders[apID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	MenuAddItem(menu, xmenu_mifare_reader[0], 0, GMENU_ITEM_ENABLE);
	MenuAddItem(menu, xmenu_special_card[4], 0, GMENU_ITEM_ENABLE);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);		
		switch(rval) {
		case 1:
			cVal = cr_cfg->crSerial.Format;
			rval = MenuRadio((GWND *)menu, ItemText, &cVal, xmenu_external_format, 3, 1, 0);
			if(rval > 0) _CRCFG_Save(apID, &cr_cfg->crSerial.Format-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			break;
		case 2:
			if(cr_cfg->crSerial.Format == 2) {
				cVal = cr_cfg->crSerial.DataBitLength;
				rval = MenuRadio((GWND *)menu, ItemText, &cVal, xmenu_em_format, 3, 1, 0);
				if(rval > 0) _CRCFG_Save(apID, &cr_cfg->crSerial.DataBitLength-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			} else {
				cVal = cr_cfg->crSerial.DataType;
				rval = MenuRadio((GWND *)menu, ItemText, &cVal, xmenu_data_type, 3, 1, 0);
				if(rval > 0) _CRCFG_Save(apID, &cr_cfg->crSerial.DataType-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			}
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuIPReaderSetting(GMENU *ParentMenu, char *Title, int apID);
void WizardIPReaderSetting(char *Title, int apID);

void MenuIPReader(GMENU *ParentMenu, char *Title, int apID)
{
	GMENU	*menu, _menu;
	CR_CFG	*cr_cfg;
	char	*ItemText;
	unsigned char	cVal;
	int		rval;

	if(apID < 0) cr_cfg = &sys_cfg->CredentialReaders[0];
	else	     cr_cfg = &sys_cfg->CredentialReaders[apID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	MenuAddItem(menu, xmenu_mifare_reader[0], 0, GMENU_ITEM_ENABLE);
	MenuAddItem(menu, xmenu_special_card[4], 0, GMENU_ITEM_ENABLE);
	if(apID >= 0) MenuAddItem(menu, xmenu_ip_reader[0], 0, GMENU_ITEM_ENABLE);

	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			cVal = cr_cfg->crIP.Format;
			rval = MenuRadio((GWND *)menu, ItemText, &cVal, xmenu_external_format, 3, 1, 0);
			if(rval > 0) _CRCFG_Save(apID, &cr_cfg->crIP.Format-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			break;
		case 2:
			if(cr_cfg->crIP.Format == 2) {
				cVal = cr_cfg->crIP.DataBitLength;
				rval = MenuRadio((GWND *)menu, ItemText, &cVal, xmenu_em_format, 3, 1, 0);
				if(rval > 0) _CRCFG_Save(apID, &cr_cfg->crIP.DataBitLength-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			} else {
				cVal = cr_cfg->crIP.DataType;
				rval = MenuRadio((GWND *)menu, ItemText, &cVal, xmenu_data_type, 3, 1, 0);
				if(rval > 0) _CRCFG_Save(apID, &cr_cfg->crIP.DataType-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			}
			break;
		case 3:
			if(MenuGetAdminConsole()) MenuIPReaderSetting(menu, ItemText, apID);
			else	WizardIPReaderSetting(ItemText, apID);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuIPReaderSetting(GMENU *ParentMenu, char *Title, int apID)
{
	GMENU	*menu, _menu;
	CR_IP_CFG	*cr_cfg;
	char	*ItemText;
	int		rval;

	cr_cfg = &sys_cfg->CredentialReaders[apID].crIP;
	menu = &_menu;
	MenuInitialize(menu, Title, &xmenu_ip_reader[1], 2);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			WndInputIPAddress((GWND *)menu, ItemText, cr_cfg->ReaderIPAddress, 0, 1);
			break;
		case 2:
			WndInputNumber((GWND *)menu, ItemText, NULL, &cr_cfg->ReaderPort, 2, 1, 65536, R_INVALID_SERVER_PORT, 1);
			break;
		}
	}
	MenuTerminate(menu);
}

int _ProcessServerSetup(GWND *WizardWnd, int *Wizard, unsigned char *IPAddr, unsigned short *Port, char *MsgIPAddr, char *MsgPort);
int _SaveIPReaderSetting(GWND *WizardWnd, char *Title, unsigned char *NewIPAddr, unsigned short NewPort, int apID);

void WizardIPReaderSetting(char *Title, int apID)
{
	GWND		*wnd, _wnd;
	CR_IP_CFG	*cr_cfg;
	unsigned char	NewIPAddr[4];
	unsigned short	NewPort;
	int		rval, Stage, Wizard;

	cr_cfg = &sys_cfg->CredentialReaders[apID].crIP;
	memcpy(NewIPAddr, cr_cfg->ReaderIPAddress, 4);
	NewPort = cr_cfg->ReaderPort;
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		switch(Stage) {
		case 1:	rval = _ProcessServerSetup(wnd, &Wizard, NewIPAddr, &NewPort, xmenu_ip_reader[1], xmenu_ip_reader[2]); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else {
			_SaveIPReaderSetting(wnd, Title, NewIPAddr, NewPort, apID);
			Stage = 0;
		}
		taskYield();
	}
}

void _CRCFG_Save(int crID, int offset, GMENU *menu, char *Title,  unsigned char NewVal)
{
	CR_CFG	*cr_cfg;
	unsigned char	*p, OldVals[MAX_CR_SZ];
	int		rval, i, cnt;

	if(crID < 0) {
		for(i = 0;i < MAX_CR_SZ;i++) {
			p = (unsigned char *)&sys_cfg->CredentialReaders[i];
			OldVals[i] = *(p+offset);
		}
		for(i = cnt = 0;i < MAX_CR_SZ;i++) {
			cr_cfg = &sys_cfg->CredentialReaders[i];
			p = (unsigned char *)cr_cfg;
			if(NewVal != OldVals[i]) {
#ifndef STAND_ALONE
				cr_cfg->Version++; if(!cr_cfg->Version) cr_cfg->Version = 1;
#endif
				*(p+offset) = NewVal;
				cnt++;
			}
		}
		if(cnt) {
			rval = WndSaveConfig((GWND *)menu, Title, 1);
			if(rval < 0) {
				for(i = 0;i < MAX_CR_SZ;i++) { 
					cr_cfg = &sys_cfg->CredentialReaders[i];
					p = (unsigned char *)cr_cfg;
					if(NewVal != OldVals[i]) {
						*(p+offset) = OldVals[crID];
#ifndef STAND_ALONE
						cr_cfg->Version--; if(!cr_cfg->Version) cr_cfg->Version = 255;
#endif
					}
				}
			} else {
#ifndef STAND_ALONE
				for(i = 0;i < MAX_CR_SZ;i++)
					if(NewVal != OldVals[i]) {
						if(unitGetType(i) == 2) CliCRConfigChanged(i);
						else	unitSetCruVersion(i);
					}
#endif
			}
		}
	} else {
		cr_cfg = &sys_cfg->CredentialReaders[crID];
		p = (unsigned char *)cr_cfg;
		OldVals[crID] = *(p+offset);
		if(NewVal != OldVals[crID]) {
			*(p+offset) = NewVal;
#ifndef STAND_ALONE
			cr_cfg->Version++; if(!cr_cfg->Version) cr_cfg->Version = 1;
#endif
			rval = WndSaveConfig((GWND *)menu, Title, 1);
			if(rval < 0) {
				*(p+offset) = OldVals[crID];
#ifndef STAND_ALONE
				cr_cfg->Version--; if(!cr_cfg->Version) cr_cfg->Version = 255;
#endif
			}
#ifndef STAND_ALONE
			if(unitGetType(crID) == 2) CliCRConfigChanged(crID);
			else	unitSetCruVersion(crID);
#endif
		}
	}
}

int _SaveIPReaderSetting(GWND *WizardWnd, char *Title, unsigned char *NewIPAddr, unsigned short NewPort, int apID)
{
	CR_IP_CFG	*cr_cfg;
	unsigned char	OldIPAddr[4];
	unsigned short	OldPort;
	int		rval;

	cr_cfg = &sys_cfg->CredentialReaders[apID].crIP;
	memcpy(OldIPAddr, cr_cfg->ReaderIPAddress, 4);
	OldPort = cr_cfg->ReaderPort;
	if(NewPort != OldPort || n_memcmp(NewIPAddr, OldIPAddr, 4)) {
		memcpy(cr_cfg->ReaderIPAddress, NewIPAddr, 4);
		cr_cfg->ReaderPort = NewPort;
		rval = WndSaveConfig(WizardWnd, Title, 1);
		if(rval < 0) { 
			memcpy(cr_cfg->ReaderIPAddress, OldIPAddr, 4);
			cr_cfg->ReaderPort = OldPort;
		}
	} else	rval = 0;
	return rval;
}
