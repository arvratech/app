#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NSEnum.h"
#include "prim.h"
#include "syscfg.h"
#include "cr.h"
#include "cf.h"

static CF_FP		_FpFormat;
static CF_PIN		_PinFormat;
static CF_CARD		_CardFormats[MAX_CF_CARD_SZ];
static CF_WIEGAND	_WiegandFormats[MAX_CF_WIEGAND_SZ];
static CF_WIEGAND	_WiegandOutputFormats[MAX_CF_WIEGAND_SZ];
static CF_WIEGAND_PIN	_WiegandPinOutputFormats[MAX_CF_WIEGAND_PIN_SZ];


void cfsInit(void)
{
	CF_FP		*ff;
	CF_PIN		*pf;
	CF_CARD		*cf;
	CF_WIEGAND	*wf;
	CF_WIEGAND_PIN	*wpf;
	int		i;

	ff = &_FpFormat;
	ff->id = 0;
	ff->cfg = &sys_cfg->fpFormat;
	pf = &_PinFormat;
	pf->id = 0;
	pf->cfg = &sys_cfg->pinFormat;
	for(i = 0, cf = _CardFormats;i < MAX_CF_CARD_SZ;i++, cf++) {
		cf->id			= i;
		cf->cfg			= &sys_cfg->cardFormats[i];
	}
	for(i = 0, wf = _WiegandFormats;i < MAX_CF_WIEGAND_SZ;i++, wf++) {
		wf->id			= i;
		wf->type		= 0;
		wf->ctx			= NULL;
		wf->cfg			= &sys_cfg->wiegandFormats[i];
	}
	for(i = 0, wf = _WiegandOutputFormats;i < MAX_CF_WIEGAND_SZ;i++, wf++) {
		wf->id			= i;
		wf->type		= 1;
		wf->ctx			= NULL;
		wf->cfg			= &sys_cfg->wiegandOutputFormats[i];
	}
	for(i = 0, wpf = _WiegandPinOutputFormats;i < MAX_CF_WIEGAND_PIN_SZ;i++, wpf++) {
		wpf->id			= i;
		wf->ctx			= NULL;
		wpf->cfg		= &sys_cfg->wiegandPinOutputFormats[i];
	}
}

// Credential Format FP Object

void *cfFpGet(void)
{
	return (void *)&_FpFormat;
}

void cfFpCopy(void *d, void *s)
{
	CF_FP *df, *sf;

	df = d; sf = s;
	if(sf->cfg) memcpy(df->cfg, sf->cfg, sizeof(CF_FP_CFG));
	df->id = sf->id;
}

// Credential Format PIN Object

void *cfPinGet(void)
{
	return (void *)&_PinFormat;
}

void cfPinCopy(void *d, void *s)
{
	CF_PIN *df, *sf;

	df = d; sf = s;
	if(sf->cfg) memcpy(df->cfg, sf->cfg, sizeof(CF_PIN_CFG));
	df->id = sf->id;
}

int cfPinOutputWiegand(void *self)
{
	CF_PIN	*pf = self;
	int		val;

	if(pf->cfg->outputWiegand == 0xff) val = -1;
	else	val = pf->cfg->outputWiegand;
	return val;
}

void cfPinSetOutputWiegand(void *self, int outputWiegand)
{
	CF_PIN	*pf = self;

	if(outputWiegand < 0) pf->cfg->outputWiegand = 0xff;
	else	pf->cfg->outputWiegand = outputWiegand;
}

int cfPinEntryInterval(void *self)
{
	CF_PIN	*cf = self;

	return (int)cf->cfg->entryInterval;
}

void cfPinSetEntryInterval(void *self, int interval)
{
	CF_PIN	*cf = self;

	cf->cfg->entryInterval = interval;
}

int cfPinEntryStopSentinel(void *self)
{
	CF_PIN	*cf = self;

	return (int)cf->cfg->entryStopSentinel;
}

void cfPinSetEntryStopSentinel(void *self, int c)
{
	CF_PIN	*cf = self;

	cf->cfg->entryStopSentinel = c;
}

int cfPinMaximumLength(void *self)
{
	CF_PIN	*cf = self;

	return (int)cf->cfg->maxLength;
}

void cfPinSetMaximumLength(void *self, int length)
{
	CF_PIN	*cf = self;

	cf->cfg->maxLength = length;
}

int cfPinEncodeData(void *self, unsigned char *credData, unsigned char *wgData)
{
	CF_PIN	*pf = self;
	CF_WIEGAND_PIN	*wpf;
	unsigned char	*s;
	int		wo, val, len;

	s = credData;
	val = *s++; len = *s++;
	if(val != CRED_PIN) return -1;
	wo = cfPinOutputWiegand(pf);
	wpf = cfWgPinsGet(wo);
	if(wo == 2) { 
		wgData[0] = cfWgPinFacilityCode(wpf);
		val = bcd2int(s, len);
		IDtoPACK3(val, wgData+1);
		len = 24;
	} else if(wo == 3) {
		len <<= 2;
		memcpy(wgData, s, (len+7)>>3);
	} else	len = 0;
	return len;
}

