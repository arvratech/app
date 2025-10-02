#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "system_hdr.h"
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "key.h"
#include "sio.h"
#include "iodev.h"
#include "syscfg.h"
#include "em4095.h"

#define	EM_SHD_H		(rGPBDAT |= 0x00000002)		// GPB1=1(sleep mode)
#define	EM_SHD_L		(rGPBDAT &= 0xfffffffd)		// GPB1=0(operation mode)
#define	EM_MOD_H		(rGPDDAT |= 0x00000002)		// GPD1=1(100% of modulation)
#define	EM_MOD_L		(rGPDDAT &= 0xffffdffd)		// GPD1=0(no modulation)

#define StartTimer0() {	rTCON = (rTCON & 0xffffffe0) | 0x00000002; ;rTCON = (rTCON & 0xffffffe0) | 0x00000001; }
	// T0 auto reload on/off[3]=0(One-shot) output inverter on/off[2]=0(Inverter off) manual update[1]=1(Update TCNTB0) start/stop[0]=0(Stop)
	// T0 auto reload on/off[3]=0(One-shot) output inverter on/off[2]=0(Inverter off) manual update[1]=0(No operation)  start/stop[0]=1(Start)
volatile unsigned char  em_recv, em_new;
volatile unsigned short em_loop;
static unsigned char em_rxdata[8];
static unsigned long em_error;

unsigned char	*mcc_p, mcc_rxdata[8];
unsigned char	mcc_state, mcc_mask, mcc_bitcnt;
int		mcc_tcnt;

/*
void __irq Timer0Done(void)
{
	int		c;

	c = rSRCPND; c = rINTPND;
WrUTXH2('O');
	mcc_state = 0;
	rSRCPND = BIT_TIMER0; rINTPND = BIT_TIMER0; 
}
*/
int		xxxx[1024];
int		xi, end_tcnt;

void EM4095_ISR(void)
{
	unsigned long	lval;
	unsigned char	cpar, cval;
	int		tcnt, c, i, j;
	
//	c = rSRCPND; c = rINTPND; c = rEINTPEND;
	tcnt = rTCNTO0;
	if(rGPGDAT & 0x00000020) c = 0; else c = 1;
	if(!mcc_state || mcc_tcnt > tcnt+160) {	// 120
//if(c) WrUTXH2(mcc_state+'0'+5);
//else  WrUTXH2(mcc_state+'0');
		if(c) {
//xxxx[xi++] = mcc_tcnt - tcnt; if(xi >= 64) xi = 0;
//WrUTXH2('A');
			rTCNTB0 = mcc_tcnt = 0xffff;	// 5*65535=327675us(327ms)	
			StartTimer0();
			mcc_bitcnt = 1;
			mcc_state = 1;
		} else {
//WrUTXH2('B');
			mcc_state = 0;
		}
		if(em_new) {
			em_loop++;
			if(em_loop > 250) em_new = 0;
		}
	} else if(mcc_tcnt > tcnt+92) {	// 89 // one cycle:512us=102.1 Observation Mean=105
		if(mcc_state == 1) {
			if(mcc_bitcnt >= 9) {
				mcc_p = mcc_rxdata;
				if(c) *mcc_p = 0x80; else *mcc_p = 0x00; // 0(High:0 Low:1) 1(High:1 Low:0)
				mcc_mask = 0x80; mcc_bitcnt = 1;
				mcc_state = 2;
			} else if(c) {
				mcc_bitcnt++;
			} else {
				mcc_state = 0;
//WrUTXH2('E');
			}
		} else if(mcc_state == 2) {
//xxxx[xi++] = mcc_tcnt - tcnt; if(xi >= 1024) xi = 0;
			mcc_bitcnt++; mcc_mask >>= 1;
			if(!mcc_mask) {
				mcc_p++;
				if(c) *mcc_p = 0x80; else *mcc_p = 0x00; // 0(High:0 Low:1) 1(High:1 Low:0)
				mcc_mask = 0x80;
			} else {
				if(c) (*mcc_p) |= mcc_mask;
				if(mcc_bitcnt == 55) {
//WrUTXH2('.');
					lval = mcc_rxdata[0];  lval <<= 8;
					lval |= mcc_rxdata[1]; lval <<= 8;
					lval |= mcc_rxdata[2]; lval <<= 8;
					lval |= mcc_rxdata[3];
					for(i = j = 0;i < 6;i++) {
//printf("[%d] %08x\n", i, ch & 0xf8000000); 
						if(lval & 0x80000000) c = 1; else c = 0;
						if(lval & 0x40000000) c ^= 1;
						if(lval & 0x20000000) c ^= 1;
						if(lval & 0x10000000) c ^= 1;
						if(c && !(lval & 0x08000000) || !c && (lval & 0x08000000)) break;
						if(i & 1) {
							mcc_rxdata[j] |= (unsigned char)((lval >> 28) & 0x0f); j++;
						} else {
							mcc_rxdata[j] = (unsigned char)((lval >> 24)& 0xf0);
						}
						lval <<= 5;
					}
					if(i >= 6) {
						lval >>= 22;
						lval |= mcc_rxdata[4]; lval <<= 8;
						lval |= mcc_rxdata[5]; lval <<= 8;
						lval |= mcc_rxdata[6]; lval <<= 6;
						for( ;i < 10;i++) {
//printf("[%d] %08x\n", i, ch & 0xf8000000);
							if(lval & 0x80000000) c = 1; else c = 0;
							if(lval & 0x40000000) c ^= 1;
							if(lval & 0x20000000) c ^= 1;
							if(lval & 0x10000000) c ^= 1;
							if(c && !(lval & 0x08000000) || !c && (lval & 0x08000000)) break;
							if(i & 1) {
								mcc_rxdata[j] |= (unsigned char)((lval >> 28) & 0x0f); j++;
							} else {
								mcc_rxdata[j] = (unsigned char)((lval >> 24)& 0xf0);
							}
							lval <<= 5;
						}
					}
					c = 0;
					if(i >= 10) {
						cpar = 0;
						for(i = 0;i < 5;i++) cpar ^= mcc_rxdata[i];
						cval = cpar << 4;
						cpar ^= cval;
						cval = (unsigned char)(lval >> 24);
						if((cval & 0xf0) == (cpar & 0xf0) && !(cval & 0x08)) c = 1;
					}
					if(c && (mcc_rxdata[0] || mcc_rxdata[1] || mcc_rxdata[2] || mcc_rxdata[3] || mcc_rxdata[4])) {
cprintf("\n%02x", (int)mcc_rxdata[0]); for(c = 1;c < 5;c++) cprintf("-%02x", (int)mcc_rxdata[c]);  
						if(!em_new || n_memcmp(em_rxdata, mcc_rxdata, 5)) { 
							memcpy(em_rxdata, mcc_rxdata, 5); em_new = em_recv = 1;
						}
						em_loop = 0;							
					} else {
cprintf("\nErr: %02x,", (int)mcc_rxdata[0]); for(c = 1;c < 5;c++) cprintf("-%02x", (int)mcc_rxdata[c]); 
						em_error++;
					}
					mcc_state = 0;
				}
			}
		}
		mcc_tcnt = tcnt;
	}
}

