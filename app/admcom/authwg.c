#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "cr.h"
#include "wglib.h"
#include "cf.h"
#include "cfprim.h"
//#include "mod.h"
#include "authwg.h"


void AuthWiegand(CREDENTIAL_READER *cr)
{
	void	*wf, *pf, *wpf;
	unsigned char	data[100], wgdata[100];
	int		len, wo;

	if(cr->data[0] == CRED_PIN) {
		pf = cfPinGet();
		if(len > 0) {
			len = cfPinEncodeData(pf, cr->data, data);
			wgdata[0] = 0; wgdata[1] = 0x20;
			wo = cfPinOutputWiegand(pf);
			wpf = cfWgPinsGet(wo);
			if(cfWgPinParity(wpf)) len = wgEncode(data, len, wgdata+3);
			else	memcpy(wgdata+3, data, (len+7)>>3);
			wgdata[2] = len;
		}
	} else if(cr->data[0] == CRED_WIEGAND) {
		wgdata[0] = 0; wgdata[1] = 0x20;
		len = cr->data[1];
		memcpy(wgdata+3, data, (len+7)>>3);
	} else {
		wf = cfWiegandsGet((int)cr->outputWiegand, 1);
crPrintCred(cr);
		len = cfWiegandEncodeData(wf, cr->data, data);
printf("wgData: %d %02x-%02x-%02x-%02x\n", len, (int)data[0], (int)data[1], (int)data[2], (int)data[3]);
		wgdata[0] = 0; wgdata[1] = 0x20;
		if(cfWiegandParity(wf)) len = wgEncode(data, len, wgdata+3);
		else	memcpy(wgdata+3, data, (len+7)>>3);
		wgdata[2] = len;
	}
//	if(len) modWriteCredBuffer(wgdata, 3 + ((len+7)>>3));
}

void PinCharWiegand(int ch)
{
	void	*pf, *wpf;
	unsigned char	val, data[4], wgdata[4];
	int		wo, len;

	pf = cfPinGet();
	wo = cfPinOutputWiegand(pf);
	wpf = cfWgPinsGet(wo);
	if(ch == '*') data[0] = 10;
	else if(ch == '#') data[0] = 11;
	else	data[0] = ch - '0';
	if(wo == 0) {
		wgdata[0] = 0; wgdata[1] = 0x20; 
		len = 4;
		if(cfWgPinParity(wpf)) len = wgEncode(data, len, wgdata+3);
		wgdata[2] = len;
		memcpy(wgdata+3, data, (len+7)>>3);
	} else {
		wgdata[0] = 0; wgdata[1] = 0x20; wgdata[2] = len = 8;
		val = data[0];
		data[0] |= ~(val << 4 | 0x0f);
		memcpy(wgdata+3, data, (len+7)>>3);
	}
//	modWriteCredBuffer(wgdata, 3 + ((len+7)>>3));
}

void PinStrWiegand(char *str)
{
	void	*pf, *wpf;
	unsigned char	data[4], wgdata[4];
	int		wo, val, len;

	pf = cfPinGet();
	wo = cfPinOutputWiegand(pf);
	wpf = cfWgPinsGet(wo);
	if(wo == 2) { 
		data[0] = cfWgPinFacilityCode(pf);
		val = n_atoi(str);
		IDtoPACK3(val, data+1);
		wgdata[0] = 0; wgdata[1] = 0x20;
		len = wgEncode(data, 24, wgdata+3);
		wgdata[2] = len;
	} else if(wo == 3) {
		len = strlen(str);
		string2bcd(str, len, data);
		len <<= 2;
		wgdata[0] = 0; wgdata[1] = 0x20;
		if(cfWgPinParity(wpf)) len = wgEncode(data, len, wgdata+3);
		else	memcpy(wgdata+3, data, (len+7)>>3);
		wgdata[2] = len;
	} else	len = 0;
//	if(len) modWriteCredBuffer(wgdata, 3 + ((len+7)>>3));
}

