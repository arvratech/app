#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
////////////////////////////////
#include "system_hdr.h"
////////////////////////////////
#include "prim.h"
#include "rtc.h"
#include "acad.h"
#include "bio.h"
#include "cr.h"
#include "iodev.h"

// Input
#define	MUX					(rGPGDAT &  0x00000002)		// GPG1(MUX)
#define	READER_TAMPER1		(rGPFDAT &  0x00000008)		// GPF3(1.ReaderTamper)
#define	READER_TAMPER2		(rGPGDAT &  0x00000020)		// GPG5(2.ReaderTamper)
// Output
#define	LOCK1_H				(rGPDDAT |= 0x00000008)		// GPD3=1(1.Lock)
#define	LOCK1_L				(rGPDDAT &= 0xfffffff7)		// GPD3=0(1.Lock)
#define	LOCK2_H				(rGPDDAT |= 0x00000001)		// GPD0=1(2.Lock)
#define	LOCK2_L				(rGPDDAT &= 0xfffffffe)		// GPD0=0(2.Lock)
#define	BLUE_LED1_H			(rGPCDAT |= 0x00000200)		// GPC9=1(1.BlueLED)
#define	BLUE_LED1_L			(rGPCDAT &= 0xfffffdff)		// GPC9=0(1.BlueLED)
#define	BLUE_LED2_H			(rGPDDAT |= 0x00000040)		// GPD6=1(2.BlueLED)
#define	BLUE_LED2_L			(rGPDDAT &= 0xffffffbf)		// GPD6=0(2.BlueLED)
#define	RED_LED1_H			(rGPDDAT |= 0x00000002)		// GPD1=1(1.RedLED)
#define	RED_LED1_L			(rGPDDAT &= 0xfffffffd)		// GPD1=0(1.RedLED)
#define	RED_LED2_H			(rGPDDAT |= 0x00000200)		// GPD9=1(2.RedLED)
#define	RED_LED2_L			(rGPDDAT &= 0xfffffdff)		// GPD9=0(2.RedLED)
#define	BUZZER1_H			(rGPDDAT |= 0x00000004)		// GPD2=1(1.Buzzer)
#define	BUZZER1_L			(rGPDDAT &= 0xfffffffb)		// GPD2=0(1.Buzzer)
#define	BUZZER2_H			(rGPDDAT |= 0x00000400)		// GPD10=1(2.Buzzer)
#define	BUZZER2_L			(rGPDDAT &= 0xfffffbff)		// GPD10=0(2.Buzzer)
#define	HOLD1_H				(rGPDDAT |= 0x00000020)		// GPD5=1(1.Hold)
#define	HOLD1_L				(rGPDDAT &= 0xffffffdf)		// GPD5=0(1.Hold)
#define	HOLD2_H				(rGPCDAT |= 0x00000010)		// GPC4=1(2.Hold)
#define	HOLD2_L				(rGPCDAT &= 0xffffffef)		// GPC4=0(2.Hold)

extern unsigned long Pclk;

static unsigned char  InMux, InputPV[10];
static unsigned long  InputTimer[10];


void ioInit(void)
{
	int		i;

	// Dead zone length[23:16]=0 Prescaler 1[15:8]=166(0xa6) prescaler value for Timer 2,3,4
	// Prescaler 0[7:0]=166(0xa6) prescaler value for Timer 0,1
#ifdef CPUCLK_300
	rTCFG0 = (rTCFG0 & 0xff000000) | 0x00007a7a;	// FCLK=296.352MHz MHZ HCLK=98.784000MHz PCLK=49.392000MHz
	// (1/(PCLK/(Prescaler+1)/divider)=(122+1)*2/49.392000MHz= 4.9805637us
	//                                 (122+1)*4/49.392000MHz= 9.961127us	
#else
	rTCFG0 = (rTCFG0 & 0xff000000) | 0x00007c7c;	// FCLK=399.6518MHz    HCLK=99.912960MHz PCLK=49.956480MHz
	// (1/(PCLK/(Prescaler+1)/divider)=(124+1)*2/49.956480MHz= 5.004356us
	//                                 (124+1)*4/49.956480MHz= 10.00871us	
#endif

	// DMA mode[23:20]=b0000(No select=all interrupt) MUX 4[19:16]=b0000(1/2) MUX 3[15:12]=b0000(1/2)
	// MUX 2[11:8]=b0000(1/2) MUX 1[7:4]=b0000(1/2) MUX 0[3:0]=b0000(1/2)
	rTCFG1 = (rTCFG1 & 0xff000000) | 0x00000000;

	rGPCCON = (rGPCCON & 0xfc03fcff) | 0x01540100;	// GPC12-10[25:20]=010101(Output:MUX Sel) GPC9[19:18]=01(Output:1.BlueLED)
													// GPC4[9:8]=01(Output:2.Hold)
	rGPDCON = (rGPDCON & 0xffc3c300) | 0x00141455;	// GPD10[21:20]=01(Output:2.Buzzer) GPD9[19:18]=01(Output:2.RedLED) 
													// GPD6[13:12]=01(Output:2.BlueLED) GPD5[11:10]=01(Output:1.Hold)
													// GPD3[7:6]=01(Output:1.Lock) GPD2[5:4]=01(Output:1.Buzzer)
													// GPD1[13:12]=01(Output:1.RedLED) GPD0[1:0]=01(Output:2.Lock)
	rGPFCON = (rGPFCON & 0xffffff3f) | 0x00000000;	// GPF3[7:6]=00(Input:1.ReaderTamper)
	rGPGCON = (rGPGCON & 0xfffff3f3) | 0x00000000;	// GPG5[11:10]=00(Input:2.ReaderTamper) GPG1[3:2]=00(Input:MUX)
	ioOffLock(0); ioOffLock(1);
	for(i = 0;i < 10;i++) {
		InputPV[i] = 0;
	}
	InMux = 0;
	rGPCDAT = (rGPCDAT & 0xffffe3ff) | 0x00000000;	// 000: CH1-Exit
}

