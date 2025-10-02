#include "lcdc.h"
#include "key.h"
#include "vox.h"
#include "ment.h"
//#include "cam.h"
#include "sio.h"
#include "wiegand.h"
#include "toptask.h"

int TestKey(GWND *wnd, int Wizard);
int TestSound(GWND *wnd, int Wizard);
int TestDoorInOut(GWND *wnd, int Wizard);
int TestCardReader(GWND *wnd, int Wizard);
//int TestCamera(GWND *wnd, int Wizard);
int TestWiegand(GWND *wnd, int Wizard);
int TestSerial(GWND *wnd, int Wizard);
int TestFinal(GWND *wnd, int Wizard);


void MenuTestSystem(GMENU *ParentMenu, char *Title)
{
	GWND	*wnd, _wnd;
	int		rval, Stage, Wizard;

	AdminConsoleUnsupported();
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
//	taskReset(t_inout);
	
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		if(Stage < 8) Wizard |= GWND_WIZARD_NEXT;
		if(Stage > 1) Wizard |= GWND_WIZARD_BACK;
		switch(Stage) {
		case 1:	rval = TestKey(wnd, Wizard); break;
		case 2: rval = TestSound(wnd, Wizard); break;
		case 3: rval = TestDoorInOut(wnd, Wizard); break;
		case 4: rval = TestCardReader(wnd, Wizard); break;
		case 5: rval = TestWiegand(wnd, Wizard); break;
		//case 6: rval = TestCamera(wnd, Wizard); break;
		case 6: rval = TestSerial(wnd, Wizard); break;
		case 7: rval = TestFinal(wnd, Wizard); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else if(rval == 1) {
			Stage--;
		} else {
			if(Wizard & GWND_WIZARD_NEXT) Stage++;
			else	Stage = 0;
		}
		taskYield();
	}
//	taskBegin(t_inout);
}

static void _TestLowerButtons(int Wizard, int Btn2, int Btn3, int Transparent)
{
	int		Btn1, Btn4;
	
	if(Wizard & GWND_WIZARD_BACK) Btn1 = BTN_BACK; else Btn1 = 0;
	if(Wizard & GWND_WIZARD_NEXT) Btn4 = BTN_NEXT; else Btn4 = BTN_OK;
	_DrawLowerButtons(Btn1, Btn2, Btn3, Btn4, Transparent);
}

int TestKey(GWND *wnd, int Wizard)
{
	RECT	rect;
	char	temp[32];
	int		c, x, y, w, Color;

	wndDrawClientArea(wnd);
	wndGetClientArea(wnd, &rect);
	_TestLowerButtons(Wizard, 0, 0, 0);
	Color = wndGetBackGroundTextColor(wnd);
	x = rect.x + 10; y = rect.y + 16;
	lcdPutText(x, y, "Key...", Color);
	y += 30;
	lcdPutText(x, y, "Strike key !", Color);
	x += 120; y += 40; w = 0;	
	while(1) {
		c = keyGet();
		if(c == KEY_F4 || c == KEY_OK || c == KEY_CANCEL || c == KEY_F1 && (Wizard & GWND_WIZARD_BACK)) break;
		else if(c) {
			if(c >= '0' && c <= '9' || c == '*' || c == '#') sprintf(temp, "[%c]", c);
			else if(c == KEY_CALL) strcpy(temp, "[CALL]");	
			else if(c == KEY_MENU) strcpy(temp, "[MENU]");
			else if(c == KEY_CANCEL) strcpy(temp, "[CLR]");	
			else if(c == KEY_OK) strcpy(temp, "[Enter]");	
			else if(c == KEY_F1) strcpy(temp, "[F1]");	
			else if(c == KEY_F2) strcpy(temp, "[F2]");	
			else if(c == KEY_F3) strcpy(temp, "[F3]");	
			else if(c == KEY_F4) strcpy(temp, "[F4]");	
			else	strcpy(temp, "Unknown");	
			if(w) lcdFillRect(x, y, w, GetFontHeight(), wndGetBackGroundColor(wnd));
			w = lcdPutText(x, y, temp, Color);
		}
		taskYield();
	}
	if(c == KEY_CANCEL) c = 0; else if(c == KEY_F1) c = 1; else c = 2;
	return c;
}