void cfPinValidate(void *self)
{
	CF_PIN	*pf = self;
	int		val;

	cfPinValidateOutputWiegandFormat(pf);
	val = cfPinOutputWiegand(pf);
	if(val == 2 && cfPinMaximumLength(pf) > 5) cfPinSetMaximumLength(pf, 5);
}

int cfPinValidateOutputWiegandFormat(void *self)
{
	CF_PIN	*pf = self;
	int		val, rval;

	val = cfPinOutputWiegand(pf);
	if(val >= MAX_CF_WIEGAND_PIN_SZ || cfPinEntryStopSentinel(pf) && val == MAX_CF_WIEGAND_PIN_SZ-1) {
		cfPinSetOutputWiegand(pf, 0); rval = 1;
	} else	rval = 0;
//printf("cfPinValidateOutputWiegandFormat: return=%d wo=%d\n", rval, cfPinOutputWiegand(pf));
	return rval;
}

// Credential Format Card List Object
int cfId(void *self)
{
	CF_CARD	*cf = self;

	return (int)cf->id;
}

void *cfCardsGet(int id)
{
	CF_CARD *cf;

	if(id >= 0 && id < MAX_CF_CARD_SZ) cf = &_CardFormats[id];
	else	cf = NULL;
	return (void *)cf; 
}

void *cfCardsAt(int index)
{
	CF_CARD *cf;

	if(index >= 0 && index < MAX_CF_CARD_SZ) cf = &_CardFormats[index];
	else	cf = NULL;
	return (void *)cf; 
}

// Credential Format Wiegand List Object

void *cfWiegandsGet(int id, int type)
{
	CF_WIEGAND	*cf;

	if(id >= 0 && id < MAX_CF_WIEGAND_SZ) {
		if(type) cf = &_WiegandOutputFormats[id];
		else	 cf = &_WiegandFormats[id];
		if(cf->cfg->credential == 0xff) cf = NULL;
	} else	cf = NULL;
	return (void *)cf;
}

void *cfWiegandsAt(int index, int type)
{
	CF_WIEGAND	*cf;
	int		i, count;

	if(type) cf = _WiegandOutputFormats;
	else	 cf = _WiegandFormats;
	for(i = count = 0;i < MAX_CF_WIEGAND_SZ;i++, cf++)
		if(cf->cfg->credential != 0xff)  {
			if(count == index) break;
			count++;
		}
	if(i >= MAX_CF_WIEGAND_SZ) cf = NULL;
	return (void *)cf;
}

int cfWiegandsIndexFor(int id, int type)
{
	CF_WIEGAND	*cf;
	int		i, index;

	if(type) cf = _WiegandOutputFormats;
	else	 cf = _WiegandFormats;
	for(i = index = 0;i < MAX_CF_WIEGAND_SZ;i++, cf++)
		if(cf->cfg->credential != 0xff)  {
			if(cf->id == id) break;
			index++;
		}
	if(i >= MAX_CF_WIEGAND_SZ) index = -1;
	return index;
}

void *cfWiegandsAdd(void *self, int type)
{
	CF_WIEGAND	*cf = self;
	CF_WIEGAND	*cfa;
	int		i;

	if(type) cfa = _WiegandOutputFormats;
	else	 cfa = _WiegandFormats;
	for(i = 0;i < MAX_CF_WIEGAND_SZ;i++, cfa++)
		if(cfa->cfg->credential == 0xff) break;
	if(i < MAX_CF_WIEGAND_SZ) {
		memcpy(cfa->cfg, cf->cfg, sizeof(CF_WIEGAND_CFG));
	} else	cfa = NULL;
	return cfa;
}

void cfWiegandsRemove(int id, int type)
{
	if(id > 3 && id < MAX_CF_WIEGAND_SZ)
		cfWiegandSetDefault(cfWiegandsGet(id, type), -1);
}

void cfWiegandsRemoveAt(int index, int type)
{
	CF_WIEGAND	*cf;
	int		i, count;

	if(type) cf = _WiegandOutputFormats;
	else	 cf = _WiegandFormats;
	for(i = count = 0;i < MAX_CF_WIEGAND_SZ;i++, cf++)
		if(cf->cfg->credential != 0xff)  {
			if(count == index) break;
			count++;
		}
	if(i < MAX_CF_WIEGAND_SZ) cfWiegandSetDefault(cf, -1);
}

/*
void cfWiegandsRemove(int id, int type)
{
	CF_WIEGAND	*cf;
	int		i;

	if(type) cf = &sys_cfg->wiegandOutFormats[id];
	else	 cf = &sys_cfg->wiegandFormats[id];
	i = id;
	for( ;i < MAX_CF_WIEGAND_SZ-1;i++, cf++) {
		if(cf->credential == 0xff) break;
		*cf = *(cf+1);
	}
	if(i >= MAX_CF_WIEGAND_SZ-1) cfWiegandSetDefault(cf, -1);
}
*/

