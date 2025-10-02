#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "asciidefs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
//#include "wglib.h"
#include "user.h"
#include "msg.h"
#include "cf.h"
#include "cfprim.h"
#include "dev.h"
#include "subdev.h"
#include "ad.h"
#include "cr.h"

extern CREDENTIAL_READER	_CredentialReaders[];

void CardCaptureBeep(void);
void TcpReaderOpen(void);
void TcpReaderClose(void);
int  TcpReaderPeek(void);
int  TcpReaderRead(char *CardData);


void crsInit(void)
{
	CREDENTIAL_READER	*cr;
	int		i, size;

	if(devLocalUnitModel(NULL)) size = MAX_CH_SZ;
	else	  size = MAX_CH_HALF_SZ;
	for(i = 0, cr = _CredentialReaders;i < size;i++, cr++) {
		cr->cls			= CLS_CREDENTIAL_READER;
		cr->id			= i;
		cr->dataLength	= 0;
		cr->cfg			= &sys_cfg->credentialReaders[i];
		cr->credType	= 0xff;
	}
}

void *_CrsGet(int id)
{
	return &_CredentialReaders[id];
}

void *crsGet(int id)
{
	void	*cr, *unit;

	if(!devLocalUnitModel(NULL) && id == 0) cr = &_CredentialReaders[id];
	else {
		unit = unitsGet(CH2UNIT(id));
		if(unit && unitIsNonNull(unit) && unitModel(unit) < 3) cr = &_CredentialReaders[id];
		else	cr = NULL;
	}
	return cr;
}

int crsMaxSize(void)
{
	int		size;

	if(devLocalUnitModel(NULL)) size = MAX_CH_SZ;
	else	  size = MAX_CH_HALF_SZ;
	return size;
}

void _CrSetDefault(CREDENTIAL_READER_CFG *cfg)
{
	cfg->type			= 0xff;
	cfg->readers		= 0x80;
#ifdef _HYNIX
	cfg->scFormat		= 1;
#else
	cfg->scFormat		= 0;
#endif
	cfg->wiegandFormats	= 0x00;
	cfg->authMode		= 1;
	cfg->ledBeep		= 1;
	cfg->option			= 0x00;
	SetDefaultIPAddress(cfg->ipReaderIpAddress);
	cfg->ipReaderPort	= 6000;
}

int _CrEncode(CREDENTIAL_READER_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cfg->type;
	*p++ = cfg->readers;
	*p++ = cfg->scFormat;
	*p++ = cfg->wiegandFormats;
	*p++ = cfg->authMode;
	*p++ = cfg->ledBeep;
	*p++ = cfg->option;
	memcpy(p, cfg->ipReaderIpAddress, 4); p += 4;
	SHORTtoBYTE(cfg->ipReaderPort, p); p += 2;
	return p - (unsigned char *)buf;
}

int _CrDecode(CREDENTIAL_READER_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cfg->type			= *p++;
	cfg->readers		= *p++;
	cfg->scFormat		= *p++;
	cfg->wiegandFormats	= *p++;
	cfg->authMode		= *p++;
	cfg->ledBeep		= *p++;
	cfg->option			= *p++;
	memcpy(cfg->ipReaderIpAddress, p, 4); p += 4;
	BYTEtoSHORT(p, &cfg->ipReaderPort); p += 2;
	return p - (unsigned char *)buf;
}

int crId(void *self)
{
	CREDENTIAL_READER	*cr = self;

	return (int)cr->id;
}

BOOL crIsNonNull(void *self)
{
	CREDENTIAL_READER	*cr = self;
	BOOL	bVal;

	if(cr->cfg->type == 0xff) bVal = FALSE;
	else	bVal = TRUE;
	return bVal;
}

// -1:Null  0:General  1:Primary  2:Secondary  3:Elevator  4..254:Undefined
int crType(void *self)
{
	CREDENTIAL_READER	*cr = self;
	int		type;

	if(cr->cfg->type == 0xff) type = -1;
	else	type = cr->cfg->type; 
	return type;
}

void crSetType(void *self, int type)
{
	CREDENTIAL_READER	*cr = self;

	if(type < 0) cr->cfg->type = 0xff;
	else	cr->cfg->type = type;
}

void *_SubdevsGet(int id);

BOOL crHasDevice(void *self)
{
	CREDENTIAL_READER	*cr = self;
	void	*subdev;
	BOOL	bVal;
	int		id;

	id = crId(cr);
	if(!devLocalUnitModel(NULL) && id == 0) bVal = TRUE;
	else {
		subdev = _SubdevsGet(id);
		if(subdevIsNonNull(subdev)) bVal = TRUE;
		else	bVal = FALSE;
	}
	return bVal;
}

