#ifndef _ADM_PRIM_H_
#define _ADM_PRIM_H_


void GetUnitName(int UnitID, char *Name);
void GetIOUnitName(int ID, char *Name);
void GetAccessDoorName(int adID, char *Name);
void GetAccessPointName(int apID, char *Name);
void GetBinaryInputName(int biID, char *Name);
void GetBinaryOutputName(int boID, char *Name);
void GetCredentialReaderName(int crID, char *Name);
void GetAccessZoneName(int azID, char *Name);
void GetAccessZoneShortName(int azID, char *Name);
void GetFirePointName(int fzID, char *Name);
void GetFireZoneName(int faID, char *Name);
void GetAlarmActionName(int actID, char *Name);
void GetOutputCommandName(int actID, char *Name);
int  GetAccessPointIcon(int apID);
int  GetFireZoneIcon(int faID);
int  GetAccessZoneIcon(int azID);
int  GetAlarmActionIcon(int actID);
int  IouSaveByte(int UnitID, GWND *ParentWnd, char *Title, unsigned char *pVal, int NewVal);
int  CruSaveByte(int UnitID, GWND *ParentWnd, char *Title, unsigned char *pVal, int NewVal);
int  CruSaveCfg(int UnitID, CR_CFG *tmp_cfg);
int  CruSaveSCKey(unsigned char *NewKey);


#endif
