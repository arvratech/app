#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "msg.h"
#include "syscfg.h"
#include "tacfg.h"
#include "sche.h"
#include "evt.h"
#include "func.h"
#include "dev.h"
#include "bio.h"
#include "ad.h"
#include "cf.h"
#include "acuobj.h"


int acuAdDecode(void *buf)
{
	void		*bi, *bo;
	ACCESS_DOOR	*ad;
	unsigned char	*p, val;
	int		n;
	p = (unsigned char *)buf;
	ad = adsGet(0);
	val = *p++;
	if(val & 0x80) {
		adCreate(ad);
		bi = bisGet(0);
		if(val & 0x40) biCreate(bi, BI_TYPE_DOOR);
		else	biDelete(bi);
		bi = bisGet(1);
		if(val & 0x20) biCreate(bi, BI_TYPE_DOOR);
		else	biDelete(bi);
		bo = bosGet(1);
		if(val & 0x10) boCreate(bo, BO_TYPE_DOOR);
		else	boDelete(bo);
		if(val & 0x08) n = 1; else n = 0;
		adSetLockType(ad, 0);
		n = *p++; adSetDoorPulseTime(ad, n);
		n = *p++; adSetDoorExtendedPulseTime(ad, n);
		n = *p++; adSetDoorOpenTooLongTime(ad, n);
		n = *p++; adSetTimeDelay(ad, n);
		n = *p++; adSetTimeDelayNormal(ad, n);
printf("ad(0): %d,%d,%d,%d,%d\n", adDoorPulseTime(ad), adDoorExtendedPulseTime(ad), adTimeDelay(ad), adTimeDelayNormal(ad), adDoorOpenTooLongTime(ad));
	} else {
printf("ad(0) null\n");
		adDelete(ad);
		p += 5;
	}
	return p - (unsigned char *)buf;
}

int acuBioDecode(void *buf)
{
	void	*bi, *bo;
	unsigned char	*p, val, mask;
	int		i, n;

	p = (unsigned char *)buf;
	val = *p++; mask = 0x80;
printf("acuBioDeclode: %02x\n", val);
	for(i = 0;i < 4;i++, mask >>= 1) {
		if(val & mask) n = 1; else n = 0;
		bi = bisGet(i);
		biSetPolarity(bi, n);
	}
	for(i = 0;i < 2;i++, mask >>= 1) {
		if(val & mask) n = 1; else n = 0;
		bo = bosGet(i);
		boSetPolarity(bo, n);
	}
	for(i = 0;i < 4;i++) {
		bi = bisGet(i);
		n = *p++; biSetTimeDelay(bi, n);
		n = *p++; biSetTimeDelayNormal(bi, n);
	}
printf("bi: %d,%d %d,%d %d,%d %d,%d\n", (int)*(p-8), (int)*(p-7), (int)*(p-6), (int)*(p-5), (int)*(p-4), (int)*(p-2), (int)*(p-2), (int)*(p-1));
	return p - (unsigned char *)buf;
}

