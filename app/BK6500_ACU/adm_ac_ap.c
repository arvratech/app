void MenuAccessPoint(GMENU *ParentMenu, char *Title, int adID, int apID);

void MenuAccessPoints(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[32];
	int		rval, i, Item, IDs[MAX_AP_SZ];

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {	
		MenuClearItems(menu);
		for(Item = i = 0;i < MAX_AP_SZ;i++) {
			if(apIsEnable(i) && apGetAssignedDoor(i) < 0) {
				GetAccessPointName(i, temp);
				MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
				IDs[Item] = i; Item++;
			}
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		MenuAccessPoint(menu, ItemText, -1, IDs[rval-1]);
	}
	MenuTerminate(menu);
}

void _CRCFG_Save(int crID, int offset, GMENU *menu, char *Title,  unsigned char NewVal);

#ifndef STAND_ALONE

void MenuSetupReader(GMENU *ParentMenu, char *Title, int apID);
void MenuWiegandReader(GMENU *ParentMenu, char *Title, int apID);
void MenuAssignWiegandReader(GMENU *ParentMenu, char *Title, int apID);
void MenuAssignRS485Reader(GMENU *ParentMenu, char *Title, int apID);
void MenuPassback(GMENU *ParentMenu, char *Title, int apID);

// adID=-1 
void MenuAccessPoint(GMENU *ParentMenu, char *Title, int adID, int apID)
{
	GMENU	*menu, _menu;
	ACCESS_POINT_CFG	*ap_cfg;
	CR_CFG	*cr_cfg;
	char	*ItemText;
	unsigned char	cVal;
	int		rval, i;

	ap_cfg = &sys_cfg->AccessPoints[apID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		for(i = 0;i < 3;i++) MenuAddItem(menu, xmenu_all_readers[i], 0, GMENU_ITEM_ENABLE);
		MenuAddItem(menu, xmenu_access_point[0], 0, GMENU_ITEM_ENABLE);	
		if(unitGetType(apID) == 2) MenuAddItem(menu, xmenu_armed[0], 0, GMENU_ITEM_ENABLE);	
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			if(unitGetType(apID) == 2 || crGetType(apID)) MenuSetupReader(menu, ItemText, apID);
			else	MenuWiegandReader(menu, ItemText, apID);
			break;
		case 2:
			if(unitGetType(apID) == 2 || crGetType(apID)) MenuAssignRS485Reader(menu, ItemText, apID);
			else	MenuAssignWiegandReader(menu, ItemText, apID);
			break;
		case 3:
			cr_cfg = &sys_cfg->CredentialReaders[apID];
			if(cr_cfg->TamperAlarmInhibit) cVal = 2; else cVal = 1;
			rval = MenuRadio((GWND *)ParentMenu, ItemText, &cVal, xmenu_yesno, 2, 0, 0);
			if(rval > 2) {
				if(cVal == 1) cVal = 0; else cVal = 1;
				_CRCFG_Save(apID, &cr_cfg->TamperAlarmInhibit-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			}
			break;
		case 4:
			MenuRadio((GWND *)menu, ItemText, &ap_cfg->AuthorizationMode, xmenu_authorization_mode, 4, 0, 1);
			break;
		case 5:
			cr_cfg = &sys_cfg->CredentialReaders[apID];
			if(cr_cfg->IntrusionMode) cVal = 1; else cVal = 2;
			rval = MenuRadio((GWND *)ParentMenu, ItemText, &cVal, xmenu_yesno, 2, 0, 0);
			if(rval > 2) {
				if(cVal == 1) cVal = 1; else cVal = 0;
				_CRCFG_Save(apID, &cr_cfg->IntrusionMode-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			}		
		}
	}
	MenuTerminate(menu);
}

#else

void MenuSetupReader(GMENU *ParentMenu, char *Title, int apID);
void MenuAssignRS485Reader(GMENU *ParentMenu, char *Title, int apID);
void MenuOperationMode(GMENU *ParentMenu, char *Title);

void MenuAccessPoint(GMENU *ParentMenu, char *Title, int adID, int apID)
{
	GMENU	*menu, _menu;
	ACCESS_POINT_CFG	*ap_cfg;
	CR_CFG	*cr_cfg;
	char	*ItemText;
	unsigned char	cVal;
	int		rval, i;

	ap_cfg = &sys_cfg->AccessPoints[apID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	MenuClearItems(menu);
	for(i = 0;i < 3;i++) MenuAddItem(menu, xmenu_all_readers[i], 0, GMENU_ITEM_ENABLE);
	MenuAddItem(menu, xmenu_all_readers[5], 0, GMENU_ITEM_ENABLE);
	MenuAddItem(menu, xmenu_access_point[0], 0, GMENU_ITEM_ENABLE);	
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1: MenuSetupReader(menu, ItemText, apID); break;
		case 2: MenuAssignRS485Reader(menu, ItemText, apID); break;
		case 3:
			cr_cfg = &sys_cfg->CredentialReaders[apID];
			if(cr_cfg->TamperAlarmInhibit) cVal = 2; else cVal = 1;
			rval = MenuRadio((GWND *)ParentMenu, ItemText, &cVal, xmenu_yesno, 2, 0, 0);
			if(rval > 2) {
				if(cVal == 1) cVal = 0; else cVal = 1;
				_CRCFG_Save(apID, &cr_cfg->TamperAlarmInhibit-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			}
			break;
		case 4: MenuOperationMode(menu, ItemText); break;
		case 5:
			MenuRadio((GWND *)menu, ItemText, &ap_cfg->AuthorizationMode, xmenu_authorization_mode, 4, 0, 1);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuSetupTimeAttend(GMENU *ParentMenu, char *Title);
void WizardSetupMeals(char *Title);

void MenuOperationMode(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	unsigned char	OldVal, NewVal;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		MenuAddItem(menu, xmenu_op_mode[0], 0, GMENU_ITEM_ENABLE);
		if(sys_cfg->OperationMode == 4) OldVal = 4; else OldVal = 3;
		MenuAddItem(menu, xmenu_all_readers[OldVal], 0, GMENU_ITEM_ENABLE);
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			if(sys_cfg->OperationMode == 4) OldVal = 1; else OldVal = 0;
			NewVal = OldVal;
			MenuRadio((GWND *)menu, ItemText, &NewVal, &xmenu_op_mode[1], 2, 1, 0);
			if(OldVal != NewVal) {
				if(NewVal) NewVal = 4; else NewVal = 1;
				rval = WndSaveByte((GWND *)menu, ItemText, &sys_cfg->OperationMode, (int)NewVal, 1);	// pVal, NewVal, Section
				InitOperationMode();
			}
			break;
		case 2:
			//if(sys_cfg->OperationMode == 4) WizardSetupMeals(ItemText);
			//else	MenuSetupTimeAttend(menu, ItemText);
			MenuSetupTimeAttend(menu, ItemText);
			break;
		}
	}
	MenuTerminate(menu);	
}

#endif

void MenuAssignWiegandReader(GMENU *ParentMenu, char *Title, int apID)
{
	CR_CFG	*cr_cfg;
	unsigned char	cVal, tmpVal;
	int		rval, enable;

	if(apID < 0) cr_cfg = &sys_cfg->CredentialReaders[0];
	else	     cr_cfg = &sys_cfg->CredentialReaders[apID];
	if(crGetWiegandReaderEnable(cr_cfg)) cVal = 0x01; else cVal = 0x00;
	if(crGetPINReaderEnable(cr_cfg)) cVal |= 0x02;
	rval = MenuCheck((GWND *)ParentMenu, Title, &cVal, xmenu_wiegand_reader, 2, 0);
	if(rval > 0) {
		tmpVal = cr_cfg->Reader;
		if(cVal & 0x01) enable = 1; else enable = 0;
		crSetWiegandReaderEnable(cr_cfg, enable);
		if(cVal & 0x02) enable = 1; else enable = 0;
		crSetPINReaderEnable(cr_cfg, enable);
		cVal = cr_cfg->Reader; cr_cfg->Reader = tmpVal;
		_CRCFG_Save(apID, &cr_cfg->Reader-(unsigned char *)cr_cfg, ParentMenu, Title, cVal);
	}
}

void MenuAssignRS485Reader(GMENU *ParentMenu, char *Title, int apID)
{
	GMENU	*menu, _menu;
	CR_CFG	*cr_cfg;
	char	*ItemText;
	unsigned char	cVal, tmpVal, mask1, mask2;
	int		rval, i, enable;

	if(apID < 0) cr_cfg = &sys_cfg->CredentialReaders[0];
	else	     cr_cfg = &sys_cfg->CredentialReaders[apID];
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_readers, 3);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			if(crGetFPReaderEnable(cr_cfg)) cVal = 0x01; else cVal = 0x00;
			rval = MenuCheck((GWND *)menu, ItemText, &cVal, xmenu_readers, 1, 0);
			if(rval > 0) {
				tmpVal = cr_cfg->Reader;
				if(cVal & 0x01) enable = 1; else enable = 0;
				crSetFPReaderEnable(cr_cfg, enable);
				cVal = cr_cfg->Reader; cr_cfg->Reader = tmpVal;
				_CRCFG_Save(apID, &cr_cfg->Reader-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			}
			break;
		case 2:
			if(crGetPINReaderEnable(cr_cfg)) cVal = 0x01; else cVal = 0x00;
			rval = MenuCheck((GWND *)menu, ItemText, &cVal, &xmenu_readers[1], 1, 0);
			if(rval > 0) {
				tmpVal = cr_cfg->Reader;
				if(cVal & 0x01) enable = 1; else enable = 0;
				crSetPINReaderEnable(cr_cfg, enable);
				cVal = cr_cfg->Reader; cr_cfg->Reader = tmpVal;
				_CRCFG_Save(apID, &cr_cfg->Reader-(unsigned char *)cr_cfg, menu, ItemText, cVal);
			}
			break;
		case 3:
			cVal = 0;
			for(i = 0, mask1=0x04, mask2=0x01;i < 5;i++, mask1<<=1, mask2<<=1) if(cr_cfg->Reader & mask1) cVal |= mask2;
			rval = MenuCheck((GWND *)menu, ItemText, &cVal, xmenu_card_readers, 5, 0);
			if(rval > 0) {
				tmpVal = cr_cfg->Reader & 0x03;
				for(i = 0, mask1=0x04, mask2=0x01;i < 5;i++, mask1<<=1, mask2<<=1) if(cVal & mask2) tmpVal |= mask1;
				_CRCFG_Save(apID, &cr_cfg->Reader-(unsigned char *)cr_cfg, menu, ItemText, tmpVal);
			}
			break;
		}
	}
	MenuTerminate(menu);
}
