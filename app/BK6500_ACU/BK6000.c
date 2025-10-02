#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "system_hdr.h"
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "..\BootLoader\flash.h"
#include "cotask.h"
#include "mmu.h"
#include "rtc.h"
#include "sio.h"
#include "sin.h"
#include "vox.h"
#include "key.h"
#include "gfont.h"
#include "lcdc.h"
#include "iodev.h"
#include "sdi.h"
#include "jpeg_api.h"
#include "syscfg.h"
#include "unit.h"
#include "acad.h"
#include "acap.h"
#include "bio.h"
#include "alm.h"
#include "fire.h"
#include "lang.h"
#include "fsuser.h"
#include "fsar.h"
#include "cr.h"
#include "sysdep.h"
#include "fpapi_new.h"
#include "fpimage.h"
#include "event.h"
#include "ment.h"
#include "menuprim.h"
#include "wnds.h"
#include "desktop.h"
#include "devprim.h"
#include "topprim.h"
#include "toptask.h"

int		gRequest,  gIOStatus;
unsigned char	gTouch, gSvrSioPort, gSound, gSoundStop, gLocalDateTimeVersion, taState;
FP_USER	*gUser, _gUser, *gUser2, _gUser2;
UNIT			_gUnits[MAX_UNIT_SZ];
IOUNIT			_gIOUnits[MAX_IOUNIT_SZ];
ACCESS_DOOR		_gAccessDoors[MAX_AD_SZ];
ACCESS_POINT	_gAccessPoints[MAX_AP_SZ];
BINARY_INPUT	_gBinaryInputs[MAX_BI_SZ+MAX_XBI_SZ];
BINARY_OUTPUT	_gBinaryOutputs[MAX_BO_SZ+MAX_XBO_SZ];
CREDENTIAL_READER _gCredentialReaders[MAX_CR_SZ];
FIRE_POINT		_gFirePoints[MAX_UNIT_HALF_SZ];
FIRE_ZONE		_gFireZones[MAX_UNIT_HALF_SZ];
ALARM_ACTION	_gAlarmActions[MAX_UNIT_SZ];

CO_TASK		*t_main, *t_clock, *t_key, *t_svr, *t_sin, *t_units, *t_console;

void CPUInitialize(void);
void HeapInitialize(void);
void InitializeUnits(void);
void LocalDoorInit(void);
//extern char Image$$RO$$Limit[], Image$$RO$$Base[];
extern char Image$$RW$$Limit[], Image$$RW$$Base[];
//extern char Image$$ZI$$Limit[], Image$$ZI$$Base[];