int TestSound(GWND *wnd, int Wizard)
{
	return 2;
}

int TestDoorInOut(GWND *wnd, int Wizard)
{
	return 2;
}

int TestCardReader(GWND *wnd, int Wizard)
{	
	return 2;
}

/*
int TestCamera(GWND *wnd, int Wizard)
{
	GWND	*cwnd, _cwnd;
	BMPC	*bmp, _bmp;
	RECT	rect;
	unsigned long	CamFrameCount, timer;
	unsigned short	*lcdBuffer;	
	char	Title[32], temp[60];
	int		c, x, y, w, h, rval, size, state, chan, Color;

	strcpy(Title, "Camera...");
	wndDrawClientArea(wnd);
	wndGetClientArea(wnd, &rect);
	_TestLowerButtons(Wizard, 0, 0, 0);
	Color = wndGetBackGroundTextColor(wnd);
	x = rect.x + 10; y = rect.y + 16;
	lcdPutText(x, y, Title, Color);
	y += 30;
	cwnd = &_cwnd;
	w = lcdGetWidth(); h = lcdGetHeight();
	wndInitialize(cwnd, NULL, 0, 0, w, h);
	wndModifyStyle(cwnd, WS_CAPTION | WS_BORDER, WS_VISIBLE);
	bmp = &_bmp;
	lcdBuffer = lcdGetBuffer(0, 0);
	bmpcInit(bmp, lcdBuffer, w, h);
	while(1) {
		PromptMsg(wnd, Title, M_WAIT_A_MOMENT);
		camRunPreview();
		timer = DS_TIMER + 30;
		while(1) {
			c = keyGet();
			state = camGetPreviewState();
			if(c == KEY_F4 || c == KEY_OK || c == KEY_CANCEL || c == KEY_F1 && (Wizard & GWND_WIZARD_BACK)) rval = -1;
			else if(state == CAM_RUNNING) rval = 1;
			else if(DS_TIMER > timer) rval = 2;
			else	rval = 9;
			if(rval < 9) break;
			taskYield();
		}
		if(rval < 2) break;
		camStopPreview();
		GetResultWithoutNewLine(temp, R_FAILED);
		ResultMsgEx(wnd, Title, 30, MB_ICONERROR, "Camera isn't exist\n%s", temp);
	}
	if(rval < 0) {
		if(c == KEY_CANCEL) c = 0; else if(c == KEY_F1) c = 1; else c = 2;
		camStopPreview();
		return c;
	}
	wndDrawWindow(cwnd);
	chan = sys_cfg->Camera.Channel;
	CamFrameCount = 0L;
	while(1) {
		state = camGetPreviewState();
		if(state != CAM_RUNNING) {
			c = KEY_F2;
			break;
		}
		size = camCapturePreview(lcdBuffer, &CamFrameCount, 0);
		if(size > 0) {
			lcdHoldRefresh();
			temp[0] = chan + '0'; temp[1] = 0;
			lcdPut32x48Digits(20, 20, temp, RGB(255,255,255));
			_TestLowerButtons(Wizard, BTN_UP_ARROW, BTN_DOWN_ARROW, 1);
			lcdRefresh(0, 0, w, h);
		}
		c = keyGet();
		if(c == KEY_F4 || c == KEY_OK || c == KEY_CANCEL || c == KEY_F1 && (Wizard & GWND_WIZARD_BACK)) break;
		if(c == KEY_F2) {
			chan--; if(chan < 0) chan = 3;
			camSelectMux(chan);
		} else if(c == KEY_F3) {
			chan++; if(chan > 3) chan = 0;
			camSelectMux(chan);
		}
		taskYield();
	}
	state = camGetPreviewState();
	camStopPreview();
	if(state != CAM_RUNNING) {
		GetResultWithoutNewLine(temp, R_FAILED);	
		ResultMsgEx(wnd, Title, 30, MB_ICONERROR, "%s", GetResultMsg(R_FAILED));
	}
	chan = sys_cfg->Camera.Channel;
	camSelectMux(chan);	
	wndDrawWindow(wnd);
	if(c == KEY_CANCEL) c = 0; else if(c == KEY_F1) c = 1; else c = 2;
	return c;
}
*/