int cfWiegandsCount(int type)
{
	CF_WIEGAND	*cf;
	int		i, count;

	if(type) cf = _WiegandOutputFormats;
	else	 cf = _WiegandFormats;
	for(i = count = 0;i < MAX_CF_WIEGAND_SZ;i++, cf++)
		if(cf->cfg->credential != 0xff) count++;
	return count;
}

// Credential Format PIN Wiegand List Object

void *cfWgPinsGet(int id)
{
	CF_WIEGAND_PIN *cf;

	if(id >= 0 && id < MAX_CF_WIEGAND_PIN_SZ) cf = &_WiegandPinOutputFormats[id];
	else	cf = NULL;
	return (void *)cf; 
}

// Credential Format Card Object

void cfCardSetDefault(void *self, int cred)
{
	CF_CARD	*cf = self;

	_CfCardSetDefault(cf->cfg, (int)cf->id, cred);
}

void cfCardSetDefaultSeialBarCode(void *self)
{
	CF_CARD	*cf = self;

	_CfCardSetDefaultSerialBarCode(cf->cfg);
}

void cfCardCopy(void *d, void *s)
{
	CF_CARD *df, *sf;

	df = d; sf = s;
	if(sf->cfg) memcpy(df->cfg, sf->cfg, sizeof(CF_CARD_CFG));
	df->id = sf->id;
}

int cfCardId(void *self)
{
	CF_CARD	*cf = self;

	return (int)cf->id;
}

int cfCardCredential(void *self)
{
	CF_CARD	*cf = self;

	return (int)cf->cfg->credential;
}

void cfCardSetCredential(void *self, int credential)
{
	CF_CARD	*cf = self;

	cf->cfg->credential = credential;
}

int cfCardGetAvailableCredentials(void *self, int *arr)
{
	CF_CARD		*cf = self;
	int		i, count;

	if(cf->id == CFCARD_SC_UID) count = 2;
	else if(cf->id == CFCARD_EM) count = 3;
	else	count = MAX_CREDENTIAL_SZ;
	for(i = 0;i < count;i++) {
		arr[i] = i;
	}
	return count;
}

int cfCardCardType(void *self)
{
	CF_CARD	*cf = self;

	return (int)cf->cfg->cardType;
}

void cfCardSetCardType(void *self, int cardType)
{
	CF_CARD	*cf = self;

	cf->cfg->cardType = cardType;
}

int cfCardCardApp(void *self)
{
	CF_CARD	*cf = self;

	return (int)cf->cfg->cardApp;
}

void cfCardSetCardApp(void *self, int cardApp)
{
	CF_CARD	*cf = self;

	cf->cfg->cardApp = cardApp;
}

int cfCardOutputWiegand(void *self)
{
	CF_CARD	*cf = self;

	return (int)cf->cfg->outputWiegand;
}

void cfCardSetOutputWiegand(void *self, int wfId)
{
	CF_CARD	*cf = self;

	cf->cfg->outputWiegand = wfId;
}

int cfCardGetAvailableOutputWiegands(void *self, int *arr)
{ 
	CF_CARD		*cf = self;
	CF_WIEGAND_CFG	*wf;
	int		i, cred, count;

	cred = cf->cfg->credential;
	for(i = count = 0, wf = sys_cfg->wiegandOutputFormats;i < MAX_CF_WIEGAND_SZ;i++, wf++)
		if(wf->credential == cred) {
			arr[count] = i; count++;
		}
	return count;
}

int cfCardCoding(void *self)
{
	CF_CARD	*cf = self;

	return (int)cf->cfg->coding;
}

void cfCardSetCoding(void *self, int coding)
{
	CF_CARD	*cf = self;

	cf->cfg->coding = coding;
}

int cfCardGetAvailableCodings(void *self, int *arr)
{
	CF_CARD		*cf = self;
	int		i, cred, min, max, count;

	cred = cfCardCredential(cf);
	if(cred == CRED_CHARACTER_STRING) {
		min = 2; max = 3;
	} else if(cred == CRED_BCD_DIGITS) { 
		min = 1; max = 3;
	} else {
		min = 0; max = 1;
	}
	for(i = min;i < max;i++) {
		count = 0;
		for(i = min;i < max;i++, count++) arr[count] = i;
	}
	return count;
}

void *cfCardCardFieldAtIndex(void *self, int index)
{
	CF_CARD		*cf = self;
	void	*fld;

	if(index >= 0 && index < MAX_CARD_FIELD_SZ) fld = &cf->cfg->cardFields[index];
	else	fld = NULL;
	return fld;
} 

int cfCardMaximumCardFieldOffset(void *self, int fldId)
{
	return 4;
}

int cfCardMinimumCardFieldLength(void *self, int fldId)
{
	CF_CARD	*cf = self;
	int		cred, min;

	cred = cf->cfg->credential;
	if(cred == CRED_INT64) min = 5;
	else if(cf->id == CFCARD_SERIAL && cfCardCardType(cf) == 1) min = 0;
	else	min = 1;
	return min;
}

int cfCardMaximumCardFieldLength(void *self, int fldId)
{
	CF_CARD	*cf = self;
	int		cred, max;

	cred = cf->cfg->credential;
	if(cred == CRED_INT64) max = 8;
	else if(cred == CRED_INT32 || cred == CRED_FC32_CN32) max = 4;
	else	max = 32;
	return max;
}

