#ifndef _CR_CFG_H_
#define _CR_CFG_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


#define MAX_CR_FP_SZ			1
#define MAX_CR_CARD_SZ			5


// Credential Reader FP Configuration Structure
typedef struct _CR_FP_CFG {
	unsigned char	OSExposure;
	unsigned char	CaptureMode;	
	unsigned char	reserve[2];
} CR_FP_CFG;

// Credential Reader PIN Configuration Structure
typedef struct _CR_PIN_CFG {
	unsigned char	reserve[4];
} CR_PIN_CFG;

// Credential Reader Mifare Configuration Structure
typedef struct _CR_MIFARE_CFG {
	unsigned char	Format;				// 0:SerialNo 1:SpecialCard 2:T-money
	unsigned char	MifareType;			// 0:TypeA 1:TypeB 2:TypeA+TypeB
	unsigned char	DataPosition;		// Sector# x 4 + Block#
	unsigned char	DataOffset;			// Start offset in block(0-15)
	unsigned char	DataLength;			// Data length
	unsigned char	DataType;			// 0:Digit 1:AlphaNumeric 2:Binary	
	unsigned char	reserve[2];
} CR_MIFARE_CFG;

// Credential Reader EM Configuration Structure
typedef struct _CR_EM_CFG {
	unsigned char	DataBitLength;		// 0:32-bit 1:24-bit 2:20-bit
	unsigned char	reserve[3];
} CR_EM_CFG;

// Credential Reader Wiegand Configuration Structure
typedef struct _CR_WIEGAND_CFG {
	unsigned char	Format26Bit;		// 0:Normal 1:Card Number Only 2:Normal+Parity
	unsigned char	ByteOrder;			// 0:Normal 1:Reverse
	unsigned char	reserve[2];
} CR_WIEGAND_CFG;

// Credential Reader Serial Configuration Structure
typedef struct _CR_SERIAL_CFG {
	unsigned char	Format;				// 0:900MHz 1:BarCode 2:EM Card
	unsigned char	DataBitLength;		// 0:32-bit 1:24-bit 2:20-bit   * EM Card
	unsigned char	DataType;			// 0:Digit 1:AlphaNumeric 2:Binary
	unsigned char	reserve[1];
} CR_SERIAL_CFG;

// Credential Reader IP Configuration Structure
typedef struct _CR_IP_CFG {
	unsigned char	Format;				// 0:900MHz 1:BarCode 2:EM Card
	unsigned char	DataBitLength;		// 0:32-bit 1:24-bit 2:20-bit   * EM Card
	unsigned char	DataType;			// 0:Digit 1:AlphaNumeric 2:Binary
	unsigned char	reserve[3];
	unsigned short	ReaderPort;
	unsigned char	ReaderIPAddress[4];
} CR_IP_CFG;

typedef struct _CR_CFG {
	unsigned char	Type;				// 0:Wiegand 1:RS-485 2:IP
	unsigned char	Reader;
	unsigned char	TamperAlarmInhibit;
	unsigned char	IntrusionMode;
	unsigned char	reserve[3];
	unsigned char	Version;
	CR_FP_CFG		crFP;
	CR_PIN_CFG		crPIN;
	CR_MIFARE_CFG	crMifare;
	CR_EM_CFG		crEM;
	CR_WIEGAND_CFG	crWiegand;
	CR_SERIAL_CFG	crSerial;
	CR_IP_CFG		crIP;
} CR_CFG;