//=============================Main Funnction=========================================
void Main(void)
{
	unsigned char	*p;
	int 	rval, name;

	CPUInitialize();
//	_init_alloc(Image$$RW$$Limit, Image$$RW$$Limit+10240);
//	HeapInitialize();
	flashOpen();
	ioInit();
	beepOpen();
	consoleOpen();
	lcdInit();
	SetLanguage(LANG_ENG);
	keyInit(KeyBeep);
	OnBackLight();
	lcdClear(RGB(255, 255, 255));
	lcdPutText(32, 32, "Initializing...", RGB(0,0,0));
	rval = fsOpenFiles(GetUserFileSize(), GetFPTemplateFileSize(), GetMainBuffer());
	rval = fsOpenScheFiles();
	syscfgInitialize();
	InitializeLanguage();
	lcdSetContrast((int)sys_cfg->Device.LCDContrast);
	lcdSetBrightness((int)sys_cfg->Device.LCDBrightness);
	gLocalDateTimeVersion = 1;
	StartBackLightTimer();
	uDelay(30);
	// Ver1.0 2015.1.18
	//	  -
	// Ver1.1 2015.3.27
	//	  - RTC, SIO 워치독 기능 추가
	//	  - 	
	// Ver2.0 2015.4.27
	//	  - 단독형 UDP Slave 수용
	//	  - 	
	// Ver3.0 2015.9.10
	//	  - 단독형 UDP Slave 통신 프로토콜 수정
	//	  - 카메라 기능 삭제	
	// Ver3.1 2015.10.22
	//	  - 사용자 Hash Indexing 추가
	//	  - 구 암호변경 미지원 리더기에서 암호변경 CliReq 등이 실패하여 무한 반복 버그 수정	
	//	  - SD 메모리에서 펌웨어 갱신시 파일명을 "BK6000.bin" 고정에서 Device.Model+".bin" 으로 변경	
	// Ver3.2 2015.12.1
	//	  - Slave 리더로부터 이벤트통지(EventNotification) 추가
	// Ver3.3 2017.10.27
	//	  - 복수 출입문 명령 추가	
	// Ver3.4 2018.1.24
	//	  - 지문 다운로드 버그 수정	
	// Ver3.5 2018.8.27
	//	  - UDP Slave Reader 로부터 별도 세션에 의한 BLE 비콘 이벤트 수신  	
	// Ver3.6 2019.5.15
	//	  - Slave 와 통신에서 CLI_BYPASS_REQUEST 개념 도입
	//	  - client xfer WRQ 확장(transferSize, checksum) 	
	// Ver3.7 2019.8.6
	//	  - Passback On Exit 기능 추가
	//	  - Passback RepeatAccessDelay 기능 추가 	
	// Ver3.8 2020.2.13
	//	  - door alarm 이 Enable 엽 변경에 따라 자동으로 해제하도록 수정
	//	  - door alarm 방식 주성, adAlarmState 추가, adAlarmChanged 수정
	// Ver3.9 2020.9.11
	//	  - Hynix EMV, BLE(serial) 카드 추가에 따른 이벤트 변경 수정
	// Ver3.9.1 2020.10.09
	//	  - EMV 카드 E_ALL_GRANTED 이벤트 버그 수정
	// Ver3.9.2 2021.6.22
	//	  - Video Download(tftpc) 추가
	// Ver3.9.3 2021.7.22
	//	  - event 처리 방식 수정(evt.c fsevt.c)
	//	  - slave UDP 수신 ip-address, port check 추가  
	// Ver3.9.4 2021.7.22
	//	  - XferBuffer size (1.2MB => 1.4MB)		
	cprintf("################################\n");
	cprintf("#  BK6500 Ver3.9.4 2022.7.11   #\n");
	cprintf("################################\n");
	cprintf("Device ID: %d\n", (int)sys_cfg->DeviceID);
	p = syscfgGetMACAddress();
	cprintf("MAC Address: %02x-%02x-%02x-%02x-%02x-%02x\n", (int)p[0], (int)p[1], (int)p[2], (int)p[3], (int)p[4], (int)p[5]);
//cprintf("FCLK=%dHz HCLK=%dHz PCLK=%dHz UCLK=%dHz\n", FCLK, HCLK, PCLK, UCLK);
//cprintf("RW: %x - %x (%x)\n", Image$$RO$$Base, Image$$RO$$Limit, Image$$RO$$Limit-Image$$RO$$Base);
//cprintf("RW: %x - %x (%x)\n", Image$$RW$$Base, Image$$RW$$Limit, Image$$RW$$Limit-Image$$RW$$Base);
//cprintf("ZI: %x - %x (%x)\n", Image$$ZI$$Base, Image$$ZI$$Limit, Image$$ZI$$Limit-Image$$RW$$Base);
	InitializeUnits();
//	uDelay(27000);
	sfpInitTemplate(1);
cprintf("%ld InitFPRecord(%d)\n", DS_TIMER, rval);
//cprintf("InitFPRecord(%d)=%ld T=%d\n", rval, uval, DS_TIMER-timer);
	FPImageInitialize();
	sdiOpen();
	gRequest = gIOStatus = 0;
	gTouch = gSound = gSoundStop = 0;
	EventInit();
	menuInit();
	MenuSetAdminConsole(0);
	gUser = &_gUser; gUser2 = &_gUser2;
	WndsInitialize();
	DesktopInitialize();
	DesktopDrawAll();
	wdtOpen();
	wdtReset();
	simSeed((unsigned long)sys_cfg->DeviceID);
	ctMainInit();
	ctSvrInit();
	ctSinInit();
	ctUnitsInit();
	ctConsoleInit();

	taskInit(79000, 1000);
	name = 1;
	t_main		= taskStart((CO_TASK_FNP)ctMain,    &name, 12000, CO_TASK_ALWAYS_ON);
	t_clock		= taskStart((CO_TASK_FNP)ctClock,   &name,  2000, CO_TASK_ALWAYS_ON);
	t_key		= taskStart((CO_TASK_FNP)ctKey,     &name,  1000, CO_TASK_ALWAYS_ON);
	t_svr		= taskStart((CO_TASK_FNP)ctSvr,     &name,  9000, CO_TASK_ALWAYS_ON);	// 6000 부족
	t_sin		= taskStart((CO_TASK_FNP)ctSin,     &name,  1000, CO_TASK_ALWAYS_ON);
	t_units		= taskStart((CO_TASK_FNP)ctUnits,   &name, 38000, CO_TASK_ALWAYS_ON);
	t_console	= taskStart((CO_TASK_FNP)ctConsole, &name,  7000, CO_TASK_ALWAYS_ON);
	StartBackLightTimer();

	while(1) {
		taskYield();
	}
}

