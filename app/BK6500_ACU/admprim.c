#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "gfont.h"
#include "symbol.h"
#include "lcdc.h"
#include "iodev.h"
#include "syscfg.h"
#include "unit.h"
#include "acad.h"
#include "acap.h"
#include "msg.h"
#include "cr.h"
#include "sysdep.h"
#include "gwnd.h"
#ifndef STAND_ALONE
#include "cli.h"
#endif
#include "admprim.h"


#ifndef STAND_ALONE

void GetUnitName(int ID, char *Name)
{
	char	*p;
	int		n;
	
	p = Name;
	if(ID < 2) {
		strcpy(p, xmenu_hw_config[4]); p += strlen(p);
	} else {
		n = (ID >> 1) - 1;
		strcpy(p, xmenu_hw_config[0]); p += strlen(p);
		if(n > 9) {
			*p++ = (n / 10) + '0';
			n %= 10;
		}	
		*p++ = n + '0';
	}
	*p++ = '-'; *p++ = (ID & 1) + '1';
	*p = 0;
}

void GetIOUnitName(int ID, char *Name)
{
	char	*p;
	
	p = Name;
	strcpy(p, xmenu_hw_config[15]); p += strlen(p);
	*p++ = ID + '0';
	*p = 0;
}

#endif


void GetAccessDoorName(int adID, char *Name)
{
	char	*p;

	p = Name;
#ifndef STAND_ALONE
	GetUnitName(adID, p); p += strlen(p);	
	*p++ = '.';
#endif
	strcpy(p, xmenu_hw_config[6]);
}

void GetAccessPointName(int apID, char *Name)
{
	char	*p;

	p = Name;
#ifndef STAND_ALONE
	GetUnitName(apID, p); p += strlen(p);	
	*p++ = '.';
#endif
	strcpy(p, xmenu_hw_config[11]);
}

