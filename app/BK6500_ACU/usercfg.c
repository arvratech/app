#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "prim.h"
#include "sche.h"
#include "acar.h"
#include "user.h"


void userSetDefault(FP_USER *user)
{
	memset(user->ActivateDate, 0, 3);
	memset(user->ExpireDate, 0, 3);
	user->AccessMode		= USER_FP;
	user->AccessRights		= 1;		// All-Always
	user->PIN[0] = user->PIN[1] = 0x11;
	user->PIN[2] = user->PIN[3] = 0xff;
	user->CardData[0]		= 0; memset(user->CardData+1, 0xff, 8);
	user->CardStatus		= 1;
	user->FPSecurityLevel	= 0;
	user->FPFlag			= 0;
	userClearEx(user);
	userClearPhoto(user);
	userClearAccessRights(user);
}

int userEncode(FP_USER *user, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	bin2bcd(user->ActivateDate, 3, p); p += 3;
	bin2bcd(user->ExpireDate, 3, p); p += 3;
	*p++ = user->AccessMode;
	IDtoPACK3(user->AccessRights, p); p += 3;
	memset(p, 0xff, 3); p += 3;
	memcpy(p, user->PIN, 4); p += 4;
	memcpy(p, user->CardData, 9); p += 9;
	*p++ = user->CardStatus;
	*p++ = user->FPSecurityLevel;
	return p - (unsigned char *)buf;
}

int userDecode(FP_USER *user, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	bcd2bin(p, 3, user->ActivateDate); p += 3;
	bcd2bin(p, 3, user->ExpireDate); p += 3;
	user->AccessMode		= *p++;
	PACK3toID(p, &user->AccessRights); p += 3;
	p += 3;
	memcpy(user->PIN, p, 4); p += 4;
	memcpy(user->CardData, p, 9); p += 9;
	user->CardStatus		= *p++;
	user->FPSecurityLevel	= *p++;
//cprintf("ID=%d AM=%d AR=%d CS=%d SL=%d ", user->ID, (int)user->AccessMode, user->AccessRights, (int)user->CardStatus, (int)user->FPSecurityLevel); 
//cprintf("Card=[%02x", (int)user->CardData[0]); for(i = 1;i < 9;i++) cprintf("-%02x", (int)user->CardData[i]); cprintf("]\n");
	return p - (unsigned char *)buf;
}

int userValidate(FP_USER *user)
{
//cprintf("AccessMode=%02x CS=%d SL=%d\n", (int)user->AccessMode, (int)user->CardStatus, (int)user->FPSecurityLevel);
	if(!(user->AccessMode & 0x3f) || user->AccessMode & 0xc0) return 0;
	if(user->CardStatus > 4 || user->FPSecurityLevel > 9) return 0;
	return 1;
}

void userEncodeID(long nID, void *buf)
{
	IDtoPACK3(nID, (unsigned char *)buf);	
}

void userEncodeFPID(unsigned long FPID, void *buf)
{
	memcpy(buf, &FPID, 4);
}

void userDecodeID(long *pID, void *buf)
{
	PACK3toID((unsigned char *)buf, pID);
}

void userDecodeFPID(unsigned long *pFPID, void *buf)
{
	memcpy(pFPID, buf, 4);
}

int userValidateID(long nID)
{
	if(nID > 0 && nID < 0xffffff) return 1;
	else	return 0;
}

void userClearEx(FP_USER *user)
{
	user->UserName[0]	= 0;
	user->UserExternalID[0] = 0;
}

int userEncodeEx(FP_USER *user, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	memcpy_pad(p, user->UserName, 20); p += 20;
	string2bcd(user->UserExternalID, 14, p); p += 7;
	*p++ = 0xff;
	return p - (unsigned char *)buf;
}

int userDecodeEx(FP_USER *user, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	memcpy_chop(user->UserName, p, 20); p += 20;
	bcd2string(p, 14, user->UserExternalID); p += 7;
	p++;
	return p - (unsigned char *)buf;
}

