#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "key.h"
#include "lcdc.h"
#include "lang.h"
#include "gfont.h"
#include "iodev.h"
#include "vox.h"
#include "msg.h"
#include "evt.h"
#include "ment.h"
#include "syscfg.h"
#include "acad.h"
#include "acap.h"
#include "user.h"
#include "cr.h"
#include "event.h"
#include "menuprim.h"
#include "wnds.h"
#include "desktop.h"
#include "toptask.h"
#include "topprim.h"

extern int		gIOStatus;
extern unsigned char ServerNetState;
extern unsigned char ExtKey;


void PutBeep(int Beep)
{
	beepPlay();
}

void StopBeep(void)
{
//	beepStop();
}

void KeyBeep(void)
{
	StopBeep();
	PutBeep(BEEP_KEY);
}

void CardCaptureBeep(void)
{
	StopBeep();
	PutBeep(BEEP_CARD_CAPTURED);
}

void FPCaptureBeep(void)
{
	StopBeep();
	PutBeep(BEEP_FP_CAPTURED);
}

int GetCapturePromptMent(int mode)
{
	return 0;
}

void MentUserNormal(int MentNo)
{
}

void MentUserNormalTwo(int MentNo1, int MentNo2)
{
}

void MentAddUserNormal(int MentNo)
{
}

void MentAdminNormal(int MentNo)
{
}

void AddResultMent(int MsgIndex)
{
}

static GWND	  *wndPromptMsg, _wndPromptMsg;
static GLABEL *lblPromptMsg, _lblPromptMsg;

char *GetPromptMsg(int MsgIndex)
{
	return xmsgs[MsgIndex];
}

void GetPromptMsgWithoutNewLine(char *Msg, int MsgIndex)
{
	char	*s, *d;
	int		c;

	s = xmsgs[MsgIndex];
	d = Msg;
	while(1) {
		c = *s++;
		if(!c) break;
		else if(c =='\n') c = ' ';
		*d++ = c;
	}
	*d = 0;
}

void PromptMsg(GWND *ParentWnd, char *Caption, int MsgIndex)
{
	char	*s, *d, buf[64];
	int		c;
	
	if(MenuGetAdminConsole()) {
		s = xmsgs[MsgIndex];
		d = buf;
		*d++ = '>'; *d++ = ' ';
		while(1) {
			c = *s++;
			if(!c) break;
			else if(c == '\n') c = ' ';
			*d++ = c;
		}
		c = *(d - 1);
		if(c == ':') {
			*(d-1) = ' '; *d++ = ':'; *d++ = ' ';
		} else {
			*d++ = ' ';
		}
		*d = 0;
		cPutText(buf);
	} else {
		wndPromptMsg = &_wndPromptMsg;
		wndInitialize(wndPromptMsg, NULL, 35, 63, 250, 114); 
		wndSetParentWnd(wndPromptMsg, ParentWnd);
		lblPromptMsg = &_lblPromptMsg;
		labelInitialize(lblPromptMsg, wndPromptMsg, 10, 10, 220, 46); 
		wndSetWindowText(wndPromptMsg, Caption);
		wndDrawWindow(wndPromptMsg);
		labelSetWindowText(lblPromptMsg, xmsgs[MsgIndex]);	
		lblPromptMsg->Style = LS_ALIGN_CENTER;
		labelDrawWindow(lblPromptMsg);
	}
}

void DestroyPromptMsg(void)
{
	GWND	*ParentWnd;
	RECT	rect;

	if(MenuGetAdminConsole()) return;
	ParentWnd = wndPromptMsg->ParentWnd;
	wndGetDrawArea(wndPromptMsg, &rect);
	lcdSetClipRect(rect.x, rect.y, rect.Width, rect.Height);
	if(ParentWnd) {
		if(ParentWnd->WndType == 1) MenuShow((GMENU *)ParentWnd);
		else	wndDrawWindow(ParentWnd);
	} else	DesktopDrawBackGround();
	lcdSetDefaultClipRect();
}

static GWND	  *wndResultMsg, _wndResultMsg;
static GLABEL *lblResultMsg, _lblResultMsg;

