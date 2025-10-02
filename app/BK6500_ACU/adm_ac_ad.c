void MenuAccessDoor(GMENU *ParentMenu, char *Title, int adID);

void MenuAccessDoors(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[32];
	int		rval, i, Item, IDs[MAX_AD_SZ];

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {	
		MenuClearItems(menu);
		for(Item = i = 0;i < MAX_AD_SZ;i++) {
			if(adIsEnable(i)) {
				GetAccessDoorName(i, temp);
				MenuAddItem(menu, temp, NULL, GMENU_ITEM_ENABLE);
				IDs[Item] = i; Item++;
			}
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		MenuAccessDoor(menu, ItemText, IDs[rval-1]);
	}
	MenuTerminate(menu);
}

void MenuDoorType(GMENU *ParentMenu, char *Title, int adID);
void MenuAccessPoint(GMENU *ParentMenu, char *Title, int adID, int apID);
void MenuDoorInputOutput(GMENU *ParentMenu, char *Title, int adID);
void MenuDoorAlarm(GMENU *ParentMenu, char *Title, int adID);

void MenuAccessDoor(GMENU *ParentMenu, char *Title, int adID)
{
	GMENU	*menu, _menu;
	ACCESS_DOOR_CFG		*ad_cfg;
	char	*ItemText, temp[32];
	int		rval, i, apID, exitID;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	ad_cfg = &sys_cfg->AccessDoors[adID];
	while(1) {
		MenuClearItems(menu);
		apID = adGetAccessPoint(adID);
		GetAccessPointName(apID, temp);
		rval = GetAccessPointIcon(apID);
		MenuAddItem(menu, temp, rval, GMENU_ITEM_ENABLE);
		exitID = adGetExitPoint(adID);
		if(exitID >= 0) {
			GetAccessPointName(exitID, temp);
			rval = GetAccessPointIcon(exitID);
			MenuAddItem(menu, temp, rval, GMENU_ITEM_ENABLE);
		}
		for(i = 0;i < 7;i++) MenuAddItem(menu, xmenu_access_door[i], 0, GMENU_ITEM_ENABLE);
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);	
		if(rval > 1 && exitID < 0) rval += 1;
		switch(rval) {
		case 1:
			MenuAccessPoint(menu, ItemText, adID, apID);
			break;
		case 2:
			MenuAccessPoint(menu, ItemText, adID, exitID);
			break;
		case 3:
			MakeGuideNumberSecondPulse(temp, 255);
			rval = WndInputNumber((GWND *)menu, ItemText, temp, &ad_cfg->DoorPulseTime, 1, 0, 255, R_INVALID_TIME_INTERVAL, 1);
#ifndef STAND_ALONE
			if(rval > 2) unitEncodeIouConfig(adID);
#endif
			break;
		case 4:
			MenuDoorInputOutput(menu, ItemText, adID);
			break;
		case 5:
			MakeGuideNumberSecond(temp, 0, 255);		
			rval = WndInputNumber((GWND *)menu, ItemText, temp, &ad_cfg->TimeDelay, 1, 0, 255, R_INVALID_TIME_INTERVAL, 1);
#ifndef STAND_ALONE
			if(rval > 2) unitEncodeIouConfig(adID);	
#endif
			break;
		case 6:
			MakeGuideNumberSecond(temp, 0, 255);
			rval = WndInputNumber((GWND *)menu, ItemText, temp, &ad_cfg->TimeDelayNormal, 1, 0, 255, R_INVALID_TIME_INTERVAL, 1);
#ifndef STAND_ALONE
			if(rval > 2) unitEncodeIouConfig(adID);	
#endif
			break;
		case 7:
			MenuDoorAlarm(menu, ItemText, adID);
			break;
		case 8:
			MenuInputSchedule((GWND *)menu, ItemText, &ad_cfg->LockSchedule, 1);
			break;
		case 9:
			MenuInputSchedule((GWND *)menu, ItemText, &ad_cfg->UnlockSchedule, 1);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuLockOutput(GMENU *ParentMenu, char *Title, int adID);
void MenuRequestToExitInput(GMENU *ParentMenu, char *Title, int adID);
void MenuDoorStatusInput(GMENU *ParentMenu, char *Title, int adID);
void MenuAuxLockOutput(GMENU *ParentMenu, char *Title, int adID);

void MenuDoorInputOutput(GMENU *ParentMenu, char *Title, int adID)
{
	GMENU	*menu, _menu;
	ACCESS_DOOR_CFG	*ad_cfg;
	char	*ItemText;
	int		rval, i, max;

	ad_cfg = &sys_cfg->AccessDoors[adID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		if(boIsEnable(adGetAuxLockOutput(adID))) max = 4;
		else	max = 3;
		for(i = 0;i < max;i++) MenuAddItem(menu, xmenu_door_inout[i], 0, GMENU_ITEM_ENABLE);
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuLockOutput(menu, ItemText, adID);
			break;
		case 2:
			MenuRequestToExitInput(menu, ItemText, adID);
			break;
		case 3:
			MenuDoorStatusInput(menu, ItemText, adID);
			break;
		case 4:
			MenuAuxLockOutput(menu, ItemText, adID);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuBinaryOutput(GMENU *ParentMenu, char *Title, int adID, int boID);
void MenuBinaryInput(GMENU *ParentMenu, char *Title, int adID, int biID);

void MenuLockOutput(GMENU *ParentMenu, char *Title, int adID)
{
	GMENU	*menu, _menu;
	ACCESS_DOOR_CFG	*ad_cfg;
	char	*ItemText, temp[32];
	int		rval, boID;

	ad_cfg = &sys_cfg->AccessDoors[adID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	MenuClearItems(menu);
	MenuAddItem(menu, xmenu_door_inout[4], 0, GMENU_ITEM_ENABLE);
	boID = adGetLockOutput(adID);
	GetBinaryOutputName(boID, temp);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			rval = MenuRadio((GWND *)menu, ItemText, &ad_cfg->LockType, xmenu_lock_type, 2, 1, 1);
#ifndef STAND_ALONE
			if(rval > 2) unitEncodeIouConfig(adID);	
#endif
			break;
		case 2:
			MenuBinaryOutput(menu, ItemText, adID, boID);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuRequestToExitInput(GMENU *ParentMenu, char *Title, int adID)
{
	GMENU	*menu, _menu;
	ACCESS_DOOR_CFG	*ad_cfg;
	char	*ItemText, temp[32];
	int		rval, biID;

	ad_cfg = &sys_cfg->AccessDoors[adID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		MenuAddItem(menu, Title, 0, GMENU_ITEM_ENABLE);
		if(adGetRequestToExitEnable(adID)) {
			biID = adGetRequestToExitInput(adID);
			GetBinaryInputName(biID, temp);
			MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);		
		switch(rval) {
		case 1:
			rval = MenuNoYes((GWND *)menu, ItemText, &ad_cfg->RequestToExitEnable, 0x01, 1);
#ifndef STAND_ALONE
			if(rval > 2) unitEncodeIouConfig(adID);	
#endif
			break;
		case 2:
			MenuBinaryInput(menu, ItemText, adID, biID);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuDoorStatusInput(GMENU *ParentMenu, char *Title, int adID)
{
	GMENU	*menu, _menu;
	ACCESS_DOOR_CFG	*ad_cfg;
	char	*ItemText, temp[32];
	int		rval, biID;

	ad_cfg = &sys_cfg->AccessDoors[adID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		MenuAddItem(menu, Title, 0, GMENU_ITEM_ENABLE);
		if(adGetDoorStatusEnable(adID)) {
			biID = adGetDoorStatusInput(adID);
			GetBinaryInputName(biID, temp);
			MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);		
		switch(rval) {
		case 1:
			rval = MenuNoYes((GWND *)menu, ItemText, &ad_cfg->DoorStatusEnable, 0x01, 1);
#ifndef STAND_ALONE
			if(rval > 2) {
				unitEncodeIouConfig(adID);	
				if(unitGetType(adID) != 2 && crGetType(adID)) unitSetCruDoor(adID);
			}
#endif
			break;
		case 2:
			MenuBinaryInput(menu, ItemText, adID, biID);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuAuxLockOutput(GMENU *ParentMenu, char *Title, int adID)
{
	GMENU	*menu, _menu;
	ACCESS_DOOR_CFG	*ad_cfg;
	char	*ItemText, temp[32];
	int		rval, boID;

	ad_cfg = &sys_cfg->AccessDoors[adID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		MenuAddItem(menu, Title, 0, GMENU_ITEM_ENABLE);
		if(adGetAuxLockEnable(adID)) {
			boID = adGetAuxLockOutput(adID);
			GetBinaryOutputName(boID, temp);
			MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);		
		switch(rval) {
		case 1:
			rval = MenuNoYes((GWND *)menu, ItemText, &ad_cfg->AuxLockEnable, 0x01, 1);
#ifndef STAND_ALONE
			if(rval > 2) unitEncodeIouConfig(adID);
#endif
			break;
		case 2:
			MenuBinaryOutput(menu, ItemText, adID, boID);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuDoorAlarmInfo(GMENU *ParentMenu, char *Title, int adID, int AlarmID);
void WizardDoorAlarmInfo(char *Title, int adID, int AlarmID);

void MenuDoorAlarm(GMENU *ParentMenu, char *Title, int adID)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_door_alarm, 2);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		if(MenuGetAdminConsole()) MenuDoorAlarmInfo(menu, ItemText, adID, rval);
		else	WizardDoorAlarmInfo(ItemText, adID, rval);
	}
	MenuTerminate(menu);
}

void MenuDoorAlarmInfo(GMENU *ParentMenu, char *Title, int adID, int AlarmID)
{
	GMENU	*menu, _menu;
	ACCESS_DOOR_CFG	*ad_cfg;
	char	*ItemText, temp[32];
	unsigned char	*pVal;
	int		rval;
	
	ad_cfg = &sys_cfg->AccessDoors[adID-1];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		MenuAddItem(menu, xmenu_door_alarm_info[0], 0, GMENU_ITEM_ENABLE);
		if(AlarmID == 1) MenuAddItem(menu, xmenu_door_alarm_info[1], 0, GMENU_ITEM_ENABLE);
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		if(AlarmID != 1 && rval > 1) rval += 1;
		switch(rval) {
		case 1:
			if(AlarmID == 1) pVal = &ad_cfg->DoorOpenTooLongEnable;	 
			else			 pVal = &ad_cfg->DoorForcedOpenEnable;
			MenuRadio((GWND *)menu, ItemText, pVal, xmenu_noyes, 2, 1, 1);
			break;
		case 2:
			MakeGuideNumberSecond(temp, 0, 255);
			WndInputNumber((GWND *)menu, ItemText, temp, &ad_cfg->DoorOpenTooLongTime, 2, 0, 255, R_INVALID_TIME_INTERVAL, 1);
			break;
		}
	}
	MenuTerminate(menu);
}

int  _DoorAlarmEnable(GWND *WizardWnd, int *Wizard, unsigned char *NewVals, int adID, int AlarmID);
int  _DoorAlarmAction(GWND *WizardWnd, int *Wizard, unsigned char *NewVals, int adID, int AlarmID);

void WizardDoorAlarmInfo(char *Title, int adID, int AlarmID)
{
	GWND	*wnd, _wnd;
	ACCESS_DOOR_CFG	*ad_cfg;
	unsigned char	NewVals[4];
	int		rval, Stage, Wizard;

	ad_cfg = &sys_cfg->AccessDoors[adID];
	if(AlarmID == 1) {
		NewVals[0] = ad_cfg->DoorOpenTooLongTime;
		NewVals[1] = ad_cfg->DoorOpenTooLongEnable;	 
	} else {
		NewVals[0] = 0;
		NewVals[1] = ad_cfg->DoorForcedOpenEnable;
	}
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		//if(Stage < 2) Wizard |= GWND_WIZARD_NEXT;
		//if(Stage > 1) Wizard |= GWND_WIZARD_BACK;
		switch(Stage) {
		case 1:	rval = _DoorAlarmEnable(wnd, &Wizard, NewVals, adID, AlarmID); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else if(rval == 1) {
			Stage--;
		} else {
			if(Wizard & GWND_WIZARD_NEXT) Stage++;
			else {	// Final Stage
				_SaveDoorAlarm(wnd, Title, NewVals, adID, AlarmID);
				Stage = 0;
			}
		}
		taskYield();
	}
}

int _DoorAlarmEnable(GWND *WizardWnd, int *Wizard, unsigned char *NewVals, int adID, int AlarmID)
{
	GLABEL	*lblAlarmTime, _lblAlarmTime, *lblAlarmTime2, _lblAlarmTime2;
	GEDIT	*edtAlarmTime, _edtAlarmTime;
	RECT	rect, rect2;
	ACCESS_DOOR_CFG *ad_cfg;
	char	temp[32];
	int		rval, x, y, val, w_lbl, h_lbl, w_edt, h_edt;

	ad_cfg = &sys_cfg->AccessDoors[adID];
	wndDrawClientArea(WizardWnd);
	wndGetClientArea(WizardWnd, &rect);
	lblAlarmTime = &_lblAlarmTime; edtAlarmTime = &_edtAlarmTime; lblAlarmTime2 = &_lblAlarmTime2;
	w_lbl = 100; h_lbl = 24; w_edt = 42; h_edt = 26;  
	x = rect.x + 4; y = rect.y + 78;
	labelInitialize(lblAlarmTime, WizardWnd, x, y, w_lbl, h_lbl);
	y += 24; x = rect.x + 24;
	editInitialize(edtAlarmTime, WizardWnd, x, y, w_edt, h_edt); x += w_edt+16;
	labelInitialize(lblAlarmTime2, WizardWnd, x, y, 100, h_lbl);
	editSetFont(edtAlarmTime, GWND_NORMAL_FONT);
	editLimitText(edtAlarmTime, 3);
	editModifyStyle(edtAlarmTime, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
	labelSetWindowText(lblAlarmTime, xmenu_door_alarm_info[1]); 
	sprintf(edtAlarmTime->Text, "%d", (int)NewVals[0]);
	MakeGuideNumberDeciSecond(temp, 0, 255);	
	labelSetWindowText(lblAlarmTime2, temp);
	while(1) {
		if(AlarmID == 1) {
			labelDrawWindow(lblAlarmTime); editDrawWindow(edtAlarmTime); labelDrawWindow(lblAlarmTime2);
			x = rect.x + 60; y = rect.y + 152;
			lcdPutIconFile(x, y, 24, BTN_BACKSPACE); x += 26;
			lcdPutIconFile(x, y, 24, BTN_SHARP);
		}
		rect2.x = 16; rect2.y = 10; rect2.Width = 180; rect2.Height = 76;
		rval = MenuRadioWizardEx(WizardWnd, &rect2, xmenu_door_alarm_info[0], &NewVals[1], xmenu_noyes, 2, 1, *Wizard);
		if(rval == 3 && AlarmID == 1) rval = EditWizard(edtAlarmTime, *Wizard);
		if(rval < 1) break;
		if(rval == 1 || rval == 2 || AlarmID == 2 && rval == 3) {
			if(AlarmID == 1) {
				val = n_atol(edtAlarmTime->Text);
				if(val < 0 || val > 255) {
					ResultMsg(WizardWnd, xmenu_door_alarm_info[1], R_INVALID_TIME_INTERVAL);
					rval = 0;
				} else {
					NewVals[0] = val;
				}
			}
		} else	rval = 0;	
		if(rval) break;
		taskYield();
	}
	return rval;
}

int _SaveDoorAlarm(GWND *WizardWnd, char *Title, unsigned char *NewVals, int adID, int AlarmID)
{
	ACCESS_DOOR_CFG *ad_cfg;
	unsigned char	OldVals[4];
	int		rval;

	ad_cfg = &sys_cfg->AccessDoors[adID];
	if(AlarmID == 1) {
		OldVals[0] = ad_cfg->DoorOpenTooLongTime;
		OldVals[1] = ad_cfg->DoorOpenTooLongEnable;	 
	} else {
		OldVals[0] = 0;
		OldVals[1] = ad_cfg->DoorForcedOpenEnable;
	}
	if(n_memcmp(NewVals, OldVals, 2)) {
		if(AlarmID == 1) {
			ad_cfg->DoorOpenTooLongTime		= NewVals[0];
			ad_cfg->DoorOpenTooLongEnable	= NewVals[1];
		} else {
			ad_cfg->DoorForcedOpenEnable	= NewVals[1];
		}
		rval = WndSaveConfig(WizardWnd, Title, 1);
		if(rval < 0) {
			if(AlarmID == 1) {
				ad_cfg->DoorOpenTooLongTime		= OldVals[0];
				ad_cfg->DoorOpenTooLongEnable	= OldVals[1];
			} else {
				ad_cfg->DoorForcedOpenEnable	= OldVals[1];
			}
		}
	} else	rval = 0;
	return rval;
}