BOOL crIsCreatable(void *self)
{
	CREDENTIAL_READER	*cr = self;
	BOOL	bVal;

	if(crHasDevice(cr) && (!crIsNonNull(cr) || crType(cr) == CR_TYPE_GENERAL)) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

BOOL crIsCreatablePrimary(void *self)
{
	CREDENTIAL_READER	*cr = self;
	BOOL	bVal;

	if(crHasDevice(cr) && (!crIsNonNull(cr) || crType(cr) == CR_TYPE_PRIMARY)) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

BOOL crIsCreatableSecondary(void *self)
{
	CREDENTIAL_READER	*cr = self;
	BOOL	bVal;

	if(crHasDevice(cr) && (!crIsNonNull(cr) || crType(cr) == CR_TYPE_SECONDARY)) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

BOOL crIsPrimary(void *self)
{
	CREDENTIAL_READER	*cr = self;	
	BOOL	bVal;

	if(crType(cr) == CR_TYPE_PRIMARY) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

BOOL crIsSecondary(void *self)
{
	CREDENTIAL_READER	*cr = self;	
	BOOL	bVal;

	if(crType(cr) == CR_TYPE_SECONDARY) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void _CrDelete(int id)
{
	CREDENTIAL_READER	*cr;

	cr = _CrsGet(id);
	cr->cfg->type = 0xff;
}

void crCreate(void *self, int type)
{
	CREDENTIAL_READER	*cr = self;
	
	_CrDelete((int)cr->id);
	_CrSetDefault(cr->cfg);
	cr->cfg->type = (unsigned char)type;
}

void crDelete(void *self)
{
	CREDENTIAL_READER	*cr = self;

	_CrDelete((int)cr->id);
}

void *crAssignedDoor(void *self)
{
	CREDENTIAL_READER	*cr = self;	
	void	*ad;
	int		id, type;

	id = crId(cr);
	type = crType(cr);
	if(type == CR_TYPE_PRIMARY) ad = adsGet(id);
	else if(type == CR_TYPE_SECONDARY) {
		if(id & 1) id--;
		else	   id++;	
		ad = adsGet(id); 
printf("secondary adId=%d ad=%x\n", id, ad);
	} else	ad = NULL;
	return ad;
}

BOOL crIsEnableCardReader(void *self)
{
	CREDENTIAL_READER	*cr = self;
	BOOL	enable;

	if(cr->cfg->readers & 0xf8) enable = TRUE;
	else	enable = FALSE;
}

BOOL crIsEnableScReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x80) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableScReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x80;
	else	sys_cfg->credentialReaders[0].readers &= 0x7f;
}

BOOL crIsEnableEmReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x40) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableEmReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x40;
	else	sys_cfg->credentialReaders[0].readers &= 0xbf;
}

BOOL crIsEnableWiegandReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x20) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableWiegandReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x20;
	else	sys_cfg->credentialReaders[0].readers &= 0xdf;
}

BOOL crIsEnableSerialReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x10) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableSerialReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x10;
	else	sys_cfg->credentialReaders[0].readers &= 0xef;
}

BOOL crIsEnableIpReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x08) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableIpReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x08;
	else	sys_cfg->credentialReaders[0].readers &= 0xf7;
}

BOOL crIsEnablePinReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x02) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnablePinReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x02;
	else	sys_cfg->credentialReaders[0].readers &= 0xfd;
}

BOOL crIsEnableFpReader(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0x01) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableFpReader(void *cr, BOOL enable)
{
	if(enable) sys_cfg->credentialReaders[0].readers |= 0x01;
	else	sys_cfg->credentialReaders[0].readers &= 0xfe;
}