char *GetResultMsg(int MsgIndex)
{
	return xrsp_msgs[MsgIndex];
}

void GetResultWithoutNewLine(char *Msg, int MsgIndex)
{
	char	*s, *d;
	int		c;

	s = xrsp_msgs[MsgIndex];
	d = Msg;
	while(1) {
		c = *s++;
		if(!c) break;
		else if(c =='\n') c = ' ';
		*d++ = c;
	}
	*d = 0;
}

void _DrawResultMsg(GWND *ParentWnd, char *Caption, int Icon, char *Msg, int Right)
{
	RECT	rect;
	int		file, y, h;
	
	if(MenuGetAdminConsole()) {
		cPutText(Msg);
	} else {
		wndResultMsg = &_wndResultMsg;
		if(Right) wndInitialize(wndResultMsg, NULL, 134, 63, 186, 114); 
		else	wndInitialize(wndResultMsg, NULL, 35, 63, 250, 114); 
		wndSetParentWnd(wndResultMsg, ParentWnd);
		wndSetWindowText(wndResultMsg, Caption);
		switch(Icon) {
		case MB_ICONINFORMATION: file = INFORMATION_ICON; break;
		case MB_ICONWARNING:	 file = EXCLAMATION_ICON; break;
		case MB_ICONERROR:		 file = BTN_DELETE; break;
		case MB_ICONWAITING:	 file = WAITING_ICON; break;
		default:				 file = 0;
		}
		lblResultMsg = &_lblResultMsg;
		if(str_linenum(Msg) > 1) y = 16; else y = 25;
		h = 46;
		if(Right) {
			if(file) labelInitialize(lblResultMsg, wndResultMsg, 48, y, 144, h);
			else	labelInitialize(lblResultMsg, wndResultMsg, 0, y, 192, h);		
		} else {
			if(file) labelInitialize(lblResultMsg, wndResultMsg, 48, y, 198, h);
			else	labelInitialize(lblResultMsg, wndResultMsg, 0, y, 246, h);
		}
		labelSetWindowText(lblResultMsg, Msg);	
		lblResultMsg->Style = LS_ALIGN_CENTER;
		wndDrawWindow(wndResultMsg);
		labelDrawWindow(lblResultMsg);
		if(file) {
			wndGetClientArea(wndResultMsg, &rect);
			lcdPutIconFile(rect.x+8, rect.y+16, 48, file);
		}
	}
}

void ResultMsgNoWait(GWND *ParentWnd, char *Caption, int MsgIndex)
{
	char	*s, *d, buf[80];
	int		c, icon;
	
	if(MsgIndex < 0) s = xmenu_unsupported;
	else	s = xrsp_msgs[MsgIndex];
	if(MenuGetAdminConsole()) {
		 d = buf;
		*d++ = ' '; *d++ = '*'; *d++ = ' ';
		while(1) {
			c = *s++;
			if(!c) break;
			else if(c == '\n') c = ' ';
			*d++ = c;
		}
		*d++ = '\r'; *d++ = '\n'; *d = 0;
		cPutText(buf);
	} else {
		if(MsgIndex < 0) icon = MB_ICONINFORMATION; else icon = xrsp_msgs_icon[MsgIndex];
		_DrawResultMsg(ParentWnd, Caption, icon, s, 0);
		AddResultMent(MsgIndex);
	}
}

void ResultMsgNoWaitRight(GWND *ParentWnd, char *Caption, int MsgIndex)
{
	char	*s, *d, buf[80];
	int		c, icon;
	
	if(MsgIndex < 0) s = xmenu_unsupported;
	else	s = xrsp_msgs[MsgIndex];
	if(MenuGetAdminConsole()) {
		d = buf;
		*d++ = ' '; *d++ = '*'; *d++ = ' ';
		while(1) {
			c = *s++;
			if(!c) break;
			else if(c == '\n') c = ' ';
			*d++ = c;
		}
		*d++ = '\r'; *d++ = '\n'; *d = 0;
		cPutText(buf);
	} else {
		if(MsgIndex < 0) icon = MB_ICONINFORMATION; else icon = xrsp_msgs_icon[MsgIndex];
		_DrawResultMsg(ParentWnd, Caption, icon, s, 1);
		AddResultMent(MsgIndex);
	}
}