void __irq EINT8_23_ISR(void);

void emOpen(void)
{
	int		mask;

	rGPBCON = (rGPBCON & 0xfffffff0) | 0x00000004;	// GPB1[3:2]=01(Output:EM_SHD) GPB0[1:0]=00(Input:EM_RDY/CLK)
	rGPDCON = (rGPDCON & 0xfffffff3) | 0x00000004;	// GPD1[3:2]=01(Output:EM_MOD)
	rGPGCON = (rGPGCON & 0xfffff3ff) | 0x00000800;	// GPG5[11:10]=10(EINT13:EM_DEMOD_OUT)
//	pISR_TIMER0 = (int)Timer0Done;
//	rSRCPND = BIT_TIMER0; rINTPND = BIT_TIMER0;
//	SET_IRQFIQ(); rINTMSK &= ~BIT_TIMER0; CLR_IRQFIQ();
	rEXTINT1  = (rEXTINT1  & 0xff0fffff) | 0x00600000;	// EINT13[22:20]=11x(Both edge triggered)
//	rEINTFLT3 = (rEINTFLT3 & 0xffffff00) | 0x000000ff;  // EINTFLT20[6:0]=(Filtering width of EINT20)
	mask = BIT_EINT11 | BIT_EINT12 | BIT_EINT13 | BIT_EINT19 | BIT_EINT20;
	if((rEINTMASK & mask) == mask) {
		pISR_EINT8_23 = (unsigned)EINT8_23_ISR;
		rSRCPND = BIT_EINT8_23; rINTPND = BIT_EINT8_23;
		SET_IRQFIQ(); rINTMSK &= ~BIT_EINT8_23; CLR_IRQFIQ();
//cprintf("BIT_EINT8_23 Opened...\n");
	}
	rEINTPEND = BIT_EINT13; rSRCPND = BIT_EINT13; rINTPND = BIT_EINT13;
	SET_IRQFIQ(); rEINTMASK &= ~BIT_EINT13; CLR_IRQFIQ();
	rTCNTB0 = 0;
	rTCON &= 0xffffffe0;	// stop Timer0
	mcc_state = 0;
	em_recv = em_new = 0; em_loop = 0;
	em_error = 0L;
	EM_SHD_H; EM_MOD_H;
	uDelay(2000);
	EM_SHD_L; EM_MOD_L;
}

void emClose(void)
{
	int		mask;

	rEINTMASK |= BIT_EINT13;
	mask = BIT_EINT11 | BIT_EINT12 | BIT_EINT13 | BIT_EINT19 | BIT_EINT20;	
	if((rEINTMASK & mask) == mask) {
		SET_IRQFIQ(); rINTMSK |= BIT_EINT8_23; CLR_IRQFIQ();
cprintf("BIT_EINT8_23 Closed...\n");
	}
	EM_SHD_H;
	EM_MOD_H;
}

int emPeek(void)
{
	if(em_recv) return 1;
	else	return 0;
}

int emReadData(unsigned char *Data)
{
	int		rval;

	if(em_recv) {
		memcpy(Data, em_rxdata+1, 4);
		em_recv = 0;
		if(!Data[0] && !Data[1] && !Data[2] && !Data[3]) rval = 0;
		else	rval = 4;
	} else	rval = 0;
	return rval;
}
