#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NSEnum.h"
#include "prim.h"
#include "syscfg.h"
#include "cf.h"


void _CfFpSetDefault(CF_FP_CFG *cfg)
{
	cfg->exposure		= 5;	// 3
	cfg->captureMode	= 0;
}

int _CfFpEncode(CF_FP_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cfg->exposure;
	*p++ = cfg->captureMode;
	return p - (unsigned char *)buf;
}

int _CfFpDecode(CF_FP_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cfg->exposure	= *p++;
	cfg->captureMode	= *p++;
	return p - (unsigned char *)buf;
}

void _CfPinSetDefault(CF_PIN_CFG *cfg)
{
	cfg->outputWiegand		= 0;
	cfg->entryInterval		= 7;
	cfg->entryStopSentinel	= 0;
	cfg->maxLength			= 4;
}

int _CfPinEncode(CF_PIN_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cfg->outputWiegand;
	*p++ = cfg->entryInterval;
	*p++ = cfg->entryStopSentinel;
	*p++ = cfg->maxLength;
	return p - (unsigned char *)buf;
}

int _CfPinDecode(CF_PIN_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cfg->outputWiegand		= *p++;
	cfg->entryInterval		= *p++;
	cfg->entryStopSentinel	= *p++;
	cfg->maxLength			= *p++;
	return p - (unsigned char *)buf;
}

void _CardFieldSetDefault(CARD_FIELD *fld)
{
	fld->fieldType	= 0xff;
	fld->offset		= 0xff;
	fld->length		= 0xff;
}

static int _CardFieldEncode(CARD_FIELD *fld, void *buf)
{
	unsigned char	*p;
	int		i;
	
	p = (unsigned char *)buf;
	*p++ = fld->fieldType;
	*p++ = fld->offset;
	*p++ = fld->length;
	return p - (unsigned char *)buf;
}

static int _CardFieldDecode(CARD_FIELD *fld, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	fld->fieldType	= *p++;
	fld->offset		= *p++;
	fld->length		= *p++;
	return p - (unsigned char *)buf;
}

void _UidFormatCardFieldsSetDefault(CF_CARD_CFG *cfg)
{
	CARD_FIELD	*fld;
	int		i, val, cred;

	cred = cfg->credential;
	i = 0; fld = cfg->cardFields;
	fld->fieldType = 0; fld->offset = 0;
	if(cred == CRED_INT32) val = 4;
	else if(cfg->cardType == 1) val = 8;
	else	val = 7;
	fld->length = val;
	i++; fld++;
	for( ;i < MAX_CARD_FIELD_SZ;i++, fld++) _CardFieldSetDefault(fld);  
}

void _EmFormatCardFieldsSetDefault(CF_CARD_CFG *cfg)
{
	CARD_FIELD	*fld;
	int		i, val, cred;

	cred = cfg->credential;
	i = 0; fld = cfg->cardFields;
	if(cred == CRED_FC32_CN32) val = 1; else val = 0;
	fld->fieldType = val;
	fld->offset = 0;
	switch(cred) {
	case CRED_INT32:	 fld->offset = 1; fld->length = 4; break;
	case CRED_INT64:	 fld->offset = 0; fld->length = 5; break;
	case CRED_FC32_CN32: fld->offset = 1; fld->length = 2; break;
	}
	i++; fld++;
	if(cred == CRED_FC32_CN32) {
		fld->fieldType = 0; fld->offset = 0; fld->length = 2;
		i++; fld++;
	}
	for( ;i < MAX_CARD_FIELD_SZ;i++, fld++) _CardFieldSetDefault(fld);  
}

