void WizardSetupUnits(char *Title);
void MenuViewUnits(char *Title);
void MenuViewUnitsInOut(char *Title);
void MenuViewUnitsIPAddress(char *Title);
void WizardSetupIOUnits(char *Title);
void MenuViewIOUnits(char *Title);

void MenuHardwareConfiguration(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_hw_setup, 6);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1: WizardSetupUnits(ItemText); break;
		case 2: MenuViewUnits(ItemText); break;
		case 3: MenuViewUnitsInOut(ItemText); break;
		case 4: MenuViewUnitsIPAddress(ItemText); break;
		case 5: WizardSetupIOUnits(ItemText); break;
		case 6: MenuViewIOUnits(ItemText); break;
		}
	}
	MenuTerminate(menu);
}

#define HW_ICON_SZ		20

int  _SaveUnitsConfig(GWND *WizardWnd, char *Title, UNITS_CFG *NewUnits);
void _PutUnitAllItem(GMENU *menu, int Item, int x, int y);
void _PutUnitTitle(GMENU *menu);
int  _ProcessUnitConfig(GWND *WizardWnd, int *Wizard, GMENU *menu, UNITS_CFG *NewUnits);

void WizardSetupUnits(char *Title)
{
	UNITS_CFG	*NewUnits, _NewUnits;
	GWND	*wnd, _wnd;
	GMENU	*menu, _menu;
	int		rval, Stage, Wizard;

	NewUnits = &_NewUnits;
	unitscfgSet(NewUnits);
	syscfgCopyToUnits(sys_cfg);
	syscfgSetUnits(sys_cfg);	
	menu = &_menu;
	MenuInitialize(menu, NULL, NULL, MAX_UNIT_SZ >> 1);
	menuSetPutItemCallBack(menu, _PutUnitAllItem);
	menuSetPutTitleCallBack(menu, _PutUnitTitle);
	menuSetContext(menu, NewUnits);
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		switch(Stage) {
		case 1:	rval = _ProcessUnitConfig(wnd, &Wizard, menu, NewUnits); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else {
			_SaveUnitsConfig(wnd, Title, &_NewUnits);
			Stage = 0;
		}
		taskYield();
	}
}

int _GetUnitNode(int UnitID)
{
	int		ID, enable, file;

	if(unitGetType(UnitID) == 2) {
		if(unitGetEnable(UnitID)) file = READER_ICON;
		else	file = READER_DISABLE_ICON;
	} else {
		if(UnitID & 1) ID = UnitID - 1; else ID = UnitID + 1;
		if(unitGetEnable(UnitID) || unitGetEnable(ID)) enable = 1;
		else	enable = 0;
		if(UnitID < 2) file = MASTER_ICON; // local
		else if(enable) file = UNIT_ICON;
		else	file = UNIT_DISABLE_ICON;
	}
	return file;
}

int _GetDoorNode(int UnitID)
{
	int		file;

	if(adGetEnable(UnitID)) file = DOOR_ICON;
	else	file = DOOR_DISABLE_ICON;
	return file;
}

int _GetPointNode(int UnitID, char *Text)
{
	int		type, enable, file;

	if(unitGetType(UnitID) == 2) {
		strcpy(Text, xmenu_hw_config[4]);
		if(crIsEnable(UnitID)) file = READER_ICON;
		else	file = READER_DISABLE_ICON;
	} else {
		type = crGetType(UnitID);
		enable = crIsEnable(UnitID);
		if(type == 0) {		// Wiegand
			strcpy(Text, xmenu_hw_config[2]);
			if(enable) file = WIEGAND_ICON; else file = WIEGAND_DISABLE_ICON;
		} else {			// RS-485, IP
			strcpy(Text, xmenu_hw_config[3]);
			if(enable) file = READER_ICON; else file = READER_DISABLE_ICON;	
		}
	}
	return file;
}

int _PutNodeLabel(int UnitID, int x, int y, int fColor)
{
	char	temp[4];

	if(UnitID < 2) temp[0] = 'L';
	else 	temp[0] = (UnitID >> 1) - 1 + '0';
	temp[1] = 0;
	lcdPutText(x, y+1, temp, fColor);	// L=12 0-9=11
	return 12;
}

void _PutUnitItem(int UnitID, int x, int y, int fColor)
{
	int		file;

	x += _PutNodeLabel(UnitID, x, y, fColor); x += 3;
	file = _GetUnitNode(UnitID);
	lcdPutIconFile(x, y+1, HW_ICON_SZ, file);
}

void _PutDoorItem(int UnitID, int x, int y, int fColor)
{
	int		file;

	file = _GetDoorNode(UnitID);
	lcdPutIconFile(x, y+1, HW_ICON_SZ, file); x += 24;
}

