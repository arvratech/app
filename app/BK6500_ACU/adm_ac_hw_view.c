extern FP_USER	*gUser;

#define TOP_ICON_SZ		16

static unsigned char ViewEnable, ViewPage;


void MenuViewUnitsIPAddress(char *Title)
{
	GMENU	*menu, _menu;
	unsigned char	IPAddr[4];
	char	*p, temp[64];
	int		rval, i;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);	// menu, Title, MenuStr, Size
	menuSetType(menu, GMENU_TYPE_VIEW_ONLY);
	menuSetPrefixNumber(menu, 0);
	for(i = 0;i < MAX_CR_SZ;i++) {
		if(unitGetType(i) == 2 && crIsEnable(i) && crGetType(i)) {
			p = temp;
			GetCredentialReaderName(i, p); p += strlen(p);
			*p++ = ' '; *p++ = ' ';
			SlaveSinGetIPAddress(i, IPAddr);
			strcpy(p, inet2addr(IPAddr));
			MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		}
	}
	rval = MenuPopup(menu);
	MenuTerminate(menu);
}

void MenuSetViewEnable(int Enable)
{
	if(Enable) ViewEnable = 1; else ViewEnable = 0;
}

void MenuSetViewPage(int Page)
{
	ViewPage = (unsigned char)Page;
}

int MenuGetViewPage(void)
{
	return (int)ViewPage;
}

static int _GetMainIcon(int UnitID)
{
	int		ID, file;

	if(unitGetType(UnitID) == 2) {
		if(UnitID & 1) ID = UnitID - 1; else ID = UnitID + 1;
		if(!crIsEnable(UnitID) && !crIsEnable(ID)) file = READER_DISABLE_ICON;
		else	file = READER_ICON;
	} else {
		if(UnitID & 1) ID = UnitID - 1; else ID = UnitID + 1;
		if(UnitID < 2) file = MASTER_ICON;	// Local
		else if(!unitGetEnable(UnitID) && !unitGetEnable(ID)) file = UNIT_DISABLE_ICON;
		else if(unitGetReliability(UnitID)) file = UNIT_FAIL_ICON;
		else	file = UNIT_ICON;
	}
	return file;
}

static int _GetIOUnitIcon(int ID)
{
	int		file;

	if(!iounitGetEnable(ID)) file = UNIT_DISABLE_ICON;
	else if(iounitGetReliability(ID)) file = UNIT_FAIL_ICON;
	else	file = UNIT_ICON;
	return file;
}

static int _GetDoorIcon(int adID)
{
	ACCESS_DOOR		*ad;
	int		state, file;

//	if(adIsEnable(UnitID)) file = DOOR_ICON;
//	else	file = DOOR_DISABLE_ICON;
	ad = adsGetAt(adID);
	if(!adIsEnable(adID)) file = DOOR_DISABLE_ICON;
	else {
		state = adAlarmState(adID);
		if(state == 1) file = OPEN_TOO_LONG_ICON;
		else if(state == 2) file = FORCED_OPEN_ICON;
		else if(ad->cfg->DoorStatusEnable && biGetPresentValue(adGetDoorStatusInput(adID))) file = DOOR_OPENED_ICON;
		else	file = DOOR_ICON;
	}
	return file;
}

static int _GetLockIcon(int adID)
{
	ACCESS_DOOR	*ad;
	int		file;

	ad = adsGetAt(adID);
	if(!adIsEnable(adID)) file = 0;
	else if(ad->PresentValue & 1) file = UNLOCK_ICON;
	else	file = LOCK_ICON;
	return file;
}

static int _GetReaderIcon(int crID)
{
	int		enable, file;

	enable = crIsEnable(crID);
	if(unitGetType(crID) == 2 || crGetType(crID) == 1) {	// StandAlone, RS-485
		if(!enable) file = READER_DISABLE_ICON;
		else if(!crGetReliability(crID)) file = READER_ICON;
		else	file = READER_FAIL_ICON;
	} else {	// Wiegand
		if(enable) file = WIEGAND_ICON;
		else	file = WIEGAND_DISABLE_ICON;
	}
	return file;
}

static void _GetReaderEvent(int crID, char *s)
{
	ACCESS_POINT	*ap;
	int		val;
	
	ap = apsGetAt(crID);
	if(ap->AccessEvent) {
		val = ap->AccessTime[3];
		if(val >= 10) s[0] = '0' + val / 10;
		else	s[0] = '0';
		s[1] = '0' + val % 10;
		s[2] = ':';
		val = ap->AccessTime[4];
		if(val >= 10) s[3] = '0' + val / 10;
		else	s[3] = '0';
		s[4] = '0' + val % 10;
		s[5] = ':';
		val = ap->AccessTime[5];
		if(val >= 10) s[6] = '0' + val / 10;
		else	s[6] = '0';
		s[7] = '0' + val % 10;
  		s[8] = ' ';
  		if(evtIsGranted(ap->AccessEvent)) s[9] = 'O';
  		else	s[9] = 'X';
  		s[10] = 0;
 //cprintf("Event=%d [%s]\n", (int)ap->AccessEvent, s);
  	} else	s[0] = 0;
}