void ioProcessInputAlarm(void)
{
	unsigned char	val;
	int		i;

	// Inputs: 1-Exit, 1-Door, 1-Input 1-Fire, 2-Exit, 2-Door, 2-Input, 2-Tamper	
	i = InMux;
	if(i == 8) val = !READER_TAMPER1;
	else if(i == 9) val = !READER_TAMPER2;
	else	val = !MUX;
	if(val != InputPV[i]) {
		InputPV[i] = val; InputTimer[i] = TICK_TIMER;
	} else if(val != biPV[i] && (TICK_TIMER-InputTimer[i]) > 6) {	// 100ms
		biPV[i] = val;
		if(i < 8) biLocalPVChanged(i, (int)val);
		else	crLocalTamperPVChanged(i-8, (int)val);
	}
	InMux++; if(InMux > 9) InMux = 0;
	if(InMux < 8) rGPCDAT = (rGPCDAT & 0xffffe3ff) | (InMux << 10);
}

void ioOnLock(int ch)
{
	if(ch) LOCK2_H;
	else   LOCK1_H;
//cprintf("CH=%d OnLock: %08x\n", ch);
}

void ioOffLock(int ch)
{
	if(ch) LOCK2_L;
	else   LOCK1_L;
//cprintf("CH=%d OffLock\n", ch);
}

void ioOnAuth(int ch)
{
	if(ch) {
		BLUE_LED2_L;; RED_LED2_H;	// BlueLED=On RedLED=Off
	} else {
		BLUE_LED1_L;; RED_LED1_H;	// BlueLED=On RedLED=Off 
	}
}

void ioOffAuth(int ch)
{
	if(ch) {
		BLUE_LED2_H;; RED_LED2_L;	// BlueLED=Off RedLED=On
	} else {
		BLUE_LED1_H;; RED_LED1_L;	// BlueLED=Off RedLED=On 
	}
}

void ioOnBuzzer(int ch)
{
	if(ch) BUZZER2_L; else BUZZER1_L;
}

void ioOffBuzzer(int ch)
{
	if(ch) BUZZER2_H; else BUZZER1_H;
}

void ioOnHold(int ch)
{
	if(ch) HOLD2_L; else HOLD1_L;
}

void ioOffHold(int ch)
{
	if(ch) HOLD2_H; else HOLD1_H;
}

void ioOnMuteSpeaker(void)
{
}

void ioOffMuteSpeaker(void)
{
}

unsigned long	WdtResetTimer;

void wdtOpen(void)
{
	// Prescaler value[15:8]=PCLK/1M*4 Watchdog timer[5]=0(Disable) Clock select[4:3]=11(128)
	// Interrupt generarion[2]=0(Disable) Reset[0]=0(Disable)	
	rWTCON = (((Pclk*3+500000)/1000000-1)<<8) | 0x18;	// resolution=384us @any PCLK 
	rWTDAT = 0x4c4b;	// for first update(7.5second)
	rWTCNT = 0x4c4b;	// seconds = (rWTCON >> 8) + 1) * 128 * rWTDAT / PCLK     
	// Prescaler value[15:8]=PCLK/1M*4 Watchdog timer[5]=1(Enable) Clock select[4:3]=11(128)
	// Interrupt generarion[2]=0(Disable) Reset[0]=1(Enable)
	rWTCON = (((Pclk*3+500000)/1000000-1)<<8) | 0x39;
}

void wdtReset(void)
{
	rWTCNT = 0x3938;	// 5.625 seconds
	WdtResetTimer = DS_TIMER;
}

void wdtResetLong(void)
{
	rWTCNT = 0xffff;	// 25.19 seconds
	WdtResetTimer = DS_TIMER;
}

void wdtResetSystem(void)
{
	rWTCON = (((Pclk*3+500000)/1000000-1)<<8) | 0x18;	// resolution=384us @any PCLK 
	rWTDAT = 0x104;		// for first update(100ms)
	rWTCNT = 0x104;		// seconds = (rWTCON >> 8) + 1) * 512 * rWTDAT / PCLK     
	rWTCON = (((Pclk*3+500000)/1000000-1)<<8) | 0x39;
	while(1) ;
}

void __irq Timer2_ISR(void)
{
	int		c;

	c = rSRCPND; c = rINTPND;
	rGPHDAT &= 0xfffffbff;
	rSRCPND = BIT_TIMER2; rINTPND = BIT_TIMER2;
}

void beepOpen(void)
{
    pISR_TIMER2 = (unsigned)Timer2_ISR;
	rSRCPND = BIT_TIMER2; rINTPND = BIT_TIMER2;
	SET_IRQFIQ(); rINTMSK &= ~BIT_TIMER2; CLR_IRQFIQ();
	// Timer 2 [15]=0(One-shot) [13]=0(No Operation) [12]=0(Stop)
	rTCON &= 0xffff0fff;
	rGPHCON = (rGPHCON & 0xffcfffff) | 0x00100000;	// GPH10[21:20]=01(Output:Buzzer)	
}

void beepPlay(void)
{
	// Timer 2 [15]=0(One-shot) [13]=1(Update TCNTB1) [12]=0(Stop)
	rTCNTB2 = 1200;
	rTCON   = (rTCON & 0xffff0fff) | 0x00002000;
	// Timer 2 [15]=0(One-shot) [13]=0(No operation) [12]=1(Start)
	rTCON   = (rTCON & 0xffff0fff) | 0x00001000;
	rGPHDAT |= 0x00000400;	
}
