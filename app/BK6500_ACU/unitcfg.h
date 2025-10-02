#ifndef _UNIT_CFG_H_
#define _UNIT_CFG_H_


#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


// Unit type definitions
#define	UNIT_TYPE_K200			0
#define UNIT_TYPE_K220			1

// Access Point type definitions
#define	AP_TYPE_NORMAL			0
#define	AP_TYPE_EXIT			1

// Binary Input type definitions
#define	BI_TYPE_EXIT			0
#define	BI_TYPE_DOOR			1
#define	BI_TYPE_INPUT			2
#define	BI_TYPE_FIRE_TAMPER		3
#define	BI_TYPE_READER_TAMPER	4

// Binary Output type definitions
#define	BO_TYPE_RELAY			0
#define	BO_TYPE_AUX_RELAY		1

// Credential Reader type definitions
#define	CR_TYPE_CARD			0
#define	CR_TYPE_PIN				1
#define	CR_TYPE_FP				2

// Hardware Configuration
#define MAX_UNIT_HALF_SZ		11
#define MAX_UNIT_SZ				22
#define MAX_AD_SZ				22		// AccessDoor
#define MAX_AP_SZ				22		// AccessPoint
#define MAX_AZ_SZ				23		// AccessZone
#define MAX_BI_SZ				88		// BinaryInput
#define MAX_BO_SZ				44		// BinaryOutput
#define MAX_CR_SZ				22		// CredentialReader
#define MAX_OC_SZ				16		// OutputCommand
#define MAX_IOUNIT_SZ			10
#define MAX_XBI_SZ				144		// Expansion Binarynput
#define MAX_XBO_SZ				120		// Expansion BinaryOutput

#define MAX_IOUNIT_BI_SZ		24
#define MAX_IOUNIT_BO_SZ		12

// Unit ID conversion macro
#define BI2UNIT(ID)			((ID)>>2)
#define UNIT2BI(ID)			((ID)<<2)
#define BO2UNIT(ID)			((ID)>>1)
#define UNIT2BO(ID)			((ID)<<1)
#define XBI2UNIT(ID)		(((ID)-MAX_BI_SZ)/MAX_IOUNIT_BI_SZ)
#define UNIT2XBI(ID)		((ID)*MAX_IOUNIT_BI_SZ+MAX_BI_SZ)
#define XBO2UNIT(ID)		(((ID)-MAX_BO_SZ)/MAX_IOUNIT_BO_SZ)
#define UNIT2XBO(ID)		((ID)*MAX_IOUNIT_BO_SZ+MAX_BO_SZ)


// Unit Configuration Data Structure
typedef struct _UNIT_CFG {
	unsigned char	Enable;		// 0:Disable 1:Enable
	unsigned char	Type;		// 0:K-200 1:K-220
	unsigned char	reserve[2];
} UNIT_CFG;

// IOUnit Configuration Data Structure
typedef struct _IOUNIT_CFG {
	unsigned char	Enable;		// 0:Disable 1:Enable
	unsigned char	Type;		// 0:K-500(24 Inputs) 1:K-700(12 Outputs)
	unsigned char	reserve[2];
} IOUNIT_CFG;


// Units Configuration Data Structure
typedef struct _UNITS_CFG {
	UNIT_CFG		Units[MAX_UNIT_SZ];
	unsigned long	Doors[MAX_AD_SZ];
	unsigned long	Readers[MAX_CR_SZ];
	unsigned long	FireZones[MAX_UNIT_HALF_SZ];
	unsigned long	AlarmActions[MAX_UNIT_SZ];
	IOUNIT_CFG		IOUnits[MAX_IOUNIT_SZ];
} UNITS_CFG;


