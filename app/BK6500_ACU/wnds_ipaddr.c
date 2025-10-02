

// Return	-1:Timeout, System error  0:Cancelled  1:Invalid  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
int WndInputIPAddress(GWND *ParentWnd, char *Title, unsigned char *IPAddr, int SubnetMask, int Section)
{
	unsigned char	NewIPAddr[4];
	char	buf[20];
	int		rval;

	if(MenuGetAdminConsole()) {
		memcpy(NewIPAddr, IPAddr, 4);
		cPutTextLine(inet2addr(NewIPAddr)); if(menu_errno) return GWND_ERROR;
		buf[0] = 0;
		rval = menucInputString("Input: ", buf, 15);
		if(rval == 1) rval = 0;
		if(rval > 0) {
			rval = addr2inet(buf, NewIPAddr);
			if(rval < 0 || CheckIPAddr(NewIPAddr, SubnetMask)) rval = 1;
			else	rval = 4;
		}
	} else {
		wndSetSize(wndInput, 52, 63, 216, 114); 
		wndSetWindowText(wndInput, Title);
		labelSetSize(lblInput, 6, 10, 150, 24); 	
		ipaddrSetSize(ipaddrInput, 16, 38, 176, 26); 
		ipaddrSetFont(ipaddrInput, GWND_NORMAL_FONT);
		ipaddrModifyStyle(ipaddrInput, ES_PASSWORD|ES_READONLY, ES_NUMBER|ES_WILDDIGIT);
		labelSetWindowText(lblInput, "IP"); 
		memcpy(ipaddrInput->Address, IPAddr, 4);
		wndDrawWindow(wndInput);
		labelDrawWindow(lblInput);
		ipaddrDrawWindow(ipaddrInput);	
		rval = ipaddrProcessWindow(ipaddrInput, 0);
		memcpy(NewIPAddr, ipaddrInput->Address, 4);
	}
	if(rval <= 0) {
		if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
		return rval;
	}
	if(rval < 4) {
		ResultMsg(ParentWnd, Title, R_INVALID_IP_ADDRESS);
		rval = 1;
	} else {
		rval = WndSaveByteData(ParentWnd, Title, IPAddr, 4, NewIPAddr, Section);
	}
	return rval;
}

void WndViewIPAddress(GWND *ParentWnd, char *Title, unsigned char *IPAddr)
{
	unsigned char	NewIPAddr[4];
	int		rval;

	if(MenuGetAdminConsole()) {
		memcpy(NewIPAddr, IPAddr, 4);
		cPutTextLine(inet2addr(NewIPAddr)); if(menu_errno) return;
	} else {
		wndSetParentWnd(wndInput, ParentWnd);
		wndSetSize(wndInput, 52, 63, 216, 114); 
		wndSetWindowText(wndInput, Title);
		labelSetSize(lblInput, 6, 10, 150, 24); 	
		ipaddrSetSize(ipaddrInput, 16, 38, 176, 26); 
		ipaddrSetFont(ipaddrInput, GWND_NORMAL_FONT);
		ipaddrModifyStyle(ipaddrInput, ES_PASSWORD, ES_NUMBER|ES_WILDDIGIT|ES_READONLY);
		labelSetWindowText(lblInput, "IP"); 
		memcpy(ipaddrInput->Address, IPAddr, 4);
		wndDrawWindow(wndInput);
		labelDrawWindow(lblInput);
		ipaddrDrawWindow(ipaddrInput);
		rval = editReadOnlyWindow(&ipaddrInput->Edit, 0);
	}
	if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
}