void _PutPointItem(int UnitID, int x, int y, int fColor)
{
	char	temp[20];
	int		file;

	file = _GetPointNode(UnitID, temp);
	lcdPutIconFile(x, y+1, HW_ICON_SZ, file); x += 22;
	lcdPutText(x, y+1, temp, fColor);
}

void _PutUnitTitle(GMENU *menu)
{
	char	*p;
	int		x, y, w, size;

	y = menu->y + 2;
	p = xmenu_hw_config[0];
	size = 34; w = GetStringWidth(p);
	x = 22;
	lcdPutText(x+((size-w)>>1), y, p, (int)menu->TitleBarTextColor);
	p = xmenu_hw_config[9];
	x = 56 + 20;
	lcdPutText(x, y, p, (int)menu->TitleBarTextColor);
	x = 188 + 20;
	p = xmenu_hw_config[10];
	lcdPutText(x, y, p, (int)menu->TitleBarTextColor);
}

void _PutUnitAllItem(GMENU *menu, int Item, int x, int y)
{
	BMPC	*bmpD, _bmpD, *bmpS, _bmpS;
	unsigned short	buf[352];
	int		w, fColor, ID;

	unitscfgSet((UNITS_CFG *)menuGetContext(menu));
	if(menu->MenuStatus & GMENU_PREFIX_NUMBER) {
		bmpD = &_bmpD; bmpS = &_bmpS; 
		bmpcInit(bmpD, buf, 22, 16);
		if(Item < 9) w = 0; else w = (Item+1) / 10; 
		bmpcInit(bmpS, digits_8x16[w], 8, 16);
		bmpcClear(bmpD, RGB(0,40,128));
		bmpcPutMask(bmpD, 2, 0, RGB(255,255,255), bmpS);
		if(Item < 9) w = Item+1; else w = (Item+1) % 10; 
		bmpcInit(bmpS, digits_8x16[w], 8, 16);
		bmpcPutMask(bmpD, 12, 0, RGB(255,255,255), bmpS);
		lcdPut(x+2, y+3, bmpD);
		x += bmpcGetWidth(bmpD) + 4;
	} else	x += 8;
	ID = Item << 1;
	x += 2;		// 6
	fColor = menu->BackGroundTextColor;
	_PutUnitItem(ID, x, y, fColor);
	x += 36;	// 38
	_PutDoorItem(ID, x, y, fColor);
	x += 23;	// 26
	_PutPointItem(ID, x, y, fColor);
	x += 106;
	_PutDoorItem(ID+1, x, y, fColor);
	x += 23;	// 26
	_PutPointItem(ID+1, x, y, fColor);
	syscfgSetUnits(sys_cfg);
}

void WizardUnitItem(int Item, UNITS_CFG *NewUnits);

int _ProcessUnitConfig(GWND *WizardWnd, int *Wizard, GMENU *menu, UNITS_CFG *NewUnits)
{
	int		c, rval;

	while(1) {
		rval = MenuPopupEx(menu, *Wizard, GWND_TIMER);
		if(rval < 1) break;
		c = menu->LastKey;
		if(c == KEY_F1) rval = 1;
		else if(c == KEY_F4) rval = 2;
		else {
			WizardUnitItem(rval-1, NewUnits);
			rval = 0;	
		}
		if(rval) break;
		taskYield();
	}
	return rval;
}

extern SYS_CFG	*tmp_cfg, _tmp_cfg;
void _SetupUnits(SYS_CFG *tmp_cfg);

int _SaveUnitsConfig(GWND *WizardWnd, char *Title, UNITS_CFG *NewUnits)
{
	int		rval;

	tmp_cfg = &_tmp_cfg;
	syscfgCopy(tmp_cfg, sys_cfg);
	unitscfgSet(NewUnits);
	syscfgCopyFromUnits(sys_cfg);
	syscfgSetUnits(sys_cfg);
	if(syscfgCompare(sys_cfg, tmp_cfg)) {
		rval = WndSaveConfig(WizardWnd, Title, 1);
		if(rval < 0) syscfgCopy(sys_cfg, tmp_cfg);
		else	_SetupUnits(tmp_cfg);
	} else	rval = 0;
	return rval;
}