#ifdef WIN32
_EXTERN_C void unitsEnter(UNITS_CFG *units_cfg);
_EXTERN_C void unitsLeave(void);
#else
_EXTERN_C void unitscfgSet(UNITS_CFG *units_cfg);
_EXTERN_C void unitscfgGet(void **pUnits, void **pDoors, void **pReaders, void **pAlarmActions, void **pIOUnits);
#endif
_EXTERN_C void unitsSetDefault(void);
_EXTERN_C int  unitsEncode(void *buf);
_EXTERN_C int  unitsDecode(void *buf, int size);
_EXTERN_C int  unitsValidate(void);
_EXTERN_C void unitSetDefault(UNIT_CFG *unit_cfg, int ID);
_EXTERN_C int  unitEncode(UNIT_CFG *unit_cfg, void *buf);
_EXTERN_C int  unitDecode(UNIT_CFG *unit_cfg, void *buf);
_EXTERN_C int  unitValidate(UNIT_CFG *unit_cfg);
_EXTERN_C int  unitGetEnable(int ID);
_EXTERN_C void unitSetEnable(int ID, int Enable);
_EXTERN_C int  unitGetType(int ID);
_EXTERN_C void unitSetType(int ID, int Type);
_EXTERN_C void iounitSetDefault(IOUNIT_CFG *unit_cfg, int ID);
_EXTERN_C int  iounitEncode(IOUNIT_CFG *unit_cfg, void *buf);
_EXTERN_C int  iounitDecode(IOUNIT_CFG *unit_cfg, void *buf);
_EXTERN_C int  iounitValidate(IOUNIT_CFG *unit_cfg);
_EXTERN_C int  iounitGetEnable(int ID);
_EXTERN_C void iounitSetEnable(int ID, int Enable);
_EXTERN_C int  iounitGetType(int ID);
_EXTERN_C void iounitSetType(int ID, int Type);
_EXTERN_C int  adGetEnable(int adID);
_EXTERN_C void adSetEnable(int adID, int Enable);
_EXTERN_C int  adGetRequestToExitEnable(int adID);
_EXTERN_C void adSetRequestToExitEnable(int adID, int Enable);
_EXTERN_C int  adGetDoorStatusEnable(int adID);
_EXTERN_C void adSetDoorStatusEnable(int adID, int Enable);
_EXTERN_C int  adGetAuxLockEnable(int adID);
_EXTERN_C void adSetAuxLockEnable(int adID, int Enable);
_EXTERN_C int  adIsUsable(int adID);
_EXTERN_C int  adIsEnable(int adID);
_EXTERN_C int  adGetExitPoint(int adID);
_EXTERN_C int  apIsUsable(int apID);
_EXTERN_C int  apIsEnable(int apID);
_EXTERN_C int  apGetAssignedDoor(int apID);
_EXTERN_C int  biIsUsable(int biID);
_EXTERN_C int  biIsEnable(int biID);
_EXTERN_C int  biGetType(int biID);
_EXTERN_C int  biGetAssignedDoor(int biID);
_EXTERN_C int  biGetAssignedFirePoint(int biID);
_EXTERN_C int  boIsUsable(int boID);
_EXTERN_C int  boIsEnable(int boID);
_EXTERN_C int  boGetType(int boID);
_EXTERN_C int  boGetAssignedDoor(int boID);
_EXTERN_C int  boIsAssignedAlarmAction(int boID);
_EXTERN_C int  crIsUsable(int crID);
_EXTERN_C int  crIsEnable(int crID);
_EXTERN_C int  crGetType(int crID);
_EXTERN_C void crSetType(int crID, int Type);
_EXTERN_C int  fpIsUsable(int fpID);
_EXTERN_C int  actGetAlarmOutput(int actID);
_EXTERN_C void actSetAlarmOutput(int actID, int boID);
_EXTERN_C int  actAlarmOutputIsEnable(int actID);
_EXTERN_C int  actIsUsable(int actID);
_EXTERN_C int  actIsEnable(int actID);
_EXTERN_C int  actGetUsableAlarmOutputs(int actID, unsigned char *Outputs);

_EXTERN_C int  unitsGetMaxUnitID(void);
_EXTERN_C int  unitsGetMaxIOUnitID(void);
_EXTERN_C int  unitsGetMaxDoorID(void);
_EXTERN_C int  unitsGetMaxPointID(void);
_EXTERN_C int  unitsGetMaxInputID(void);
_EXTERN_C int  unitsGetMaxOutputID(void);
_EXTERN_C int  unitsGetMaxReaderID(void);
_EXTERN_C int  unitsGetMaxFirePointID(void);
_EXTERN_C int  unitsGetMaxFireZoneID(void);
_EXTERN_C int  unitsGetMaxAlarmActionID(void);
_EXTERN_C int  unitGetPair(int ID);
_EXTERN_C int  unitGetAccessDoor(int ID);
_EXTERN_C int  unitGetAccessPoint(int ID);
_EXTERN_C int  unitGetBinaryInput(int ID);
_EXTERN_C int  unitGetBinaryOutput(int ID);
_EXTERN_C int  unitGetCredentialReader(int ID);
_EXTERN_C int  unitGetFirePoint(int ID);
_EXTERN_C int  unitGetAlarmAction(int ID);
_EXTERN_C void unitGetName(int ID, char *Name);
_EXTERN_C int  iounitGetBinaryInput(int ID);
_EXTERN_C int  iounitGetBinaryOutput(int ID);
_EXTERN_C void iounitGetName(int ID, char *Name);
_EXTERN_C int  iounitCanTypeInput(int ID);
_EXTERN_C int  adGetUnit(int adID);
_EXTERN_C int  adGetAccessPoint(int adID);
_EXTERN_C int  adGetPairAccessPoint(int adID);
_EXTERN_C int  adGetLockOutput(int adID);
_EXTERN_C int  adGetRequestToExitInput(int adID);
_EXTERN_C int  adGetDoorStatusInput(int adID);
_EXTERN_C int  adGetAuxLockOutput(int adID);
_EXTERN_C void adGetName(int adID, char *Name);
_EXTERN_C int  apGetUnit(int apID);
_EXTERN_C int  apGetAccessDoor(int apID);
_EXTERN_C int  apGetOtherAccessDoor(int apID);
_EXTERN_C int  apGetCredentialReader(int apID);
_EXTERN_C void apGetName(int apID, char *Name);
_EXTERN_C int  biGetUnit(int biID);
_EXTERN_C int  biGetIOUnit(int biID);
_EXTERN_C int  biGetAccessDoor(int biID);
_EXTERN_C int  biGetFirePoint(int biID);
_EXTERN_C void biGetName(int biID, char *Name);
_EXTERN_C int  boGetUnit(int boID);
_EXTERN_C int  boGetIOUnit(int boID);
_EXTERN_C int  boGetAccessDoor(int boID);
_EXTERN_C void boGetName(int boID, char *Name);
_EXTERN_C int  crGetAccessPoint(int crID);
_EXTERN_C void crGetName(int crID, char *Name);
_EXTERN_C int  fpGetBinaryInput(int fpID);
_EXTERN_C void fpGetName(int fpID, char *Name);
_EXTERN_C void actGetName(int ocID, char *Name);


#endif	/* _UNIT_CFG_H_ */