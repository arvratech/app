#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "sche.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "tacfg.h"
#include "wpaprim.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "cf.h"
#include "cfprim.h"


char *credTitle(void)
{
	return xmenu_format_setting[0];
}

char *credName(int id)
{
	return xmenu_credential[id];
}

char *credClassName(int classId)
{
	return xmenu_cred_format[classId];
}

void credGetDataName(unsigned char *data, char *name)
{
	unsigned long	val1, val2;
//	int64_t		val;
	char	*p;
	int		len;

	switch(data[0]) {
	case CRED_INT32:
		p = name; BYTEtoLONG(data+1, &val1); sprintf(p, "%lu(0x", val1); p += strlen(p);
		bin2hexstr(data+1, 4, p); p += strlen(p); *p++ = ')'; *p = 0;
		break;
	case CRED_INT64:	  bin2hexstr(data+1, 8, name); break;
	case CRED_FC32_CN32:  BYTEtoLONG(data+1, &val1); BYTEtoLONG(data+5, &val2); sprintf(name, "%lu,%lu", val1, val2); break;
	case CRED_BCD_DIGITS: len = data[1]; bcd2string(data+2, len, name); break;
	case CRED_CHARACTER_STRING:	len = data[1]; memcpy(name, data+2, len); name[len] = 0; break;
	}
}

char *cardFieldTypeTitle(void)
{
	return xmenu_card_field[0];
}

char *cardFieldOffsetTitle(void)
{
	return xmenu_card_field[1];
}

char *cardFieldLengthTitle(void)
{
	return xmenu_card_field[2];
}

// Card Field Object

char *cardFieldTypeName(void *self)
{
	CARD_FIELD	*fld = self;

	return xmenu_field_type[fld->fieldType];
}

void cardFieldGetOffsetBitName(void *self, char *name)
{
	CARD_FIELD	*fld = self;

	sprintf(name, "%d %s", (int)fld->offset, admBitUnitTitle());
}

void cardFieldGetLengthBitName(void *self, char *name)
{
	CARD_FIELD	*fld = self;

	sprintf(name, "%d %s", (int)fld->length, admBitUnitTitle());
}

void cardFieldGetOffsetByteName(void *self, char *name)
{
	CARD_FIELD	*fld = self;

	sprintf(name, "%d %s", (int)fld->offset, admByteUnitTitle());
}

void cardFieldGetLengthByteName(void *self, char *name)
{
	CARD_FIELD	*fld = self;

	sprintf(name, "%d %s", (int)fld->length, admByteUnitTitle());
}

char *cfCardTypeTitle(void)
{
	return xmenu_format_setting[1];
}

char *cfCardAppTitle(void)
{
	return xmenu_format_setting[2];
}

char *cfOutputWiegandTitle(void)
{
	return xmenu_format_setting[3];
}

char *cfCodingTitle(void)
{
	return xmenu_format_setting[5];
}

char *cfParityTitle(void)
{
	return xmenu_format_setting[4];
}

char *cfPaddingTitle(void)
{
	return xmenu_card_field[3];
}

char *cfCardFieldsTitle(void)
{
	return xmenu_format_setting[6];
}

char *cfReaderSettingTitle(void)
{
	return xmenu_format_setting[7];
}

char *cfMifareKeyTitle(void)
{
	return xmenu_reader_setting[6];
}

char *cfBlockNoTitle(void)
{
	return xmenu_reader_setting[0];
}

char *cfSectorTitle(void)
{
	return xmenu_reader_setting[1];
}

char *cfBlockTitle(void)
{
	return xmenu_reader_setting[2];
}

char *cfAidTitle(void)
{
	return xmenu_reader_setting[7];
}

char *cfSerialSpeedTitle(void)
{
	return xmenu_reader_setting[3];
}

char *cfSerialParityTitle(void)
{
	return xmenu_reader_setting[4];
}

char *cfSerialStopBitsTitle(void)
{
	return xmenu_reader_setting[5];
}

char *cfAddTitle(void)
{
	return xmenu_wiegand_format[4];
}

