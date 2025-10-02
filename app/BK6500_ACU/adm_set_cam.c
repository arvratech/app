void MenuCamMode(GMENU *ParentMenu, char *Title);
void MenuCamRunState(GMENU *ParentMenu, char *Title);
void MenuCamView(GMENU *ParentMenu, char *Title);
void MenuCamAssignChannel(GMENU *ParentMenu, char *Title);

void MenuCam(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	CAM_CFG		*cam_cfg;
	char	*ItemText;
	int		rval;

	cam_cfg = &sys_cfg->Camera;
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_cam, 7);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuCamMode(menu, ItemText);
			break;
		case 2:
			MenuCamRunState(menu, ItemText);
			break;
		case 3:
			rval = MenuRadio((GWND *)ParentMenu, Title, &cam_cfg->Channel, xmenu_cam_channel, 4, 1, 1);	// Title, pVal, menuStr[], menuSize, FromZero, Section
			if(rval > 2) camSelectMux((int)cam_cfg->Channel);
			break;
		case 4:
			//MenuCamView(menu, ItemText);	
			break;
		case 5:
			MenuCamView(menu, ItemText);	
			break;
		case 6:
			MenuYesNo((GWND *)menu, ItemText, &cam_cfg->Option, EVENT_WITH_CAM_IMAGE, 1);
			break;
		case 7:
			MenuCamAssignChannel(menu, ItemText);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuCamMode(GMENU *ParentMenu, char *Title)
{
	CAM_CFG		*cam_cfg;
	int		rval, state;

	cam_cfg = &sys_cfg->Camera;
	// 0=사용안함 1=자동 2=수동
	rval = MenuRadio((GWND *)ParentMenu, Title, &cam_cfg->Mode, xmenu_cam_mode, 3, 1, 1);	// Title, pVal, menuStr[], menuSize, FromZero, Section
	state = camGetCodecState();	
	if(rval > 1) {
		if(cam_cfg->Mode == 2 && cam_cfg->RunState && (state == CAM_STOPPED || state == CAM_NULL)) {
			RunCam(); rval = 1;
		} else if((cam_cfg->Mode != 2 || !cam_cfg->RunState) && state != CAM_STOPPED) {
			StopCam(); rval = 1;
		} else	rval = 0;
		if(rval) ResultMsg((GWND *)ParentMenu, Title, R_PROCESSING);
	}
}

void MenuCamRunState(GMENU *ParentMenu, char *Title)
{
	CAM_CFG		*cam_cfg;
	GMENU	*menu, _menu;
	char	temp[40];
	int		rval, state;

	cam_cfg = &sys_cfg->Camera;
	if(cam_cfg->Mode == 2) {	// 2=수동
		// 0=켜짐 1=꺼짐				
		rval = MenuRadio((GWND *)ParentMenu, Title, &cam_cfg->RunState, xmenu_cam_run, 2, 1, 1);	// Title, pVal, menuStr[], menuSize, FromZero, Section
		state = camGetCodecState();	
		if(rval > 1) {
			if(cam_cfg->RunState && (state == CAM_STOPPED || state == CAM_NULL)) {
				RunCam(); rval = 1;
			} else if(!cam_cfg->RunState && state != CAM_STOPPED) {
				StopCam(); rval = 1;
			} else	rval = 0;
			if(rval) ResultMsg((GWND *)ParentMenu, Title, R_PROCESSING);
		}
	} else {	// 0=사용안함, 1=자동
		menu = &_menu;
		MenuInitialize(menu, Title, NULL, 0);	// menu, Title, MenuStr, Size
		menuSetType(menu, GMENU_TYPE_VIEW_ONLY);
		menuSetPrefixNumber(menu, 0);
		temp[0] = 0;
		MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		if(cam_cfg->Mode == 1) {
			state = camGetCodecState();	
			if(state == CAM_RUNNING) sprintf(temp, "%s: On", xmenu_setup[6]);
			else	sprintf(temp, "%s: Off", xmenu_setup[6]);
		} else	sprintf(temp, "%s: %s", xmenu_setup[6], xmenu_cam_mode[0]);
		MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		rval = MenuPopup(menu);
		MenuTerminate(menu);		
	}	
}