void _CfCardSetDefault(CF_CARD_CFG *cfg, int id, int cred)
{
	CARD_FIELD	*fld;
	int		i, val;

	cfg->credential		= cred;
	cfg->cardType		= 0;
#ifdef _HYNIX
	if(id == CFCARD_SC_BLOCK) val = 1;
	else	val = 0;
#else
	val = 0;
#endif
	cfg->cardApp		= val;
#ifdef _HYNIX
	val = 0xff;
#else
	if(cred == CRED_INT32) val = 2;
	else if(cred == CRED_INT64)	val = 3;
	else	val	= 0xff;
val = 0xff;
#endif
	cfg->outputWiegand	= val;
	cfg->coding			= CF_CODING_BINARY;
	memset(cfg->data, 0xff, 12);
	if(id == CFCARD_SC_UID) {
		_UidFormatCardFieldsSetDefault(cfg);
	} else if(id == CFCARD_EM) {
		_EmFormatCardFieldsSetDefault(cfg);
	} else {
//printf("cfSetDefault: cfId=%d cred=%d\n", id, (int)cf->credential);
		if(cred == CRED_BCD_DIGITS) val = CF_CODING_BCD;
		else if(cred == CRED_CHARACTER_STRING) val = CF_CODING_ASCII;
		else	val = CF_CODING_BINARY;
		cfg->coding = val;
		i = 0; fld = cfg->cardFields;
		if(cred == CRED_FC32_CN32) {
			fld->fieldType = 1; fld->offset = 0; fld->length = 2;
			i++; fld++;
		}
		fld->fieldType = 0; fld->offset = 0;
		if(cred == CRED_INT64) val = 8;
		else if(cred == CRED_INT32 || cred == CRED_FC32_CN32) val = 4;
		else	val = 2;
		fld->length = val;
		i++; fld++;
		for( ;i < MAX_CARD_FIELD_SZ;i++, fld++) _CardFieldSetDefault(fld);  
	}
	switch(id) {
	case CFCARD_SC_BLOCK:
#ifdef _HYNIX
		val = 48;
#else
		val = 4;	// 0
#endif
		cfg->data[6] = val; 	// blockNumber
		break;
	case CFCARD_SC_FILE:
		cfg->data[0] = 0;		// aidLength
		break;
	case CFCARD_SERIAL:
		cfg->data[0] = 0x20;	// 9600bps, non parity, 1 stop bit
		break;
	}
}

int _CfCardEncode(CF_CARD_CFG *cfg, int id, void *buf)
{
	CARD_FIELD	*fld;
	unsigned char	*p;
	int		i, n;

	p = (unsigned char *)buf;
	if(id == CFCARD_SC_UID) {
		*p++ = cfg->credential;
		*p++ = cfg->cardType;
		*p++ = cfg->outputWiegand;
	} else if(id == CFCARD_EM) {
		*p++ = cfg->credential;
		*p++ = cfg->outputWiegand;
	} else {
		*p++ = cfg->credential;
		*p++ = cfg->cardType;
		*p++ = cfg->cardApp;
		*p++ = cfg->outputWiegand;
		*p++ = cfg->coding;
//printf("Encode(%d): cred=%d cardType=%d cardApp=%d\n", id, (int)cfg->credential, (int)cfg->cardType,  (int)cfg->cardApp); 
		for(i = 0, fld = cfg->cardFields;i < MAX_CARD_FIELD_SZ;i++, fld++)
			p += _CardFieldEncode(fld, p);
		switch(id) {
		case CFCARD_SC_BLOCK:   n =  7; break;
		case CFCARD_SC_FILE: n = 11; break;
		case CFCARD_SERIAL:  n =  1; break;
		default: n = 0;
		}
		if(n) {
			memcpy(p, cfg->data, n); p += n;
		}
	}
	return p - (unsigned char *)buf;
}