int acuCrDecodeAll(void *buf)
{
	void	*cr, *cf, *fld;
	unsigned char	*p, val;
	BOOL	bVal, bOrgVal;
	int		n, format;

	p = (unsigned char *)buf;
	*p++;		// type
	val = *p++;
printf("crEnable=0x%02x\n", (int)val);
#ifdef _HYNIX
	val = 0x04;
#endif
#ifndef _HYNIX		// crSetEnableHynixEmvReader
	if(val & 0x01) bVal = TRUE; else bVal = FALSE;
	crSetEnableFpReader(cr, bVal);
#endif
	if(val & 0x02) bVal = TRUE; else bVal = FALSE;
	crSetEnablePinReader(cr, bVal);
	if(val & 0x04) bVal = TRUE; else bVal = FALSE;
	crSetEnableScReader(cr, bVal);
	if(val & 0x08) bVal = TRUE; else bVal = FALSE;
	crSetEnableEmReader(cr, bVal);
	if(val & 0x10) bVal = TRUE; else bVal = FALSE;
	crSetEnableWiegandReader(cr, bVal);
	if(val & 0x20) bVal = TRUE; else bVal = FALSE;
	crSetEnableSerialReader(cr, bVal);
	if(val & 0x40) bVal = TRUE; else bVal = FALSE;
	crSetEnableIpReader(cr, bVal);
	val = *p++;
//printf("crOption=0x%02x.........\n", (int)val);
	bOrgVal = tamperIsEnable(NULL);
	if(val & 0x01) bVal = FALSE; else bVal = TRUE;
	if(bVal != bOrgVal) { 
		tamperSetEnable(NULL, bVal);
		tamperClearPresentValue(NULL);
	}
	bOrgVal = intrusionIsEnable(NULL);
	if(val & 0x02) bVal = TRUE; else bVal = FALSE;
	if(bVal != bOrgVal) { 
		intrusionSetEnable(NULL, bVal);
		intrusionDisarm(NULL);
	}
	// fp
	*p++;	// exposure
	*p++;	// captureMode
	// pin
	*p++;	// reserve
	// mifare
	val = *p++;
#ifdef _HYNIX
	format = 1;
#else
	format = val;
#endif
	cr = crsGet(0);
	crSetScFormat(cr, format);
	cf = cfCardsGet(format);
#ifdef _HYNIX
	cfCardSetCredential(cf, CRED_INT64);
#endif
	val = *p++;
	if(format == 0) n = 0;
	else if(format == 1) n = 0; 	
	else	n = val;
	cfCardSetCardType(cf, n);
	if(format == 1) {
#ifdef _HYNIX
		n = 1;		// diversified(Hynix)
#else
		n = 0;		// plain
#endif
		cfCardSetCardApp(cf, n);
	} else if(format == 2) {
		cfCardSetCardApp(cf, 1);	// travel card
	}
printf("scReader=%d scFormat=%d cardApp=%d\n", crIsEnableScReader(cr), crScFormat(cr), cfCardCardApp(cf));
	n = *p++;
#ifdef _HYNIX
	n = 48;
#endif
	cfCardSetBlockNoSingle(cf, n);
printf("BlockNo=%d\n", n);
	fld = cfCardCardFieldAtIndex(cf, 0);
	n = *p++;
	if(format == 1) cardFieldSetOffset(fld, n);
	n = *p++;
	if(format == 1) cardFieldSetLength(fld, n);
	val = *p++;
	if(val == 0) n = CF_CODING_BCD;
	else if(val == 1) n = CF_CODING_ASCII;
	else	n = CF_CODING_BINARY;
#ifdef _HYNIX
	cardFieldSetOffset(fld, 0);
	cardFieldSetLength(fld, 8);
	n = CF_CODING_BINARY;
#endif
	if(format == 1) cfCardSetCoding(cf, n);
	// em
	val = *p++;
	cf = cfCardsGet(CFCARD_EM);
	fld = cfCardCardFieldAtIndex(cf, 0);
	//cardFieldSetType(fld, x);
	if(val == 1) n = 2;
	else	n = 1;
	cardFieldSetOffset(fld, n);
	if(val == 1) n = 3;
	else	n = 4;
	cardFieldSetLength(fld, n);
	// wiegand
	*p++;	// Format26Bit
	*p++;	// ByteOrder
	// serial
	cf = cfCardsGet(CFCARD_SERIAL);
	val = *p++;
	cfCardSetCardType(cf, (int)val);
	*p++;		// DataBitLength
	val = *p++;
	if(val == 0) n = CF_CODING_BCD;
	else if(val == 1) n = CF_CODING_ASCII;
	else	n = CF_CODING_BINARY;
	cfCardSetCoding(cf, n);
	// ip
	cf = cfCardsGet(CFCARD_IP);
	*p++;		// Format
	*p++;		// DataBitLength
	*p++;		// DataType	
	p += 4;		// ReaderIPAddress
	p += 2;		// ReaderPort
 	return p - (unsigned char *)buf;
}

int acuCrDecodeScKey(void *buf)
{
	void	*cf;
	unsigned char	*p;

	p = (unsigned char *)buf;
	cf = cfCardsAt(1);
	cfCardSetMifareKey(cf, p);
	p += 13;
 	return p - (unsigned char *)buf;
}

int acuTaDecode(void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
//	syscfg->TAOption		= *s++;
//	syscfg->FuncKeyTimer	= *s++;
//	p += taDecodeItems(syscfg, p);
 	return p - (unsigned char *)buf;
}