void GetBinaryInputName(int biID, char *Name)
{
	char	*p;
	int		ID, val, type;

	p = Name;
	ID = biGetUnit(biID);
	if(ID >= 0) {
#ifndef STAND_ALONE
		GetUnitName(ID, p); p += strlen(p);	
		*p++ = '.';
#endif
		type = biID & 03;
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
		GetIOUnitName(ID, p); p += strlen(p);	
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

void GetBinaryOutputName(int boID, char *Name)
{
	char	*p;
	int		ID, val, type;

	p = Name;
	ID = boGetUnit(boID);
	if(ID >= 0) {
#ifndef STAND_ALONE
		GetUnitName(ID, p); p += strlen(p);	
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
		GetIOUnitName(ID, p); p += strlen(p);	
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

void GetCredentialReaderName(int crID, char *Name)
{
	char	*p;

	p = Name;
#ifndef STAND_ALONE
	GetUnitName(crID, p); p += strlen(p);
	*p++ = '.';
#endif
	strcpy(p, "Reader");
}

#ifndef STAND_ALONE

void GetAccessZoneName(int azID, char *Name)
{
	char	*p;
	int		n;

	p = Name;
	if(azID < 0 || azID >= 0xff) {
		*p++ = '-'; *p = 0;
	} else if(azID == 0) {
		strcpy(p, xmenu_access_zone[5]);
	} else {
		strcpy(p, xmenu_hw_config[12]); p += strlen(p);
		*p++ = ' ';
		n = azID;	
		if(n > 9) {
			*p++ = (n / 10) + '0';
			n %= 10;
		}
		*p++ = n + '0';
		*p = 0;
	}
}

void GetAccessZoneShortName(int azID, char *Name)
{
	char	*p;
	int		n;

	p = Name;
	if(azID < 0 || azID >= 0xff) {
		*p++ = '-'; *p = 0;
	} else if(azID == 0) {
		strcpy(p, xmenu_access_zone[5]);
	} else {
		n = azID;
		if(n > 9) {
			*p++ = (n / 10) + '0';
			n %= 10;
		}
		*p++ = n + '0';
		*p = 0;
	}
}

#endif

void GetFirePointName(int fzID, char *Name)
{
	char	*p;

	p = Name;
#ifndef STAND_ALONE
	GetUnitName(fzID<<1, p); p += strlen(p);
	p -= 2; *p++ = '.';
#endif	
	strcpy(p, xmenu_hw_config[17]);
}

void GetFireZoneName(int fzID, char *Name)
{
	char	*p;
	int		n;

	p = Name;
	strcpy(p, xmenu_hw_config[13]);
	p += strlen(p);
	*p++ = ' ';
	n = fzID;
	if(n > 9) {
		*p++ = (n / 10) + '0';
		n %= 10;
	}
	*p++ = n + '0';
	*p = 0;
}

void GetAlarmActionName(int actID, char *Name)
{
	char	*p;

	p = Name;
#ifndef STAND_ALONE
	GetUnitName(actID, p); p += strlen(p);
	*p++ = '.';
#endif	
	 strcpy(p, xmenu_hw_config[14]);
}

void GetOutputCommandName(int ocID, char *Name)
{
	char	*p;
	int		n;

	p = Name;
	if(ocID == 0) strcpy(p, xmenu_other[15]);
	else if(ocID == 1) strcpy(p, xmenu_other[16]);
	else {
		strcpy(p, xmenu_alarm[2]); p += strlen(p);
		*p++ = ' ';
		n = ocID;
		if(n > 9) {
			*p++ = (n / 10) + '0';
			n %= 10;
		}
		*p++ = n + '0';
		*p = 0;
	}
}

int GetAccessPointIcon(int apID)
{
	int		type, enable, file;

	type = crGetType(apID);
	enable = crIsEnable(apID);
	if(type == 0) {		// Wiegand
		if(enable) file = WIEGAND_ICON;
		else	file = WIEGAND_DISABLE_ICON;
	} else {			// RS-485, IP
		if(enable) file = READER_ICON;
		else	file = READER_DISABLE_ICON;
	}
	return file;
}

int GetFireZoneIcon(int fzID)
{
	int		file;

	if(fzGetEnable(fzID)) file = FIRE_ICON;
	else	file = FIRE_DISABLE_ICON;
	return file;
}

int GetAlarmActionIcon(int actID)
{
	int		file;

	if(actIsEnable(actID)) file = ACTION_ICON;
	else	file = ACTION_DISABLE_ICON;
	return file;
}

/*
// Return	-1:System error  2:OK(newVal=OldVal) 3:OK(newVal!=OldVal)
int IouSaveByte(int UnitID, GWND *ParentWnd, char *Title, unsigned char *pVal, int NewVal)
{
	int		rval, OldVal;

	OldVal = *pVal;
	*pVal = (unsigned char)NewVal;
	if(NewVal == OldVal) rval = 2;
	else {
		if(UnitID > 1) syscfgIncIouVersion(sys_cfg, UnitID);
		rval = syscfgWrite(sys_cfg);
		if(rval == 0) {
			unitEncodeIouConfig(UnitID);	
			ResultMsg(ParentWnd, Title, R_DATA_SAVED);
			rval = 3;
		} else {
			*pVal = (unsigned char)OldVal;
			if(UnitID > 1) syscfgDecIouVersion(sys_cfg, UnitID);
			ResultMsg(ParentWnd, Title, R_MSG_SYSTEM_ERROR);
			rval = -1;
		}
	}
	return rval;
}
*/
int CruSaveCfg(int UnitID, CR_CFG *tmp_cfg)
{
	CR_CFG	*cr_cfg, _old_cfg;
	int		rval;

	cr_cfg = &sys_cfg->CredentialReaders[UnitID];
	memcpy(&_old_cfg, cr_cfg, sizeof(CR_CFG));
	memcpy(cr_cfg, tmp_cfg, sizeof(CR_CFG));
#ifndef STAND_ALONE
	cr_cfg->Version++; if(!cr_cfg->Version) cr_cfg->Version = 1;
#endif
	rval = syscfgWrite(sys_cfg);;
	if(rval < 0) memcpy(cr_cfg, &_old_cfg, sizeof(CR_CFG));
	else {
		rval = 0;
#ifndef STAND_ALONE
		if(unitGetType(UnitID) == 2) CliCRConfigChanged(UnitID);
		else	unitSetCruVersion(UnitID);
#endif
	}
	return rval;
}

int CruSaveSCKey(unsigned char *NewKey)
{
	unsigned char	temp[20];
	int		rval;

	rval = 0;
	if(sckeyCompare(sys_cfg->SCPrimaryKey, NewKey)) {
#ifndef STAND_ALONE
		temp[0] = sys_cfg->SCKeyVersion; 
#endif
		sckeyDecode(temp+1, sys_cfg->SCPrimaryKey);
#ifndef STAND_ALONE
		sys_cfg->SCKeyVersion++; if(!sys_cfg->SCKeyVersion) sys_cfg->SCKeyVersion = 1;
#endif
		sckeyEncode(NewKey, sys_cfg->SCPrimaryKey);
		rval = syscfgWrite(sys_cfg);
//scPrintKey(sys_cfg->SCPrimaryKey, "Updated PrimaryKey");
//scPrintKey(sys_cfg->SCSecondaryKey, "Updated SecondaryKey");
		if(rval) {
#ifndef STAND_ALONE
			sys_cfg->SCKeyVersion = temp[0];
#endif
			sckeyEncode(temp+1, sys_cfg->SCPrimaryKey);
		} else {
#ifndef STAND_ALONE
			ClisSCKeyChanged();
#endif
		}
	}
	return rval;
}
