#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "prim.h"
#include "sche.h"
#include "user.h"


void userSetDefault(FP_USER *user)
{
	memset(user->activateDate, 0, 3);
	memset(user->expireDate, 0, 3);
	user->accessMode		= USER_FP;
	user->accessRights		= 1;		// All-Always
	user->pin[0] = user->pin[1] = 0x11;
	user->pin[2] = user->pin[3] = 0xff;
	user->cardData[0]		= 0; memset(user->cardData+1, 0xff, 8);
	user->cardStatus		= 1;
	user->fpSecurityLevel	= 0;
	user->fpFlag			= 0;
	userClearEx(user);
	userClearPhoto(user);
	userClearAccessRights(user);
}

int userEncode(FP_USER *user, void *buf)
{
	unsigned char	*p;
int		i;

	p = (unsigned char *)buf;
	bin2bcd(user->activateDate, 3, p); p += 3;
	bin2bcd(user->expireDate, 3, p); p += 3;
	*p++ = user->accessMode;
	IDtoPACK3(user->accessRights, p); p += 3;
	memset(p, 0xff, 3); p += 3;
	memcpy(p, user->pin, 4); p += 4;
	memcpy(p, user->cardData, 9); p += 9;
//printf("encode: id=%d am=%d ar=%d cs=%d sl=%d ", user->id, (int)user->accessMode, user->accessRights, (int)user->cardStatus, (int)user->fpSecurityLevel); 
//printf("encode: card=[%02x", (int)user->cardData[0]); for(i = 1;i < 9;i++) printf("-%02x", (int)user->cardData[i]); printf("]\n");
	*p++ = user->cardStatus;
	*p++ = user->fpSecurityLevel;
	return p - (unsigned char *)buf;
}

int userDecode(FP_USER *user, void *buf)
{
	unsigned char	*p;
int	i;

	p = (unsigned char *)buf;
	bcd2bin(p, 3, user->activateDate); p += 3;
	bcd2bin(p, 3, user->expireDate); p += 3;
	user->accessMode		= *p++;
	PACK3toID(p, &user->accessRights); p += 3;
	p += 3;
	memcpy(user->pin, p, 4); p += 4;
	memcpy(user->cardData, p, 9); p += 9;
	user->cardStatus		= *p++;
	user->fpSecurityLevel	= *p++;
//printf("decode: id=%d am=%d ar=%d cs=%d sl=%d ", user->id, (int)user->accessMode, user->accessRights, (int)user->cardStatus, (int)user->fpSecurityLevel); 
//printf("decode: card=[%02x", (int)user->cardData[0]); for(i = 1;i < 9;i++) printf("-%02x", (int)user->cardData[i]); printf("]\n");
	return p - (unsigned char *)buf;
}

int userValidate(FP_USER *user)
{
//cprintf("AccessMode=%02x CS=%d SL=%d\n", (int)user->accessMode, (int)user->cardStatus, (int)user->fpSecurityLevel);
	if(!(user->accessMode & 0x3f) || user->accessMode & 0xc0) return 0;
	if(user->cardStatus > 2 || user->fpSecurityLevel > 9) return 0;
	return 1;
}

long userId(void *self)
{
	FP_USER		*user = self;

	return user->id;
}

void userSetId(void *self, long id)
{
	FP_USER		*user = self;

	user->id = id;
}

unsigned char *userCardData(void *self)
{
	FP_USER		*user = self;

	return user->cardData;
}

void userSetCardData(void *self, unsigned char *cardData)
{
	FP_USER		*user = self;

	memcpy(user->cardData, cardData, 9);
}

unsigned char *userPin(void *self)
{
	FP_USER		*user = self;

	return user->pin;
}

void userSetPin(void *self, unsigned char *pin)
{
	FP_USER		*user = self;

	memcpy(user->pin, pin, 4);
}

void userEncodeId(long id, void *buf)
{
	IDtoPACK3(id, (unsigned char *)buf);	
}

void userEncodeFpid(unsigned long fpid, void *buf)
{
	memcpy(buf, &fpid, 4);
}

void userDecodeId(long *pId, void *buf)
{
	PACK3toID((unsigned char *)buf, pId);
}

void userDecodeFpid(unsigned long *pFpId, void *buf)
{
	memcpy(pFpId, buf, 4);
}

