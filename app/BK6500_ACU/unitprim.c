#include <string.h>
#include "unitcfg.h"


#ifndef STAND_ALONE

int unitsGetMaxUnitID(void)
{
	return MAX_UNIT_SZ-1;
}

int unitsGetMaxIOUnitID(void)
{
	return MAX_IOUNIT_SZ-1;
}

#endif

int unitsGetMaxDoorID(void)
{
	return MAX_AD_SZ-1;
}

int unitsGetMaxPointID(void)
{
	return MAX_AP_SZ-1;
}

int unitsGetMaxInputID(void)
{
	return MAX_BI_SZ+MAX_XBI_SZ-1;
}

int unitsGetMaxOutputID(void)
{
	return MAX_BO_SZ+MAX_XBO_SZ-1;
}

int unitsGetMaxReaderID(void)
{
	return MAX_CR_SZ-1;
}

int unitsGetMaxFirePointID(void)
{
	return MAX_UNIT_HALF_SZ-1;
}

int unitsGetMaxFireZoneID(void)
{
	return MAX_UNIT_HALF_SZ-1;
}

int unitsGetMaxAlarmActionID(void)
{
	return MAX_UNIT_SZ-1;
}

#ifndef STAND_ALONE

int unitGetPair(int ID)
{
	int		PairID;
	
	if(ID & 1) PairID = ID - 1;
	else	PairID = ID + 1;
	return PairID;
}

int unitGetAccessDoor(int ID)
{
	return ID;
}

int unitGetAccessPoint(int ID)
{
	return ID;
}

int unitGetBinaryInput(int ID)
{
	return UNIT2BI(ID);
}

int unitGetBinaryOutput(int ID)
{
	return UNIT2BO(ID);
}

int unitGetCredentialReader(int ID)
{
	return ID;
}

int unitGetFirePoint(int ID)
{
	return ID >> 1;
}

int unitGetAlarmAction(int ID)
{
	return ID;
}

void unitGetName(int ID, char *Name)
{
	char	*p;
	int		n;
	
	p = Name;
	if(ID < 2) {
		strcpy(p, "Local"); p += strlen(p);
	} else {
		n = (ID >> 1) - 1;
		strcpy(p, "Unit"); p += strlen(p);
		if(n > 9) {
			*p++ = (n / 10) + '0';
			n %= 10;
		}	
		*p++ = n + '0';
	}
	*p++ = '-'; *p++ = (ID & 1) + '1'; 
	*p = 0;
}

int iounitGetBinaryInput(int ID)
{
	return UNIT2XBI(ID);
}

int iounitGetBinaryOutput(int ID)
{
	return UNIT2XBO(ID);
}

void iounitGetName(int ID, char *Name)
{
	char	*p;

	p = Name;
	strcpy(p, "IOUnit"); p += strlen(p);
	*p++ = ID + '0';
	*p = 0;
}

int iounitCanTypeInput(int ID)
{
	if(ID < MAX_IOUNIT_SZ-4) return 1;
	else	return 0;
}	

#endif

int adGetUnit(int adID)
{
	return adID;
}

int adGetAccessPoint(int adID)
{
	return adID;
}

int adGetPairAccessPoint(int adID)
{
	int		apID;
	
	if(adID & 1) apID = adID - 1;
	else	apID = adID + 1;
	return apID;
}

int adGetLockOutput(int adID)
{
	int		ID;

	ID = UNIT2BO(adID);
	return ID;
}

int adGetRequestToExitInput(int adID)
{
	int		ID;

	ID = UNIT2BI(adID);
	return ID;
}

int adGetDoorStatusInput(int adID)
{
	int		ID;

	ID = UNIT2BI(adID) + 1;
	return ID;
}

int adGetAuxLockOutput(int adID)
{	
	int		ID;
	
	ID = UNIT2BO(adID) + 1;
	return ID;
}

void adGetName(int adID, char *Name)
{
	char	*p;

	p = Name;
#ifndef STAND_ALONE
	unitGetName(adID, p); p += strlen(p);
	*p++ = '.';
#endif
	strcpy(p, "Door");
}

int apGetUnit(int apID)
{
	return apID;
}

int apGetAccessDoor(int apID)
{
	return apID;
}

int apGetOtherAccessDoor(int apID)
{
	int		adID;

	if(apID & 1) adID = apID - 1;
	else	adID = apID + 1;
	return adID;
}

int apGetCredentialReader(int apID)
{
	return apID;
}

void apGetName(int apID, char *Name)
{
	char	*p;

	p = Name;
#ifndef STAND_ALONE
	unitGetName(apID, p); p += strlen(p);
	*p++ = '.';
#endif
	strcpy(p, "Point");
}

