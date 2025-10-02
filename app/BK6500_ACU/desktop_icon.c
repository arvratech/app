void DesktopDrawIcons(void)
{
	DesktopDrawServerNetState();
	DesktopDrawSDCard();
}

int DesktopIconsIsVisible(void)
{
	if(desktop_flag & 0x01) return 1;
	else	return 0;
}

void DesktopSetIconsVisible(int Enable)
{
	if(Enable) desktop_flag |= 0x01; else desktop_flag &= 0xfe;
}

static void _DrawIcon(int x, int y, int file)
{
	int		w;
	
	w = 24;
	lcdSetClipRect(x, y, w, w);
	DesktopDrawBackGround();	
	if(file) lcdPutIconFile(x, y, w, file);
	lcdSetDefaultClipRect();
}

extern unsigned char ServerNetState;

int niGetState(void);

void DesktopDrawServerNetState(void)
{
	int		file, file2, val;

	if(!DesktopIconsIsVisible()) return;
	file2 = 0;
	val = sys_cfg->NetworkPort.NetworkType;
	if(val == NETWORK_SERIAL) {
		if(ServerNetState >= 3) file = SERIAL_ICON;
		else	file = SERIAL_FAIL_ICON;
	} else if(val == NETWORK_DIALUP) {
		if(ServerNetState >= 3) file = SERIAL_ICON;
		else	file = SERIAL_FAIL_ICON;
	} else {
		val = niGetState();
		if(val == 3) {
			if(ServerNetState >= 3) file = TCPIP_ICON;
			else	file = TCPIP_FAIL_ICON;
		} else if(val == 2) {
			file = TCPIP_WAIT_ICON;
		} else {
			file = TCPIP_ICON; file2 = BTN_NO;
		}
	}
	_DrawIcon(10, 0, file);
	if(file2) lcdPutIconFile(14, 4, 16, file2);
}

void DesktopDrawSDCard(void)
{
	int		state, x;

	if(!DesktopIconsIsVisible()) return;
	state = sdiGetState();
	x = 90;
	if(state) {
		_DrawIcon(x, 0, SD_CARD_ICON);
		if(state == 1) lcdPutIconFile(x+4, 4, 16, BTN_NO);
		else if(state == 2 || sdiGetError()) lcdPutIconFile(x+4, 4, 16, FAIL_ICON);
	} else {
		_DrawIcon(x, 0, 0);
	}
}