int userValidateEx(FP_USER *user)
{
	if(user->UserExternalID[0] && isdigits(user->UserExternalID) <= 0) return 0;
	return 1;
}

int userIsNullEx(FP_USER *user)
{
	if(user->UserName[0] || user->UserExternalID[0]) return 0;
	return 1;
}

void userClearPhoto(FP_USER *user)
{
	user->Width = user->Height = 0;
	user->QuantityLevel = 0;
	user->JPEGSize = 0;
	user->JPEGBuffer = NULL;
}

int userEncodePhoto(FP_USER *user, void *buf)
{
	unsigned char	*p;
	int		size, body_sz;

	p = (unsigned char *)buf;
	memset(p, 0xff, 3); p += 3;
	*p++ = user->QuantityLevel; 
	SHORTtoBYTE(user->Width, p); p += 2;
	SHORTtoBYTE(user->Height, p); p += 2;
	memcpy(p, user->JPEGBuffer, (int)user->JPEGSize); p += user->JPEGSize;
	size = p - (unsigned char *)buf;
	body_sz = USER_PHOTO_RECORD_SZ - 12;
	if(size < body_sz) {
		size = body_sz - size;
		memset(p, 0xff, size); p += size;
	}
	return p - (unsigned char *)buf;
}

int userDecodePhoto(FP_USER *user, void *buf)
{
	unsigned char	*p;
	int		size;

	p = (unsigned char *)buf;
	p += 3;
	user->QuantityLevel = *p++;
	BYTEtoSHORT(p, &user->Width); p += 2;
	BYTEtoSHORT(p, &user->Height); p += 2;
	size = USER_PHOTO_RECORD_SZ - 12;
	memcpy(user->JPEGBuffer, p, size); p += size;
	user->JPEGSize = size;
	return p - (unsigned char *)buf;
}

int userValidatePhoto(FP_USER *user)
{
	if(!user->QuantityLevel || user->QuantityLevel > 5 || user->Width != 120 || user->Height != 160) return 0;
	return 1;
}

void userClearAccessRights(FP_USER *user)
{
	user->DefaultTimeRange = 1;		// Always
	user->Count = 0;
}

int userEncodeAccessRights(FP_USER *user, void *buf)
{
	unsigned char	*p;
	int		size, i;

	p = (unsigned char *)buf;
	memset(p, 0xff, 4); p += 4;
	*p++ = user->DefaultTimeRange;
	size = user->Count << 1;
	memcpy(p, user->AccessRules, size);
	size = p - (unsigned char *)buf;
	if(size < USER_AR_BODY_SZ) memset(p, 0xff, USER_AR_BODY_SZ-size);
cprintf("Encode User=%ld D=%d Rules=%d [", user->ID, (int)user->DefaultTimeRange, (int)user->Count);
for(i = 0;i < user->Count;i++) cprintf("%d,%d ", (int)user->AccessRules[i][0], (int)user->AccessRules[i][1]);
cprintf("]\n");
	return USER_AR_BODY_SZ;
}

int userDecodeAccessRights(FP_USER *user, void *buf)
{
	unsigned char	*p, c;
	int		i;

	p = (unsigned char *)buf;
	p += 4;
	user->DefaultTimeRange = *p++;
	for(i = 0;i < 27;i++) {
		c = *p++;
		if(c == 0xff) break;
		user->AccessRules[i][0] = c;
		user->AccessRules[i][1] = *p++;
	}
	user->Count = i;
cprintf("Decode User=%ld D=%d Rules=%d [", user->ID, (int)user->DefaultTimeRange, (int)user->Count);
for(i = 0;i < user->Count;i++) cprintf("%d,%d ", (int)user->AccessRules[i][0], (int)user->AccessRules[i][1]);
cprintf("]\n");
	return USER_AR_BODY_SZ;
}

int userValidateAccessRights(FP_USER *user)
{
	int		i, rval;

	if(user->DefaultTimeRange == 0xff) return 0;
	rval = 0;
	for(i = 0;i < user->Count;i++) {
		rval = aruleValidate(user->AccessRules[i]); break;
		if(!rval) break;
	}
	return rval;
}