BOOL crIsEnableHynixEmvReader(void *self)
{
	CREDENTIAL_READER	*cr = self;
	BOOL	enable;

	if(cr->cfg->readers & 0x01) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableHynixEmvReader(void *self, BOOL enable)
{
	CREDENTIAL_READER	*cr = self;
	int		rval;

	if(enable) cr->cfg->readers |= 0x01;
	else	cr->cfg->readers &= 0xfe;
}

int crScFormat(void *cr)
{
	int		rval;

	rval = sys_cfg->credentialReaders[0].scFormat;
	if(rval > 2) rval = 0;
	return rval;
}

void crSetScFormat(void *cr, int scf)
{
	sys_cfg->credentialReaders[0].scFormat = scf;
}

BOOL crIsEnableWiegandFormat(void *cr, int wfId)
{
	unsigned char	val;
	BOOL	enable;

	val = 0x80 >> wfId;
	if(sys_cfg->credentialReaders[0].wiegandFormats & val) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

void crSetEnableWiegandFormat(void *cr, int wfId, BOOL enable)
{
	unsigned char	val;

	val = 0x80 >> wfId;
	if(enable) sys_cfg->credentialReaders[0].wiegandFormats |= val;
	else	   sys_cfg->credentialReaders[0].wiegandFormats &= ~val;
}

BOOL crIsEnableCardReaders(void *cr)
{
	BOOL	enable;

	if(sys_cfg->credentialReaders[0].readers & 0xf1) enable = TRUE;
	else	enable = FALSE;
	return enable;
}

int crMainKeypad(void *cr)
{
	int		mode;

	mode = crAuthMode(cr);
	if(crIsEnableScReader(cr) && (mode == 0 || mode > 2)) mode = 1;
	else	mode = 0;
	return mode;
}

// 0:General  1:Card  2:Card+PIN  3:Card or PIN  4:PIN
int crAuthMode(void *self)
{
	CREDENTIAL_READER	*cr = self;
	BOOL	cf, pf;
	int		val;

	cf = crIsEnableCardReaders(cr);
	pf = crIsEnablePinReader(cr);
	val = sys_cfg->credentialReaders[0].authMode;
	if(!cf && !pf || val > 4) val = 0;
	else if(cf && !pf && val > 1) val = 0;
	else if(!cf && pf && (val > 0 && val < 4)) val = 0;
	return val;
}

void crSetAuthMode(void *self, int authMode)
{
	CREDENTIAL_READER	*cr = self;

	cr->cfg->authMode = authMode;
}

int crLedBeep(void *cr)
{
	int		val;

	val = sys_cfg->credentialReaders[0].ledBeep;
	return val;
}

void crSetLedBeep(void *cr, int ledBeep)
{
	sys_cfg->credentialReaders[0].ledBeep = ledBeep;
}

void *crMatchWiegandFormat(CREDENTIAL_READER *cr, int bitLength)
{
	void	*wf;
	int		i;

	if(crIsEnableWiegandReader(NULL)) {
		for(i = 0;i < MAX_CF_WIEGAND_SZ;i++) {
			if(crIsEnableWiegandFormat(NULL, i)) {
				wf = cfWiegandsGet(i, 1);
				if(cfWiegandEncodedLength(wf) == bitLength) break;
			}
		}
		if(i >= MAX_CF_WIEGAND_SZ) wf = NULL;
	} else	wf = NULL;
	return wf;
}

int crCheckWiegandOverlap(CREDENTIAL_READER *cr)
{
	void	*wf;
	int		i, j, count, found, lentbl[MAX_CF_WIEGAND_SZ];

	count = 0;
	if(crIsEnableWiegandReader(NULL)) {
		for(i = 0;i < MAX_CF_WIEGAND_SZ;i++) {
			if(crIsEnableWiegandFormat(NULL, i)) {
				wf = cfWiegandsGet(i, 1);
				lentbl[count] = cfWiegandEncodedLength(wf);
				count++;
			}
		}
	}
	found = 0;
	for(i = 0;i < count;i++) {
		for(j = 0;j < count;j++) {
			if(j != i && lentbl[j] == lentbl[i]) {
				found = 1;
				break;
			}
		}
		if(found) break;
	}
	return found;
}
	
void crAccessTimeName(CREDENTIAL_READER *cr, char *name)
{
	unsigned char	*p;

	p = cr->accessTime;
	sprintf(name, "%04d-%02d-%02d %02d:%02d:%02d", p[0]+2000, (int)p[1],  (int)p[2], (int)p[3], (int)p[4], (int)p[5]);
}

char *crName(void *self, int type)
{
	CREDENTIAL_READER *cr = self;

	return xmenu_reader[type];
}

int crAuthorizationMode(void *self)
{
	CREDENTIAL_READER	*cr = self;

	return cr->cfg->authorizationMode;
}

void crSetAuthorizationMode(void *self, int authorizationMode)
{
	CREDENTIAL_READER	*cr = self;

	cr->cfg->authorizationMode = authorizationMode;
}

int crDuressAlarm(void *self)
{
	CREDENTIAL_READER	*cr = self;

	return (int)cr->cfg->duressAlarm;
}

void crSetDuressAlarm(void *self, int duressAlarm)
{
	CREDENTIAL_READER	*cr = self;

	cr->cfg->duressAlarm = duressAlarm;
}

unsigned char *crDuressDigits(void *self)
{
	CREDENTIAL_READER	*cr = self;

	return cr->cfg->duressDigits;
}

void crSetDuressDigits(void *self, unsigned char *duressDigits)
{
	CREDENTIAL_READER	*cr = self;

	memcpy(cr->cfg->duressDigits, duressDigits, 4);
}

int crEncodeStatus(void *self, void *buf)
{
	CREDENTIAL_READER	*cr = self;
	unsigned char	*p, c;
	
	p = (unsigned char *)buf;
	if(crType(cr) >= 0) {
		c = 0x01;
	} else {
		c = 0x00;
	}
	*p++ = c;	
	return p - (unsigned char *)buf;
}

int crResult(void *self)
{
	CREDENTIAL_READER	*cr = self;

	return (int)cr->result;
}

void crSetResult(void *self, int result)
{
	CREDENTIAL_READER	*cr = self;

	cr->result = result;
}

int crAccessEvent(void *self)
{
	CREDENTIAL_READER	*cr = self;

	return (int)cr->accessEvent;
}

void crSetAccessEvent(void *self, int accessEvent)
{
	CREDENTIAL_READER	*cr = self;

	cr->accessEvent = accessEvent;
}

int crCaptureMode(void *self)
{
	CREDENTIAL_READER	*cr = self;

	return (int)cr->captureMode;
}

void crSetCaptureMode(void *self, int captureMode)
{
	CREDENTIAL_READER	*cr = self;

	cr->captureMode = captureMode;
}

