void MenuInitDeviceEvent(GMENU *ParentMenu, char *Title);
void MenuInitDeviceDefault(GMENU *ParentMenu, char *Title);
void MenuInitDeviceFactory(GMENU *ParentMenu, char *Title);

void MenuInitDevice(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	AdminConsoleUnsupported();
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_init_device, 3);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuInitDeviceEvent(menu, ItemText);
			break;
		case 2:
			MenuInitDeviceDefault(menu, ItemText);
			break;
		case 3:
			MenuInitDeviceFactory(menu, ItemText);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuInitDeviceEvent(GMENU *ParentMenu, char *Title)
{
	int		rval;
	
	rval = wndMessageBox((GWND *)ParentMenu, Title, Title, MB_YESNO);
	if(rval != IDYES) return;
	ResultMsgNoWait((GWND *)ParentMenu, Title, R_PROCESSING);
	wdtResetLong();
	rval = fsRemoveAllCamImage();
	rval = fsRemoveAllEvent();
	wdtReset();
	if(rval < 0) ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
	else	ResultMsg((GWND *)ParentMenu, Title, R_PROCESSED);
}

void MenuInitDeviceDefault(GMENU *ParentMenu, char *Title)
{
	int		rval;

	rval = wndMessageBox((GWND *)ParentMenu, Title, Title, MB_YESNO);
	if(rval != IDYES) return;
	ResultMsgNoWait((GWND *)ParentMenu, Title, R_PROCESSING);
	syscfgReset(sys_cfg);
	ResultMsg((GWND *)ParentMenu, Title, R_PROCESSED);
	ResultMsg((GWND *)ParentMenu, Title, R_SYSTEM_RESTART);
	ResetSystem();
}

void MenuInitDeviceFactory(GMENU *ParentMenu, char *Title)
{
	int		rval;

	rval = wndMessageBox((GWND *)ParentMenu, Title, Title, MB_YESNO);
	if(rval != IDYES) return;
	ResultMsgNoWait((GWND *)ParentMenu, Title, R_PROCESSING);
	rval = fsRemoveAllCamImage();
	rval = fsRemoveAllEvent();
	if(rval < 0) {
		ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
		return;
	}
	rval = userfsRemoveAll();
	if(rval < 0) {
		ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
		return;
	}
	rval = schefsRemoveAll();
	if(rval < 0) {
		ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
		return;
	}
	rval = calfsRemoveAll();
	if(rval < 0) {
		ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
		return;
	}
	rval = arfsRemoveAll();
	if(rval < 0) {
		ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
		return;
	}	
	rval = pnofsRemoveAll();
	if(rval < 0) {
		ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
		return;
	}
	syscfgReset(sys_cfg);
	ResultMsg((GWND *)ParentMenu, Title, R_PROCESSED);	
	ResultMsg((GWND *)ParentMenu, Title, R_SYSTEM_RESTART);
	ResetSystem();
}