void ResultMsg(GWND *ParentWnd, char *Caption, int MsgIndex)
{
	ResultMsgNoWait(ParentWnd, Caption, MsgIndex);
	if(!MenuGetAdminConsole()) {
		wndProcessWindow(ParentWnd, 0, MSG_TIMEOUT);
		DestroyResultMsg();
	}
}

void ResultMsgRight(GWND *ParentWnd, char *Caption, int MsgIndex)
{
	ResultMsgNoWaitRight(ParentWnd, Caption, MsgIndex);
	if(!MenuGetAdminConsole()) {
		wndProcessWindow(ParentWnd, 0, MSG_TIMEOUT);
		DestroyResultMsg();
	}
}

#include <stdio.h>
#include <stdarg.h>

void ResultMsgEx(GWND *ParentWnd, char *Caption, int Timeout, int Icon, char *fmt, ...)
{
	va_list	arg;
	char	Msg[80];

	va_start(arg, fmt);
	vsprintf(Msg, fmt, arg);
	va_end(arg);	
	_DrawResultMsg(ParentWnd, Caption, Icon, Msg, 0);
	if(!MenuGetAdminConsole()) {
		wndProcessWindow(ParentWnd, 0, Timeout);
		DestroyResultMsg();
	}
}

void DestroyResultMsg(void)
{
	if(!MenuGetAdminConsole()) DestroyWindow(wndResultMsg);
}

void DestroyWindow(GWND *wnd)
{
	GWND	*ParentWnd;
	RECT	rect;

	ParentWnd = wnd->ParentWnd;
	wndGetDrawArea(wnd, &rect);
	lcdSetClipRect(rect.x, rect.y, rect.Width, rect.Height);
	if(ParentWnd) {
		if(ParentWnd->WndType == 1) menuShow((GMENU *)ParentWnd);
		else	wndDrawWindow(ParentWnd);
	} else	DesktopDrawBackGround();
	lcdSetDefaultClipRect();
}

void GetIDTitle(long nID, char *Title)
{
	char	*p;
	
	p = Title;
	*p++ = 'I'; *p++ = 'D'; *p++ = ':'; *p++ = ' ';
	if(nID) sprintf(p, "%ld", nID);
	else	*p = 0;
}

void GetIDStrTitle(char *StrID, char *Title)
{
	char	*p;
	
	p = Title;
	*p++ = 'I'; *p++ = 'D'; *p++ = ':'; *p++ = ' ';
	if(StrID) strcpy(p, StrID);
	else	*p = 0;
}

void OnIndicator(int err)
{
//	if(!MenuGetAdminConsole()) {
//		if(err) ioSetRedLED();
//		else	ioSetBlueLED();
//	}
}

void OffIndicator(void)
{
	//if(!MenuGetAdminConsole()) ioClearAllLED();
}

void EnrollMsgProcessing(void)
{
}

void PutBackNextWizard(int Stage, int End)
{
}

//#include "cam.h"
//#include "jpeg_api.h"
static int		BackLight;
static unsigned long BackLightTimer;

void OnBackLight(void)
{
	if(BackLight == 0) {
		lcdBacklight(2);
	}
	BackLight = 1;
}

void OffBackLight(void)
{
	if(BackLight) {
		lcdBacklight(0);
		BackLight = 0;
	}
}

int GetBackLight(void)
{
	return BackLight; 
}

void StartBackLightTimer(void)
{
	BackLight = 2;
	BackLightTimer = DS_TIMER + sys_cfg->Device.BackLightTime*10;
}

void ProcessBackLightTimer(void)
{
	if(BackLight >= 2 && sys_cfg->Device.BackLightTime > 0) {
		if(DS_TIMER > BackLightTimer) OffBackLight();
	}
}

void StartCursor(int x, int y, int bmpSize)
{
}

void EndCursor(void)
{
}

void PutCursor(void)
{
}

void RestoreCursor(void)
{
}