void CloseCardForTest(void)
{
/*	
	ACCESS_POINT	*ap[2];
	CR_CARD		*cr;
	int		doorcfg;

	ap[0] = adsGetAt(0); ap[1] = adsGetAt(1);
	doorcfg = GetDoorCfg();
	cr = _gCardReaders;
	cr++;
	if(cr->Type) {
	 	cr->AccessPoint = 0; cr->CardFormat = 0;
		cardOpenClose(cr);	// EM
	}
	cr++;
	cr->AccessPoint = 0;
	cardOpenClose(cr);		// Wiegand 1
	cr++;	
	if(cr->Type) {
		cr->AccessPoint = 0;
		cardOpenClose(cr);	// Wiegand 2
	}
*/
}

void wiegandOpenTest(int port);
void wiegandCloseTest(int port);
void wiegandStartTest(int port);
void wiegandEndTest(int port, int *ZeroCount, int *OneCount);

int TestWiegand(GWND *wnd, int Wizard)
{
	RECT	rect;
	char	Title[32], temp[60];
	unsigned long	timer;
	unsigned char	buf[100];
	int		c, rval, size, x, y, port, Color, ZeroCount, OneCount;

	strcpy(Title, "Wiegand...");
	wndDrawClientArea(wnd);
	wndGetClientArea(wnd, &rect);
	_TestLowerButtons(Wizard, 0, 0, 0);
	Color = wndGetBackGroundTextColor(wnd);
	x = rect.x + 10; y = rect.y + 16;
	lcdPutText(x, y, Title, Color);
	y += 30;
	// close wiegand reader
	CloseCardForTest();
	sioOpen(0, 19200L, 0);
	size = 100;
	for(c = 0;c < size;c++) buf[c] = 0xaa;
	port = 0;
	while(1) {
		_TestLowerButtons(Wizard, 0, 0, 0);
		PromptMsg(wnd, Title, M_WAIT_A_MOMENT);
		wiegandOpenTest(port);
		wiegandStartTest(port);
		sioWrite(0, buf, 100);
		timer = DS_TIMER + 25;
		size = 0;
		while(1) {
			c = keyGet();
			if(c == KEY_F4 || c == KEY_OK || c == KEY_CANCEL || c == KEY_F1 && (Wizard & GWND_WIZARD_BACK)) rval = -1;
			else if(DS_TIMER > timer) rval = 1;
			else	rval = 9;
			if(rval < 9) break;
			taskYield();
		}
		wiegandEndTest(port, &ZeroCount, &OneCount);
		wiegandCloseTest(port);	
		if(rval < 0) break;
		if(ZeroCount > 100 && OneCount > 100) {			
			ResultMsgEx(wnd, Title, 30, MB_ICONINFORMATION, "Port %d %d,%d\nTest is OK", port, ZeroCount, OneCount);
		} else {
			GetResultWithoutNewLine(temp, R_FAILED);
			ResultMsgEx(wnd, Title, 30, MB_ICONERROR, "Port %d\n%s", port, temp);
		}
		port++; if(port > 1) port = 0;
	}
	sioClose(0);
	if(c == KEY_CANCEL) c = 0; else if(c == KEY_F1) c = 1; else c = 2;
	return c;
}
/*
#include "stdarg.h"
#include "system_hdr.h"
void rs1_putch(int c)
{
	if(c == '\n') {
		WrUTXH1('\r');
		while(!(rUTRSTAT1 & 0x4)) ;	// wait until THR & shifter register is empty(for RS485)
		uDelay(1);	// because the slow response of hyper_terminal 
	}
	WrUTXH1(c);
	while(!(rUTRSTAT1 & 0x4)) ;	// wait until THR & shifter register is empty(for RS485)
	uDelay(1);
}

void c1printf(char *fmt, ...)
{
	va_list	arg;
	unsigned char	*p;
	char	s[256];
	int		c;

	va_start(arg, fmt);
	vsprintf(s, fmt, arg);
	va_end(arg);
	p = (unsigned char *)s;
	while(*p) {
		c = *p++;
		rs1_putch(c);
	}
}
*/
int TestSerial(GWND *wnd, int Wizard)
{
	RECT	rect;
	char	Title[32], temp[60], temp2[20];
	unsigned long	timer;
	unsigned char	buf1[200], buf2[200];
	int		c, rval, size, max, TxPort, RxPort, Color;

	strcpy(Title, "RS485...");
	wndDrawClientArea(wnd);
	wndGetClientArea(wnd, &rect);
	_TestLowerButtons(Wizard, 0, 0, 0);
	Color = wndGetBackGroundTextColor(wnd);
	lcdPutText(rect.x+10, rect.y+16, Title, Color);
	TxPort = 0; RxPort = 1;
	sioClose(TxPort);
	sioOpen(TxPort, 115200L, 0);
	sioClose(RxPort);
	sioOpen(RxPort, 115200L, 0);
	max = 200;
	for(c = 0;c < max;c++) buf1[c] = (unsigned char)c;
	while(1) {
		_TestLowerButtons(Wizard, 0, 0, 0);
		PromptMsg(wnd, Title, M_WAIT_A_MOMENT);
		sioClearRead(TxPort); sioClearRead(RxPort);
		sioWrite2(TxPort, buf1, max);
		size = 0;
		timer = DS_TIMER + 20;
		while(1) {
			c = keyGet();
			if(c == KEY_F4 || c == KEY_OK || c == KEY_CANCEL || c == KEY_F1 && (Wizard & GWND_WIZARD_BACK)) rval = -1;
			else if(DS_TIMER > timer) rval = 1;
			else {
				c = sioGetc(RxPort);
				if(c >= 0 && size < max) {
//if(port) cprintf("[%d]=%d ", size, c);
					buf2[size++] = c;
				}
				rval = 9;
			}
			if(rval < 9 || size >= max && DS_TIMER > timer-10L) break;
			//taskYield();   console, server 포트 때문에
		}
		if(rval < 0) break;
		if(TxPort == 0) strcpy(temp2, "Transmit"); else strcpy(temp2, "Receive");
		if(size >= max && !n_memcmp(buf1, buf2, size)) {
//if(!TxPort) cprintf("\nTx(0 => 2) OK size=%d\n", size);
//else	    cprintf("\nRx(2 => 0) OK size=%d\n", size);
			ResultMsgEx(wnd, Title, 30, MB_ICONINFORMATION, "%s is OK", temp2);
		} else {
//if(!TxPort) cprintf("\nTx(0 => 2) FAIL size=%d\n", size);
//else	    cprintf("\nRx(2 => 0) FAIL size=%d\n", size);
			GetResultWithoutNewLine(temp, R_FAILED);
			ResultMsgEx(wnd, Title, 30, MB_ICONERROR, "%s\n%s", temp2, temp);
		}
		if(TxPort == 0) {
			TxPort = 1; RxPort = 0;
		} else {
			TxPort = 0; RxPort = 1;
		}
		taskYield();
	}
	sioClose(TxPort);
	sioClose(RxPort);
	if(c == KEY_CANCEL) c = 0; else if(c == KEY_F1) c = 1; else c = 2;
	return c;
}

int TestFinal(GWND *wnd, int Wizard)
{
	RECT	rect;
	int		c, x, y, Color;

	wndDrawClientArea(wnd);
	wndGetClientArea(wnd, &rect);
	_TestLowerButtons(Wizard, 0, 0, 0);
	Color = wndGetBackGroundTextColor(wnd);
	x = rect.x + 10; y = rect.y + 16;
	lcdPutText(x, y, "Test Completed", Color);
	y += 30;
	while(1) {
		c = keyGet();
		if(c == KEY_F4 || c == KEY_OK || c == KEY_CANCEL || c == KEY_F1 && (Wizard & GWND_WIZARD_BACK)) break;
		taskYield();
	}
	sioClose(0);
	if(c == KEY_CANCEL) c = 0; else if(c == KEY_F1) c = 1; else c = 2;
	return c;
}
