void MenuBinaryInput(GMENU *ParentMenu, char *Title, int adID, int biID);

void MenuBinaryInputs(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[32];
	int		rval, i, Item, IDs[MAX_BI_SZ];

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {	
		MenuClearItems(menu);
		for(Item = i = 0;i < MAX_BI_SZ;i++) {
			if(biIsEnable(i) && biGetAssignedDoor(i) < 0 && biGetAssignedFirePoint(i) < 0) {
				GetBinaryInputName(i, temp);
				MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
				IDs[Item] = i; Item++;
			}
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		MenuBinaryInput(menu, ItemText, -1, IDs[rval-1]);
	}
	MenuTerminate(menu);
}

void MenuBinaryInput(GMENU *ParentMenu, char *Title, int adID, int biID)
{
	GMENU	*menu, _menu;
	BINARY_INPUT_CFG	*bi_cfg;
	char	*ItemText, temp[32];
	int		rval, UnitID, IOUnitID;

	bi_cfg = &sys_cfg->BinaryInputs[biID];
	IOUnitID = biGetIOUnit(biID);
	if(IOUnitID < 0) UnitID = biGetUnit(biID);
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		MenuAddItem(menu, xmenu_binary_input[0], 0, GMENU_ITEM_ENABLE);
		if(adID < 0) {
			MenuAddItem(menu, xmenu_binary_input[1], 0, GMENU_ITEM_ENABLE);
			MenuAddItem(menu, xmenu_binary_input[2], 0, GMENU_ITEM_ENABLE);
			MenuAddItem(menu, xmenu_binary_input[3], 0, GMENU_ITEM_ENABLE);
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			rval = MenuRadio((GWND *)menu, ItemText, &bi_cfg->Polarity, xmenu_io_polarity, 2, 1, 1);
#ifndef STAND_ALONE
			if(rval > 2) {
				if(IOUnitID >= 0) iounitEncodeConfig(IOUnitID);
				else	unitEncodeIouConfig(UnitID);
			}
#endif
			break;
		case 2:
			MakeGuideNumberSecond(temp, 0, 255);		
			rval = WndInputNumber((GWND *)menu, ItemText, temp, &bi_cfg->TimeDelay, 1, 0, 255, R_INVALID_TIME_INTERVAL, 1);
#ifndef STAND_ALONE
			if(rval > 2) {
				if(IOUnitID >= 0) iounitEncodeConfig(IOUnitID);
				else	unitEncodeIouConfig(UnitID);
			}
#endif
			break;
		case 3:
			MakeGuideNumberSecond(temp, 0, 255);		
			rval = WndInputNumber((GWND *)menu, ItemText, temp, &bi_cfg->TimeDelayNormal, 1, 0, 255, R_INVALID_TIME_INTERVAL, 1);
#ifndef STAND_ALONE
			if(rval > 2) {
				if(IOUnitID >= 0) iounitEncodeConfig(IOUnitID);
				else	unitEncodeIouConfig(UnitID);
			}
#endif
			break;
		case 4:
			MenuYesNo((GWND *)menu, ItemText, &bi_cfg->AlarmInhibit, 0x01, 1);
			break;
		}
	}
	MenuTerminate(menu);
}
