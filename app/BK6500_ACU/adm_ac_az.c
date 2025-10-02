void MenuAccessZones(GMENU *ParentMenu, char *Title);
void MenuPointDefinitions(GMENU *ParentMenu, char *Title);
void MenuSetupPassback(GMENU *ParentMenu, char *Title);

void MenuAccessZonesPassback(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	MenuClearItems(menu);
	MenuAddItem(menu, Title, 0, GMENU_ITEM_ENABLE);
	MenuAddItem(menu, xmenu_access_zone[1], 0, GMENU_ITEM_ENABLE);
	MenuAddItem(menu, xmenu_access_zone[2], 0, GMENU_ITEM_ENABLE);
	while(1) {	
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuAccessZones(menu, ItemText);
			break;
		case 2:
			MenuPointDefinitions(menu, ItemText);
			break;
		case 3:
			MenuSetupPassback(menu, ItemText);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuNewAccessZone(GMENU *ParentMenu, char *Title);
void MenuAccessZone(GMENU *ParentMenu, char *Title, int azID);

void MenuAccessZones(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[32];
	int		rval, i, enable, Item, IDs[MAX_AZ_SZ-1];

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {	
		MenuClearItems(menu);
		MenuAddItem(menu, xmenu_access_zone[0], 0, GMENU_ITEM_ENABLE);
		for(Item = i = 0;i < MAX_AZ_SZ;i++) {
			if(azExist(i)) {
				GetAccessZoneName(i, temp);
				if(i < 1) enable = 0; else enable = GMENU_ITEM_ENABLE;
				MenuAddItem(menu, temp, ZONE_ICON, enable);
				IDs[Item] = i; Item++;
			}
		}
		if(menu->CurrentItem >= menu->ItemCount) menu->CurrentItem = menu->ItemCount - 1;
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		if(rval == 1) MenuNewAccessZone(menu, ItemText);
		else	MenuAccessZone(menu, ItemText, IDs[rval-2]);
	}
	MenuTerminate(menu);
}

void MenuNewAccessZone(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	temp[MAX_AZ_SZ-1][30]; 
	int		rval, i, Item, IDs[MAX_AZ_SZ-1];

	if(ParentMenu->WndType == 1) menuDarken(ParentMenu); 
	menu = &_menu;
	MenuInitialize(menu, Title,NULL, 0);
	menuSetParentWnd(menu, (GWND *)ParentMenu);
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_RADIO);
	menuSetSelectNumber(menu, 0);
	for(Item = i = 0;i < MAX_AZ_SZ;i++) {
		if(!azExist(i)) {
			GetAccessZoneName(i, temp[Item]);
			MenuAddItem(menu, temp[Item], 0, GMENU_ITEM_ENABLE);	 
			IDs[Item] = i; Item++;
		}
	}
	_SetMenuProperSize(menu);
	_SetSelectMenuColor(menu);
	menuSetCurrentItem(menu, 0);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
	if(rval > 0) {
		tmp_cfg = &_tmp_cfg;
		syscfgCopy(tmp_cfg, sys_cfg);
		i = IDs[rval-1];		
		rval = azAdd(i);
		GetAccessZoneName(i, temp[0]);
		if(rval < 0) ResultMsg((GWND *)menu, temp[0], R_MSG_SYSTEM_ERROR);
		else if(rval > 0) ResultMsg((GWND *)menu, temp[0], R_DATA_SAVED);
	}
}

void MenuAccessZone(GMENU *ParentMenu, char *Title, int azID)
{
	GMENU	*menu, _menu;
	ACCESS_ZONE_CFG		*az_cfg;
	char	*ItemText, temp[32];
	int		rval, cont;

	az_cfg = &sys_cfg->AccessZones[azID-1];
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_passback, 6);
	cont = 1;
	while(cont) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			sprintf(temp, "%s", GetPromptMsg(M_DETETE_CONFIRM));
			rval = WndMessageBoxYesNo((GWND *)ParentMenu, Title, temp);
			if(rval == IDYES) {
				rval = azRemove(azID);
				if(rval < 0) ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
				else {
					if(rval > 0) ResultMsg((GWND *)ParentMenu, Title, R_DELETED);
					cont = 0;
				}
			}	
			break;		
		case 2:
			MenuRadio((GWND *)menu, ItemText, &az_cfg->PassbackMode, xmenu_passback_mode, 3, 1, 1);
			break;
		case 3:
			MenuNoYes((GWND *)menu, ItemText, &az_cfg->PassbackOption, 0x01, 1);
			break;
		case 4:
			MakeGuideNumberMinuteAlways(temp, 255);
			WndInputNumber((GWND *)menu, ItemText, temp, &az_cfg->PassbackTimeout, 1, 0, 255, R_INVALID_TIME_INTERVAL, 1);
			break;
		case 5:
			MenuInputSchedule((GWND *)menu, ItemText, &az_cfg->HardPassbackSchedule, 1);
			break;
		case 6:
			MenuInputSchedule((GWND *)menu, ItemText, &az_cfg->SoftPassbackSchedule, 1);
			break;
		}
	}
	MenuTerminate(menu);	
}