void MenuCamView(GMENU *ParentMenu, char *Title)
{
	GWND	*wnd, _wnd;
	unsigned long	CamFrameCount, timer;
	unsigned short	*lcd_buf;	
	int		c, w, h, size, contflg;

	AdminConsoleUnsupported();
	wnd = &_wnd;
	w = lcdGetWidth(); h = lcdGetHeight();
	wndInitialize(wnd, NULL, 0, 0, w, h);
	wndModifyStyle(wnd, WS_CAPTION | WS_BORDER, WS_VISIBLE);
	wndDrawWindow(wnd);
	c = camGetCodecState();	
	if(c == CAM_NULL) {
		ResultMsg((GWND *)ParentMenu, Title, R_FAILED);
		return;
	}
	lcd_buf = lcdGetBuffer(0, 0);
	camRunPreview();
	timer = DS_TIMER + 70;
	while(1) {
		c = camGetPreviewState();
		if(c == CAM_RUNNING || DS_TIMER > timer) break;
		taskYield();
	}
//cprintf("PreviwState=%d CodecState=%d %d(RUN)\n", c, camGetCodecState(), CAM_RUNNING);
	if(c != CAM_RUNNING) {
		camStopPreview();
		ResultMsg((GWND *)ParentMenu, Title, R_FAILED);
		return;
	}
	CamFrameCount = 0L;
	contflg = 1;
	while(contflg) {
		c = camGetPreviewState();
		if(c != CAM_RUNNING) break;
		size = camCapturePreview(lcd_buf, &CamFrameCount, 0);
		if(size > 0) {
			lcdHoldRefresh();
			_DrawLowerButtons(BTN_CANCEL, 0, 0, BTN_OK, 1);
			lcdRefresh(0, 0, w, h);
		}
		c = keyGet();
		switch(c) {
		case KEY_CANCEL:
		case KEY_F1:
		case KEY_OK:
		case KEY_F4:		
			contflg = 0;
			break;
		}
		taskYield();
	}
	c = camGetPreviewState();
	camStopPreview();
	if(c != CAM_RUNNING) ResultMsg((GWND *)ParentMenu, Title, R_FAILED);
}

/*
void MenuCamView(GMENU *ParentMenu, char *Title)
{
	GWND	*wnd1, _wnd1, *wnd2, _wnd2;
	BMPC	*bmp, _bmp;
	unsigned long	CamFrameCount, timer;
	unsigned short	*lcd_buf;	
	int		c, size, dw, contflg;

	wnd1 = &_wnd1; wnd2 = &_wnd2;
	wndInitialize(wnd1, NULL,  12, 24, 144, 288);
	wndInitialize(wnd2, NULL, 164, 24, 144, 288);
	wndModifyStyle(wnd1, WS_CAPTION | WS_BORDER, WS_VISIBLE);
	wndModifyStyle(wnd2, WS_CAPTION | WS_BORDER, WS_VISIBLE);
	wndDrawWindow(wnd1);
	wndDrawWindow(wnd2);
	bmp = &_bmp;	
	bmpcInit(bmp, _CamBuffer, 144, 192);
	lcd_buf = lcdGetBuffer(12, 24);
	dw = lcdGetBufferWidth();
	camRunPreview();
	timer = DS_TIMER + 70;
	while(1) {
		c = camGetPreviewState();
		if(c == CAM_RUNNING || DS_TIMER > timer) break;
		taskYield();
	}
cprintf("%d %d %d\n", c, camGetCodecState(), CAM_RUNNING);
	if(c != CAM_RUNNING) {
		camStopPreview();
		ResultMsg((GWND *)ParentMenu, Title, R_FAILED);
		return;
	}
	CamFrameCount = 0L;
	contflg = 1;
	while(contflg) {
		c = camGetPreviewState();
		if(c != CAM_RUNNING) break;
		size = camCapturePreview(lcd_buf, &CamFrameCount, 0);
		if(size > 0) lcdRefresh(12, 24, 144, 192);
		c = keyGet();
		switch(c) {
		case 'F':
			contflg = 0;
			break;
		case 'G':
			lcdGet(12, 24, bmp);
			lcdPut(164, 24, bmp);
			break;
		}
		taskYield();
	}	
	c = camGetPreviewState();
	camStopPreview();
	if(c != CAM_RUNNING) ResultMsg((GWND *)ParentMenu, Title, R_FAILED);
}
*/

void MenuCamAssignChannel(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	CAM_CFG		*cam_cfg;
	unsigned char	cVal, OldVal, IDs[MAX_AP_SZ+1];
	char	*ItemText, temp[MAX_AP_SZ+1][MAX_MSG_LEN], *menuStr[MAX_AP_SZ+1];
	int		rval, i, idx, Item;

	cam_cfg = &sys_cfg->Camera;
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_cam_channel, 4);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval-1);
		idx = rval - 1;
		Item = 0;
		temp[0][0] = '-'; temp[0][1] = 0;
		menuStr[Item] = temp[0]; IDs[Item] = 0xff; Item++;
		for(i = 0;i < MAX_AP_SZ;i++) {
			if(apIsEnable(i)) {
				GetAccessPointName(i, temp[Item]);
				menuStr[Item] = temp[Item];
				IDs[Item] = i; Item++;
			}
		}
		OldVal = cVal = cam_cfg->AccessPoints[idx];
		for(i = 0;i < Item;i++) if(cVal == IDs[i]) break;
		if(i >= Item) i = 0;
		cVal = i;
		rval = MenuRadio((GWND *)menu, ItemText, &cVal, menuStr, Item, 1, 0);	// Title, pVal, menuStr[], menuSize, FromZero, Section
		if(rval > 2) {
			cam_cfg->AccessPoints[idx] = IDs[cVal];
			rval = WndSaveConfig((GWND *)menu, ItemText, 1);
			if(rval < 0) cam_cfg->AccessPoints[idx] = OldVal;
		}
	}
	MenuTerminate(menu);
}