int biGetUnit(int biID)
{
	int		ID;
	
	if(biID >= 0 && biID < MAX_BI_SZ) ID = BI2UNIT(biID);
	else	ID = -1;
	return ID;
}

int biGetIOUnit(int biID)
{
	int		ID;
	
#ifndef STAND_ALONE
	if(biID >= MAX_BI_SZ && biID < MAX_BI_SZ+MAX_XBI_SZ) ID = XBI2UNIT(biID);
	else	ID = -1;
#else
	ID = -1;
#endif
	return ID;
}

int biGetAccessDoor(int biID)
{
	return BI2UNIT(biID);
}

int biGetFirePoint(int biID)
{
	int		ID;
	
	ID = BI2UNIT(biID);
	return ID >> 1;
}

void biGetName(int biID, char *Name)
{
	char	*p;
	int		ID, val, type;
	
	p = Name;
	ID = biGetUnit(biID);
	if(ID >= 0) {
#ifndef STAND_ALONE
		unitGetName(ID, p); p += strlen(p);
		*p++ = '.';
#endif
		type = biID & 0x03;
		switch(type) {
		case 0:	strcpy(p, "Exit"); break;
		case 1:	strcpy(p, "Door"); break;
		case 2:	strcpy(p, "Input"); break;
		case 3: if(ID & 1) strcpy(p, "Tamper"); else strcpy(p, "Fire"); break;
		}
	} 
#ifndef STAND_ALONE
	else {
		ID = biGetIOUnit(biID);
		iounitGetName(ID, p); p += strlen(p);	
		*p++ = '.';
		strcpy(p, "Input"); p += strlen(p);
		val = iounitGetBinaryInput(0);
		val = ((biID-val) % MAX_IOUNIT_BI_SZ) + 1;
		if(val < 10) *p++ = val + '0';
		else {
			*p++ = val / 10 + '0';
			*p++ = val % 10 + '0';
		}
		*p = 0;
	}
#endif
}

int boGetUnit(int boID)
{
	int		ID;
	
	if(boID >= 0 && boID < MAX_BO_SZ) ID = BO2UNIT(boID);
	else	ID = -1;
	return ID;
}

int boGetIOUnit(int boID)
{
	int		ID;
	
#ifndef STAND_ALONE
	if(boID >= MAX_BO_SZ && boID < MAX_BO_SZ+MAX_XBO_SZ) ID = XBO2UNIT(boID);
	else	ID = -1;
#else
	ID = -1;
#endif
	return ID;
}

int boGetAccessDoor(int boID)
{
	int		ID;
	
	ID = BO2UNIT(boID);	
	return ID;
}

void boGetName(int boID, char *Name)
{
	char	*p;
	int		ID, val, type;
	
	p = Name;
	ID = boGetUnit(boID);
	if(ID >= 0) {
#ifndef STAND_ALONE
		unitGetName(ID, p); p += strlen(p);
		*p++ = '.';
#endif
		type = boID & 1;
		switch(type) {
		case 0:	strcpy(p, "Lock"); break;
		case 1: strcpy(p, "Relay"); break;
		}
	}
#ifndef STAND_ALONE	
	 else {
		ID = boGetIOUnit(boID);
		iounitGetName(ID, p); p += strlen(p);	
		*p++ = '.';
		strcpy(p, "Output"); p += strlen(p);
		val = iounitGetBinaryOutput(0);
		val = ((boID-val) % MAX_IOUNIT_BO_SZ) + 1;
		if(val < 10) *p++ = val + '0';
		else {
			*p++ = val / 10 + '0';
			*p++ = val % 10 + '0';
		}
		*p = 0;
	}
#endif	
}

int crGetAccessPoint(int crID)
{
	return crID;
}

void crGetName(int crID, char *Name)
{
	char	*p;
	
	p = Name;
#ifndef STAND_ALONE
	unitGetName(crID, p); p += strlen(p);
	*p++ = '.';
#endif
	strcpy(p, "Reader");
}

int fpGetBinaryInput(int fpID)
{
	return (fpID << 3) + 3;
}

void fpGetName(int fpID, char *Name)
{
	char	*p;
	
	p = Name;
#ifndef STAND_ALONE
	unitGetName(fpID<<1, p); p += strlen(p);
	p -= 2; *p++ = '.';
#endif
	strcpy(p, "FirePoint");
}

void actGetName(int actID, char *Name)
{
	char	*p;
	
	p = Name;
#ifndef STAND_ALONE
	unitGetName(actID, p); p += strlen(p);
	*p++ = '.';
#endif
	strcpy(p, "AlarmAction");
}