_EXTERN_C void crfpSetDefault(CR_FP_CFG *cr_cfg);
_EXTERN_C int  crfpEncode(CR_FP_CFG *cr_cfg, void *buf);
_EXTERN_C int  crfpDecode(CR_FP_CFG *cr_cfg, void *buf);
_EXTERN_C int  crfpValidate(CR_FP_CFG *cr_cfg);
_EXTERN_C void crpinSetDefault(CR_PIN_CFG *cr_cfg);
_EXTERN_C int  crpinEncode(CR_PIN_CFG *cr_cfg, void *buf);
_EXTERN_C int  crpinDecode(CR_PIN_CFG *cr_cfg, void *buf);
_EXTERN_C int  crpinValidate(CR_PIN_CFG *cr_cfg);
_EXTERN_C void crmifareSetDefault(CR_MIFARE_CFG *cr_cfg);
_EXTERN_C int  crmifareEncode(CR_MIFARE_CFG *cr_cfg, void *buf);
_EXTERN_C int  crmifareDecode(CR_MIFARE_CFG *cr_cfg, void *buf);
_EXTERN_C int  crmifareValidate(CR_MIFARE_CFG *cr_cfg);
_EXTERN_C void cremSetDefault(CR_EM_CFG *cr_cfg);
_EXTERN_C int  cremEncode(CR_EM_CFG *cr_cfg, void *buf);
_EXTERN_C int  cremDecode(CR_EM_CFG *cr_cfg, void *buf);
_EXTERN_C int  cremValidate(CR_EM_CFG *cr_cfg);
_EXTERN_C void crwiegandSetDefault(CR_WIEGAND_CFG *cr_cfg);
_EXTERN_C int  crwiegandEncode(CR_WIEGAND_CFG *cr_cfg, void *buf);
_EXTERN_C int  crwiegandDecode(CR_WIEGAND_CFG *cr_cfg, void *buf);
_EXTERN_C int  crwiegandValidate(CR_WIEGAND_CFG *cr_cfg);
_EXTERN_C void crserialSetDefault(CR_SERIAL_CFG *cr_cfg);
_EXTERN_C int  crserialEncode(CR_SERIAL_CFG *cr_cfg, void *buf);
_EXTERN_C int  crserialDecode(CR_SERIAL_CFG *cr_cfg, void *buf);
_EXTERN_C int  crserialValidate(CR_SERIAL_CFG *cr_cfg);
_EXTERN_C void cripSetDefault(CR_IP_CFG *cr_cfg);
_EXTERN_C int  cripEncode(CR_IP_CFG *cr_cfg, void *buf);
_EXTERN_C int  cripDecode(CR_IP_CFG *cr_cfg, void *buf);
_EXTERN_C int  cripValidate(CR_IP_CFG *cr_cfg);

_EXTERN_C void crSetDefault(CR_CFG *cr_cfg);
_EXTERN_C int  crEncode(CR_CFG *cr_cfg, void *buf);
_EXTERN_C int  crDecode(CR_CFG *cr_cfg, void *buf);
_EXTERN_C int  crDecode2(CR_CFG *cr_cfg, void *buf);
_EXTERN_C int  crValidate(CR_CFG *cr_cfg);
_EXTERN_C void crSetDefaultAll(CR_CFG *cr_cfg);
_EXTERN_C int  crEncodeAll(CR_CFG *cr_cfg, void *buf);
_EXTERN_C int  crDecodeAll(CR_CFG *cr_cfg, void *buf);
_EXTERN_C int  crGetFPReaderEnable(CR_CFG *cr_cfg);
_EXTERN_C void crSetFPReaderEnable(CR_CFG *cr_cfg, int Enable);
_EXTERN_C int  crGetPINReaderEnable(CR_CFG *cr_cfg);
_EXTERN_C void crSetPINReaderEnable(CR_CFG *cr_cfg, int Enable);
_EXTERN_C int  crGetCardReaderEnable(CR_CFG *cr_cfg);
_EXTERN_C int  crGetMifareReaderEnable(CR_CFG *cr_cfg);
_EXTERN_C void crSetMifareReaderEnable(CR_CFG *cr_cfg, int Enable);
_EXTERN_C int  crGetEMReaderEnable(CR_CFG *cr_cfg);
_EXTERN_C void crSetEMReaderEnable(CR_CFG *cr_cfg, int Enable);
_EXTERN_C int  crGetWiegandReaderEnable(CR_CFG *cr_cfg);
_EXTERN_C void crSetWiegandReaderEnable(CR_CFG *cr_cfg, int Enable);
_EXTERN_C int  crGetSerialReaderEnable(CR_CFG *cr_cfg);
_EXTERN_C void crSetSerialReaderEnable(CR_CFG *cr_cfg, int Enable);
_EXTERN_C int  crGetIPReaderEnable(CR_CFG *cr_cfg);
_EXTERN_C void crSetIPReaderEnable(CR_CFG *cr_cfg, int Enable);
_EXTERN_C void crSetIPPINReaderEnable(CR_CFG *cr_cfg, int Enable);
_EXTERN_C void sckeySetDefault(unsigned char *Key);
_EXTERN_C int  sckeyEncode(unsigned char *Key, unsigned char *buf);
_EXTERN_C int  sckeyDecode(unsigned char *Key, unsigned char *buf);
_EXTERN_C int  sckeyValidate(unsigned char *Key);
_EXTERN_C int  sckeyEncrypt(unsigned char *Key, unsigned char *EncryptedKey, int DeviceID);
_EXTERN_C int  sckeyDecrypt(unsigned char *Key, unsigned char *EncryptedKey, int DeviceID);
_EXTERN_C void sckeySetValidate(unsigned char *Key);
_EXTERN_C int  sckeyCompare(unsigned char *Key, unsigned char *NewKey);
_EXTERN_C void sckeyPrint(unsigned char *Key, char *msg);


#endif	/* _CR_CFG_H_ */