#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "nwpcfg.h"
#include "crcfg.h"


void crfpSetDefault(CR_FP_CFG *cr_cfg)
{
	cr_cfg->OSExposure	= 5;	// 3
	cr_cfg->CaptureMode	= 0;
}

int crfpEncode(CR_FP_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cr_cfg->OSExposure;
 	*p++ = cr_cfg->CaptureMode;
	return p - (unsigned char *)buf;
}

int crfpDecode(CR_FP_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cr_cfg->OSExposure	= *p++;
 	cr_cfg->CaptureMode	= *p++;
	return p - (unsigned char *)buf;
}

int crfpValidate(CR_FP_CFG *cr_cfg)
{
	return 1;
}

void crpinSetDefault(CR_PIN_CFG *cr_cfg)
{
	cr_cfg->reserve[0]	= 0;
}

int crpinEncode(CR_PIN_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cr_cfg->reserve[0];
 	return p - (unsigned char *)buf;
}

int crpinDecode(CR_PIN_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cr_cfg->reserve[0] 	= *p++;
	return p - (unsigned char *)buf;
}

int crpinValidate(CR_PIN_CFG *cr_cfg)
{
	return 1;
}

void crmifareSetDefault(CR_MIFARE_CFG *cr_cfg)
{
	cr_cfg->Format			= 0;		// 0:SerialNo 1:SpecialCard 2:T-money
	cr_cfg->MifareType		= 0;		// 0:TypeA 1:TypeB 2:TypeA+TypeB
	cr_cfg->DataPosition	= 0;		// Sector# x 4 + Block#
	cr_cfg->DataOffset		= 0;		// Start offset in block(0-15)
	cr_cfg->DataLength		= 16;		// Data length
	cr_cfg->DataType		= 0;		// 0:Digit 1:AlphaNumeric 2:Binary
}

int crmifareEncode(CR_MIFARE_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cr_cfg->Format;
	*p++ = cr_cfg->MifareType;
	*p++ = cr_cfg->DataPosition;
	*p++ = cr_cfg->DataOffset;
	*p++ = cr_cfg->DataLength;
	*p++ = cr_cfg->DataType;
 	return p - (unsigned char *)buf;
}

int crmifareDecode(CR_MIFARE_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cr_cfg->Format			= *p++;
	cr_cfg->MifareType		= *p++;
	cr_cfg->DataPosition	= *p++;
	cr_cfg->DataOffset		= *p++;
	cr_cfg->DataLength		= *p++;
	cr_cfg->DataType		= *p++;	
 	return p - (unsigned char *)buf;
}

int crmifareValidate(CR_MIFARE_CFG *cr_cfg)
{
	return 1;
}

void cremSetDefault(CR_EM_CFG *cr_cfg)
{
	cr_cfg->DataBitLength	= 0;		// 0:32-bit 1:24-bit 2:20-bit
}

int cremEncode(CR_EM_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cr_cfg->DataBitLength;
 	return p - (unsigned char *)buf;
}

int cremDecode(CR_EM_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cr_cfg->DataBitLength	= *p++;
 	return p - (unsigned char *)buf;
}

int cremValidate(CR_EM_CFG *cr_cfg)
{
	return 1;
}

void crwiegandSetDefault(CR_WIEGAND_CFG *cr_cfg)
{
	cr_cfg->Format26Bit		= 0;		// 0:Normal 1:Facility Only 2:Normal+Parity
	cr_cfg->ByteOrder		= 0;		// 0:Normal 1:Reverse
}

int crwiegandEncode(CR_WIEGAND_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cr_cfg->Format26Bit;
	*p++ = cr_cfg->ByteOrder;
 	return p - (unsigned char *)buf;
}

int crwiegandDecode(CR_WIEGAND_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cr_cfg->Format26Bit		= *p++;
	cr_cfg->ByteOrder		= *p++;
 	return p - (unsigned char *)buf;
}

int crwiegandValidate(CR_WIEGAND_CFG *cr_cfg)
{
	return 1;
}

void crserialSetDefault(CR_SERIAL_CFG *cr_cfg)
{
	cr_cfg->Format			= 0;		// 0:900MHz 1:BarCode 2:EM Card
	cr_cfg->DataBitLength	= 0;		// 0:32-bit 1:24-bit 2:20-bit   * EM Card
	cr_cfg->DataType		= 0;		// 0:Digit 1:AlphaNumeric 2:Binary
}