int _CfCardDecode(CF_CARD_CFG *cfg, int id, void *buf)
{
	CARD_FIELD	*fld;
	unsigned char	*p;
	int		i, n;

	p = (unsigned char *)buf;
	if(id == CFCARD_SC_UID) {
		cfg->credential		= *p++;
		cfg->cardType		= *p++;
		cfg->outputWiegand	= *p++;
	} else if(id == CFCARD_EM) {
		cfg->credential		= *p++;
		cfg->outputWiegand	= *p++;
	} else {
		cfg->credential		= *p++;
		cfg->cardType		= *p++;
		cfg->cardApp		= *p++;
		cfg->outputWiegand	= *p++;
		cfg->coding			= *p++;
//printf("Decode(%d): cred=%d cardType=%d cardApp=%d\n", id, (int)cfg->credential, (int)cfg->cardType,  (int)cfg->cardApp); 
		for(i = 0, fld = cfg->cardFields;i < MAX_CARD_FIELD_SZ;i++, fld++)
			p += _CardFieldDecode(fld, p);
		switch(id) {
		case CFCARD_SC_BLOCK:   n =  7; break;
		case CFCARD_SC_FILE: n = 11; break;
		case CFCARD_SERIAL:  n =  1; break;
		default: n = 0;
		}
		if(n) {
			memcpy(cfg->data, p, n); p += n;
		}
	}	
	return p - (unsigned char *)buf;
}

void _CfWiegandSetDefault(CF_WIEGAND_CFG *cfg, int id, int cred)
{
	CARD_FIELD	*fld;
	int		i, val;

	i = 0; fld = cfg->cardFields;
	switch(id) {
	case 0:		// Standard 26-bit format - H10301
		cfg->credential	= CRED_FC32_CN32;
		cfg->outputWiegand	= 0xff;
		cfg->coding		= CF_CODING_BINARY;
		cfg->parity		= CF_EVEN_ODD_PARITY;
		fld->fieldType = 1; fld->offset = 0; fld->length =  8;
		i++; fld++;
		fld->fieldType = 0; fld->offset = 0; fld->length = 16;
		i++; fld++;
		break;
	case 1:		// 35-bit HID Coporate 1000
		cfg->credential	= CRED_FC32_CN32;
		cfg->outputWiegand	= 0xff;
		cfg->coding		= CF_CODING_BINARY;
		cfg->parity		= CF_EVEN_ODD_PARITY;
		fld->fieldType = 1; fld->offset = 0; fld->length = 12;
		i++; fld++;
		fld->fieldType = 0; fld->offset = 0; fld->length = 20;
		i++; fld++;
		break;
	case 2:		// 34-bit format
		cfg->credential	= CRED_INT32;
		cfg->outputWiegand	= 2;
		cfg->coding		= CF_CODING_BINARY;
		cfg->parity		= CF_EVEN_ODD_PARITY;
		fld->fieldType = 0; fld->offset = 0; fld->length = 32;
		i++; fld++;
		break;
	case 3:		// 66-bit format
		cfg->credential	= CRED_INT64;
		cfg->outputWiegand	= 3;
		cfg->coding		= CF_CODING_BINARY;
		cfg->parity		= CF_EVEN_ODD_PARITY;
		fld->fieldType = 0; fld->offset = 0; fld->length = 64;
		i++; fld++;
		break;
/*
	case 2:		// HID 37-bit format - H10302
		cfg->credential	= CRED_INT64;
		cfg->parity		= CF_EVEN_ODD_PARITY;
		cfg->coding		= CF_CODING_BINARY;
		cfg->outputWiegand	= 3;
		fld->fieldType = 0; fld->offset = 0; fld->length = 35;
		i++; fld++;
		break;
	case 3:		// HID 37-bit format with facility - H10304
		cfg->credential	= CRED_FC32_CN32;
		cfg->parity		= CF_EVEN_ODD_PARITY;
		cfg->coding		= CF_CODING_BINARY;
		cfg->outputWiegand	= 0xff;
		fld->fieldType = 1; fld->offset = 0; fld->length = 16;
		i++; fld++;
		fld->fieldType = 0; fld->offset = 0; fld->length = 19;
		i++; fld++;
		break;
*/
	default:
		if(cred < 0) {
			cfg->credential	= 0xff; 
			cfg->outputWiegand	= 0xff;
			cfg->coding		= 0xff;
			cfg->parity 	= 0xff;
		} else {
			cfg->credential	= cred;
			if(cred == CRED_INT64) val = 3;
			else if(cred == CRED_INT32) val = 2;
			else	val = 0xff;
			cfg->outputWiegand	= val;
			if(cred == CRED_BCD_DIGITS) val = CF_CODING_BCD;
			else if(cred == CRED_CHARACTER_STRING) val = CF_CODING_ASCII;
			else	val = CF_CODING_BINARY;
			cfg->coding		= val;
			cfg->parity 	= CF_NONE_PARITY;
			if(cred == CRED_FC32_CN32) {
				fld->fieldType = 1; fld->offset = 0; fld->length = 16;
				i++; fld++;
			}
			fld->fieldType = 0; fld->offset = 0;
			if(cred == CRED_INT64) val = 64;
			else if(cred == CRED_INT32 || cred == CRED_FC32_CN32) val = 32;
			else	val = 16;
			fld->length = val;
			i++; fld++;
		}
		cfg->padding	= 0;
	}
	for( ;i < MAX_CARD_FIELD_SZ;i++, fld++) _CardFieldSetDefault(fld);  
}