int cfCardSerialSpeed(void *self)
{
	CF_CARD	*cf = self;

	return (int)((cf->cfg->data[0] >> 5) & 0x07);
}

void cfCardSetSerialSpeed(void *self, int speed)
{
	CF_CARD	*cf = self;

	cf->cfg->data[0] = (cf->cfg->data[0] & 0x1f) | (speed << 5);
}

int cfCardSerialParity(void *self)
{
	CF_CARD	*cf = self;

	return (int)((cf->cfg->data[0] >> 3) & 0x03);
}

void cfCardSetSerialParity(void *self, int parity)
{
	CF_CARD	*cf = self;

	cf->cfg->data[0] = (cf->cfg->data[0] & 0xe7) | (parity << 3);
}

int cfCardSerialStopBits(void *self)
{
	CF_CARD	*cf = self;

	return (int)((cf->cfg->data[0] >> 2) & 0x01);
}

void cfCardSetSerialStopBits(void *self, int stopBits)
{
	CF_CARD	*cf = self;

	if(stopBits) cf->cfg->data[0] |= 0x04;
	else		 cf->cfg->data[0] &= 0xfb;
}

unsigned char *cfCardMifareKey(void *self)
{
	CF_CARD	*cf = self;

	return cf->cfg->data;
}

void cfCardSetMifareKey(void *self, unsigned char *mifareKey)
{
	CF_CARD	*cf = self;

	memcpy(cf->cfg->data, mifareKey, 6);
}

void cfCardGetBlockNo(void *self, int *values)
{
	CF_CARD	*cf = self;
	int		val;

	val = cf->cfg->data[6];
	values[0] = val >> 2;
	values[1] = val & 0x03;
}

void cfCardSetBlockNo(void *self, int *values)
{
	CF_CARD	*cf = self;

	cf->cfg->data[6] = (values[0] << 2) + values[1];
}

int cfCardBlockNoSingle(void *self)
{
	CF_CARD	*cf = self;

	return (int)cf->cfg->data[6];
}

void cfCardSetBlockNoSingle(void *self, int value)
{
	CF_CARD	*cf = self;

	cf->cfg->data[6] = value;
}

unsigned char *cfCardGetAid(void *self, int *aidLength)
{
	CF_CARD	*cf = self;

	*aidLength = cf->cfg->data[0];
	return cf->cfg->data + 1;
}

void cfCardSetAid(void *self, unsigned char *aid, int aidLength)
{
	CF_CARD	*cf = self;

	cf->cfg->data[0] = aidLength;
	memcpy(cf->cfg->data+1, aid, aidLength);
}

void cfCardValidate(void *self)
{
	CF_CARD	*cf = self;
	CARD_FIELD	*fld;
	int		i, val, cred;

	cred = cf->cfg->credential;
	switch(cf->id) {
	case CFCARD_SC_UID:
		if(cred > CRED_INT64) cfCardSetDefault(cf, CRED_INT32);
		else {
			val = cf->cfg->cardType;
			if(cf->cfg->credential == CRED_INT32) val = 0; 
			else if(val > 1) val = 1;
			cf->cfg->cardType = val;
			cf->cfg->cardApp = 0;
			cfCardValidateOutputWiegandFormat(cf);
			cf->cfg->coding = CF_CODING_BINARY;
			_UidFormatCardFieldsSetDefault(cf->cfg);
		}
		break;
	case CFCARD_EM:
		if(cred > CRED_FC32_CN32) cfCardSetDefault(cf, CRED_INT32);
		else {
			cf->cfg->cardType = 0;
			cf->cfg->cardApp	 = 0;
			cfCardValidateOutputWiegandFormat(cf);
			cf->cfg->coding	 = CF_CODING_BINARY;
			 _EmFormatCardFieldsSetDefault(cf->cfg);
		}
		 break;
	default:
		if(cred > CRED_CHARACTER_STRING) cfCardSetDefault(cf, CRED_INT32);
		else {
			val = cf->cfg->cardType;
//printf("1 cfId=%d cardType=%d\n", (int)cf->id, val);
			if(cf->id == CFCARD_SC_BLOCK && val > 1) val = 0;
			else if(cf->id != CFCARD_SC_BLOCK && val > 2) val = 0;
			cf->cfg->cardType = val;
//printf("2 cfId=%d cardType=%d\n", (int)cf->id, val);
			val = cf->cfg->cardApp;
			if(cf->id == CFCARD_SC_FILE && val > 2) val = 0;
			else if(cf->id != CFCARD_SC_FILE && val > 1) val = 0;
			cf->cfg->cardApp = val;
			cfCardValidateOutputWiegandFormat(cf);
			val = cf->cfg->coding;
			if(cred == CRED_BCD_DIGITS) {
				if(val != CF_CODING_BCD && val != CF_CODING_ASCII) val = CF_CODING_BCD;
			} else if(cred == CRED_CHARACTER_STRING) {
				if(val != CF_CODING_ASCII) val = CF_CODING_ASCII;
			} else {
				if(val != CF_CODING_BINARY) val = CF_CODING_BINARY;
			}
			cf->cfg->coding = val;
			i = 0; fld = cf->cfg->cardFields;
			if(cred == CRED_FC32_CN32) {
				fld->fieldType = 1;
				val = cfCardMaximumCardFieldOffset(cf, i);
				if(fld->offset > val) fld->offset = val;
				val = cfCardMinimumCardFieldLength(cf, i);
				if(fld->length < val) fld->length = val;
				val = cfCardMaximumCardFieldLength(cf, i);
				if(fld->length > val) fld->length = val;
				i++; fld++;
			}
			fld->fieldType = 0;
			val = cfCardMaximumCardFieldOffset(cf, i);
			if(fld->offset > val) fld->offset = val;
			val = cfCardMinimumCardFieldLength(cf, i);
			if(fld->length < val) fld->length = val;
			val = cfCardMaximumCardFieldLength(cf, i);
			if(fld->length > val) fld->length = val;
			i++; fld++;
			for( ;i < MAX_CARD_FIELD_SZ;i++, fld++) _CardFieldSetDefault(fld);   
		}
	}
}

