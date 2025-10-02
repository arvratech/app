void _TA_Save(GMENU *menu, char *Title, unsigned char *pVal, unsigned char NewVal)
{
	CR_CFG	*cr_cfg;
	int		rval, i;

#ifndef STAND_ALONE
	for(i = 0;i < MAX_CR_SZ;i++) {
		cr_cfg = &sys_cfg->CredentialReaders[i];
		cr_cfg->Version++; if(!cr_cfg->Version) cr_cfg->Version = 1;
	}
#endif
	rval = WndSaveByte((GWND *)menu, Title, pVal, (int)NewVal, 1);	// pVal, NewVal, Section
#ifndef STAND_ALONE
	if(rval < 0) {
		for(i = 0;i < MAX_CR_SZ;i++) { 
			cr_cfg = &sys_cfg->CredentialReaders[i];
			cr_cfg->Version--; if(!cr_cfg->Version) cr_cfg->Version = 255;
		}
	} else {
		ClisTAConfigChanged();
	}
#endif
}

void MenuTAKeyOption(GMENU *ParentMenu, char *Title);
void MenuTAItems(GMENU *ParentMenu, char *Title);

void MenuSetupTimeAttend(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[30];
	unsigned char	cVal;
	int		rval;
	
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);	
	while(1) {
		MenuClearItems(menu);
		MenuAddItem(menu,  xmenu_time_attend[0], 0, GMENU_ITEM_ENABLE);
		MenuAddItem(menu,  xmenu_time_attend[2], 0, GMENU_ITEM_ENABLE);
		MenuAddItem(menu,  xmenu_time_attend[3], 0, GMENU_ITEM_ENABLE);	
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuTAKeyOption(menu, ItemText);	
			break;
		case 2:
			MenuTAItems(menu, ItemText);
			break;
		case 3:
			MakeGuideNumberSecond(temp, 0, 255);
			cVal = sys_cfg->FuncKeyTimer;
			rval = WndInputNumber((GWND *)menu, ItemText, temp, &cVal, 1, 0, 255, R_INVALID_TIME_INTERVAL, 0);
			if(rval > 2) _TA_Save(menu, ItemText, &sys_cfg->FuncKeyTimer, cVal);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuTAKeyOption(GMENU *ParentMenu, char *Title)
{
	unsigned char	OldVal, NewVal;
	int		rval, val;
		
	val = taGetKeyOption();
	if(val < 1) val = 1;
	NewVal = (unsigned char)val;
	rval = MenuRadio((GWND *)ParentMenu, Title, &NewVal, xmenu_ta_key, 2, 0, 0);	// Title, pVal, menuStr[], menuSize, FromZero, Section
	if(rval > 2) {
		OldVal = taGetOption();
		taSetKeyOption((int)NewVal);
		NewVal = taGetOption();
		taSetOption((int)OldVal);
		if(OldVal != NewVal) _TA_Save(ParentMenu, Title, &sys_cfg->TAOption, NewVal);
	}
}

//void MenuTAItem(GMENU *ParentMenu, int Item);
void WizardTAItem(int Item);

void MenuTAItems(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*p, Text[60];
	int		i, rval, size;

	AdminConsoleUnsupported();
	size = taGetItemSize();
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		_TAItemOffset = menu->ItemCount;
		for(i = 0;i < size;i++) {
			p = Text;
			if(MenuGetAdminConsole()) {
				_MakeTAItemPrefix(p, i); p += strlen(p);
				_MakeConsoleTAItem(p, i);
				MenuAddItem(menu, Text, 0, GMENU_ITEM_ENABLE);	
			} else {
				MenuAddItemCallBack(menu, GMENU_ITEM_ENABLE, _PutNonConsoleTAItem);
			}
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		//MenuTAItem(menu, rval - 1);
		WizardTAItem(rval - 1);
	}
	MenuTerminate(menu);
}

int _TAItemEnable(GWND *WizardWnd, int Wizard, int Item, unsigned char *Option);
int _TAItemText(GWND *WizardWnd, int Wizard, int Item, unsigned char *Index);
int _TAItemTimeSection(GWND *WizardWnd, int Wizard, int Item, unsigned char Index, unsigned char *Tms);
int _TAItemFinalStage(GWND *WizardWnd, int Wizard, int Item, unsigned char Option, unsigned char Index, unsigned char *Tms);
int _SaveTAItemSetup(GWND *WizardWnd, char *Title, int Item, unsigned char NewOption, unsigned char NewIndex, unsigned char *NewTms);

void WizardTAItem(int Item)
{
	GWND	*wnd, _wnd;
	char	Title[32];
	unsigned char	Tms[4], Option, Index;
	int		rval, Stage, Wizard;

	_MakeTAItemPrefix(Title, Item);
	if(taItemIsEnable(Item)) Option = 0x01; else Option = 0x00;
	Index = (unsigned char)taGetItemIndex(Item);
	if(Item == 1 || Item == 2) memcpy(Tms, sys_cfg->InOutTimeSection[Item-1], 4);
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		if(Item == 1 || Item == 2) {
			if(Stage < 4) Wizard |= GWND_WIZARD_NEXT;
		} else {
			if(Stage < 3) Wizard |= GWND_WIZARD_NEXT;
		}			
		if(Stage > 1) Wizard |= GWND_WIZARD_BACK;
		switch(Stage) {
		case 1:	rval = _TAItemEnable(wnd, Wizard, Item, &Option); break;
		case 2: rval = _TAItemText(wnd, Wizard, Item, &Index); break;
		case 3: if(Item == 1 || Item == 2) _TAItemTimeSection(wnd, Wizard, Item, Index, Tms);
				else	rval = _TAItemFinalStage(wnd, Wizard, Item, Option, Index, Tms);
				break;
		case 4: rval = _TAItemFinalStage(wnd, Wizard, Item, Option, Index, Tms); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else if(rval == 1) {
			Stage--;
		} else {
			if(Wizard & GWND_WIZARD_NEXT) Stage++;
			else {	// Final Stage
				_SaveTAItemSetup(wnd, Title, Item, Option, Index, Tms);
				Stage = 0;
			}
		}
		taskYield();
	}
}

int _TAItemEnable(GWND *WizardWnd, int Wizard, int Item, unsigned char *Option)
{
	int		rval;

	wndDrawClientArea(WizardWnd);
	rval = MenuNoYesWizard(WizardWnd, xmenu_ta_item[0], Option, 0x01, Wizard);
	return rval;
}

int _TAItemText(GWND *WizardWnd, int Wizard, int Item, unsigned char *Index)
{
	char	*xmenu[40];
	int		rval, i;

	wndDrawClientArea(WizardWnd);
	rval = taGetTextSize();
	for(i = 0;i < rval;i++) xmenu[i] = taGetTextAt(i);
	rval = MenuRadioWizard(WizardWnd, xmenu_ta_item[1], Index, xmenu, rval, 1, Wizard);
	return rval;
}

int _TAItemTimeSection(GWND *WizardWnd, int Wizard, int Item, unsigned char Index, unsigned char *Tms)
{
	GLABEL	*lblName, _lblName, *lblTime, _lblTime, *lblTimeRange, _lblTimeRange;
	GDATE	*tmsInput, _tmsInput;
	RECT	rect;
	int		rval, c, y;

	wndDrawClientArea(WizardWnd);
	y = 8;
	lblName = &_lblName;
	labelInitialize(lblName, WizardWnd, 8, y, 150, 24);
	labelSetWindowText(lblName, taGetTextAt(Index));		
	y += 24;
	lblTime = &_lblTime;
	labelInitialize(lblTime, WizardWnd, 16, y, 150, 24);
	labelSetWindowText(lblTime, xmenu_ta_sub[0]); 
	y += 23;
	tmsInput = &_tmsInput;
	dateInitialize(tmsInput, WizardWnd, 24, y, 130, 26); 
	dateSetFont(tmsInput, GWND_NORMAL_FONT);
	dateModifyStyle(tmsInput, ES_PASSWORD|ES_READONLY, ES_NUMBER|ES_WILDDIGIT);
	dateSetType(tmsInput, DT_TIME_SECTION);		
	memcpy(tmsInput->Data, Tms, 4);				
	lblTimeRange = &_lblTimeRange;
	labelInitialize(lblTimeRange, WizardWnd, 160, y+1, 100, 24);
	labelSetWindowText(lblTimeRange, "(hh:mm-hh:mm)"); 
	wndGetClientArea(WizardWnd, &rect);
	
	while(1) {
		labelDrawWindow(lblName);
		labelDrawWindow(lblTime);
		labelDrawWindow(lblTimeRange);
		dateDrawWindow(tmsInput);
		y = 82;		// 8 + 24 + 23 + 27	
		DrawInputDateTimeDesp(rect.x+40, rect.y+y);
		rval = dateProcessWindow(tmsInput, Wizard);
		if(rval > 0) {
			dateDrawWindow(tmsInput); c = tmsInput->Edit.LastKey;
		}	
		if(rval < 1) break;
		rval = 0;
		if(c == KEY_F1 || c == KEY_F4 || c == KEY_OK) {
			if(!tmsIsNull(tmsInput->Data) && !tmsValidateEx(tmsInput->Data)) {
				ResultMsg(WizardWnd, xmenu_ta_sub[0], R_INVALID_DATA);
			} else {
				memcpy(Tms, tmsInput->Data, 4);
				if(c == KEY_F1) rval = 1; else rval = 2;
			}
		}
		if(rval) break;
		taskYield();
	}
	return rval;
}

int _TAItemFinalStage(GWND *WizardWnd, int Wizard, int Item, unsigned char Option, unsigned char Index, unsigned char *Tms)
{
	RECT	rect;
	char	temp[32];
	unsigned long	timer;
	int		c, x, y, fh, BtnFile, Color, timeout, contflg;

	wndDrawClientArea(WizardWnd);
	Color = wndGetBackGroundTextColor(WizardWnd);
	wndGetClientArea(WizardWnd, &rect);
	fh = GetFontHeight() + 4;
	x = rect.x + 10; y = rect.y + 10;
	lcdPutText(x, y, xmenu_ta_item[0], Color);
	if(Option & 0x01) c = BOX_CHECKED; else c = BOX_UNCHECKED;
	lcdPutIconFile(x+110, y, fh, c);
	y += fh;
	lcdPutText(x, y, xmenu_ta_item[1], Color);
	lcdPutText(x+110, y, taGetTextAt(Index), Color);
	if(Item == 1 || Item == 2) {
		y += fh;
		lcdPutText(x, y, xmenu_ta_sub[0], Color);
		TimeSection2Str(Tms, temp);
		lcdPutText(x+110, y, temp, Color);
	}

	if(Wizard & GWND_WIZARD_BACK) c = BTN_BACK; else c = 0;
	_DrawLowerButtons(c, 0, 0, BTN_OK, 0);
	timer = DS_TIMER; timeout = 150;
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

int _SaveTAItemSetup(GWND *WizardWnd, char *Title, int Item, unsigned char NewOption, unsigned char NewIndex, unsigned char *NewTms)
{
	unsigned char	OldTms[4], OldOption, OldIndex, OldVal, NewVal;
	int		rval, val;

	if(taItemIsEnable(Item)) OldOption = 0x01; else OldOption = 0x00;
	OldIndex = taGetItemIndex(Item);
	rval = 0;
	if(OldOption != NewOption || OldIndex != NewIndex) {
		OldVal = taGetItem(Item);
		taSetItemEnable(Item, (int)NewOption);
		taSetItemIndex(Item, NewIndex);
		NewVal =  taGetItem(Item);
		taSetItem(Item, (int)OldVal);
		if(OldVal != NewVal) _TA_Save((GMENU *)WizardWnd, Title, &sys_cfg->TAItems[Item], NewVal);
	}
	if(rval >= 0 && (Item == 1 || Item == 2)) {
		val = rval;
		memcpy(OldTms, sys_cfg->InOutTimeSection[Item-1], 4);
		if(n_memcmp(OldTms, NewTms, 4)) {
			memcpy(sys_cfg->InOutTimeSection[Item-1], NewTms, 4);
			rval = syscfgWrite(sys_cfg);
			if(rval == 0) {
				if(!val) ResultMsg(WizardWnd, Title, R_DATA_SAVED);
				rval = 1;
			} else {
				memcpy(sys_cfg->InOutTimeSection[Item-1], OldTms, 4);
				if(!val) ResultMsg(WizardWnd, Title, R_MSG_SYSTEM_ERROR);
				rval = -1;
			}
		}
	}
	return rval;
}

/*
void _PutNonConsoleTAItemTitle(GMENU *menu, int Item, int x, int y)
{
	char	Prefix[8], Text[32];
	int		fh, fColor, File;

	fColor = menu->BackGroundTextColor;
	Item = menu->UserItem;
	x = menu->x; y = menu->y;
	_MakeTAItemPrefix(Prefix, Item);
	if(taItemIsEnable(Item)) File = BOX_CHECKED; else File = BOX_UNCHECKED;
	strcpy(Text, taGetTextAt(taGetItemIndex(Item)));
	x += 8;
	x += lcdPutText(x, y+1, Prefix, fColor);
	fh = GetFontHeight();
	lcdPutIconFile(x, y+1, 24, File); x += fh+2;
	x += lcdPutText(x, y+1, Text, fColor);
}

void MenuTAItemEnable(GMENU *ParentMenu, int Item);
void MenuTAItemText(GMENU *ParentMenu, int Item);

void MenuTAItem(GMENU *ParentMenu, int Item)
{
	GMENU	*menu, _menu;
	char	*p, temp[32];
	int		rval, menuSize;

	menu = &_menu;
//	if(Item) menuSize = 2;
//	else	menuSize = 1;
	menuSize = 2;
	if(MenuGetAdminConsole()) {
		p = temp;
		_MakeTAItemPrefix(p, Item); p += strlen(p); *p++ = ' ';
		_MakeConsoleTAItem(p, Item);
//		if(Item) MenuInitialize(menu, temp, xmenu_ta_item, menuSize);
//		else	 MenuInitialize(menu, temp, &xmenu_ta_item[1], menuSize);
		MenuInitialize(menu, temp, xmenu_ta_item, menuSize);
	} else {
//		if(Item) MenuInitialize(menu, NULL, xmenu_ta_item, menuSize);
//		else	 MenuInitialize(menu, NULL, &xmenu_ta_item[1], menuSize);
		MenuInitialize(menu, NULL, xmenu_ta_item, menuSize);
		menuSetPutTitleCallBack(menu, _PutNonConsoleTAItemTitle);
	}
	menuSetUserItem(menu, Item);	
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		switch(rval) {
		case 1:
//			if(Item) MenuTAItemEnable(menu, Item);
//			else	 MenuTAItemText(menu, Item);
			MenuTAItemEnable(menu, Item);
			break;
		case 2:
			MenuTAItemText(menu, Item);
			break;
		}
	}
	MenuTerminate(menu);
}

void _GetTAItemSubTitle(char *title, char *ta_item, int Item)
{
	int		size;
	
	size = taGetItemSize();
	if(size > 10) {
		if(GetBidi()) {	
			if(Item == 0) sprintf(title, "%s: No", ta_item);
			else	sprintf(title, "%s:F%02d", ta_item, Item);
		} else {
			if(Item == 0) sprintf(title, "No :%s", ta_item);
			else	sprintf(title, "F%02d:%s", Item, ta_item);		
		}		
	} else {
		if(GetBidi()) {	
			if(Item == 0) sprintf(title, "%s:No", ta_item);
			else	sprintf(title, "%s:F%d", ta_item, Item);
		} else {
			if(Item == 0) sprintf(title, "No:%s", ta_item);
			else	sprintf(title, "F%d:%s", Item, ta_item);		
		}
	}
}

void MenuTAItemEnable(GMENU *ParentMenu, int Item)
{
	char	title[32];
	unsigned char	cVal;
	int		rval, val;
	
	_GetTAItemSubTitle(title, xmenu_ta_item[0], Item);
	if(taItemIsEnable(Item)) val = 1;
	else	val = 2;
	cVal = (unsigned char)val;	
	rval = MenuRadio((GWND *)ParentMenu, title, &cVal, xmenu_yesno, 2, 0, 0);	// Title, pVal, menuStr[], menuSize, FromZero, Section
	if(rval > 2) {
		val = taGetItem(Item);
		if(cVal == 1) rval = 1; else rval = 0;
		taSetItemEnable(Item, rval);
		rval = WndSaveConfig((GWND *)ParentMenu, "", 6);
		if(rval < 0) taSetItem(Item, val);
			ResultMsg((GWND *)ParentMenu, "", R_MSG_SYSTEM_ERROR);
		}
	}
}

void MenuTAItemText(GMENU *ParentMenu, int Item)
{
	GMENU	*menu, _menu;
	char	title[32], *p;
	int		rval, i, val, max;

	_GetTAItemSubTitle(title, xmenu_ta_item[1], Item);
	menu = &_menu;
	MenuInitialize(menu, title, NULL, 0);
//	menuSetBorderBox(menu, 1);
	menuSetSelectNumber(menu, 0);
	max = taGetTextSize();
	for(i = 0;i < max;i++) {
		p = taGetTextAt(i);
		MenuAddItem(menu, p, 0, GMENU_ITEM_ENABLE);
	}
	val = taGetItemIndex(Item);
	if(val < 0 || val >= max) val = 0;
	menuSetCurrentItem(menu, val);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
	if(rval <= 0) return;
	rval--;
	if(rval != val) {
		val = taGetItem(Item);
		taSetItemIndex(Item, rval);
		rval = WndSaveConfig((GWND *)ParentMenu, "", 6);
		if(rval < 0) taSetItem(Item, val);
			ResultMsg((GWND *)ParentMenu, "", R_MSG_SYSTEM_ERROR);
		}
	}
}
*/