void WizardSetupPointZones(char *Title, int apID);
void _PutPointZoneTitle(GMENU *menu);
void _PutPointZoneItem(GMENU *menu, int Item, int x, int y);

void MenuPointDefinitions(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval, i, Item, IDs[MAX_AP_SZ];

	menu = &_menu;
	MenuInitialize(menu, NULL, NULL, 0);
	menuSetPutTitleCallBack(menu, _PutPointZoneTitle);
	menuSetPutItemCallBack(menu, _PutPointZoneItem);
	menuSetContext(menu, IDs);
	while(1) {
		MenuClearItems(menu);
		for(Item = i = 0;i < MAX_AD_SZ;i++) {
			if(adIsEnable(i)) {
				MenuAddItem(menu, "", 0, GMENU_ITEM_ENABLE);
				//i = adGetAccessPoint(i);
				IDs[Item] = i; Item++;
			}
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		WizardSetupPointZones(ItemText, IDs[rval-1]);
	}
	MenuTerminate(menu);
}

void _PutPointZoneTitle(GMENU *menu)
{
	char	*p, temp[32];
	int		x, y, w, dw, fColor;

	fColor = menu->TitleBarTextColor;
	x = menu->x + 8; y = menu->y + 2;
	lcdPutText(x, y+1, xmenu_access_zone[1], fColor);
	w = 71;		// 71=GetStringWidth("Outside");
	x = 177;
	if(GetLanguage() == LANG_ENG) {
		p = xmenu_access_zone[3];
		p = strcpy_chr(temp, p, ' '); strcpy_chr(temp, p, ' ');
	} else	strcpy(temp, xmenu_access_zone[3]);
	dw = GetStringWidth(temp);
	lcdPutText(x+((w-dw)>>1), y+1, temp, fColor);	
	x = 249;
	if(GetLanguage() == LANG_ENG) {
		p = xmenu_access_zone[4];
		p = strcpy_chr(temp, p, ' '); strcpy_chr(temp, p, ' ');
	} else	strcpy(temp, xmenu_access_zone[4]);
	dw = GetStringWidth(temp);
	lcdPutText(x+((w-dw)>>1), y+1, temp, fColor);	
}

void _PutPointZoneItem(GMENU *menu, int Item, int x, int y)
{
	GMENU_ITEM	*item;
	BMPC		*bmpD, _bmpD, *bmpS, _bmpS;
	unsigned short	buf[352];
	ACCESS_POINT_CFG	*ap_cfg;
	char		temp[32];
	int		w, dw, fh, fColor, file, ID, *IDs;

	if(menu->MenuStatus & GMENU_PREFIX_NUMBER) {
		bmpD = &_bmpD; bmpS = &_bmpS;
		if(menu->ItemCount < 10) {
			bmpcInit(bmpD, buf, 12, 16);
			bmpcInit(bmpS, digits_8x16[Item+1], 8, 16);
			if(item->Status & GMENU_ITEM_ENABLE) {
				bmpcClear(bmpD, RGB(0,40,128));
				bmpcPutMask(bmpD, 2, 0, RGB(255,255,255), bmpS);
			} else {
				bmpcClear(bmpD, RGB(120,120,120)); 
				bmpcPutMask(bmpD, 2, 0, RGB(220,220,220), bmpS);
			}
			lcdPut(x+2, y+3, bmpD);
			x += bmpcGetWidth(bmpD) + 4;
		} else {
			bmpcInit(bmpD, buf, 22, 16);
			if(Item < 9) w = 0; else w = (Item+1) / 10; 
			bmpcInit(bmpS, digits_8x16[w], 8, 16);
			if(item->Status & GMENU_ITEM_ENABLE) {
				bmpcClear(bmpD, RGB(0,40,128));
				bmpcPutMask(bmpD, 2, 0, RGB(255,255,255), bmpS);
			} else {
				bmpcClear(bmpD, RGB(120,120,120)); 
				bmpcPutMask(bmpD, 2, 0, RGB(220,220,220), bmpS);
			}
			if(Item < 9) w = Item+1; else w = (Item+1) % 10; 
			bmpcInit(bmpS, digits_8x16[w], 8, 16);
			if(item->Status & GMENU_ITEM_ENABLE) {
				bmpcPutMask(bmpD, 12, 0, RGB(255,255,255), bmpS);
			} else {
				bmpcPutMask(bmpD, 12, 0, RGB(220,220,220), bmpS);
			}
			lcdPut(x+2, y+3, bmpD);
			x += bmpcGetWidth(bmpD) + 4;
		}
	}
	fh = GetFontHeight();
	IDs = (int *)menuGetContext(menu);
	ID = IDs[Item];
	file = GetAccessPointIcon(ID);
	lcdPutIconFile(x, y+1, fh, file);
	x += fh+2;
	fColor = menu->BackGroundTextColor;
	//rval = GetAccessPointIcon(i);
	GetAccessPointName(ID, temp);
	lcdPutText(x, y+1, temp, fColor);
	w = 71;		// 71=GetStringWidth("Outside");
	x = 177;
	ap_cfg = &sys_cfg->AccessPoints[ID];
	GetAccessZoneShortName((int)ap_cfg->ZoneFrom, temp);
	dw = GetStringWidth(temp);
	lcdPutText(x+((w-dw)>>1), y+1, temp, fColor);	
	x = 249;
	GetAccessZoneShortName((int)ap_cfg->ZoneTo, temp);
	dw = GetStringWidth(temp);
	lcdPutText(x+((w-dw)>>1), y+1, temp, fColor);	
}

int  _SetupZoneFromTo(GWND *WizardWnd, int *Wizard, unsigned char *NewVals, int apID, int FromTo);
int  _SavePointZones(GWND *WizardWnd, char *Title, unsigned char *NewVals, int apID);

void WizardSetupPointZones(char *Title, int apID)
{
	GWND	*wnd, _wnd;
	ACCESS_POINT_CFG	*ap_cfg;
	unsigned char	NewVals[2];
	int		rval, Stage, Wizard;

	ap_cfg = &sys_cfg->AccessPoints[apID];
	NewVals[0] = ap_cfg->ZoneFrom;
	NewVals[1] = ap_cfg->ZoneTo;	 
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		if(Stage < 2) Wizard |= GWND_WIZARD_NEXT;
		if(Stage > 1) Wizard |= GWND_WIZARD_BACK;
		switch(Stage) {
		case 1:	rval = _SetupZoneFromTo(wnd, &Wizard, &NewVals[0], apID, 0); break;
		case 2:	rval = _SetupZoneFromTo(wnd, &Wizard, &NewVals[1], apID, 1); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else if(rval == 1) {
			Stage--;
		} else {
			if(Wizard & GWND_WIZARD_NEXT) Stage++;
			else {	// Final Stage
				if(NewVals[0] == 0xff && NewVals[1] != 0xff || NewVals[0] != 0xff && NewVals[1] == 0xff
					|| NewVals[0] != 0xff && NewVals[1] != 0xff && NewVals[0] == NewVals[1]) {
					ResultMsg(wnd, Title, R_INVALID_DATA);
				} else {
					_SavePointZones(wnd, Title, NewVals, apID);
					Stage = 0;
				}
			}
		}
		taskYield();
	}
}

int _SetupZoneFromTo(GWND *WizardWnd, int *Wizard, unsigned char *NewVal, int apID, int FromTo)
{	
	RECT	rect;
	char	temp[MAX_AZ_SZ+1][24], *menuStr[MAX_AZ_SZ+1];
	unsigned char	cVal, IDs[MAX_AZ_SZ+1];
	int		rval, i, Item;

	wndDrawClientArea(WizardWnd);
	while(1) {
		Item = 0;
		temp[0][0] = '-'; temp[0][1] = 0;
		menuStr[Item] = temp[0]; IDs[Item] = 0xff; Item++;
		menuStr[Item] = xmenu_access_zone[5]; IDs[Item] = 0; Item++;
		for(i = 1;i < MAX_AZ_SZ;i++) {
			if(azExist(i)) {
				GetAccessZoneName(i, temp[Item]);
				menuStr[Item] = temp[Item];
				IDs[Item] = i; Item++;
			}
		}
		for(i = 0;i < Item;i++) if((*NewVal) == IDs[i]) break;
		if(i >= Item) i = 0;
		cVal = i;
		rect.x = 32; rect.y = 8; rect.Width = 240; rect.Height = 160;
		rval = MenuRadioWizardEx(WizardWnd, &rect, xmenu_access_zone[3+FromTo], &cVal, menuStr, Item, 1, *Wizard);
		if(rval < 1) break;
		*NewVal = IDs[cVal];
		if(rval == 1 || rval == 2 || rval == 3) {			
		} else	rval = 0;
		
		if(rval) break;
		taskYield();
	}
	return rval;
}

int _SavePointZones(GWND *WizardWnd, char *Title, unsigned char *NewVals, int apID)
{
	ACCESS_POINT_CFG	*ap_cfg;
	unsigned char	OldVals[2];
	int		rval;

	ap_cfg = &sys_cfg->AccessPoints[apID];
	OldVals[0] = ap_cfg->ZoneFrom;
	OldVals[1] = ap_cfg->ZoneTo;	 
	if(n_memcmp(NewVals, OldVals, 2)) {
		ap_cfg->ZoneFrom	= NewVals[0];
		ap_cfg->ZoneTo		= NewVals[1];
		rval = WndSaveConfig(WizardWnd, Title, 1);
		if(rval < 0) {
			ap_cfg->ZoneFrom	= OldVals[0];
			ap_cfg->ZoneTo		= OldVals[1];
		}
	} else	rval = 0;
	return rval;
}

void WizardResetPassbackTime(char *Title);

void MenuSetupPassback(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[64];
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {	
		MenuClearItems(menu);
		MenuAddItem(menu, xmenu_passback_setting[0], 0, GMENU_ITEM_ENABLE);
		if(sys_cfg->Passback & 0x01) {
			MenuAddItem(menu, xmenu_passback_setting[1], 0, GMENU_ITEM_ENABLE);
			MenuAddItem(menu, xmenu_passback_setting[2], 0, GMENU_ITEM_ENABLE);
			MenuAddItem(menu, xmenu_passback_setting[3], 0, GMENU_ITEM_ENABLE);
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuNoYes((GWND *)menu, ItemText, &sys_cfg->Passback, 0x01, 1);
			break;
		case 2:			
			MakeGuideNumberSecond(temp, 0, 255);
			WndInputNumber((GWND *)menu, ItemText, temp, &sys_cfg->RepeatAccessDelay, 1, 0, 255, R_INVALID_TIME_INTERVAL, 1);
			break;	
		case 3:
			WizardResetPassbackTime(ItemText);
			break;
		case 4:
			sprintf(temp, "%s", GetPromptMsg(M_ARE_YOU_REALLY));
			rval = WndMessageBoxYesNo((GWND *)ParentMenu, ItemText, temp);
			if(rval == IDYES) {
				userfsRemoveAllTransaction();
				ResultMsg((GWND *)ParentMenu, ItemText, R_PROCESSED);
			}
			break;
		}
	}
	MenuTerminate(menu);
}

int _ProcessPassbackResetTime(GWND *WizardWnd, int *Wizard, unsigned char *NewEnable, unsigned char *NewTime);
int _SavePassbackResetTime(GWND *WizardWnd, char *Title, unsigned char NewEnable, unsigned char *NewTime);

void WizardResetPassbackTime(char *Title)
{
	GWND		*wnd, _wnd;
	unsigned char	NewEnable, NewTime[4];
	int		rval, Stage, Wizard;

	if(sys_cfg->Passback & 0x02) NewEnable = 1; else NewEnable = 0;
	memcpy(NewTime, sys_cfg->PassbackResetTime, 2);
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		switch(Stage) {
		case 1:	rval = _ProcessPassbackResetTime(wnd, &Wizard, &NewEnable, NewTime); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else {
			_SavePassbackResetTime(wnd, Title, NewEnable, NewTime);
			Stage = 0;
		}
		taskYield();
	}
}

void _PassbackResetEnableChanged(GRADIO_GROUP *rdGroup, int Position, void *Context);

int _ProcessPassbackResetTime(GWND *WizardWnd, int *Wizard, unsigned char *NewEnable, unsigned char *NewTime)
{
	GRADIO_GROUP	rdgReset;
	GRADIO	rdReset[2], *radio;
	GLABEL	*lblTimeRange, _lblTimeRange;
	GDATE	*tmInput, _tmInput;
	RECT	rect;
	void	*Context[1];
	int		rval, x, y, w, h;

	*Wizard &= ~GWND_WIZARD_NEXT;
	wndDrawClientArea(WizardWnd);	
	radioGroupInitialize(&rdgReset, rdReset, 2);
	x = 2; y = 20;
	labelInitialize(&rdgReset.lblTitle, WizardWnd, x, y, 120, 24);
	labelSetWindowText(&rdgReset.lblTitle, xmenu_passback_setting[2]);
	y += 26;
	w = 100; h = 22;
	x = 16;
	radio = radioGroupAt(&rdgReset, 0);
	radioInitialize(radio, WizardWnd, x, y, w, h); x += w+8; 
	radioSetWindowText(radio, xmenu_inuse[0]);
	radio = radioGroupAt(&rdgReset, 1);
	radioInitialize(radio, WizardWnd, x, y, w, h);
	radioSetWindowText(radio, xmenu_inuse[1]); 
	radioGroupSetChecked(&rdgReset, *NewEnable);
	y += 26;
	tmInput = &_tmInput;
	dateInitialize(tmInput, WizardWnd, 32, y, 72, 26); 
	dateSetFont(tmInput, GWND_NORMAL_FONT);
	if(*NewEnable) dateModifyStyle(tmInput, ES_PASSWORD|ES_WILDDIGIT|ES_READONLY, ES_NUMBER);
	else		   dateModifyStyle(tmInput, ES_PASSWORD|ES_WILDDIGIT, ES_NUMBER|ES_READONLY);
	dateSetType(tmInput, DT_HH_MM_ONLY);		
	memcpy(tmInput->Data, NewTime, 2);				
	lblTimeRange = &_lblTimeRange;
	labelInitialize(lblTimeRange, WizardWnd, 110, y+1, 100, 24);
	labelSetWindowText(lblTimeRange, "(hh:mm)");
	y += 26; 
	wndGetClientArea(WizardWnd, &rect);
	while(1) {
		radioGroupDrawWindow(&rdgReset);
		labelDrawWindow(lblTimeRange);
		dateDrawWindow(tmInput);
		DrawInputDateTimeDesp(rect.x+40, rect.y+y);
		Context[0] = (void *)tmInput;
		rval = RadioGroupWizardEx(&rdgReset, *Wizard, _PassbackResetEnableChanged, Context);
		if(rval >= 3 && radioGroupGetChecked(&rdgReset)) rval = DateWizard(tmInput, *Wizard);
		if(rval == 1 || rval == 2) {
			*NewEnable = radioGroupGetChecked(&rdgReset);
			//msInput->Data) && !tmsValidateEx(tmsInput->Data)) {
			//	ResultMsg(WizardWnd, xmenu_ta_sub[0], R_INVALID_DATA);
			//} else {
			memcpy(NewTime, tmInput->Data, 4);
		}
		if(rval < 3) break;
		taskYield();
	}
	return rval;
}

void _PassbackResetEnableChanged(GRADIO_GROUP *rdGroup, int Position, void *Context)
{
	GDATE	*tmInput;
	void	**pContext;

	pContext = (void **)Context;
	tmInput  = (GDATE *)pContext[0];
//cprintf("Changed: Position=%d tmInput=%x\n", Position, tmInput);
	if(Position) {
		dateModifyStyle(tmInput, ES_PASSWORD|ES_WILDDIGIT|ES_READONLY, ES_NUMBER);
	} else {
		dateModifyStyle(tmInput, ES_PASSWORD|ES_WILDDIGIT, ES_NUMBER|ES_READONLY);
	}
	dateDrawWindow(tmInput);
}

int _SavePassbackResetTime(GWND *WizardWnd, char *Title, unsigned char NewEnable, unsigned char *NewTime)
{
	unsigned char	OldEnable, OldTime[4];
	int		rval;

	if(sys_cfg->Passback & 0x02) OldEnable = 1; else OldEnable = 0;
	memcpy(OldTime, sys_cfg->PassbackResetTime, 2);
	if(NewEnable != OldEnable || n_memcmp(NewTime, OldTime, 2)) {
		if(NewEnable) sys_cfg->Passback |= 0x02; else sys_cfg->Passback &= 0xfd;
		memcpy(sys_cfg->PassbackResetTime, NewTime, 2);
		rval = WndSaveConfig(WizardWnd, Title, 1);
		if(rval < 0) {
			if(OldEnable) sys_cfg->Passback |= 0x02; else sys_cfg->Passback &= 0xfd;
			memcpy(sys_cfg->PassbackResetTime, OldTime, 2);
			rval = -1;
		} else {
			rval = 1;
		}
	} else	rval = 0;
	return rval;
}
