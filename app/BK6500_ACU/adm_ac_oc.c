void MenuNewOutputCommand(GMENU *ParentMenu, char *Title);
void WizardOutputCommand(char *Title, int ocID, int NewOC);

void MenuOutputCommands(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[32];
	int		rval, i, enable, Item, IDs[MAX_OC_SZ];

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {	
		MenuClearItems(menu);
		MenuAddItem(menu, xmenu_access_zone[0], 0, GMENU_ITEM_ENABLE);
		for(Item = i = 0;i < MAX_OC_SZ;i++) {
			if(ocExist(i)) {
				GetOutputCommandName(i, temp);
				if(i < 2) enable = 0; else enable = GMENU_ITEM_ENABLE;
				MenuAddItem(menu, temp, OUTPUT_COMMAND_ICON, enable);
				IDs[Item] = i; Item++;
			}
		}
		if(menu->CurrentItem >= menu->ItemCount) menu->CurrentItem = menu->ItemCount - 1;
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		if(rval == 1) MenuNewOutputCommand(menu, ItemText);
		else	WizardOutputCommand(ItemText, IDs[rval-2], 0);
	}
	MenuTerminate(menu);
}

void MenuNewOutputCommand(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[MAX_OC_SZ-2][30]; 
	int		rval, i, Item, IDs[MAX_OC_SZ-2];

	if(ParentMenu->WndType == 1) menuDarken(ParentMenu); 
	menu = &_menu;
	MenuInitialize(menu, Title,NULL, 0);
	menuSetParentWnd(menu, (GWND *)ParentMenu);
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_RADIO);
	menuSetSelectNumber(menu, 0);
	for(Item = i = 0;i < MAX_OC_SZ;i++) {
		if(!ocExist(i)) {
			GetOutputCommandName(i, temp[Item]);
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
		ItemText = MenuGetItemText(menu, rval - 1);	
		WizardOutputCommand(ItemText, IDs[rval-1], 1);
	}
}

int _ProcessOutputCommand(GWND *WizardWnd, int *Wizard, OUTPUT_COMMAND_CFG *NewCfg, int ocID);
int _SaveOutputCommand(GWND *WizardWnd, char *Title, OUTPUT_COMMAND_CFG *NewCfg, int ocID);

void WizardOutputCommand(char *Title, int ocID, int NewOC)
{
	GWND		*wnd, _wnd;
	OUTPUT_COMMAND_CFG	*NewCfg, _NewCfg;
	int		rval, Stage, Wizard;

	NewCfg = &_NewCfg;
	memcpy(NewCfg, &sys_cfg->OutputCommands[ocID-2], sizeof(OUTPUT_COMMAND_CFG));
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		if(!NewOC) Wizard |= GWND_WIZARD_DELETE;
		switch(Stage) {
		case 1:	rval = _ProcessOutputCommand(wnd, &Wizard, NewCfg, ocID); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else {
			_SaveOutputCommand(wnd, Title, NewCfg, ocID);
			Stage = 0;
		}
		taskYield();
	}
}