int cfCardValidateOutputWiegandFormat(void *self)
{ 
	CF_CARD	*cf = self;
	CF_WIEGAND_CFG	*wf;
	int		i, cred, count;

	cred = cf->cfg->credential;
	if(cf->cfg->outputWiegand == 0xff) i = 0;
	else {
		for(i = count = 0, wf = sys_cfg->wiegandOutputFormats;i < MAX_CF_WIEGAND_SZ;i++, wf++)
			if(wf->credential == cred && cf->cfg->outputWiegand == i) break;
		if(i < MAX_CF_WIEGAND_SZ) i = 0;
		else {
			cf->cfg->outputWiegand = 0xff; i = 1;
		}
	}
//printf("cfCardValidateOutputWiegandFormat: return=%d wo=%d\n", i, (int)cf->cfg->outputWiegand);
	return i;
}

int cfCardDecodeData(void *self, unsigned char *data, int dataLength, unsigned char *credData)
{
	CF_CARD	*cf = self;
	CARD_FIELD	*fld;
	unsigned char	*s, *d;
	int		i, len, cred;

	if(dataLength < cfCardEncodedLength(cf)) return 0; 
	cred = cf->cfg->credential; 
	fld = cf->cfg->cardFields;
	s = data; d = credData;
	*d++ = cred;
//printf("cfCardDecodeData: len=%d cred=%d\n", dataLength, cred);
	switch(cred) {
	case CRED_INT32:
		if(cf->id == CFCARD_SC_UID && dataLength == 4 || cf->id != CFCARD_SC_UID) {
			s += fld->offset;
			len = 4 - fld->length;
			if(len > 0) {
				memset(d, 0, len); d += len;
			}
			memcpy(d, s, fld->length); d += fld->length;
		}
		break;
	case CRED_INT64:
		s += fld->offset;
		len	= 8 - fld->length;
		if(len > 0) {
			memset(d, 0, len); d += len;
		}
		memcpy(d, s, fld->length); d += fld->length;
		break;
	case CRED_FC32_CN32:
		s += fld->offset;
		len = 4 - fld->length;
		if(len > 0) {
			memset(d, 0, len); d += len;
		}
		memcpy(d, s, fld->length); d += fld->length;
		s += fld->length;
		fld++;
		s += fld->offset;
		len = 4 - fld->length;
		if(len > 0) {
			memset(d, 0, len); d += len;
		}
		memcpy(d, s, fld->length); d += fld->length;
		break;
	case CRED_BCD_DIGITS:
		*d++ = fld->length;
		if(cf->cfg->coding == CF_CODING_ASCII) {
			s += fld->offset;
			string2bcd(s, fld->length, d); d += (fld->length+1)>>1;
		} else {
			s += fld->offset >> 1;
			if(fld->offset & 1) {
				for(i = 0;i < fld->length;i++) {
					if(i & 1) {
						*d |= (*s) >> 4; d++;
					} else {
						*d = (*s) << 4; s++;
					}
				}
			} else {
				len = (fld->length+1) >> 1;
				memcpy(d, s, len); d += len;
			}
		}
		break;
	case CRED_CHARACTER_STRING:
		*d++ = fld->length;
		s += fld->offset;
		memcpy(d, s, fld->length); d += fld->length;
		break;
	}
	return d - credData;
}

int cfCardEncodedLength(void *self)
{
	CF_CARD	*cf = self;
	CARD_FIELD	*fld;
	int		i, len;

	for(i = len = 0, fld = cf->cfg->cardFields;i < MAX_CARD_FIELD_SZ;i++, fld++) {
		if(fld->fieldType == 0xff) break;
		len += fld->offset;
		len += fld->length;
	}
	return len;
}