int crserialEncode(CR_SERIAL_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
 	*p++ = cr_cfg->Format;
	*p++ = cr_cfg->DataBitLength;
	*p++ = cr_cfg->DataType;
	return p - (unsigned char *)buf;
}

int crserialDecode(CR_SERIAL_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cr_cfg->Format			= *p++;
	cr_cfg->DataBitLength	= *p++;
	cr_cfg->DataType		= *p++;	
	return p - (unsigned char *)buf;
}

int crserialValidate(CR_SERIAL_CFG *cr_cfg)
{
	return 1;
}

void cripSetDefault(CR_IP_CFG *cr_cfg)
{
	cr_cfg->Format			= 0;		// 0:900MHz 1:BarCode 2:EM Card
	cr_cfg->DataBitLength	= 0;		// 0:32-bit 1:24-bit 2:20-bit   * EM Card
	cr_cfg->DataType		= 0;		// 0:Digit 1:AlphaNumeric 2:Binary
	SetDefaultIPAddress(cr_cfg->ReaderIPAddress); cr_cfg->ReaderIPAddress[3] = 3;
	cr_cfg->ReaderPort		= 5578;
}

int cripEncode(CR_IP_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
 	*p++ = cr_cfg->Format;
	*p++ = cr_cfg->DataBitLength;
	*p++ = cr_cfg->DataType;
	memcpy(p, cr_cfg->ReaderIPAddress, 4); p += 4;
	SHORTtoBYTE(cr_cfg->ReaderPort, p); p += 2;
	return p - (unsigned char *)buf;
}

int cripDecode(CR_IP_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cr_cfg->Format			= *p++;
	cr_cfg->DataBitLength	= *p++;
	cr_cfg->DataType		= *p++;	
	memcpy(cr_cfg->ReaderIPAddress, p, 4); p += 4;
	BYTEtoSHORT(p, &cr_cfg->ReaderPort); p += 2;	
 	return p - (unsigned char *)buf;
}

int cripValidate(CR_IP_CFG *cr_cfg)
{
	return 1;
}

void crSetDefault(CR_CFG *cr_cfg)
{
	cr_cfg->Type	= 0;
#ifdef STAND_ALONE
#ifdef _HYNIX
	cr_cfg->Reader	= 0x04;
#else
	cr_cfg->Reader	= 0x03;
#endif
#else
	cr_cfg->Reader	= 0x10;
#endif
	cr_cfg->TamperAlarmInhibit	= 1;
	cr_cfg->IntrusionMode		= 0;
}

int crEncode(CR_CFG *cr_cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	*p++ = cr_cfg->Type;
	*p++ = cr_cfg->Reader;
	if(cr_cfg->TamperAlarmInhibit) val = 0x01; else val = 0x00;
	if(cr_cfg->IntrusionMode) val |= 0x02;
	*p++ = val;
	return p - (unsigned char *)buf;
}

int crDecode(CR_CFG *cr_cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	cr_cfg->Type	= *p++;
	cr_cfg->Reader	= *p++;
	val = *p++;
	if(val & 0x01) cr_cfg->TamperAlarmInhibit = 1; else cr_cfg->TamperAlarmInhibit = 0;
	if(val & 0x02) cr_cfg->IntrusionMode = 1; else cr_cfg->IntrusionMode = 0;
	return p - (unsigned char *)buf;
}

int crDecode2(CR_CFG *cr_cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	val = *p++;
	if(val & 0x01) cr_cfg->TamperAlarmInhibit = 1; else cr_cfg->TamperAlarmInhibit = 0;
	if(val & 0x02) cr_cfg->IntrusionMode = 1; else cr_cfg->IntrusionMode = 0;
	return p - (unsigned char *)buf;
}

int crValidate(CR_CFG *cr_cfg)
{
	return 1;
}

void crSetDefaultAll(CR_CFG *cr_cfg)
{
	crSetDefault(cr_cfg);
	crfpSetDefault(&cr_cfg->crFP);
	crpinSetDefault(&cr_cfg->crPIN);
	crmifareSetDefault(&cr_cfg->crMifare);
	cremSetDefault(&cr_cfg->crEM);
	crwiegandSetDefault(&cr_cfg->crWiegand);
	crserialSetDefault(&cr_cfg->crSerial);
	cripSetDefault(&cr_cfg->crIP);
}