char *cfScUidCardType(int id)
{
	if(id > 0) id = 3;
	return xmenu_card_type[id];
}

char *cfScBlockCardType(int id)
{
	if(id > 0) id = 3;
	return xmenu_card_type[id];
}

char *cfScFileCardType(int id)
{
	return xmenu_card_type[id];
}

char *cfSerialCardType(int id)
{
	return xmenu_cardtype_ext[id];
}

char *cfScBlockCardAppName(int id)
{
	return xmenu_cardapp_block[id];
}

char *cfScFileCardAppName(int id)
{
	return xmenu_cardapp_iso7816[id];
}

char *cfSerialCardAppName(int id)
{
	return xmenu_cardapp_serial[id];
}

char *cfCodingName(int id)
{
	return xmenu_field_coding[id];
}

char *cfParityName(int id)
{
	return xmenu_wiegand_parity[id];
}

char *cfSerialSpeedName(int id)
{
	return xmenu_serial_speed[id];
}

char *cfSerialParityName(int id)
{
	return xmenu_parity[id];
}

char *cfSerialStopBitsName(int id)
{
	return xmenu_stop_bits[id];
}

// Credential Format Card Object

char *cfCardName(void *self)
{
	CF_CARD	*cf = self;

	return xmenu_card_format[cf->id];
}

char *cfCardCredentialName(void *self)
{
	CF_CARD	*cf = self;

	return credName((int)cf->cfg->credential);
}

char *cfCardCardTypeName(void *self)
{
	CF_CARD	*cf = self;
	int		val;

	if(cf->id == CFCARD_SERIAL) {
		val = cf->cfg->cardType;
//printf("cfCardCardTypeName: id=%d %x %x\n", cf->cfg, &sys_cfg->cardFormats[4]);
//printf("cfCardCardTypeName: cardType=%d %d\n", val, sys_cfg->cardFormats[4].cardType);
		return xmenu_cardtype_ext[val];
	} else {
		if(cf->id == CFCARD_SC_UID || cf->id == CFCARD_SC_BLOCK) {
			if(cf->cfg->cardType) val = 3;
			else	val = 0;
		} else if(cf->id == CFCARD_EM) {
			val = 4;
		} else {
			val = cf->cfg->cardType;
		}
		return xmenu_card_type[val];
	}
}

char *cfCardCardAppName(void *self)
{
	CF_CARD	*cf = self;
	char	*p;
	int		val;

	val = cf->cfg->cardApp;
	if(cf->id == CFCARD_SC_BLOCK) p = xmenu_cardapp_block[val];
	else if(cf->id == CFCARD_SC_FILE) p = xmenu_cardapp_iso7816[val];
	else if(cf->id = CFCARD_SERIAL) p = xmenu_cardapp_serial[val];
	else	p = NULL;
	return p;
}

void cfCardGetOutputWiegandName(void *self, char *name)
{
	CF_CARD	*cf = self;
	int		id;

	if(cf->cfg->outputWiegand == 0xff) id = -1;
	else	id = cf->cfg->outputWiegand;
	cfWiegandGetName(cfWiegandsGet(id, 1), name);
}

char *cfCardCodingName(void *self)
{
	CF_CARD	*cf = self;
	int		val;

	val = cf->cfg->coding;
	return cfCodingName(val);
}

void cfCardGetMifareKeyName(void *self, char *name)
{
	CF_CARD	*cf = self;

	bin2hexstr(cfCardMifareKey(cf), 6, name);
	stoupper(name);
}

char *cfCardMifareKeyName(void *self)
{
	CF_CARD	*cf = self;

	bin2hexstr(cfCardMifareKey(cf), 6, gText);
	stoupper(gText);
	return gText;
}

void cfCardSetMifareKeyName(void *self, char *name)
{
	CF_CARD	*cf = self;
	unsigned char	data[8];

	hexstr2bin(name, data);
	cfCardSetMifareKey(cf, data);
}