/*
int cfCardDecodeCard(void *self, unsigned char *data, int dataLength, unsigned char *credData)
{
	CF_CARD	*cf = self;
	CARD_FIELD	*fld;
	unsigned char	*s, *d, temp[16];
	int		i, len, cred;

	if(dataLength < cfCardEncodedLength(cf)) return 0; 
	cred = cf->cfg->credential; 
	fld = cf->cfg->cardFields;
	s = data;
	len = 0;
	switch(cred) {
	case CRED_INT32:
		s += fld->offset;
		bin2card(s, (int)fld->length, credData);
		len = 9;
		break;
	case CRED_INT64:
		s += fld->offset;
		bin2card(s, (int)fld->length, credData);
		len = 9;
		break;
	case CRED_FC32_CN32:
		s += fld->offset;
		len = fld->length;
		memcpy(temp, s, len); s += len;
		fld++;
		s += fld->offset;
		memcpy(temp+len, s, (int)fld->length);
		len += fld->length;
		bin2card(s, len, credData);
		len = 9;
		break;
	case CRED_BCD_DIGITS:
		if(cf->cfg->coding == CF_CODING_ASCII) {
			s += fld->offset;
			digitslen2card(s, (int)fld->length, credData);
		} else {
			s += fld->offset >> 1;
			d = temp;
			if(fld->offset & 1) {
				for(i = 0;i < fld->length;i++) {
					if(i & 1) {
						*d |= (*s) >> 4; d++;
					} else {
						*d = (*s) << 4; s++;
					}
				}
				bcd2card(temp, (int)fld->length, credData);
			} else {
				bcd2card(s, (int)fld->length, credData);
			}
		}
		len = 9;
		break;
	case CRED_CHARACTER_STRING:
		s += fld->offset;
		strlen2card(s, (int)fld->length, credData);
		len = 9;
		break;
	}
	return len;
}
*/

// Credential Format Wiegand Object

void cfWiegandSetDefault(void *self, int cred)
{
	CF_WIEGAND	*cf = self;

	_CfWiegandSetDefault(cf->cfg, (int)cf->id, cred);
}

void cfWiegandCopy(void *d, void *s)
{
	CF_WIEGAND *df, *sf;

	df = d; sf = s;
	if(sf->cfg) memcpy(df->cfg, sf->cfg, sizeof(CF_WIEGAND_CFG));
	df->id = sf->id;
	df->ctx = sf->ctx;
}

BOOL cfWiegandIsEnableReader(void *self)
{
	CF_WIEGAND	*cf = self;

	return crIsEnableWiegandFormat(cf->ctx, (int)cf->id);
}

void cfWiegandSetEnableReader(void *self, BOOL enable)
{
	CF_WIEGAND	*cf = self;

	crSetEnableWiegandFormat(cf->ctx, (int)cf->id, enable);
}

int cfWiegandId(void *self)
{
	CF_WIEGAND	*cf = self;

	return (int)cf->id;
}

void cfWiegandSetId(void *self, int id)
{
	CF_WIEGAND	*cf = self;

	cf->id = id;
}

int cfWiegandType(void *self)
{
	CF_WIEGAND	*cf = self;

	return (int)cf->type;
}

int cfWiegandCredential(void *self)
{
	CF_WIEGAND	*cf = self;

	return (int)cf->cfg->credential;
}

void cfWiegandSetCredential(void *self, int credential)
{
	CF_WIEGAND	*cf = self;

	cf->cfg->credential = credential;
}

int cfWiegandOutputWiegand(void *self)
{
	CF_WIEGAND	*cf = self;

	return (int)cf->cfg->outputWiegand;
}

void cfWiegandSetOutputWiegand(void *self, int wfId)
{
	CF_WIEGAND	*cf = self;

	cf->cfg->outputWiegand = wfId;
}

int cfWiegandGetAvailableOutputWiegands(void *self, int *arr)
{ 
	CF_WIEGAND		*cf = self;
	CF_WIEGAND_CFG	*wf;
	int		i, cred, count;

	cred = cf->cfg->credential;
	for(i = count = 0, wf = sys_cfg->wiegandOutputFormats;i < MAX_CF_WIEGAND_SZ;i++, wf++)
		if(wf->credential == cred) {
			arr[count] = i; count++;
		}
	return count;
}

int cfWiegandCoding(void *self)
{
	CF_WIEGAND	*cf = self;

	return (int)cf->cfg->coding;
}

void cfWiegandSetCoding(void *self, int coding)
{
	CF_WIEGAND	*cf = self;

	cf->cfg->coding = coding;
}

int cfWiegandGetAvailableCodings(void *self, int *arr)
{
	CF_WIEGAND	*cf = self;
	int		i, cred, min, max, count;

	cred = cfWiegandCredential(cf);
	if(cred == CRED_INT32 || cred == CRED_INT64) {
		arr[0] = 0; arr[1] = 3;
		count = 2;
	} else {
		if(cred == CRED_CHARACTER_STRING) {
			min = 2; max = 3;
		} else if(cred == CRED_BCD_DIGITS) { 
			min = 1; max = 3;
		} else {
			min = 0; max = 1;
		}
		count = 0;
		for(i = min;i < max;i++, count++) arr[count] = i;
	}
	return count;
}

int cfWiegandParity(void *self)
{
	CF_WIEGAND	*cf = self;

	return (int)cf->cfg->parity;
}