int crEncodeAll(CR_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	p += crEncode(cr_cfg, p);
	p += crfpEncode(&cr_cfg->crFP, p);
	p += crpinEncode(&cr_cfg->crPIN, p);
	p += crmifareEncode(&cr_cfg->crMifare, p);
	p += cremEncode(&cr_cfg->crEM, p);
	p += crwiegandEncode(&cr_cfg->crWiegand, p);
	p += crserialEncode(&cr_cfg->crSerial, p);
	p += cripEncode(&cr_cfg->crIP, p);
	return p - (unsigned char *)buf;
}

int crDecodeAll(CR_CFG *cr_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	p += crDecode(cr_cfg, p);
	p += crfpDecode(&cr_cfg->crFP, p);
	p += crpinDecode(&cr_cfg->crPIN, p);
	p += crmifareDecode(&cr_cfg->crMifare, p);
	p += cremDecode(&cr_cfg->crEM, p);
	p += crwiegandDecode(&cr_cfg->crWiegand, p);
	p += crserialDecode(&cr_cfg->crSerial, p);
	p += cripDecode(&cr_cfg->crIP, p);
	return p - (unsigned char *)buf;
}

int crGetFPReaderEnable(CR_CFG *cr_cfg)
{
	int		enable;

	if(cr_cfg->Reader & 0x01) enable = 1;
	else	enable = 0;
	return enable;
}

void crSetFPReaderEnable(CR_CFG *cr_cfg, int Enable)
{
	if(Enable) cr_cfg->Reader |= 0x01; else cr_cfg->Reader &= 0xfe; 
}

int crGetPINReaderEnable(CR_CFG *cr_cfg)
{
	int		enable;

	if(cr_cfg->Reader & 0x02) enable = 1;
	else	enable = 0;
	return enable;
}

void crSetPINReaderEnable(CR_CFG *cr_cfg, int Enable)
{
	if(Enable) cr_cfg->Reader |= 0x02; else cr_cfg->Reader &= 0xfd; 
}

int crGetCardReaderEnable(CR_CFG *cr_cfg)
{
	int		enable;

	if(cr_cfg->Reader & 0x7c) enable = 1;
	else	enable = 0;
	return enable;
}

int crGetMifareReaderEnable(CR_CFG *cr_cfg)
{
	int		enable;

	if(cr_cfg->Reader & 0x04) enable = 1;
	else	enable = 0;
	return enable;
}

void crSetMifareReaderEnable(CR_CFG *cr_cfg, int Enable)
{
	if(Enable) cr_cfg->Reader |= 0x04; else cr_cfg->Reader &= 0xfb; 
}

int crGetEMReaderEnable(CR_CFG *cr_cfg)
{
	int		enable;

	if(cr_cfg->Reader & 0x08) enable = 1;
	else	enable = 0;
	return enable;
}

void crSetEMReaderEnable(CR_CFG *cr_cfg, int Enable)
{
	if(Enable) cr_cfg->Reader |= 0x08; else cr_cfg->Reader &= 0xf7; 
}

int crGetWiegandReaderEnable(CR_CFG *cr_cfg)
{
	int		enable;

	if(cr_cfg->Reader & 0x10) enable = 1;
	else	enable = 0;
	return enable;
}

void crSetWiegandReaderEnable(CR_CFG *cr_cfg, int Enable)
{
	if(Enable) cr_cfg->Reader |= 0x10; else cr_cfg->Reader &= 0xef; 
}

int crGetSerialReaderEnable(CR_CFG *cr_cfg)
{
	int		enable;

	if(cr_cfg->Reader & 0x20) enable = 1;
	else	enable = 0;
	return enable;
}

void crSetSerialReaderEnable(CR_CFG *cr_cfg, int Enable)
{
	if(Enable) cr_cfg->Reader |= 0x20; else cr_cfg->Reader &= 0xdf; 
}

int crGetIPReaderEnable(CR_CFG *cr_cfg)
{
	int		enable;

	if(cr_cfg->Reader & 0x40) enable = 1;
	else	enable = 0;
	return enable;
}