void UpdateDoorStatus(int ID);
void UpdateDoorPV(int ID);
void UpdateInputState(int ID);
void UpdateOutputState(int ID);

void InitializeUnits(void)
{
	unitsInit(_gUnits, MAX_UNIT_SZ);
	iounitsInit(_gIOUnits, MAX_IOUNIT_SZ);
	adsInit(_gAccessDoors, MAX_AD_SZ);
	adsSetCallBack(UpdateDoorStatus, UpdateDoorPV);
	crsInit(_gCredentialReaders, MAX_CR_SZ);
	apsInit(_gAccessPoints, MAX_AP_SZ);
	bisInit(_gBinaryInputs, MAX_BI_SZ+MAX_XBI_SZ);
	bisSetPutCallBack(UpdateInputState);
	bosInit(_gBinaryOutputs, MAX_BO_SZ+MAX_XBO_SZ);
	bosSetPutCallBack(UpdateOutputState);
	adsInitPresentValue();
	fpsInit(_gFirePoints, MAX_UNIT_HALF_SZ);
	fzsInit(_gFireZones, MAX_UNIT_HALF_SZ);
	actsInit(_gAlarmActions, MAX_UNIT_SZ);
}

//===================================================================
void Isr_Init(void);
void HaltUndef(void);
void HaltSwi(void);
void HaltPabort(void);
void HaltDabort(void);
void HaltIrq(void);
void HaltFiq(void);

void Isr_Init(void);
typedef	volatile unsigned char VUSHRT;
#define OPTI_DATA_ADDR	(0x18000000)

void CPUInitialize(void)
{
	int 	i;
	VUSHRT	usTemp;

	MMU_Init();
	//Port_Init();
#ifdef CPUCLK_300
	ChangeMPllValue(97, 1, 2);	// FCLK=296.35MHz FCLK:HCLK:PCLK=1:3:6
#else
	ChangeClockDivider(14, 12);	// FCLK=399.65MHz FCLK:HCLK:PCLK=1:4:8
#endif
	CalculateClock();
//	SetRamRefreshCount();
//	for(i = 0;i < 1000;i++) ;

	Isr_Init();
//rBANKSIZE = 0x32;

	rIISPSR = (2<<5)|(2<<0);	//IIS_LRCK=44.1Khz @384fs,PCLK=50Mhz.
	rGPHCON = rGPHCON & ~(0xf<<18)|(0x5<<18);   //CLKOUT 0,1=OUTPUT to reduce the power consumption.
	// rGPFCON = 0x0555;		// G7/G6=00 G5=01
	rtcOpen();
	rBWSCON = (rBWSCON & 0xfff000ff) | 0x00010100;	// nGCS4:16-bit LCD nGCS3:8-bit CMOS Sensor nGCS2:16-bit W5300
	rBANKCON2 = 0x7ff0;		// W5300:		[12:11]Tcos [10:8]Tacc(Access cycle)=b001=2clocks
	rBANKCON3 = 0x0200;		// CMOS Sensor:	[12:11]Tcos [10:8]Tacc(Access cycle)=b000=1clock
	// 2012.2.3  0x0000 => 0x0200 일부 보드에서 저품질 이미지가 나옴
	rBANKCON4 = 0x7ff0;		// LCD: 		[12:11]Tcos [10:8]Tacc(Access cycle)=b001=2clocks : 0x0000=> no display
	rGPBCON = (rGPBCON & 0xfffffff3) | 0x00000004;	// GPB1[3:2]=01(Output:Data Buffer Enable) 
	rGPBDAT |= 0x00000002;	// GPB1=1(Data Buffer Active)
	rGPDCON = (rGPDCON & 0xfffff3ff) | 0x00000400;	// GPD5[11:10]=01(Output):H/W Reset(W5300, CMOS Sensor)
	rGPDDAT |= 0x00000020;	// High
	for(i = 0;i < 8000;i++) usTemp = *(VUSHRT *)(OPTI_DATA_ADDR);
	rGPDDAT &= 0xffffffdf;	// Low
	for(i = 0;i < 1200;i++) usTemp = *(VUSHRT *)(OPTI_DATA_ADDR);
	rGPDDAT |= 0x00000020;	// High
	for(i = 0;i < 12000;i++) usTemp = *(VUSHRT *)(OPTI_DATA_ADDR);
	rGPBDAT &= 0xfffffffd;	// GPB1=0(Data Buffer Inactive)
	uDelay(350);
#ifdef BK_DEBUG
	cprintf("S3C2442B) FCLK=%dHz\n", Fclk);
	cprintf("\nMemory control registers value\n");
	cprintf("rBWSCON  =%8.8x\n", rBWSCON);
	cprintf("rBANKCON0=%8.8x - flash\n", rBANKCON0);
	cprintf("rBANKCON1=%8.8x - none \n", rBANKCON1);
	cprintf("rBANKCON7=%8.8x - none \n", rBANKCON7);
	cprintf("rREFRESH =%8.8x\n", rREFRESH);
	cprintf("rBANKSIZE=%8.8x\n", rBANKSIZE);
	cprintf("rMRSRB6  =%8.8x\n", rMRSRB6);
	cprintf("rMRSRB7  =%8.8x\n", rMRSRB7);
#endif
}