// TCP/IP	0:Disconnected 1:Wait IP(DHCP) 2:Connecting 3:Connected
// Serial	0:Disconnected                              3:Connected 
// DialUp	0:Disconnected                 2:Connecting 3:Connected 
void SetServerNetState(int NetState)
{
	ServerNetState = NetState;
//cprintf(".....ServerNetState=%d NetCause=%d\n", ServerNetState, NetCause);
	if(DesktopIconsIsVisible()) DesktopDrawServerNetState();
}

void UnlockByCall(int adID)
{
	int		PV;
	
	PV = adGetPresentValue(adID);
	if(PV == S_NORMAL_LOCKED || PV == S_NORMAL_UNLOCKED ) {
		adSetPresentValue(adID, S_NORMAL_UNLOCKED);
	}
}

void DoorSetByCommand(int adID, int PV)
{
	adSetPresentValue(adID, PV);
}

void ResetSystem(void)
{
	unitsResetSystem();
	sys_cfg->DeviceID = 9999;	// for disconnect network
	taskDelay(2);	// wait for Disconnect
	fsCloseFiles();
	fsCloseScheFiles();
	wdtResetSystem();
}

/* Old
#ifdef SDRAMSIZE_64
#define FRAMEBUFFER_ADDR	0x33b00000		// User=3000_0000-33b0_0000(59MB) Audio/Video 33b0_0000-33f0_0000(4BM) System=33f0_0000-3400_0000(1MB)
#else
#define FRAMEBUFFER_ADDR	0x31b00000		// User=3000_0000-31b0_0000(27MB) Audio/Video 31b0_0000-31f0_0000(4BM) System=31f0_0000-3200_0000(1MB)
#endif
#define CAM_FRAMEBUFFER_C	FRAMEBUFFER_ADDR+0x00040000
#define CAM_FRAMEBUFFER_P	FRAMEBUFFER_ADDR+0x00200000	// Max buffer size of Port: 0x00100000 (2048*2048*4)
*/