int userValidateId(long id)
{
	if(id > 0 && id < 0xffffff) return 1;
	else	return 0;
}

void userClearEx(FP_USER *user)
{
	user->userName[0]	= 0;
	user->userExternalId[0] = 0;
}

int userEncodeEx(FP_USER *user, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	memcpy_pad(p, user->userName, 20); p += 20;
	string2bcd(user->userExternalId, 14, p); p += 7;
	*p++ = 0xff;
	return p - (unsigned char *)buf;
}

int userDecodeEx(FP_USER *user, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	memcpy_chop(user->userName, p, 20); p += 20;
	bcd2string(p, 14, user->userExternalId); p += 7;
	p++;
	return p - (unsigned char *)buf;
}

int userValidateEx(FP_USER *user)
{
	if(user->userExternalId[0] && isdigits(user->userExternalId) <= 0) return 0;
	return 1;
}

int userIsNullEx(FP_USER *user)
{
	if(user->userName[0] || user->userExternalId[0]) return 0;
	return 1;
}

void userClearPhoto(FP_USER *user)
{
	user->width = user->height = 0;
	user->quantityLevel = 0;
	user->jpegSize = 0;
	user->jpegBuffer = NULL;
}

int userEncodePhoto(FP_USER *user, void *buf)
{
	unsigned char	*p;
	int		size, body_sz;

	p = (unsigned char *)buf;
	memset(p, 0xff, 3); p += 3;
	*p++ = user->quantityLevel; 
	SHORTtoBYTE(user->width, p); p += 2;
	SHORTtoBYTE(user->height, p); p += 2;
	memcpy(p, user->jpegBuffer, (int)user->jpegSize); p += user->jpegSize;
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
	user->quantityLevel = *p++;
	BYTEtoSHORT(p, &user->width); p += 2;
	BYTEtoSHORT(p, &user->height); p += 2;
	size = USER_PHOTO_RECORD_SZ - 12;
	memcpy(user->jpegBuffer, p, size); p += size;
	user->jpegSize = size;
	return p - (unsigned char *)buf;
}

int userValidatePhoto(FP_USER *user)
{
	if(!user->quantityLevel || user->quantityLevel > 5 || user->width != 120 || user->height != 160) return 0;
	return 1;
}

void userClearAccessRights(FP_USER *user)
{
	user->defaultTimeRange = 1;		// Always
	user->count = 0;
}

int userEncodeAccessRights(FP_USER *user, void *buf)
{
	unsigned char	*p;
	int		size, i;

	p = (unsigned char *)buf;
	memset(p, 0xff, 4); p += 4;
	*p++ = user->defaultTimeRange;
	size = user->count << 1;
	memcpy(p, user->accessRules, size);
	size = p - (unsigned char *)buf;
	if(size < USER_AR_BODY_SZ) memset(p, 0xff, USER_AR_BODY_SZ-size);
printf("Encode User=%ld D=%d Rules=%d [", user->id, (int)user->defaultTimeRange, (int)user->count);
for(i = 0;i < user->count;i++) printf("%d,%d ", (int)user->accessRules[i][0], (int)user->accessRules[i][1]);
printf("]\n");
	return USER_AR_BODY_SZ;
}

int userDecodeAccessRights(FP_USER *user, void *buf)
{
	unsigned char	*p, c;
	int		i;

	p = (unsigned char *)buf;
	p += 4;
	user->defaultTimeRange = *p++;
	for(i = 0;i < 27;i++) {
		c = *p++;
		if(c == 0xff) break;
		user->accessRules[i][0] = c;
		user->accessRules[i][1] = *p++;
	}
	user->count = i;
printf("Decode User=%ld D=%d Rules=%d [", user->id, (int)user->defaultTimeRange, (int)user->count);
for(i = 0;i < user->count;i++) printf("%d,%d ", (int)user->accessRules[i][0], (int)user->accessRules[i][1]);
printf("]\n");
	return USER_AR_BODY_SZ;
}

int userValidateAccessRights(FP_USER *user)
{
/*
	int		i, rval;

	if(user->defaultTimeRange == 0xff) return 0;
	rval = 0;
	for(i = 0;i < user->count;i++) {
		rval = aruleValidate(user->accessRules[i]); break;
		if(!rval) break;
	}
	return rval;
*/
return 1;
}