void cfWiegandSetParity(void *self, int parity)
{
	CF_WIEGAND	*cf = self;

	cf->cfg->parity = parity;
}

int cfWiegandPadding(void *self)
{
	CF_WIEGAND	*cf = self;

	return (int)cf->cfg->padding;
}

void cfWiegandSetPadding(void *self, int padding)
{
	CF_WIEGAND	*cf = self;

	cf->cfg->padding = padding;
}

void *cfWiegandCardFieldAtIndex(void *self, int index)
{
	CF_WIEGAND	*cf = self;
	void	*fld;

	if(index >= 0 && index < MAX_CARD_FIELD_SZ) fld = &cf->cfg->cardFields[index];
	else	fld = NULL;
	return fld;
}

int cfWiegandMaximumPadding(void *self)
{
	return 32;
}

int cfWiegandMaximumCardFieldOffset(void *self, int fldId)
{
	return 32;
}

int cfWiegandMinimumCardFieldLength(void *self, int fldId)
{
	CF_WIEGAND	*cf = self;
	int		cred, min;

	cred = cfWiegandCredential(cf);
	if(cred == CRED_INT64) min = 33;
	else	min = 1;
	return min;
}

int cfWiegandMaximumCardFieldLength(void *self, int fldId)
{
	CF_WIEGAND	*cf = self;
	int		cred, max;

	cred = cfWiegandCredential(cf);
	if(cred == CRED_INT64) max = 64;
	else	max = 32;
	return max;
}

void cfWiegandValidate(void *self)
{
	CF_WIEGAND	*cf = self;
	CARD_FIELD	*fld;
	int		i, val, cred;

	cred = cfWiegandCredential(cf);
	if(cf->id < 4 || cred > CRED_CHARACTER_STRING) {
		cfWiegandSetDefault(cf, CRED_INT32);
		return;
	}
	val = cf->cfg->parity;
	if(val > CF_EVEN_ODD_PARITY) val = CF_NONE_PARITY;
	cf->cfg->parity = val;
	val = cf->cfg->coding;
	if(cred == CRED_BCD_DIGITS) {
		if(val != CF_CODING_BCD && val != CF_CODING_ASCII) val = CF_CODING_BCD;
	} else if(cred == CRED_CHARACTER_STRING) {
		if(val != CF_CODING_ASCII) val = CF_CODING_ASCII;
	} else if(cred == CRED_INT32 || cred == CRED_INT64) {
		if(val != CF_CODING_BINARY && val != CF_CODING_REVERSE_BINARY) val = CF_CODING_BINARY;
	} else {
		if(val != CF_CODING_BINARY) val = CF_CODING_BINARY;
	}
	cf->cfg->coding = val;
	i = 0; fld = cf->cfg->cardFields;
	if(cred == CRED_FC32_CN32) {
		fld->fieldType = 1;
		val = cfWiegandMaximumCardFieldOffset(cf, i);
		if(fld->offset > val) fld->offset = val;
		val = cfWiegandMinimumCardFieldLength(cf, i);
		if(fld->length < val) fld->length = val;
		val = cfWiegandMaximumCardFieldLength(cf, i);
		if(fld->length > val) fld->length = val;
		i++; fld++;
	}
	fld->fieldType = 0;
	val = cfWiegandMaximumCardFieldOffset(cf, i);
	if(fld->offset > val) fld->offset = val;
	val = cfWiegandMinimumCardFieldLength(cf, i);
	if(fld->length < val) fld->length = val;
	val = cfWiegandMaximumCardFieldLength(cf, i);
	if(fld->length > val) fld->length = val;
	i++; fld++;
	for( ;i < MAX_CARD_FIELD_SZ;i++, fld++) _CardFieldSetDefault(fld);   
}

int cfWiegandEncodeData(void *self, unsigned char *credData, unsigned char *wgData)
{
	CF_WIEGAND	*cf = self;
	CARD_FIELD		*fld;
	unsigned char	*s, *d, data[8];
	int		i, bitp, n, len, cred, shift;

	s = credData;
	cred = *s++; 
	if(cf->cfg->credential != cred) return -1;
	switch(cred) {
	case CRED_INT32:
		n = 32; shift = 0;
		if(cf->cfg->coding == CF_CODING_REVERSE_BINARY) {
			wgReverse(s, data, 4); s = data;
		}
		break;
	case CRED_INT64:
		n = 64; shift = 0;
		if(cf->cfg->coding == CF_CODING_REVERSE_BINARY) {
			wgReverse(s, data, 8); s = data;
		}
		break;
	case CRED_FC32_CN32:
		n = 32; shift = 0;
		break;
	case CRED_BCD_DIGITS:
		n = *s++;
		if(cf->cfg->coding == CF_CODING_ASCII) shift = 3;
		else	shift = 2;
		break;
	case CRED_CHARACTER_STRING:
		n = *s++;
		shift = 3;
		break;
	}

	bitp = 0;
	d = wgData;
	fld = cf->cfg->cardFields;
	len = fld->offset; bitset(d, bitp, 0, len); bitp += len;
	len = fld->length;
	if(shift) {
		len <<= shift; n <<= shift;
	} 
	bitcpy(d, bitp, s, n-len, len); bitp += len;
	if(cred == CRED_FC32_CN32) {
		s += 4; fld++;
		len = fld->offset; bitset(d, bitp, 0, len); bitp += len;
		len = fld->length; bitcpy(d, bitp, s, n-len, len); bitp += len;
	}
	len = cf->cfg->padding; bitset(d, bitp, 0, len); bitp += len;
	return bitp;
}