int _ProcessOutputCommand(GWND *WizardWnd, int *Wizard, OUTPUT_COMMAND_CFG *NewCfg, int ocID)
{
	GLABEL	*lblRepeatCount, _lblRepeatCount, *lblRepeatCount2, _lblRepeatCount2, *lblCadences[4], _lblCadences[4];
	GLABEL	*lblOn, _lblOn, *lblOff, _lblOff, *lblCadences2, _lblCadences2;
	GEDIT	*edtRepeatCount, _edtRepeatCount, *edtCadences[4][2], _edtCadences[4][2];
	RECT	rect;
	char	temp[24];
	int		rval, i, j, c, x, y, w_lbl, h_lbl, w_edt, h_edt, val, vals[4][2];

	*Wizard &= ~GWND_WIZARD_NEXT;
	wndDrawClientArea(WizardWnd);
	lblRepeatCount = &_lblRepeatCount; lblRepeatCount2 = &_lblRepeatCount2; 
	edtRepeatCount = &_edtRepeatCount;
	for(i = 0;i < 4;i++) {
		lblCadences[i] = &_lblCadences[i];
		edtCadences[i][0] = &_edtCadences[i][0]; edtCadences[i][1] = &_edtCadences[i][1];
	}
	lblOn = &_lblOn; lblOff = &_lblOff;
	lblCadences2 = &_lblCadences2;
	w_lbl = 100; h_lbl = 24; w_edt = 42; h_edt = 26;;  
	x = 6; y = 6;
	labelInitialize(lblRepeatCount, WizardWnd, x, y, w_lbl+30, h_lbl); x += w_lbl+30;
	labelSetWindowText(lblRepeatCount, xmenu_output_command[0]); 
	editInitialize(edtRepeatCount, WizardWnd,  x, y, w_edt, h_edt); x += w_edt+16;
	editSetFont(edtRepeatCount, GWND_NORMAL_FONT);
	editLimitText(edtRepeatCount, 3);
	editModifyStyle(edtRepeatCount, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
	sprintf(edtRepeatCount->Text, "%d", (int)NewCfg->RepeatCount);
	labelInitialize(lblRepeatCount2, WizardWnd, x, y, w_lbl, h_lbl);
	sprintf(temp, "0:%s", xmenu_other[5]);
	labelSetWindowText(lblRepeatCount2, temp);
	y += 30; x = 60;
	labelInitialize(lblOn, WizardWnd, x, y, w_lbl, h_lbl); x += w_lbl;
	labelSetWindowText(lblOn, xmenu_output_command[1]); 
	labelInitialize(lblOff, WizardWnd, x, y, w_lbl, h_lbl); x += w_lbl;
	labelSetWindowText(lblOff, xmenu_output_command[2]); 
	y += 26; x = 60;
	labelInitialize(lblCadences2, WizardWnd, x+w_lbl+w_edt+8, y+10, w_lbl+6, h_lbl+h_lbl+h_lbl);
	sprintf(temp, "0:%s\n1-255\n0.1%s", xmenu_inuse[0], xmsgs[M_SECOND]);
	labelSetWindowText(lblCadences2, temp);

	for(i = 0;i < 4;i++) {
		x = 6;
		labelInitialize(lblCadences[i], WizardWnd, x, y, w_lbl, h_lbl); x += 54;
		labelSetWindowText(lblCadences[i], ""); 
		for(j = 0;j < 2;j++) {
			editInitialize(edtCadences[i][j], WizardWnd, x, y, w_edt, h_edt); x += w_lbl;
			editSetFont(edtCadences[i][j], GWND_NORMAL_FONT);
			editLimitText(edtCadences[i][j], 3);
			editModifyStyle(edtCadences[i][j], ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
			sprintf(edtCadences[i][j]->Text, "%d", (int)NewCfg->OutputCadences[i][j]);
		}
		y += 30;
	}
	while(1) {
		labelDrawWindow(lblRepeatCount); editDrawWindow(edtRepeatCount); labelDrawWindow(lblRepeatCount2); 
		labelDrawWindow(lblOn); labelDrawWindow(lblOff); labelDrawWindow(lblCadences2);
		for(i = 0;i < 4;i++) {
			labelDrawWindow(lblCadences[i]);
			for(j = 0;j < 2;j++) editDrawWindow(edtCadences[i][j]);
		}
		wndGetClientArea(WizardWnd, &rect);
		x = 60 + w_lbl + w_edt + 32; y = rect.y + 148;
		lcdPutIconFile(x, y, 24, BTN_BACKSPACE); x += 26; lcdPutIconFile(x, y, 24, BTN_SHARP);	
		rval = editProcessWindow(edtRepeatCount, *Wizard);
		if(rval > 0) editDrawWindow(edtRepeatCount);
		c = edtRepeatCount->LastKey;
		for(i = 0;i < 4;i++)
			for(j = 0;j < 2;j++)
				if(rval > 0 && c == KEY_OK) {
					rval = editProcessWindow(edtCadences[i][j], *Wizard);
					if(rval > 0) editDrawWindow(edtCadences[i][j]); 
					c = edtCadences[i][j]->LastKey;
				}
		if(rval == 0 && c == KEY_F2) {
			sprintf(temp, "%s", GetPromptMsg(M_DETETE_CONFIRM));
			i = WndMessageBoxYesNo(WizardWnd, wndGetWindowText(WizardWnd), temp);
			if(i == IDYES) {
				i = ocRemove(ocID);
				if(i < 0) ResultMsg(WizardWnd, wndGetWindowText(WizardWnd), R_MSG_SYSTEM_ERROR);
				else if(i > 0) ResultMsg(WizardWnd, wndGetWindowText(WizardWnd), R_DELETED);
			}
		}
		if(rval < 1) break;
		rval = 0;
		if(c == KEY_F1 || c == KEY_F4) {
			val = n_atol(edtRepeatCount->Text);
			for(i = 0;i < 4;i++)
				for(j = 0;j < 2;j++) vals[i][j] = n_atol(edtCadences[i][j]->Text);
			j = 0;
			if(val < 0 || val > 255) {
				ResultMsg(WizardWnd, xmenu_output_command[0], R_INVALID_DATA);
				j = 1;
			}
			if(!j) {
				for(i = 0;i < 4;i++)
					if(vals[i][0] < 0 || vals[i][0] > 255) {
						ResultMsg(WizardWnd, xmenu_output_command[1], R_INVALID_DATA);
						j = 1;
					}
			}
			if(!j) {
				for(i = 0;i < 4;i++)
					if(vals[i][1] < 0 || vals[i][1] > 255) {
						ResultMsg(WizardWnd, xmenu_output_command[2], R_INVALID_DATA);
						j = 1;
					}
			}
			if(!j) {
				NewCfg->RepeatCount = val;
				for(i = 0;i < 4;i++)
					for(j = 0;j < 2;j++) NewCfg->OutputCadences[i][j] = vals[i][j];
				if(c == KEY_F1) rval = 1; else rval = 2;
			}
		}
		if(rval) break;
		taskYield();
	}
	return rval;		
}

int _SaveOutputCommand(GWND *WizardWnd, char *Title, OUTPUT_COMMAND_CFG *NewCfg, int ocID)
{
	OUTPUT_COMMAND_CFG	*oc_cfg, *OldCfg, _OldCfg;
	int		rval;

	oc_cfg = &sys_cfg->OutputCommands[ocID-2];
	OldCfg = &_OldCfg;
	memcpy(OldCfg, oc_cfg, sizeof(OUTPUT_COMMAND_CFG));
	if(n_memcmp(NewCfg, OldCfg, sizeof(OUTPUT_COMMAND_CFG))) {
		memcpy(oc_cfg, NewCfg, sizeof(OUTPUT_COMMAND_CFG));
		rval = WndSaveConfig(WizardWnd, Title, 1);
		if(rval < 0) memcpy(oc_cfg, OldCfg, sizeof(OUTPUT_COMMAND_CFG));
	} else	rval = 0;
	return rval;	
}

/*

void OutputCommandItemChanged(GMENU *menu, int Item)
{
	if(Item < 3) _DrawLowerButtons(BTN_CANCEL, BTN_UP_ARROW, BTN_DOWN_ARROW, BTN_OK, 0);
	else	_DrawLowerButtons(BTN_DELETE, BTN_UP_ARROW, BTN_DOWN_ARROW, BTN_OK, 0);
}

		if(rval == 0 && menu->LastKey == KEY_F1 && menu->CurrentItem > 2) {
*/