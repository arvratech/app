#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "sche.h"
#include "syscfg.h"
#include "user.h"


#include "usercfg.c"


void userEncodeTransaction(FP_USER *user, void *buf)
{
	unsigned char	*p;
	unsigned long	lVal;

	p = (unsigned char *)buf;
	datetime2longtime(user->accessTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	*p++ = user->accessPoint;
	*p++ = user->zoneTo;
	*p++ = user->passback;
	*p++ = user->funcCode;
	memset(p, 0xff, 4); p += 4;
}
	
void userDecodeTransaction(FP_USER *user, void *buf)
{
	unsigned char	*p;
	unsigned long	lVal;

	p = (unsigned char *)buf;
	BYTEtoLONG(p, &lVal); p += 4; longtime2datetime(lVal, user->accessTime);
	user->accessPoint	= *p++;
	user->zoneTo		= *p++;
	user->passback		= *p++;
	user->funcCode		= *p++;
}

void userId2Fpid(long id, unsigned long *pFpId)
{
	unsigned long	fpId;

	fpId = id;
	fpId <<= 1;
	*pFpId = fpId;
}

void userFpid2Id(unsigned long fpId, long *pId)
{
	*pId = fpId >> 1;
}

int userFPData1IsValid(FP_USER *user)
{
	if(user->fpFlag & FP_DATA_1_EXIST) return 1;
	else	return 0;
}

int userFPData2IsValid(FP_USER *user)
{
	if(user->fpFlag & FP_DATA_2_EXIST) return 1;
	else	return 0;
}

int userGetFPDataCount(FP_USER *user)
{
	int		count;
	
	count = 0;
	if(user->fpFlag & FP_DATA_1_EXIST) count++;
	if(user->fpFlag & FP_DATA_2_EXIST) count++;
	return count;		
}

void userSetFPData1(FP_USER *user)
{
	user->fpFlag |= FP_DATA_1_EXIST;
}

void userSetFPData2(FP_USER *user)
{
	user->fpFlag |= FP_DATA_2_EXIST;
}

long userGetNewID(long startID, long minID, long maxID)
{
	FP_USER	*user, _user;
	long	id;

	user = &_user;
	if(startID <= minID || startID > maxID) startID = minID;
	id = startID;
	while(1) {
		user->id = id;
//printf("NewUserID [%s] %d\n", user->id, userfsGet(user));	 
		//if(userfsGet(user) <= 0) return id;
		if(userfsGet(user) <= 0) return id;
		id++;
		if(id > maxID) id = minID;
//		if(!(id & 0xf)) taskYield();
	} while(id != startID) ;
	return 0L;
}

int userIsFP(FP_USER *user)
{
	int		mode;

	mode = user->accessMode;
	if((mode & USER_FP) || (mode & USER_FP_PIN) || (mode & USER_CARD_FP) || (mode & USER_CARD_FP_PIN) ) return 1;
	else	return 0;
}

int userIsCard(FP_USER *user)
{
	int		mode;

	mode = user->accessMode;
	if((mode & USER_CARD) || (mode & USER_CARD_FP) || (mode & USER_CARD_PIN) || (mode & USER_CARD_FP_PIN)) return 1;
	else	return 0;
}

int userIsPIN(FP_USER *user)
{
	int		mode;

	mode = user->accessMode;
	if((mode & USER_PIN) || (mode & USER_CARD_PIN) || (mode & USER_CARD_FP_PIN) || (mode & USER_FP_PIN)) return 1;
	else	return 0;
}

void userCopy(FP_USER *userD, FP_USER *userS)
{
	memcpy(userD, userS, sizeof(FP_USER));	
}

int userCompare(FP_USER *user1, FP_USER *user2)
{
	unsigned char	buf[128];

	userEncodeId(user1->id, buf);
	userEncode(user1, buf+3);
	userEncodeId(user2->id, buf+64);
	userEncode(user2, buf+67);
	if(!memcmp(buf, buf+64, 31)) return 0;
	else	return -1;
}

void userCopyHead(FP_USER *userD, FP_USER *userS)
{
	memcpy(userD, userS, FP_USER_HEAD_SZ);
}

void userPrint(FP_USER *user)
{
#ifdef BK_DEBUG	
	printf("UserID=[%s]\n", user->UserID);
	printf("AccessMode=0x%02x AcessRights=%d Date=%02d%02d%02d-%02d%02d%02d\n", (int)user->AccessMode, (int)user->AccessRights, (int)user->ActivateDate[0], (int)user->ActivateDate[1], (int)user->ActivateDate[2],
												(int)user->ExpireDate[0], (int)user->ExpireDate[1], (int)user->ExpireDate[2]);
	printf("PIN=[%s] CardData=[%s] CardStatus=%d SL=%d Idx[0]=%d Idx[1]=%d\n",
						 user->PIN, user->CardData, (int)user->CardStatus, (int)user->FPSecurityLevel, (int)user->FPIndex[0], (int)user->FPIndex[1]);
#endif
}

// Return	0=Non-Matched  1=Matched 
int userVerifyMatchPin(FP_USER *user, unsigned char *data)
{
	int		rval;
	
	if(!memcmp(data, user->pin, 4)) rval = 1;
	else	rval = 0;
	return rval;
}

// Return	0=Non-Matched  1=Matched 
int userVerifyMatchCard(FP_USER *user, unsigned char *data)
{
	int		rval;
	
	if(!memcmp(data, user->cardData, 9)) rval = 1;
	else	rval = 0;
	return rval;
}

// Return	-1=Error  0=Non-Matched  1=Matched 
int userIdentifyMatchCard(FP_USER *user, unsigned char *data)
{
	return userfsGetCardData(user, data);
}