/*
// New 2013.1.29
#ifdef SDRAMSIZE_64
#define FRAMEBUFFER_ADDR	0x33e00000		// User=3000_0000-33e0_0000(62MB) Audio/Video 33e0_0000-33f0_0000(1MB) System=33f0_0000-3400_0000(1MB)
#else
#define FRAMEBUFFER_ADDR	0x31e00000		// User=3000_0000-31e0_0000(30MB) Audio/Video 31e0_0000-31f0_0000(1MB) System=31f0_0000-3200_0000(1MB)
#endif
#define VOX_FRAMEBUFFER		FRAMEBUFFER_ADDR			// 0000_0000-0004_0000(256KB=(PCM_Max:128KB)*2=256KB)
#define CAM_FRAMEBUFFER_C	FRAMEBUFFER_ADDR+0x00040000	// 0004_0000-000b_0800(450KB=(Y:320*240+Cb:320*240/4+Cr:320*240/4)*4=115200*4=460,800)
#define CAM_FRAMEBUFFER_P	FRAMEBUFFER_ADDR+0x000b0800	// 000b_0800-000d_6000(150KB=320*240*2=153600)


void InitializeCam(void)
{
	int		rval, mode;

	rval = camOpen();
	if(rval >= 0) {
		//camSetConfig(320, 240, 192, 144, 0, 0, CAM_FRAMEBUFFER_C, CAM_FRAMEBUFFER_P);
		camSetConfig(320, 240, 320, 240, 0, 0, CAM_FRAMEBUFFER_C, CAM_FRAMEBUFFER_P);
		mode = sys_cfg->Camera.Mode;
		if(mode < 3) camRunCodec();
		mode = sys_cfg->Camera.Channel;
		camSelectMux(mode);
	}
}

int RunCam(void)
{
	int		rval, state;

	state = camGetCodecState();	
	if(state == CAM_NULL) {
		rval = camOpen();
		if(rval >= 0){
			camSetConfig(320, 240, 320, 240, 0, 0, CAM_FRAMEBUFFER_C, CAM_FRAMEBUFFER_P);
			camRunCodec();
			rval = 1;
		}
	} else {
		camRunCodec();
		rval = 1;
	}
	if(DesktopIconsIsVisible()) DesktopDrawCamState();
	return rval;
}

void StopCam(void)
{
	int		rval, state;

	state = camGetCodecState();	
	if(state == CAM_NULL) {
		rval = camOpen();
		if(rval >= 0)camSetConfig(320, 240, 320, 240, 0, 0, CAM_FRAMEBUFFER_C, CAM_FRAMEBUFFER_P);
	} else {
		camStopCodec();
	}
	if(DesktopIconsIsVisible()) DesktopDrawCamState();
}

static int _MakeMyJpegHeader(JPEG *jpeg, void *Buffer)
{
	unsigned char	*p;

	p = (unsigned char *)Buffer;
	*p++ = 0;						// RS: 
	*p++ = jpeg->QuantityLevel;
	SHORTtoBYTE(jpeg->Width, p); p += 2;
	SHORTtoBYTE(jpeg->Height, p);
	return 6;
}

int CaptureCamJpeg(void *JPEGBuffer, void *YUVBuffer, unsigned long *CamFrameCount)
{
	JPEG	*jpeg, _jpeg;
	unsigned char	*p0, *p;
	int		state;
	
	state = camGetCodecState();
	if(state != CAM_RUNNING) return 0;
	camCaptureCodec(YUVBuffer, CamFrameCount, 1);
	jpeg = &_jpeg;
	jpeg->Width = 240; jpeg->Height = 320;
	jpeg->Bits = 24; jpeg->QuantityLevel = 3;
	p = p0 = (unsigned char *)JPEGBuffer;
//	p += jpegEncodeHeader(jpeg, p);
	p += _MakeMyJpegHeader(jpeg, p);
	p += jpegEncodeBodyFromYUV420(jpeg, p, (unsigned char *)YUVBuffer);
	return p - p0;
}

int CaptureSlowCamJpeg(void *JPEGBuffer, void *YUVBuffer, unsigned long *CamFrameCount)
{
	JPEG	*jpeg, _jpeg;
	unsigned char	*p0, *p;
	int		state;
	
	state = camGetCodecState();
	if(state != CAM_RUNNING) return 0;
	camCaptureCodec(YUVBuffer, CamFrameCount, 1);
	jpeg = &_jpeg;
	jpeg->Width = 240; jpeg->Height = 320;
	jpeg->Bits = 24; jpeg->QuantityLevel = 3;
	p = p0 = (unsigned char *)JPEGBuffer;
//	p += jpegEncodeHeader(jpeg, p);
	p += _MakeMyJpegHeader(jpeg, p);
	p += jpegSlowEncodeBodyFromYUV420(jpeg, p, (unsigned char *)YUVBuffer);
	return p - p0;
}

int MakeJpegHeader(void *JPEGBuffer)
{
	JPEG	*jpeg, _jpeg;
	unsigned char	*p0, *p;

	jpeg = &_jpeg;
	jpeg->Width = 240; jpeg->Height = 320;
	jpeg->Bits = 24; jpeg->QuantityLevel = 3;
	p = p0 = (unsigned char *)JPEGBuffer;
	p += jpegEncodeHeader(jpeg, p);
	return p - p0;
}
*/
/*
int GetDeviceStatus(void)
{
	int		status, InputStatus;
	
	status = gLockState;
	InputStatus = ctrlerGetInputStatus();
	if(InputStatus & 0x04) status |= DOOR_CONTACT_STATUS;
	if(InputStatus & 0x01) status |= REQUEST_TO_EXIT_STATUS;
	if(gIOStatus & G_TAMPER) status |= TAMPER;
	return status;
}
*/
int EncodeDeviceStatus(void *buf)
{
	unsigned char	*p, c;
	
	p = (unsigned char *)buf;
	if(!(sys_cfg->Device.DeviceOption & TAMPER)) c = 0x00;
	else if(gIOStatus & G_TAMPER) c = 0x02;
	else	c = 0x01;
	*p++ = c;
	return p - (unsigned char *)buf;
} 

int EncodeStatus(void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	p += EncodeDeviceStatus(p);
	for(i = 0;i < MAX_AD_SZ;i++) p += adEncodeShortStatus(i, p);
	return p - (unsigned char *)buf;
}