int _CfWiegandEncode(CF_WIEGAND_CFG *cfg, void *buf)
{
	CARD_FIELD	*fld;
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	*p++ = cfg->credential;
	*p++ = cfg->outputWiegand;
	*p++ = cfg->coding;
	*p++ = cfg->parity;
	*p++ = cfg->padding;
	for(i = 0, fld = cfg->cardFields;i < MAX_CARD_FIELD_SZ;i++, fld++)
		p += _CardFieldEncode(fld, p);
	return p - (unsigned char *)buf;
}

int _CfWiegandDecode(CF_WIEGAND_CFG *cfg, void *buf)
{
	CARD_FIELD	*fld;
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	cfg->credential	= *p++;
	cfg->outputWiegand	= *p++;
	cfg->coding		= *p++;
	cfg->parity		= *p++;
	cfg->padding	= *p++;
	for(i = 0, fld = cfg->cardFields;i < MAX_CARD_FIELD_SZ;i++, fld++)
		p += _CardFieldDecode(fld, p);
	return p - (unsigned char *)buf;
}

void _CfWiegandPinSetDefault(CF_WIEGAND_PIN_CFG *cfg, int id)
{
	switch(id) {
	case 0:
		cfg->parity	 = 0;
		cfg->data[0] = 0;
		break;
	case 1:
		cfg->parity	 = 0;
		cfg->data[0] = 0;
		break;
	case 2:
		cfg->parity	 = 1;
		cfg->data[0] = 0;	// facility code
		break;
	case 3:
		cfg->parity	 = 0;
		cfg->data[0] = 0;
		break;
	}	
}

int _CfWiegandPinEncode(CF_WIEGAND_PIN_CFG *cfg, void *buf)
{
	CARD_FIELD	*fld;
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	*p++ = cfg->parity;
	*p++ = cfg->data[0];
	return p - (unsigned char *)buf;
}

int _CfWiegandPinDecode(CF_WIEGAND_PIN_CFG *cfg, void *buf)
{
	CARD_FIELD	*fld;
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	cfg->parity		= *p++;
	cfg->data[0]	= *p++;
	return p - (unsigned char *)buf;
}

void _CfCardSetDefaultSerialBarCode(CF_CARD_CFG *cfg)
{
	CARD_FIELD	*fld;
	int		i;

	cfg->credential		= CRED_BCD_DIGITS;
	cfg->cardType		= 1;
	cfg->cardApp		= 0;
	cfg->outputWiegand	= 0xff;
	cfg->coding			= CF_CODING_ASCII;
	memset(cfg->data, 0xff, 12);
	i = 0; fld = cfg->cardFields;
	fld->fieldType = 0; fld->offset = 0;
	fld->length = 0;
	i++; fld++;
	for( ;i < MAX_CARD_FIELD_SZ;i++, fld++) _CardFieldSetDefault(fld);  
	cfg->data[0] = 0x20;	// 9600bps, non parity, 1 stop bit
}