void _DrawUnits(void)
{
	char	temp[12];
	int		i, j, ID, max, x, y, c, color, file;

	color = 0x0000;
	y = 25;
	ID = 0; max = MAX_UNIT_SZ >> 1;
	for(i = 0;i < max;i++) {
		x = 2;
		if(i == 0) c = 'L'; else c = i - 1 + '0';
		lcdPutEng16Char(x, y, c, color);
		x += 14;
		file = _GetMainIcon(ID);
		lcdPutIconFile(x, y, TOP_ICON_SZ, file);
		x += 20;
		for(j = 0;j < 2;j++) {
			file = _GetDoorIcon(ID);
			lcdPutIconFile(x, y, TOP_ICON_SZ, file);
			x += 18;
			file = _GetLockIcon(ID);
			if(file) lcdPutIconFile(x, y, TOP_ICON_SZ, file);
			x += 18;
			file = _GetReaderIcon(ID);
			lcdPutIconFile(x, y, TOP_ICON_SZ, file);
			x += 18;
			_GetReaderEvent(ID, temp);
			lcdPutEng16Text(x, y, temp, color);
			x += 88;
			ID++;
		}
		y += 18;
	}		
}

void UpdateMainState(int ID)
{
	int		x, y, file;
	
	if(ViewEnable && (ViewPage == 0 || ViewPage == 1)) {
		file = _GetMainIcon(ID);
		x = 16;
		y = 25 + (ID >> 1) * 18;
		lcdFillRect(x, y, 16, 16, RGB(220,220,220));
		lcdPutIconFile(x, y, TOP_ICON_SZ, file);
	}
}

void _DrawIOUnit(int ID, int x, int y, int erase);

void UpdateIOUnit(int ID)
{
	if(ViewEnable && ViewPage == 2) _DrawIOUnit(ID, 16, 43 + ID * 18, 1);
}

void UpdateDoorStatus(int ID)
{
	int		x, y, file;
	
	if(ViewEnable && ViewPage == 0) {
		file = _GetDoorIcon(ID);
		if(ID & 1) x = 178; else x = 36;
		y = 25 + (ID >> 1) * 18;
		lcdFillRect(x, y, 16, 16, RGB(220,220,220)); 
		if(file) lcdPutIconFile(x, y, TOP_ICON_SZ, file);
	}
}

void UpdateDoorPV(int ID)
{
	int		x, y, file;
	
	if(ViewEnable && ViewPage == 0) {
		file = _GetLockIcon(ID);
		if(ID & 1) x = 196; else x = 54;
		y = 25 + (ID >> 1) * 18;
		lcdFillRect(x, y, 16, 16, RGB(220,220,220)); 
		if(file) lcdPutIconFile(x, y, TOP_ICON_SZ, file);
	}
}

void UpdateReaderState(int ID)
{
	int		x, y, file;

	if(ViewEnable && ViewPage == 0) {
		file = _GetReaderIcon(ID);
		if(ID & 1) x = 214;
		else 	x = 72;
		y = 25 + (ID >> 1) * 18;
		lcdFillRect(x, y, 16, 16, RGB(220,220,220)); 
		lcdPutIconFile(x, y, TOP_ICON_SZ, file);
	}
}

void UpdateReaderEvent(int ID)
{
	char	temp[12];
	int		x, y;

	if(ViewEnable && ViewPage == 0) {
		_GetReaderEvent(ID, temp);
		if(ID & 1) x = 232;
		else 	x = 90;
		y = 25 + (ID >> 1) * 18;
		lcdFillRect(x, y, 88, 16, RGB(220,220,220)); 
		lcdPutEng16Text(x, y, temp, 0);
	}
}

void MenuViewUnits(char *Title)
{
	GWND	*wnd, _wnd;
	int		x, y, w, h, fh, page;

	AdminConsoleUnsupported();
	wnd = &_wnd;
	x = 0; y = 0; w = lcdGetWidth(); h = lcdGetHeight();
	wndInitialize(wnd, Title, 0, 0, w, h);
	fh = GetFontHeight() + 4;
	lcdHoldRefresh();
	lcdFillRect(x, y, w, fh, (int)wnd->TitleBarColor);
	lcdPutText(x+8, y+2, Title, (int)wnd->TitleBarTextColor);
	lcdFillRect(0, fh, w, h - fh, DesktopGetColor());
	page = MenuGetViewPage();
	MenuSetViewEnable(1);
	MenuSetViewPage(2);
	_DrawUnits();
	lcdRefresh(0, 0, w, h);
	keyGet();
	wndProcessWindow(wnd, -1, 550);	
	MenuSetViewEnable(0);
	MenuSetViewPage(page);	
}

