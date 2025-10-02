unsigned char gConsoleState, SinServerInited;

void ctConsoleInit(void)
{
	usbdOpen();
	SetConsoleSioPort(2);
	SetConsoleSinPort(SOCK_CONSOLE);
	gConsoleState = 0;
	SinServerInited = 0;
}

void ctConsole(void *arg)
{
	unsigned short	nPort;
	unsigned long	timer;
	int		rval;

	switch(gConsoleState) {
	case 0:		// Disconnected
		rval = s_getc();
		if(rval == ASCII_ESC) { 
			gConsoleState = 1;
			menucInit(1);
		} else if(niGetState() > 2) {
			if(!SinServerInited) {
				taskYield();
				nPort = 23;
				rval = sinInitServer(SOCK_CONSOLE, nPort);
				SinServerInited = 1;
//cprintf("sinInitServer(%d,%d)=%d\n", SOCK_CONSOLE, (int)nPort, rval);
			} else {
				rval = sinOpenServer(SOCK_CONSOLE, NULL);
				if(rval == 1) {
					timer = TICK_TIMER;
					while(1) {
						rval = sinOpenServer(SOCK_CONSOLE, NULL);
						if(rval != 1) break;
						if((TICK_TIMER-timer) > 64) { cprintf("long......\n"); break; } 
						taskDelayTick(4);
						//taskYield();
					}
				}
//if(rval) cprintf("sinOpenServer(%d)=%d\n", SOCK_CONSOLE, rval);
				if(rval > 1) {
					gConsoleState = 1;
					menucInit(2);
				} else if(rval) {
					sinClose(SOCK_CONSOLE);
					SinServerInited = 0;		
				}
			}
		}
		if(!gConsoleState && (usbdIsConnected() || usbdIsReconnected())) { 
			gConsoleState = 1;
			menucInit(3);
//cprintf("usb Is Connected...\n");
		}
		break;
	case 1:			// Connected
		menu_errno = 0;
		cUsbWriteNull();
		if(!menu_errno) {
			cClearReadWrite();
			gConsoleState = 2;
		}
		break;
	case 2:			// Login
		rval = MenuConsoleLogin();
		if(rval > 1) {
			gConsoleState = 4;
		} else if(rval == 1) {
			gConsoleState = 3;
		}
		break;
	case 3:			// Lockout
		MenuConsoleLockout();
		gConsoleState = 2;
		break;
	case 4:			// Shell		
		MenuConsoleShell();
		gConsoleState = 2;
		break;
	}
	if(gConsoleState > 0 && gConsoleState < 3 && menu_errno) {
 		gConsoleState = 0;
		if(menucGetMode() == 2) {
			sinClose(SOCK_CONSOLE);
			SinServerInited = 0;
		}
//cprintf("Disconnected...\n");
		menu_errno = 0;
	}
}