void _SetupUnits(SYS_CFG *tmp_cfg)
{
	unsigned char	NewVal, OldVal, NewType, OldControl[MAX_UNIT_SZ], OldType[MAX_UNIT_SZ], OldDoorEnable[MAX_UNIT_SZ];
	int		i;

	syscfgSetUnits(tmp_cfg);
	for(i = 0;i < MAX_UNIT_SZ;i++) {
		if(unitGetType(i) == 2) {
			if(unitGetEnable(i)) OldVal = 0x01; else OldVal = 0x00;	
			if(crGetType(i)) OldVal |= 0x02;
			OldType[i] = 2;
		} else {
			if(i > 1 && unitGetEnable(i)) OldVal = 0x02; else OldVal = 0x00;	
			if(crGetType(i) == 1) OldVal |= 0x01;
			OldType[i] = 0;
		}
		OldControl[i] = OldVal;
		OldDoorEnable[i] = adGetEnable(i);
	}
	syscfgSetUnits(sys_cfg);
	for(i = 0;i < MAX_UNIT_SZ;i++) {
		if(unitGetType(i) == 2) {
			if(unitGetEnable(i)) NewVal = 0x01; else NewVal = 0x00;
			if(crGetType(i)) NewVal |= 0x02;
			NewType = 2;
		} else {
			if(i > 1 && unitGetEnable(i)) NewVal = 0x02; else NewVal = 0x00;	
			if(crGetType(i) == 1) NewVal |= 0x01;
			NewType = 0;
		}
		if(OldType[i] != NewType) {
			if(NewType == 2) {
				unitAdd(i, 0);
				if(NewVal) { 
					unitcrInitRequest(i);
					unitcrDataRequest(i);
					unitcrAdd(i, (int)NewVal);
				}
			} else {
				unitcrAdd(i, OldControl[i] & 0x02);
				if(NewVal) { 
					unitInitRequest(i);	
					unitIouDataRequest(i);
					unitCruDataRequest(i);
					unitAdd(i, (int)NewVal);
				}
			}
		} else if(OldControl[i] != NewVal) {
			if(NewType == 2) {
				unitcrInitRequest(i);
				unitcrDataRequest(i);
				if(OldControl[i] & 0x01) unitcrAdd(i, OldControl[i] & 0x02);
				if(NewVal & 0x01) unitcrAdd(i, (int)NewVal);
			} else {		
				unitInitRequest(i);	
				unitIouDataRequest(i);
				unitCruDataRequest(i);
				unitAdd(i, (int)NewVal);
			}
		}
		if(OldDoorEnable[i] != adGetEnable(i)) unitEncodeIouConfig(i);
	}
}

int  _SaveIOUnitsConfig(GWND *WizardWnd, char *Title, UNITS_CFG *NewUnits);
void _PutIOUnitAllItem(GMENU *menu, int Item, int x, int y);
void _PutIOUnitTitle(GMENU *menu);
int  _ProcessIOUnitConfig(GWND *WizardWnd, int *Wizard, GMENU *menu, UNITS_CFG *NewUnits);

void WizardSetupIOUnits(char *Title)
{
	UNITS_CFG	*NewUnits, _NewUnits;
	GWND	*wnd, _wnd;
	GMENU	*menu, _menu;
	int		rval, Stage, Wizard;

	NewUnits = &_NewUnits;
	unitscfgSet(NewUnits);
	syscfgCopyToUnits(sys_cfg);
	syscfgSetUnits(sys_cfg);	
	menu = &_menu;
	MenuInitialize(menu, NULL, NULL, MAX_IOUNIT_SZ);
	menuSetPutItemCallBack(menu, _PutIOUnitAllItem);
	menuSetPutTitleCallBack(menu, _PutIOUnitTitle);
	menuSetContext(menu, NewUnits);
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		switch(Stage) {
		case 1:	rval = _ProcessIOUnitConfig(wnd, &Wizard, menu, NewUnits); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else {
			_SaveIOUnitsConfig(wnd, Title, &_NewUnits);
			Stage = 0;
		}
		taskYield();
	}
}

int _GetIOUnitNode(int ID)
{
	int		enable, file;

	if(iounitGetEnable(ID)) enable = 1;
	else	enable = 0;
	if(enable) file = UNIT_ICON;
	else	file = UNIT_DISABLE_ICON;
	return file;
}

int _PutIONodeLabel(int ID, int x, int y, int fColor)
{
	char	temp[4];

	temp[0] = ID + '0';
	temp[1] = 0;
	lcdPutText(x, y+1, temp, fColor);	// 0-9=11
	return 12;
}

void _PutIOUnitItem(int ID, int x, int y, int fColor)
{
	int		file;

	x += _PutIONodeLabel(ID, x, y, fColor); x += 3;
	file = _GetIOUnitNode(ID);
	lcdPutIconFile(x, y+1, HW_ICON_SZ, file);
}