void _DrawUnitsInOut(void)
{
	int		i, j, k, ID, biID, boID, max, x, y, c, color, val;

	color = 0x0000;
	y = 25;
	ID = biID = boID = 0;
	max = MAX_UNIT_SZ >> 1;
	for(i = 0;i < max;i++) {
		x = 2;
		if(i == 0) c = 'L'; else c = i - 1 + '0';
		lcdPutEng16Char(x, y, c, color);
		x += 14;
		val = _GetMainIcon(i << 1);
		lcdPutIconFile(x, y, TOP_ICON_SZ, val);
		x += 20;
		for(j = 0;j < 2;j++) {
			val = unitGetType(ID); 
			if(val == 2 && crIsEnable(ID) || val != 2 && unitGetEnable(ID)) {
				if(val == 2) val = crGetReliability(ID);
				else	     val = unitGetReliability(ID);
				if(!val) {
					for(k = 0;k < 4;k++) {
						if(biGetPresentValue(biID)) c = 'H';
						else	c = 'L';
						if(biGetAlarmState(biID)) val = RGB(255,0,0);
						else	val = color;
						lcdPutEng16Char(x, y, c, val);
						x += 19;
						biID++;
					}
					x += 13;
					for(k = 0;k < 2;k++) {
						if(!k || unitGetType(ID)) {
							if(boGetPresentValue(boID)) c = 'H';
							else	c = 'L';
							lcdPutEng16Char(x, y, c, color);
						}
						x += 19;
						boID++;
					}
				} else {
					for(k = 0;k < 4;k++) {
						lcdPutEng16Char(x, y, 'U', color);
						x += 19;
						biID++;
					}
					x += 13;
					for(k = 0;k < 2;k++) {
						if(!k || unitGetType(ID)) lcdPutEng16Char(x, y, 'U', color);
						x += 19;
						boID++;
					}
				}
				x += 15;
			} else {
				x += 142;
				biID += 4; boID += 2;
			}
			ID++;
		}
		y += 18;
	}
}

void UpdateInputState(int biID)
{
	int		x, y, c, color, ID;

	if(ViewEnable && (ViewPage == 1 || ViewPage == 2)) {
		if(biGetPresentValue(biID)) c = 'H';
		else	c = 'L';
		if(biGetAlarmState(biID)) color = RGB(255,0,0);
		else	color = 0;
		ID = biGetUnit(biID);
		if(ID >= 0) {
			x = biID & 0x07;
			if(x > 3) x = 178 + (x-4) * 19;
			else 	x = 36 + x * 19;
			y = 25 + (biID >> 3) * 18;
			lcdFillRect(x, y, 17, 16, RGB(220,220,220));
		} else {
			ID = biID - iounitGetBinaryInput(0);
			x =  32 + (ID % MAX_IOUNIT_BI_SZ) * 12;
			y = 25 + 18 + ID / MAX_IOUNIT_BI_SZ * 18;
			lcdFillRect(x, y, 14, 16, RGB(220,220,220));
		}
		lcdPutEng16Char(x, y, c, color);
	}
}

void UpdateOutputState(int boID)
{
	int		x, y, c, ID;

	if(ViewEnable && (ViewPage == 1 || ViewPage == 2)) {
		if(boGetPresentValue(boID)) c = 'H';
		else	c = 'L';
		ID = boGetUnit(boID);
		if(ID >= 0) {
			x = boID & 0x03;
			if(x > 1) x = 267 + (x-2) * 19;
			else 	x = 125 + x * 19;
			y = 25 + (boID >> 2) * 18;
			lcdFillRect(x, y, 17, 16, RGB(220,220,220));
		} else {
			ID = boID - iounitGetBinaryOutput(0);
			x =  32 + (ID % MAX_IOUNIT_BO_SZ) * 12;
			y = 25 + 18 + (ID / MAX_IOUNIT_BO_SZ) * 18;
			lcdFillRect(x, y, 14, 16, RGB(220,220,220));
		}
		lcdPutEng16Char(x, y, c, 0);
	}
}

void MenuViewUnitsInOut(char *Title)
{
	GWND	*wnd, _wnd;
	int		x, y, w, h, fh, page;

	AdminConsoleUnsupported();
	wnd = &_wnd;
	x = 0; y = 0; w = lcdGetWidth(); h = lcdGetHeight();
	wndInitialize(wnd, Title, 0, 0, w, h);
	fh = GetFontHeight() + 4;
	lcdHoldRefresh();
	lcdFillRect(x, y, w, fh, (int)wnd->TitleBarColor);
	lcdPutText(x+8, y+2, Title, (int)wnd->TitleBarTextColor);
	lcdFillRect(0, fh, w, h - fh, DesktopGetColor());
	page = MenuGetViewPage();
	MenuSetViewEnable(1);
	MenuSetViewPage(2);
	_DrawUnitsInOut();
	lcdRefresh(0, 0, w, h);
	keyGet();
	wndProcessWindow(wnd, -1, 550);	
	MenuSetViewEnable(0);
	MenuSetViewPage(page);	
}