void cfCardGetBlockNoName(void *self, char *name)
{
	CF_CARD	*cf = self;
	int		values[2];

	cfCardGetBlockNo(cf, values);
	if(cf->cfg->cardType) sprintf(name, "%s %d", cfBlockTitle(), (values[0]<<2)+values[1]); 
	else	sprintf(name, "%s %d  %s %d", cfSectorTitle(), values[0], cfBlockTitle(), values[1]); 
}

void cfCardGetAidName(void *self, char *name)
{
	CF_CARD	*cf = self;
	unsigned char	*p;
	int		len;

	p = cfCardGetAid(cf, &len);
	bin2hexstr(p, len, name);
	stoupper(name);
}

char *cfCardAidName(void *self)
{
	CF_CARD	*cf = self;
	unsigned char	*p;
	int		len;

	p = cfCardGetAid(cf, &len);
	bin2hexstr(p, len, gText);
	stoupper(gText);
	return gText;
}

void cfCardSetAidName(void *self, char *name)
{
	CF_CARD	*cf = self;
	unsigned char	data[20];
	int		len;

	len = hexstr2bin(name, data);
	cfCardSetAid(cf, data, len);
}

char *cfCardSerialSpeedName(void *self)
{
	CF_CARD	*cf = self;

	return xmenu_serial_speed[cfCardSerialSpeed(cf)];
}

char *cfCardSerialParityName(void *self)
{
	CF_CARD	*cf = self;

	return xmenu_parity[cfCardSerialParity(cf)];
}

char *cfCardSerialStopBitsName(void *self)
{
	CF_CARD	*cf = self;

	return xmenu_stop_bits[cfCardSerialStopBits(cf)];
}

int cfCardSetCredentialWithKeyValue(void *self, char *buf)
{
	CF_CARD	*cf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(cf->cfg->credential == val) val = 0;
	else {
		cf->cfg->credential = val; val = 1;
	}
	return val;
}

int cfCardSetCardTypeWithKeyValue(void *self, char *buf)
{
	CF_CARD	*cf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(cf->cfg->cardType == val) val = 0;
	else {
		cf->cfg->cardType = val; val = 1;
	}
	return val;
}

int cfCardSetCardAppWithKeyValue(void *self, char *buf)
{
	CF_CARD	*cf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(cf->cfg->cardApp == val) val = 0;
	else {
		cf->cfg->cardApp = val; val = 1;
	}
	return val;
}

int cfCardSetOutputWiegandWithKeyValue(void *self, char *buf)
{
	CF_CARD	*cf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(cf->cfg->outputWiegand == val) val = 0;
	else {
		cf->cfg->outputWiegand = val; val = 1;
	}
	return val;
}

int cfCardSetCodingWithKeyValue(void *self, char *buf)
{
	CF_CARD	*cf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(cf->cfg->coding == val) val = 0;
	else {
		cf->cfg->coding = val; val = 1;
	}
	return val;
}

int cfCardSetBlockMifareKeyWithKeyValue(void *self, char *buf)
{
	CF_CARD	*cf = self;
	char	key[64], value[64];
	unsigned char	data[8];
	int		val;

	get_keyvalue(buf, key, value);
	hexstr2bin(value, data);
	if(!memcmp(cfCardMifareKey(cf), data, 6)) val = 0;
	else {
		cfCardSetMifareKey(cf, data); val = 1;
	}
	return val;
}

int cfCardSetIso7816AidWithKeyValue(void *self, char *buf)
{
	CF_CARD	*cf = self;
	char	key[64], value[64];
	unsigned char	*p, data[8];
	int		val, len;

	get_keyvalue(buf, key, value);
	val = hexstr2bin(value, data);
	p = cfCardGetAid(cf, &len);
	if(len == val && memcmp(p, data, val)) val = 0;
	else {
		cfCardSetAid(cf, data, val); val = 1;
	}
	return val;
}

int cfCardSetBlockNoWithKeyValue(void *self, char *buf)
{
	CF_CARD	*cf = self;
	char	*p, key[64], value[64];
	int		rval, oldValues[2], newValues[2];

	p = get_keyvalue(buf, key, value);
	newValues[0] = n_atoi(value);
	get_keyvalue(p, key, value);
	newValues[1] = n_atoi(value);
	if(oldValues[0] != newValues[0] || oldValues[1] != newValues[1]) {
		cfCardSetBlockNo(cf, newValues);
		rval = 1;
	} else	rval = 0;	
	return rval;
}