void _PutIOUnitTitle(GMENU *menu)
{
	char	*p;
	int		x, y, w, size;
	
	y = menu->y + 2;
	p = xmenu_hw_config[0];
	size = 34; w = GetStringWidth(p);
	x = 22;
	lcdPutText(x+((size-w)>>1), y, p, (int)menu->TitleBarTextColor);
	p = xmenu_hw_config[7];
	x = 56 + 24;
	lcdPutText(x, y, p, (int)menu->TitleBarTextColor);
}

void _PutIOUnitType(int ID, int x, int y, int fColor)
{
	int		val;

	if(iounitGetType(ID)) val = 1; else val = 0;
	 lcdPutText(x, y+1, xmenu_iounit_type[val], fColor);
}

void _PutIOUnitAllItem(GMENU *menu, int Item, int x, int y)
{
	BMPC	*bmpD, _bmpD, *bmpS, _bmpS;
	unsigned short	buf[352];
	int		w, fColor, ID;

	unitscfgSet((UNITS_CFG *)menuGetContext(menu));
	if(menu->MenuStatus & GMENU_PREFIX_NUMBER) {
		bmpD = &_bmpD; bmpS = &_bmpS; 
		bmpcInit(bmpD, buf, 22, 16);
		if(Item < 9) w = 0; else w = (Item+1) / 10; 
		bmpcInit(bmpS, digits_8x16[w], 8, 16);
		bmpcClear(bmpD, RGB(0,40,128));
		bmpcPutMask(bmpD, 2, 0, RGB(255,255,255), bmpS);
		if(Item < 9) w = Item+1; else w = (Item+1) % 10; 
		bmpcInit(bmpS, digits_8x16[w], 8, 16);
		bmpcPutMask(bmpD, 12, 0, RGB(255,255,255), bmpS);
		lcdPut(x+2, y+3, bmpD);
		x += bmpcGetWidth(bmpD) + 4;
	} else	x += 8;
	ID = Item;
	x += 2;		// 6
	fColor = menu->BackGroundTextColor;
	_PutIOUnitItem(ID, x, y, fColor);
	x += 40;	// 38
	_PutIOUnitType(ID, x, y, fColor);
	syscfgSetUnits(sys_cfg);
}

void WizardIOUnitItem(int Item, UNITS_CFG *NewUnits);

int _ProcessIOUnitConfig(GWND *WizardWnd, int *Wizard, GMENU *menu, UNITS_CFG *NewUnits)
{
	int		c, rval;

	while(1) {
		rval = MenuPopupEx(menu, *Wizard, GWND_TIMER);
		if(rval < 1) break;
		c = menu->LastKey;
		if(c == KEY_F1) rval = 1;
		else if(c == KEY_F4) rval = 2;
		else {
			WizardIOUnitItem(rval-1, NewUnits);
			rval = 0;	
		}
		if(rval) break;
		taskYield();
	}
	return rval;
}

extern SYS_CFG	*tmp_cfg, _tmp_cfg;
void _SetupIOUnits(SYS_CFG *tmp_cfg);

int _SaveIOUnitsConfig(GWND *WizardWnd, char *Title, UNITS_CFG *NewUnits)
{
	int		rval;

	tmp_cfg = &_tmp_cfg;
	syscfgCopy(tmp_cfg, sys_cfg);
	unitscfgSet(NewUnits);
	syscfgCopyFromUnits(sys_cfg);
	syscfgSetUnits(sys_cfg);
	if(syscfgCompare(sys_cfg, tmp_cfg)) {
		rval = WndSaveConfig(WizardWnd, Title, 1);
		if(rval < 0) syscfgCopy(sys_cfg, tmp_cfg);
		else	_SetupIOUnits(tmp_cfg);
	} else	rval = 0;
	return rval;
}

void _SetupIOUnits(SYS_CFG *tmp_cfg)
{
	unsigned char	NewVal, OldVal, OldControl[MAX_IOUNIT_SZ];
	int		i;

	syscfgSetUnits(tmp_cfg);
	for(i = 0;i < MAX_IOUNIT_SZ;i++) {
		if(!iounitGetEnable(i)) OldVal = 0x00;
		else if(iounitGetType(i)) OldVal = 0x03;
		else	OldVal = 0x01;
		OldControl[i] = OldVal;
	}
	syscfgSetUnits(sys_cfg);
	for(i = 0;i < MAX_IOUNIT_SZ;i++) {
		if(!iounitGetEnable(i)) NewVal = 0x00;
		else if(iounitGetType(i)) NewVal = 0x03;
		else	NewVal = 0x01;
		OldVal = OldControl[i];
		if(OldVal ^ NewVal) {
			iounitInitRequest(i);
			//iounitDataRequest(i);
			iounitAdd(i, (int)NewVal);
		}
	}
}