void crSetIPReaderEnable(CR_CFG *cr_cfg, int Enable)
{
	if(Enable) cr_cfg->Reader |= 0x40; else cr_cfg->Reader &= 0xbf; 
}

void sckeySetDefault(unsigned char *Key)
{
	unsigned char	*p;
	
	p = Key;
	memset(p, 0xff, 6);	p += 6;	// PrimaryKey
	memset(p, 0xff, 6);	p += 6;	// SecondaryKey
	*p = 0;						// KeyOption
}

int sckeyEncode(unsigned char *Key, unsigned char *buf)
{
	memcpy(buf, Key, 13);
	return 13;
}

int sckeyDecode(unsigned char *Key, unsigned char *buf)
{
	memcpy(Key, buf, 13);
	return 13;
}

int sckeyValidate(unsigned char *Key)
{
	unsigned char	dfltkey[8], nullkey[8];
	
	memset(dfltkey, 0xff, 6);
	memset(nullkey, 0x00, 6);
#ifdef _WIN32
	if(!memcmp(Key, dfltkey, 6) || !memcmp(Key, nullkey, 6)) return 0;
#else
	if(!n_memcmp(Key, dfltkey, 6) || !n_memcmp(Key, nullkey, 6)) return 0;
#endif
	else	return 1;
}

// CurrentPrimaryKey(C) + NewPrimaryKey(P) + NewSecondaryKey(S) + KeyOption(O)
// Key:CCCCCCPPPPPPSSSSSSO =encode=> buf:xxxxCCCCCCxxxxPPPPPPxxxxSSSSSSxO =encrypt=> EncryptedKey
int sckeyEncrypt(unsigned char *Key, unsigned char *EncryptedKey, int DeviceID)
{
	unsigned char	*s, *d, buf[36];
	unsigned long	key;
	
	s = Key; d = EncryptedKey;	
	simRand(d, 4); d += 4;			
	memcpy(d, s, 6); s += 6; d += 6;
	simRand(d, 4); d += 4;
	memcpy(d, s, 6); s += 6; d += 6;
	simRand(d, 4); d += 4;
	memcpy(d, s, 6); s += 6; d += 6;
	simRand(d, 1); d += 1;
	*d++ = *s;
	key = DeviceID;
	key = (key + (key << 16)) * key + key;
	sim32Encrypt(buf, 32, EncryptedKey, 0xa26e95c6, key);
	return EncryptedKey - d;
}

// CurrentPrimaryKey(C) + NewPrimaryKey(P) + NewSecondaryKey(S) + KeyOption(O)
// EncryptedKey =decrypt=> buf:xxxxCCCCCCxxxxPPPPPPxxxxSSSSSSxO =decode=> Key:CCCCCCPPPPPPSSSSSSO
int sckeyDecrypt(unsigned char *Key, unsigned char *EncryptedKey, int DeviceID)
{
	unsigned char	*s, *d, buf[36];
	unsigned long	key;
	
	key = DeviceID;
	key = (key + (key << 16)) * key + key;
	sim32Decrypt(EncryptedKey, 32, buf, 0xa26e95c6, key);
	s = buf; d = Key;
	s += 4;
	memcpy(d, s, 6); s += 10; d += 6;
	memcpy(d, s, 6); s += 10; d += 6;
	memcpy(d, s, 6); s += 7;  d += 6;
	*d++ = *s;
	return d - Key;
}

void sckeySetValidate(unsigned char *Key)
{
/*	
	unsigned char	*p;
	
	p = data;	
	if(sckeyValidate(p)) memcpy(sys_cfg->SCPrimaryKey, p, 6);
	p += 6;
	if(scValidateKey(p)) memcpy(sys_cfg->SCSecondaryKey, p, 6);
	p += 6;
	sys_cfg->SCKeyOption = *p;
*/
}

int sckeyCompare(unsigned char *Key, unsigned char *NewKey)
{
#ifdef _WIN32
	if(memcmp(Key, NewKey, 13)) return 1;
#else
	if(n_memcmp(Key, NewKey, 13)) return 1;
#endif
	else return 0;
}

void sckeyPrint(unsigned char *key, char *msg)
{
//	cprintf("%s=[%02x %02x %02x %02x %02x %02x]\n", msg, (int)key[0], (int)key[1], (int)key[2], (int)key[3], (int)key[4], (int)key[5]);
}
