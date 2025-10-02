void MenuFireZone(GMENU *ParentMenu, char *Title, int faID);

void MenuFireZones(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[32];
	int		rval, i, Item, IDs[MAX_UNIT_HALF_SZ];

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {	
		MenuClearItems(menu);
		for(Item = i = 0;i < MAX_UNIT_HALF_SZ;i++) {
			rval = GetFireZoneIcon(i);
			GetFireZoneName(i, temp);
			MenuAddItem(menu, temp, rval, GMENU_ITEM_ENABLE);
			IDs[Item] = i; Item++;
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		MenuFireZone(menu, ItemText, IDs[rval-1]);
	}
	MenuTerminate(menu);
}

void MenuFireZoneFirePoints(GMENU *ParentMenu, char *Title, int fzID);
void MenuFireZoneAccessDoors(GMENU *ParentMenu, char *Title, int fzID);
void MenuFireZoneAlarmActions(GMENU *ParentMenu, char *Title, int fzID);

void MenuFireZone(GMENU *ParentMenu, char *Title, int fzID)
{
	GMENU	*menu, _menu;
	FIRE_ZONE_CFG	*fz_cfg;
	char	*ItemText;
	int		rval;

	fz_cfg = &sys_cfg->FireZones[fzID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		MenuAddItem(menu, Title, 0, GMENU_ITEM_ENABLE);
		if(fzGetEnable(fzID)) {
			MenuAddItem(menu, xmenu_fire_zone[0], 0, GMENU_ITEM_ENABLE);
			MenuAddItem(menu, xmenu_fire_zone[1], 0, GMENU_ITEM_ENABLE);
			MenuAddItem(menu, xmenu_fire_zone[2], 0, GMENU_ITEM_ENABLE);
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);		
		switch(rval) {
		case 1:
			MenuNoYes((GWND *)menu, ItemText, &fz_cfg->Enable, 0x01, 1);
			break;
		case 2:
			MenuFireZoneFirePoints(menu, ItemText, fzID);
			break;
		case 3:
			MenuFireZoneAccessDoors(menu, ItemText, fzID);		
			break;
		case 4:
			MenuFireZoneAlarmActions(menu, ItemText, fzID);		
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuFireZoneFirePoints(GMENU *ParentMenu, char *Title, int fzID)
{
	GMENU	*menu, _menu;
	char	temp[32];
	unsigned char	OldData[4];
	int		i, rval, Item, IDs[MAX_AD_SZ];

	if(ParentMenu->WndType == 1) menuDarken(ParentMenu); 
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	menuSetParentWnd(menu, (GWND *)ParentMenu);
	for(Item = i = 0;i < MAX_UNIT_HALF_SZ;i++) {
		if(fpIsUsable(i)) {
			GetFirePointName(i, temp);
			MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
			if(fzGetFirePoint(fzID, i)) menuSetItemChecked(menu, Item, 1);		// 2017.10.26 bug fixed: fzGetFirePoint <= fzGetAccessDoor
			IDs[Item] = i; Item++; 
		}
	}
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_CHECK);
	menuSetSelectNumber(menu, 0);
	_SetMenuProperSize(menu);
	//_SetSelectMenuColor(menu);
	menuSetCurrentItem(menu, 0);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
	if(rval <= 0) return;
	memcpy(OldData, sys_cfg->FireZones[fzID].FirePointBits, 2);
	for(i = 0;i < Item;i++) {
		if(menuGetItemChecked(menu, i)) rval = 1; else rval = 0;
		fzSetFirePoint(fzID, IDs[i], rval);
	}
	if(n_memcmp(OldData, sys_cfg->FireZones[fzID].FirePointBits, 2)) {
		rval = WndSaveConfig((GWND *)menu, Title, 1);
		if(rval < 0) memcpy(sys_cfg->FireZones[fzID].FirePointBits, OldData, 2);
	}
}

void MenuFireZoneAccessDoors(GMENU *ParentMenu, char *Title, int fzID)
{
	GMENU	*menu, _menu;
	char	temp[32];
	unsigned char	OldData[4];
	int		i, rval, Item, IDs[MAX_AD_SZ];

	if(ParentMenu->WndType == 1) menuDarken(ParentMenu); 
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	menuSetParentWnd(menu, (GWND *)ParentMenu);
	for(Item = i = 0;i < MAX_AD_SZ;i++) {
		if(adIsEnable(i)) {
			GetAccessDoorName(i, temp);
			MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
			if(fzGetAccessDoor(fzID, i)) menuSetItemChecked(menu, Item, 1);
			IDs[Item] = i; Item++; 
		}
	}
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_CHECK);
	menuSetSelectNumber(menu, 0);
	_SetMenuProperSize(menu);
	//_SetSelectMenuColor(menu);
	menuSetCurrentItem(menu, 0);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
	if(rval <= 0) return;
	memcpy(OldData, sys_cfg->FireZones[fzID].AccessDoorBits, 3);
	for(i = 0;i < Item;i++) {
		if(menuGetItemChecked(menu, i)) rval = 1; else rval = 0;
		fzSetAccessDoor(fzID, IDs[i], rval);
	}
	if(n_memcmp(OldData, sys_cfg->FireZones[fzID].AccessDoorBits, 3)) {
		rval = WndSaveConfig((GWND *)menu, Title, 1);
		if(rval < 0) memcpy(sys_cfg->FireZones[fzID].AccessDoorBits, OldData, 3);
	}
}

void MenuFireZoneAlarmActions(GMENU *ParentMenu, char *Title, int fzID)
{
	GMENU	*menu, _menu;
	char	temp[32];
	unsigned char	OldData[4];
	int		i, rval, Item, IDs[MAX_AD_SZ];

	if(ParentMenu->WndType == 1) menuDarken(ParentMenu); 
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	menuSetParentWnd(menu, (GWND *)ParentMenu);
	for(Item = i = 0;i < MAX_UNIT_SZ;i++) {
		if(actIsEnable(i)) {
			GetAlarmActionName(i, temp);
			MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
			if(fzGetAlarmAction(fzID, i)) menuSetItemChecked(menu, Item, 1);
			IDs[Item] = i; Item++; 
		}
	}
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_CHECK);
	menuSetSelectNumber(menu, 0);
	_SetMenuProperSize(menu);
	//_SetSelectMenuColor(menu);
	menuSetCurrentItem(menu, 0);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
	if(rval <= 0) return;
	memcpy(OldData, sys_cfg->FireZones[fzID].AlarmActionBits, 3);
	for(i = 0;i < Item;i++) {
		if(menuGetItemChecked(menu, i)) rval = 1; else rval = 0;
		fzSetAlarmAction(fzID, IDs[i], rval);
	}
	if(n_memcmp(OldData, sys_cfg->FireZones[fzID].AlarmActionBits, 3)) {
		rval = WndSaveConfig((GWND *)menu, Title, 1);
		if(rval < 0) memcpy(sys_cfg->FireZones[fzID].AlarmActionBits, OldData, 3);
	}
}

void MenuFirePoints(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[32];
	int		rval, i, biID, Item, IDs[MAX_UNIT_HALF_SZ];;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		for(Item = i = 0;i < MAX_UNIT_HALF_SZ;i++) {
			if(fpIsEnable(i)) {
				GetFirePointName(i, temp);
				MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
				IDs[Item] = i; Item++; 
			}
		}
		if(Item <= 0) break;
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		biID = fpGetBinaryInput(IDs[rval-1]);
		GetBinaryInputName(biID, temp);
		MenuBinaryInput(menu, temp, 0, biID);
	}
	MenuTerminate(menu);
}
