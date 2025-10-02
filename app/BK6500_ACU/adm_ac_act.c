void MenuAlarmAction(GMENU *ParentMenu, char *Title, int actID);

void MenuAlarmActions(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[32];
	int		rval, i, Item, IDs[MAX_UNIT_SZ];

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {	
		MenuClearItems(menu);
		for(Item = i = 0;i < MAX_UNIT_SZ;i++) {
			if(actIsUsable(i)) {
				rval = GetAlarmActionIcon(i);
				GetAlarmActionName(i, temp);
				MenuAddItem(menu, temp, rval, GMENU_ITEM_ENABLE);
				IDs[Item] = i; Item++;
			}
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		MenuAlarmAction(menu, ItemText, IDs[rval-1]);
	}
	MenuTerminate(menu);
}

void MenuAlarmOutput(GMENU *ParentMenu, char *Title, int actID);
void MenuActionCommand(GMENU *ParentMenu, char *Title, int actID);

void MenuAlarmAction(GMENU *ParentMenu, char *Title, int actID)
{
	GMENU	*menu, _menu;
	ALARM_ACTION_CFG	*act_cfg;
	char	*ItemText, temp[32];
	int		rval, boID;

	act_cfg = &sys_cfg->AlarmActions[actID];
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		MenuClearItems(menu);
		MenuAddItem(menu, Title, 0, GMENU_ITEM_ENABLE);
		if(actIsEnable(actID)) {
			MenuAddItem(menu, xmenu_alarm[2], 0, GMENU_ITEM_ENABLE);			
			boID = act_cfg->AlarmOutput;
			GetBinaryOutputName(boID, temp);
			MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);		
		switch(rval) {
		case 1:
			MenuAlarmOutput(menu, ItemText, actID);
			break;
		case 2:
			MenuActionCommand(menu, ItemText, actID);
			break;
		case 3:
			MenuBinaryOutput(menu, ItemText, 0, boID);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuAlarmOutput(GMENU *ParentMenu, char *Title, int actID)
{
	ALARM_ACTION_CFG	*act_cfg;
	unsigned char	cVal, Outputs[4];
	char		*xmenuStr[5], temp[4][24];
	int			rval, i, boID, Item, IDs[4];
	
	act_cfg = &sys_cfg->AlarmActions[actID];
	xmenuStr[0] = xmenu_cam_mode[0];
	Item = actGetUsableAlarmOutputs(actID, Outputs);
	for(i = 0;i < Item;i++) {
		boID = Outputs[i];
		GetBinaryOutputName(boID, temp[i]);
		xmenuStr[i+1] = temp[i];
		IDs[i] = boID; 
	}
	boID = act_cfg->AlarmOutput;
	for(i = 0;i < Item;i++)
		if(IDs[i] == boID) break;
	if(i < Item) cVal = i + 1; else cVal = 0; 
	rval = MenuRadio((GWND *)ParentMenu, Title, &cVal, xmenuStr, Item+1, 1, 0);
	if(rval > 2) {
		if(cVal == 0) cVal = 0xff;
		else	cVal = IDs[cVal - 1];
		WndSaveByte((GWND *)ParentMenu, Title, &act_cfg->AlarmOutput, (int)cVal, 1);	// pVal, NewVal, Section
	}
}

void MenuActionCommandItem(GMENU *ParentMenu, char *Title, int actID, int Index);

void MenuActionCommand(GMENU *ParentMenu, char *Title, int actID)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_action_index, 12);	
	while(1) {	
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		MenuActionCommandItem(menu, ItemText, actID, rval-1);
	}
}

void MenuActionCommandItem(GMENU *ParentMenu, char *Title, int actID, int Index)
{
	ALARM_ACTION_CFG	*act_cfg;
	char	*xmenuStr[MAX_OC_SZ+1], temp[MAX_OC_SZ][24];
	unsigned char	cVal, IDs[MAX_OC_SZ];
	int		i, Item, rval;

	act_cfg = &sys_cfg->AlarmActions[actID];
	xmenuStr[0] = xmenu_other[11];	
	for(Item = i = 0;i < MAX_OC_SZ;i++) {
		if(ocExist(i)) {
			GetOutputCommandName(i, temp[Item]); xmenuStr[Item+1] = temp[Item];
			IDs[Item] = i; Item++;
		}
	}
	cVal = act_cfg->OutputCommands[Index];
	for(i = 0;i < Item;i++)
		if(IDs[i] == cVal) break;
	if(i < Item) cVal = i + 1; else cVal = 0; 
	rval = MenuRadio((GWND *)ParentMenu, Title, &cVal, xmenuStr, Item+1, 1, 0);
	if(rval > 2) {
		if(cVal == 0) cVal = 0xff;
		else	cVal = IDs[cVal - 1];
		WndSaveByte((GWND *)ParentMenu, Title, &act_cfg->OutputCommands[Index], (int)cVal, 1);	// pVal, NewVal, Section
	}
}