void HaltUndef(void);
void HaltSwi(void);
void HaltPabort(void);
void HaltDabort(void);

void Isr_Init(void)
{
	pISR_UNDEF  = (unsigned)HaltUndef;
	pISR_SWI    = (unsigned)HaltSwi;
	pISR_PABORT = (unsigned)HaltPabort;
	pISR_DABORT = (unsigned)HaltDabort;
	rINTMOD		= 0x0;				// All=IRQ mode
	rINTMSK		= BIT_ALLMSK;		// All interrupt is masked.
	rINTSUBMSK	= BIT_SUB_ALLMSK;	// All sub-interrupt is masked. <- April 01, 2002 SOP
}

void HaltUndef(void)
{
	cprintf("Undefined instruction exception.\n");
	while(1) ;
}

void HaltSwi(void)
{
	cprintf("SWI exception.\n");
	while(1) ;
}

void HaltPabort(void)
{
	cprintf("%ld Pabort exception.\n", DS_TIMER);
	while(1) ;
}

void HaltDabort(void)
{
	cprintf("Dabort exception.\n");
	while(1) ;
}

/*
extern char Image$$RW$$Limit[];
#define ram_data_limit Image$$RW$$Limit

typedef struct HeapDescriptor HeapDescriptor;
extern HeapDescriptor *__rt_embeddedalloc_init(void *base, size_t size);
static HeapDescriptor *hd;
HeapDescriptor *__rt_heapdescriptor(void)
{
	return hd;
}
*/

