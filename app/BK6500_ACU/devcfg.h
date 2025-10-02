#ifndef _DEV_CFG_H_
#define _DEV_CFG_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


// Device Option definitions
#define TAMPER					0x01	// 0-bit
#define	LOCKOUT					0x02	// 1-bit
#define BW_MODE					0x04	// 2-bit

// Event Option definitions
#define	MASK_LOCK_EVENT			0x01	// 0-bit
#define	MASK_ACCESS_EVENT		0x02	// 1-bit
#define	MASK_OTHER_EVENT		0x04	// 2-bit
#define	EVENT_OUTPUT_TYPE		0x30	// 5,4-bit 00:Network 01:26-bit Wiegand 10:34-bit Wiegand
#define	EVENT_OUTPUT_26_WIEGAND	0x10
#define	EVENT_OUTPUT_34_WIEGAND	0x20
#define	EVENT_WIEGAND_WHAT		0xc0	// 7,6-bit 00:UserID 01:Card Data


typedef struct _DEVICE_CFG {
	char			FirmwareVersion[17];
	char			Maker[17];
	char			Model[9];
	unsigned char	Language;
	unsigned char	DeviceOption;
	unsigned char	DateNotation;
	unsigned char	TimeNotation;
	unsigned char	EventOption;
	unsigned char	EventOutput;
	unsigned char	EventWiegand;
	unsigned char	BackLightTime;
	unsigned char	LCDContrast;
	unsigned char	LCDBrightness;
	unsigned char	UserSoundVolume;
	unsigned char	AdminSoundVolume;
	unsigned char	KeySoundVolume;
	unsigned char	Reserve[4];
} DEVICE_CFG;


_EXTERN_C void devSetDefault(DEVICE_CFG *dev_cfg);
_EXTERN_C int  devEncode(DEVICE_CFG *dev_cfg, void *buf, int ReadOnly);
_EXTERN_C int  devDecode(DEVICE_CFG *dev_cfg, void *buf, int ReadOnly);
_EXTERN_C int  devValidate(DEVICE_CFG *dev_cfg);
_EXTERN_C char *syscfgGetFirmwareVersion(void);


#endif	/* _DEV_CFG_H_ */