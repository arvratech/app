#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "syscfg.h"
#include "fsuser.h"


// Return	0=Non-Matched  1=Matched 
int userVerifyMatchPIN(FP_USER *user, unsigned char *Data)
{
	int		rval;
	
	if(!n_memcmp(Data, user->PIN, 4)) rval = 1;
	else	rval = 0;
	return rval;
}

// Return	0=Non-Matched  1=Matched 
int userVerifyMatchCard(FP_USER *user, unsigned char *Data)
{
	int		rval;
	
	if(!n_memcmp(Data, user->CardData, 9)) rval = 1;
//if(user->CardData[0] && !strcmp(data+3, user->CardData+3)) rval = 1;	// Facility
	else	rval = 0;
	return rval;
}

// Return	-1=Error  0=Non-Matched  1=Matched 
int userIdentifyMatchCard(FP_USER *user, unsigned char *Data)
{
	return userfsGetCardData(user, Data);
}

#include "fpapi_new.h"

// -1:Error 0:Non-Matched 1:Matched 
int userVerifyMatchFP(FP_USER *user, unsigned char *tmpl)
{
	unsigned long	fpId;
	int		rval, securityLevel;
	
	securityLevel = user->FPSecurityLevel;
	if(securityLevel <= 0) securityLevel = sys_cfg->FPSecurityLevel;
	userID2FPID(user->ID, &fpId);
	rval = sfpVerifyMatch(fpId, tmpl, securityLevel);
	if(!rval) rval = sfpVerifyMatch(fpId+1, tmpl, securityLevel);
	return rval;
}

// -2:Error -1:Not-Initialized 0:Non-Matched 1:Matched
int userIdentifyMatchFP(FP_USER *user, unsigned char *tmpl)
{
	unsigned long	fpId;
	int		rval, securityLevel;
//unsigned long tm1, tm2;

	// Bug fix 2012.1.5 	
	//securityLevel = user->FPSecurityLevel;
	//if(securityLevel <= 0) securityLevel = sys_cfg->FPSecurityLevel;
	securityLevel = sys_cfg->FPSecurityLevel;
//tm1 = TICK_TIMER;
	rval = sfpIdentifyMatch(tmpl, securityLevel, &fpId);
//tm2 = TICK_TIMER; cprintf("2: %d\n", tm2-tm1);
	if(rval > 0) {
		if(fpId > 199999998) {	// 99999999*2
cprintf("Unknown FPID: %ld 0x%08x\n", fpId, fpId);
			rval = 0;
		} else {
			userFPID2ID(fpId, &user->ID);
			rval = userfsGet(user);
		}
	}
	return rval;
}