int cfWiegandEncodedLength(void *self)
{
	CF_WIEGAND	*cf = self;
	CARD_FIELD	*fld;
	int		i, len, ushift;

	for(i = len = 0, fld = cf->cfg->cardFields;i < MAX_CARD_FIELD_SZ;i++, fld++) {
		if(fld->fieldType == 0xff) break;
		len += fld->offset;
		if(cf->cfg->coding == CF_CODING_ASCII) ushift = 3;		// x 8
		else if(cf->cfg->coding == CF_CODING_BCD) ushift = 2;	// x 4
		else	ushift = 0;										// x 1
		len += fld->length << ushift;
	}
	len += cf->cfg->padding;
	if(cf->cfg->parity) len += 2;
	return len;
}

/*
int cfWiegandDecodeCard(void *self, unsigned char *data, int dataLength, unsigned char *credData)
{
	CF_WIEGAND	*cf = self;
	CARD_FIELD	*fld;
	unsigned char	*s, *d, temp[16];
	int		i, len, cred;

	if(dataLength < cfWiegandEncodedLength(cf)) return 0; 
	cred = cf->cfg->credential; 
	fld = cf->cfg->cardFields;
	s = data;
	switch(cred) {
	case CRED_INT32:
		s += fld->offset;
		bin2card(s, (int)fld->length, credData);
		break;
	case CRED_INT64:
		s += fld->offset;
		bin2card(s, (int)fld->length, credData);
		break;
	case CRED_FC32_CN32:
		s += fld->offset;
		len = fld->length;
		memcpy(temp, s, len); s += len;
		fld++;
		s += fld->offset;
		memcpy(temp+len, s, (int)fld->length);
		len += fld->length;
		bin2card(s, len, credData);
		break;
	case CRED_BCD_DIGITS:
		*d++ = fld->length;
		if(cf->cfg->coding == CF_CODING_ASCII) {
			s += fld->offset;
			digitslen2card(s, (int)fld->length, credData);
		} else {
			s += fld->offset >> 1;
			d = temp;
			if(fld->offset & 1) {
				for(i = 0;i < fld->length;i++) {
					if(i & 1) {
						*d |= (*s) >> 4; d++;
					} else {
						*d = (*s) << 4; s++;
					}
				}
				bcd2card(temp, (int)fld->length, credData);
			} else {
				bcd2card(s, (int)fld->length, credData);
			}
		}
		break;
	case CRED_CHARACTER_STRING:
		s += fld->offset;
		strlen2card(s, (int)fld->length, credData);
		break;
	}
	return 9;
}
*/

int cardFieldType(void *self)
{
	CARD_FIELD	*fld = self;	
	int		rval;

	if(fld->fieldType == 0xff) rval = -1;
	else	rval = fld->fieldType;
	return rval;
}

void cardFieldSetType(void *self, int fieldType)
{
	CARD_FIELD	*fld = self;	

	if(fieldType < 0) fld->fieldType = 0xff;
	else	fld->fieldType = fieldType;
}

int cardFieldOffset(void *self)
{
	CARD_FIELD	*fld = self;	

	return (int)fld->offset;
}

void cardFieldSetOffset(void *self, int offset)
{
	CARD_FIELD	*fld = self;	

	fld->offset = offset;
}

int cardFieldLength(void *self)
{
	CARD_FIELD	*fld = self;	

	return (int)fld->length;
} 

void cardFieldSetLength(void *self, int length)
{
	CARD_FIELD	*fld = self;	

	fld->length = length;
}

// Credential Format PIN Wiegand Object

void cfWgPinCopy(void *d, void *s)
{
	CF_WIEGAND_PIN *df, *sf;

	df = d; sf = s;
	if(sf->cfg) memcpy(df->cfg, sf->cfg, sizeof(CF_WIEGAND_PIN_CFG));
	df->id = sf->id;
}

int cfWgPinId(void *self)
{
	CF_WIEGAND_PIN *cf = self;;

	return (int)cf->id;
}

int cfWgPinParity(void *self)
{
	CF_WIEGAND_PIN	*cf = self;

	return (int)cf->cfg->parity;
}

void cfWgPinSetParity(void *self, int parity)
{
	CF_WIEGAND_PIN	*cf = self;

	cf->cfg->parity = parity;
}

int cfWgPinFacilityCode(void *self)
{
	CF_WIEGAND_PIN	*cf = self;

	return (int)cf->cfg->data[0];
}

void cfWgPinSetFacilityCode(void *self, int facilityCode)
{
	CF_WIEGAND_PIN	*cf = self;

	cf->cfg->data[0] = facilityCode;
}