int cfCardSetSerialSpeedWithKeyValue(void *self, char *buf)
{
	CF_CARD	*cf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(val == cfCardSerialSpeed(cf)) val = 0;
	else {
		cfCardSetSerialSpeed(cf, val); val = 1;
	}
	return val;
}

int cfCardSetSerialParityWithKeyValue(void *self, char *buf)
{
	CF_CARD	*cf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(val == cfCardSerialParity(cf)) val = 0;
	else {
		cfCardSetSerialParity(cf, val); val = 1;
	}
	return val;
}

int cfCardSetSerialStopBitsWithKeyValue(void *self, char *buf)
{
	CF_CARD	*cf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(val == cfCardSerialStopBits(cf)) val = 0;
	else {
		cfCardSetSerialStopBits(cf, val); val = 1;
	}
	return val;
}

void cfCardEncodeCredentialSpinner(void *self, int cfId, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	int		i, count;

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)cf->cfg->credential); p += strlen(p);
	if(cfId == CFCARD_SC_UID) count = 2;
	else if(cfId == CFCARD_EM) count = 3;
	else	count = MAX_CREDENTIAL_SZ;
	for(i = 0;i < count;i++) {
		sprintf(p, "%s,", xmenu_credential[i]); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = 0;i < count;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfCardEncodeUidCardTypeSpinner(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	int		i, count;

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)cf->cfg->cardType); p += strlen(p);
	count = 0;
	sprintf(p, "%s,", xmenu_card_type[0]); p += strlen(p); count++;
	if(cf->cfg->credential == 1) {
		sprintf(p, "%s,", xmenu_card_type[3]); p += strlen(p); count++;
	}
	*(p-1) = '\n';
	for(i = 0;i < count;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfCardEncodeBlockCardTypeSpinner(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)cf->cfg->cardType); p += strlen(p);
	sprintf(p, "%s,", xmenu_card_type[0]); p += strlen(p);
	sprintf(p, "%s,", xmenu_card_type[3]); p += strlen(p);
	*(p-1) = '\n';
	for(i = 0;i < 2;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfCardEncodeIso7816CardTypeSpinner(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)cf->cfg->cardType); p += strlen(p);
	for(i = 0;i < 3;i++) {
		sprintf(p, "%s,", xmenu_card_type[i]); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = 0;i < 3;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfCardEncodeSerialCardTypeSpinner(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)cf->cfg->cardType); p += strlen(p);
	for(i = 0;i < 3;i++) { 
		sprintf(p, "%s,", xmenu_cardtype_ext[i]); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = 0;i < 3;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfCardEncodeBlockCardAppSpinner(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)cf->cfg->cardApp); p += strlen(p);
	for(i = 0;i < 2;i++) {
		sprintf(p, "%s,", xmenu_cardapp_block[i]); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = 0;i < 2;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfCardEncodeIso7816CardAppSpinner(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)cf->cfg->cardApp); p += strlen(p);
	for(i = 0;i < 3;i++) {
		sprintf(p, "%s,", xmenu_cardapp_iso7816[i]); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = 0;i < 2;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfCardEncodeSerialCardAppSpinner(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)cf->cfg->cardApp); p += strlen(p);
	for(i = 0;i < 2;i++) {
		sprintf(p, "%s,", xmenu_cardapp_serial[i]); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = 0;i < 2;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfCardEncodeOutputWiegandSpinner(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	char	temp[64];
	int		i, count, arr[8];

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)cf->cfg->outputWiegand); p += strlen(p);
	count = cfCardGetAvailableOutputWiegands(cf, arr);
	sprintf(p, "%s,", xmenu_enable[1]); p += strlen(p);
	for(i = 0;i < count;i++) {
		cfWiegandGetName(cfWiegandsGet(arr[i], 1), temp);
		sprintf(p, "%s,", temp); p += strlen(p);
	}
	*(p-1) = '\n';
	sprintf(p, "255,"); p += strlen(p);
	for(i = 0;i < count;i++) {
		sprintf(p, "%d,", arr[i]); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

static void _EncodeCodingSpinner(int coding, int credential, char *title, char *buf)
{
	char	*p;
	int		i, min, max;

	p = buf;
	sprintf(p, "%s=%d\n", title, coding); p += strlen(p);
	if(credential == CRED_CHARACTER_STRING) {
		min = 2; max = 3;
	} else if(credential == CRED_BCD_DIGITS) { 
		min = 1; max = 3;
	} else {
		min = 0; max = 1;
	}
	for(i = min;i < max;i++) {
		sprintf(p, "%s,", cfCodingName(i)); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = min;i < max;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfCardEncodeCodingSpinner(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;

	_EncodeCodingSpinner((int)cf->cfg->coding, (int)cf->cfg->credential, title, buf);
}

void cfCardEncodeBlockNoPicker(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	int		i, values[2];

	p = buf;
	cfCardGetBlockNo(cf, values);
	sprintf(p, "%s=0\n", title); p += strlen(p);
	if(cf->cfg->cardType) {
		sprintf(p, "%s=%d\n", cfBlockTitle(), (values[0]<<2)+values[1]); p += strlen(p);
		for(i = 0;i < 20;i++) {
			sprintf(p, "%s %d,", cfBlockTitle(), i); p += strlen(p);
		}
		*(p-1) = '\n';
	} else {
		sprintf(p, "%s=%d\n", cfSectorTitle(), values[0]); p += strlen(p);
		for(i = 0;i < 16;i++) {
			sprintf(p, "%s %d,", cfSectorTitle(), i); p += strlen(p);
		}
		*(p-1) = '\n';
		sprintf(p, "%s=%d\n", cfBlockTitle(), values[1]); p += strlen(p);
		for(i = 0;i < 4;i++) {
			sprintf(p, "%s %d,", cfBlockTitle(), i); p += strlen(p);
		}
		*(p-1) = '\n';
	}
}

void cfCardEncodeSerialSpeedSpinner(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=%d\n", title, cfCardSerialSpeed(cf)); p += strlen(p);
	for(i = 1;i < 6;i++) {
		sprintf(p, "%s,", xmenu_serial_speed[i]); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = 1;i < 6;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfCardEncodeSerialParitySpinner(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=%d\n", title, cfCardSerialParity(cf)); p += strlen(p);
	for(i = 0;i < 3;i++) {
		sprintf(p, "%s,", xmenu_parity[i]); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = 0;i < 3;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfCardEncodeSerialStopBitsSpinner(void *self, char *title, char *buf)
{
	CF_CARD	*cf = self;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=%d\n", title, cfCardSerialStopBits(cf)); p += strlen(p);
	for(i = 0;i < 2;i++) {
		sprintf(p, "%s,", xmenu_stop_bits[i]); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = 0;i < 2;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

// Credential Format Wiegand Object

void cfWiegandGetName(void *self, char *name)
{
	CF_WIEGAND	*cf = self;
	CARD_FIELD	*fld;
	char	*p;
	int		i, len, ushift;

	if(!cf) strcpy(name, xmenu_enable[1]);
	else if(cf->id < 4) strcpy(name, xmenu_wiegand_format[cf->id]);
	else {
		p = name;
		sprintf(p, "%d-bit(", cfWiegandEncodedLength(cf)); p += strlen(p);
		for(i = 0, fld = cf->cfg->cardFields;i < MAX_CARD_FIELD_SZ;i++, fld++) {
			if(fld->fieldType == 0xff) break;
			if(cf->cfg->coding == CF_CODING_ASCII) ushift = 3;		// x 8
			else if(cf->cfg->coding == CF_CODING_BCD) ushift = 2;	// x 4
			else	ushift = 0;										// x 1
			len = fld->length << ushift;
			if(fld->fieldType) sprintf(p, "fc%d+", len);
			else	sprintf(p, "cn%d+", len);
			p += strlen(p);
		}
		*(p-1) = ')';	
	}
}

char *cfWiegandCredentialName(void *self)
{
	CF_WIEGAND	*cf = self;

	return credName(cf->cfg->credential);
}

void cfWiegandGetOutputWiegandName(void *self, char *name)
{
	CF_WIEGAND	*cf = self;
	int		id;

	if(cf->cfg->outputWiegand == 0xff) id = -1;
	else	id = cf->cfg->outputWiegand;
	cfWiegandGetName(cfWiegandsGet(id, 1), name);
}

char *cfWiegandCodingName(void *self)
{
	CF_WIEGAND	*cf = self;

	return cfCodingName(cf->cfg->coding);
}

char *cfWiegandParityName(void *self)
{
	CF_WIEGAND	*cf = self;

	return cfParityName(cf->cfg->parity);
}

void cfWiegandGetPaddingName(void *self, char *name)
{
	CF_WIEGAND	*cf = self;

	sprintf(name, "%d %s", cf->cfg->padding, admBitUnitTitle());
}

int cfWiegandSetCredentialWithKeyValue(void *self, char *buf)
{
	CF_WIEGAND	*cf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(cf->cfg->credential == val) val = 0;
	else {
		cf->cfg->credential = val; val = 1;
	}
	return val;
}

int cfWiegandSetParityWithKeyValue(void *self, char *buf)
{
	CF_WIEGAND	*cf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(cf->cfg->parity == val) val = 0;
	else {
		cf->cfg->parity = val; val = 1;
	}
	return val;
}

int cfWiegandSetCodingWithKeyValue(void *self, char *buf)
{
	CF_WIEGAND	*cf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(cf->cfg->coding == val) val = 0;
	else {
		cf->cfg->coding = val; val = 1;
	}
	return val;
}

void cfWiegandGetCardFieldLengthName(void *self, int fldId, char *name)
{
	CF_WIEGAND	*cf = self;
	int		ushift;

	if(cf->cfg->coding == CF_CODING_ASCII) ushift = 3;		// x 8
	else if(cf->cfg->coding == CF_CODING_BCD) ushift = 2;	// x 4
	else	ushift = 0;									// x 1
	sprintf(name, "%d %s", cf->cfg->cardFields[fldId].length << ushift, admBitUnitTitle());
}

void cfWiegandEncodeCredentialSpinner(void *self, char *title, char *buf)
{
	CF_WIEGAND	*cf = self;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)cf->cfg->credential); p += strlen(p);
	for(i = 0;i < MAX_CREDENTIAL_SZ;i++) {
		sprintf(p, "%s,", xmenu_credential[i]); p += strlen(p);
	}
	*(p-1) = '\n'; 
	for(i = 0;i < MAX_CREDENTIAL_SZ;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfWiegandEncodeParitySpinner(void *self, char *title, char *buf)
{
	CF_WIEGAND	*cf = self;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)cf->cfg->parity); p += strlen(p);
	for(i = 0;i < 2;i++) {
		sprintf(p, "%s,", xmenu_wiegand_parity[i]); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = 0;i < 2;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfWiegandEncodeCodingSpinner(void *self, char *title, char *buf)
{
	CF_WIEGAND	*cf = self;
	char	*p;
	int		i, min, max;

	if(cf->cfg->credential == CRED_INT32 || cf->cfg->credential == CRED_INT64) {
		p = buf;
		sprintf(p, "%s=%d\n", title, cf->cfg->coding); p += strlen(p);
		sprintf(p, "%s,", cfCodingName(0)); p += strlen(p);
		sprintf(p, "%s,", cfCodingName(3)); p += strlen(p);
		*(p-1) = '\n';
		sprintf(p, "%d,", 0); p += strlen(p);
		sprintf(p, "%d,", 3); p += strlen(p);
		*(p-1) = '\n'; *p = 0;
	} else {
		_EncodeCodingSpinner((int)cf->cfg->coding, (int)cf->cfg->credential, title, buf);
	}
}

