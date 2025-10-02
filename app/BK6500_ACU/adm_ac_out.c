
void MenuBinaryOutput(GMENU *ParentMenu, char *Title, int adID, int biID);

void MenuBinaryOutputs(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[32];
	int		rval, i, Item, IDs[MAX_BO_SZ];

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {	
		MenuClearItems(menu);
		for(Item = i = 0;i < MAX_BO_SZ;i++) {
			if(boIsEnable(i) && boGetAssignedDoor(i) < 0 && !boIsAssignedAlarmAction(i)) {
				GetBinaryOutputName(i, temp);
				MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
				IDs[Item] = i; Item++;
			}
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		MenuBinaryOutput(menu, ItemText, -1, IDs[rval-1]);
	}
	MenuTerminate(menu);
}

void MenuBinaryOutput(GMENU *ParentMenu, char *Title, int adID, int boID)
{
	GMENU	*menu, _menu;
	BINARY_OUTPUT_CFG	*bo_cfg;
	char	*ItemText;
	int		rval, UnitID, IOUnitID;

	IOUnitID = boGetIOUnit(boID);
	if(IOUnitID < 0) UnitID = boGetUnit(boID);
	bo_cfg = &sys_cfg->BinaryOutputs[boID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		MenuAddItem(menu, xmenu_binary_output[0], 0, GMENU_ITEM_ENABLE);
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			rval = MenuRadio((GWND *)menu, ItemText, &bo_cfg->Polarity, xmenu_io_polarity, 2, 1, 1);
#ifndef STAND_ALONE
			if(rval > 2) {
				if(IOUnitID >= 0) iounitEncodeConfig(IOUnitID);
				else	unitEncodeIouConfig(UnitID);
			}
#endif
			break;
		}
	}
	MenuTerminate(menu);
}