void _DrawIOUnit(int ID, int x, int y, int erase)
{
	int		i, c, val, type, ioID, color;
	
	color = 0x0000;
	if(erase) lcdFillRect(x, y, lcdGetWidth()-x, 16, RGB(220,220,220));
	val = _GetIOUnitIcon(ID);
	lcdPutIconFile(x, y, TOP_ICON_SZ, val);
	x += 18;
	type = iounitGetType(ID);
	if(!iounitGetReliability(ID)) {
		if(type == 0) {
			ioID = iounitGetBinaryInput(ID);
			for(i = 0;i < MAX_IOUNIT_BI_SZ;i++) {
				if(biGetPresentValue(ioID)) c = 'H';
				else	c = 'L';
				if(biGetAlarmState(ioID)) val = RGB(255,0,0);
				else	val = color;
				lcdPutEng16Char(x, y, c, val);
				x += 12;
				ioID++;
			}
		} else {
			ioID = iounitGetBinaryOutput(ID);
			for(i = 0;i < MAX_IOUNIT_BO_SZ;i++) {
				if(boGetPresentValue(ioID)) c = 'H';
				else	c = 'L';
				lcdPutEng16Char(x, y, c, color);
				x += 12;
				ioID++;
			}
		}	
	} else {
		if(type == 0) {
			ioID = iounitGetBinaryInput(ID);
			for(i = 0;i < MAX_IOUNIT_BI_SZ;i++) {
				lcdPutEng16Char(x, y, 'U', color);
				x += 12;
				ioID++;
			}
		} else {
			ioID = iounitGetBinaryOutput(ID);
			for(i = 0;i < MAX_IOUNIT_BO_SZ;i++) {
				lcdPutEng16Char(x, y, 'U', color);
				x += 12;
				ioID++;
			}
		}
	}
}

void _DrawIOUnits(void)
{
	int		i, ID, x, y, c, color;

	color = 0x0000;
	y = 25;
	x = 32;
	for(i = 0;i < MAX_IOUNIT_BI_SZ;i++) {
		c = i + 1;
		if(c >= 10) c -= 10;
		if(c >= 10) c -= 10;		 
		lcdPutEng16Char(x, y, c+'0', RGB(0,0,255));
		x += 12;
	}
	y += 18;
	for(i = 0;i < MAX_IOUNIT_SZ;i++) {
		x = 2;
		ID = i;
		lcdPutEng16Char(x, y, ID+'0', color);
		x += 12;
		if(iounitGetEnable(ID)) _DrawIOUnit(ID, x, y, 0);
		y += 18;
	}
}

void MenuViewIOUnits(char *Title)
{
	GWND	*wnd, _wnd;
	int		x, y, w, h, fh, page;

	AdminConsoleUnsupported();
	wnd = &_wnd;
	x = 0; y = 0; w = lcdGetWidth(); h = lcdGetHeight();
	wndInitialize(wnd, Title, 0, 0, w, h);
	fh = GetFontHeight() + 4;
	lcdHoldRefresh();
	lcdFillRect(x, y, w, fh, (int)wnd->TitleBarColor);
	lcdPutText(x+8, y+2, Title, (int)wnd->TitleBarTextColor);
	lcdFillRect(0, fh, w, h - fh, DesktopGetColor());
	page = MenuGetViewPage();
	MenuSetViewEnable(1);
	MenuSetViewPage(2);
	_DrawIOUnits();
	lcdRefresh(0, 0, w, h);
	keyGet();
	wndProcessWindow(wnd, -1, 550);	
	MenuSetViewEnable(0);
	MenuSetViewPage(page);
}

void MenuViewMain(void)
{
	if(ViewEnable) {
		if(ViewPage == 2) _DrawIOUnits();
		else if(ViewPage == 1) _DrawUnitsInOut();
		else	_DrawUnits();
	}
}

void UpdateViewMain(void)
{
	int		y, w, h;

	if(ViewEnable) {
		lcdHoldRefresh();
		y = 25;
		w = lcdGetWidth(); h = lcdGetHeight() - y;		
		lcdFillRect(0, y, w, h, DesktopGetColor());
		if(ViewPage == 2) _DrawIOUnits();
		else if(ViewPage == 1) _DrawUnitsInOut();
		else	_DrawUnits();		
		lcdRefresh(0, y, w, h);
	}
}