/*
int  rfread(unsigned char *buf);
void rfwait(void);
void rfprint(unsigned char *buf, int len);

void TestRF900(void)
{
	unsigned char buf[64];
	int		rval, i;

	sioOpen(1, 115200L, 0);
	buf[0] = '1';
	sioWrite(1, buf, 1);
	cprintf("%ld Wakeup...\n", TICK_TIMER);
	rval = rfread(buf);
	if(rval <= 0) cprintf("%ld Timeout\n", TICK_TIMER);
	else {
		rfprint(buf, rval);
		buf[0] = 0x7e; buf[1] = 0x09; buf[2] = 0x6c; buf[3] = 0x61; buf[4] = 0x20;
		buf[5] = 0x70; buf[6] = 0x31; buf[7] = 0x35; buf[8] = 0x62; buf[9] = 0x30;
		buf[10] = buf[2]; buf[11] = 0x7d;
		for(i = 3;i < 10;i++) buf[10] ^= buf[i];
		//buf[0] = 0x7e; buf[1] = 0x06; buf[2] = 0x6c; buf[3] = 0x70; buf[4] = 0x20; buf[5] = 0x31; buf[6] = 0x35;
		//buf[7] = buf[2]; buf[8] = 0x7d;
		//for(i = 3;i < 7;i++) buf[7] ^= buf[i];
		sioWrite(1, buf, 12);
cprintf("%ld SetControl...\n", TICK_TIMER);
		rval = rfread(buf);
		if(rval <= 0) cprintf("%ld Timeout\n", TICK_TIMER);
		else	rfprint(buf, rval);
		
		buf[0] = 0x7e; buf[1] = 0x03; buf[2] = 0x60; buf[3] = 0x65; buf[4] = 0x05; buf[5] = 0x7d;
		sioWrite(1, buf, 6);
		cprintf("%ld ReqTagRead...\n", TICK_TIMER);
		rval = rfread(buf);
		if(rval <= 0) cprintf("%ld Timeout\n", TICK_TIMER);
		else	rfprint(buf, rval);

		rval = rfread(buf);
		if(rval > 0) {
			rfprint(buf, rval);
			rfwait();
		} else {
			buf[0] = '#';
			sioWrite(1, buf, 1);
cprintf("%ld Stop...\n", TICK_TIMER);
			rval = rfread(buf);
			if(rval > 0) {
				rfprint(buf, rval);
				rfwait();
			}
		}	
	}
	while(1) {
		buf[0] = '1';
		sioWrite(1, buf, 1);
		cprintf("%ld Wakeup...\n", TICK_TIMER);
		rval = rfread(buf);
		if(rval <= 0) {
			cprintf("%ld Timeout\n", TICK_TIMER);
			continue;
		} else	rfprint(buf, rval);

		buf[0] = 0x7e; buf[1] = 0x03; buf[2] = 0x60; buf[3] = 0x65; buf[4] = 0x05; buf[5] = 0x7d;
		sioWrite(1, buf, 6);
		cprintf("%ld ReqTagRead...\n", TICK_TIMER);
		rval = rfread(buf);
		if(rval <= 0) cprintf("%ld Timeout\n", TICK_TIMER);
		else	rfprint(buf, rval);

		rval = rfread(buf);
		if(rval > 0) {
			rfprint(buf, rval);
			rfwait();
		} else {
			buf[0] = '#';
			sioWrite(1, buf, 1);
cprintf("%ld Stop...\n", TICK_TIMER);
			rval = rfread(buf);
			if(rval > 0) {
				rfprint(buf, rval);
				rfwait();
			}
		}
	}
}

void rfwrite(unsigned char *buf, int size)
{
	sioWrite(1, buf, size);
}

int rfread(unsigned char *buf)
{
	unsigned long	timer;
	int		c, cnt, len;

	timer = TICK_TIMER;
	while(1) {
		c = sioPeek(1);
		if(c >= 0 || (TICK_TIMER-timer) > 12) break;
	}
	if(c < 0) return 0;
	timer = TICK_TIMER;
	cnt = len = 0;
	while(1) {
		if((TICK_TIMER-timer) >= 2) break;
		c = sioGetc(1);
		if(c < 0) uDelay(1);
		else {					
			timer = TICK_TIMER;
			buf[cnt] = c; cnt++;
			if(cnt == 2) len = buf[1] + 3;
			if(len && cnt >= len || cnt >= 64) break; 
		}
	}
	return cnt;
}

void rfwait(void)
{
	unsigned long	timer;

	timer = TICK_TIMER;
	while(1) {
		if((TICK_TIMER-timer) > 12) break;
	}	
}

void rfprint(unsigned char *buf, int size)
{
	int		i, c;
	
	c = buf[2];
	if(c == 0x90) cprintf("%ld ACK  Len=%d [%02x", TICK_TIMER, size, (int)buf[0]);
	else if(c == 0xa1 || c == 0xae) cprintf("%ld ResTagRead  Len=%d [%02x", TICK_TIMER, size, (int)buf[0]); 
	else	cprintf("%ld Unknown  Len=%d [%02x", TICK_TIMER, size, (int)buf[0]);
	for(i = 1;i < size;i++) cprintf("-%02x", buf[i]);
	cprintf("]\n");
}

*/