int DesktopEnterPassword(char *Title, char *Data)
{
	GWND	*wndTop, _wndTop; 
	GLABEL	*lblTop, _lblTop;
	GEDIT	*edtTop, _edtTop;
	int		rval;

	DesktopClearMain();
	wndTop = &_wndTop;
	wndInitialize(wndTop, Title, 60, 64, 200, 112); 
	wndModifyStyle(wndTop, 0, WS_CAPTION | WS_BORDER | WS_VISIBLE);	
	if(IsBlackWhiteMode()) WndSetBlackWhiteMode(wndTop);
	lblTop = &_lblTop;
	labelInitialize(lblTop, wndTop,  6,  8, 150, 24); 
	if(IsBlackWhiteMode()) LabelSetBlackWhiteMode(lblTop);	
	labelSetWindowText(lblTop, GetPromptMsg(M_ENTER_PASSWORD)); 
	edtTop = &_edtTop;
	editInitialize(edtTop, wndTop, 24, 32, 150, 26); 
	editSetFont(edtTop, GWND_NORMAL_FONT);
	editLimitText(edtTop, 8);
	editModifyStyle(edtTop, ES_READONLY|ES_WILDDIGIT, ES_PASSWORD|ES_NUMBER);
	if(IsBlackWhiteMode()) EditSetBlackWhiteMode(edtTop);	
	edtTop->Text[0] = 0;
	lcdHoldRefresh();		
	wndDrawWindow(wndTop);
	labelDrawWindow(lblTop);
	editDrawWindow(edtTop);
	_DrawInputDigitDesp(wndTop);
	wndRefresh(wndTop);	
	rval = editProcessWindow(edtTop, 0);
	if(rval > 0) strcpy(Data, edtTop->Text);
	DesktopRestoreWindow(wndTop);	
	DesktopRestoreArea(0, lcdGetHeight()-32, lcdGetWidth(), 32);
	return rval;
}

void DesktopResult(char *Caption, int MsgIndex)
{
	GWND	*wndTop, _wndTop; 
	GLABEL	*lblTop, _lblTop;
	RECT	rect;
	int		file;

	wndTop = &_wndTop;
	wndInitialize(wndTop, Caption, 35, 63, 250, 114); 
	wndModifyStyle(wndTop, 0, WS_CAPTION | WS_BORDER | WS_VISIBLE);	
	if(IsBlackWhiteMode()) WndSetBlackWhiteMode(wndTop);
	AddResultMent(MsgIndex);
	lblTop = &_lblTop;
	file = xrsp_msgs_icon[MsgIndex];
	switch(file) {
	case MB_ICONINFORMATION: file = INFORMATION_ICON; break;
	case MB_ICONWARNING:	 file = EXCLAMATION_ICON; break;
	case MB_ICONERROR:		 file = BTN_DELETE; break;
	default:				 file = 0;
	}
	if(file) labelInitialize(lblTop, wndTop, 48, 16, 198, 46);
	else	labelInitialize(lblTop, wndTop, 0, 16, 246, 46);
	if(IsBlackWhiteMode()) LabelSetBlackWhiteMode(lblTop);
	labelSetWindowText(lblTop, GetResultMsg(MsgIndex));	
	lblTop->Style = LS_ALIGN_CENTER;
	lcdHoldRefresh();
	wndDrawWindow(wndTop);
	labelDrawWindow(lblTop);
	if(file) {
		wndGetClientArea(wndTop, &rect);
		lcdPutIconFile(rect.x+4, rect.y+16, 48, file);
	}
	wndRefresh(wndTop);
	wndShowWindow(wndResult);
	wndResult->Timer = DS_TIMER + MSG_TIMEOUT;
}
