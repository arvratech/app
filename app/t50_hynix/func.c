#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "tacfg.h"
#include "sche.h"
#include "func.h"


int funcResetState(void)
{
	unsigned char	ctm[8];
	int		i, FuncState;
	
	rtcGetTime(ctm);
#ifdef _TA_EXT
	i = 2;
#else
	for(i = 0;i < 2;i++)
		if(taItemIsEnable(i+1) && tmsValidateEx(sys_cfg->InOutTimeSection[i]) && tmsBoundEx(sys_cfg->InOutTimeSection[i], ctm)) {
			FuncState = i + 1;		// 1:Ãâ±Ù 2:Åð±Ù
			break;
		}
#endif
	if(i >= 2) FuncState = 0;
	return FuncState;
}

int funcSetCode(int FuncState, int FuncKey)
{
	int		val, idx, FuncCode;

#ifdef _TA_EXT
	FuncCode = FuncKey - 1; if(FuncCode < 0) FuncCode = 0;
#else
	if(FuncKey <= 1 && (FuncState == 1 || FuncState == 2)) FuncCode = FuncState;	// Ãâ±Ù,Åð±Ù
	else {
		FuncCode = FuncKey - 1; if(FuncCode < 0) FuncCode = 0;
	}
#endif
	return FuncCode;
} 

extern unsigned char gFuncState, gFuncKey, gFuncTimer;

int GetFuncState(void)
{
	return (int)gFuncState;
}

int AuthIsEnable(void)
{
	return 1;
}

void FuncKeyInitialize(void)
{
	if(gFuncState == 1 || gFuncState == 2) gFuncKey = gFuncState + 1;
	else	gFuncKey = 1;
	gFuncTimer = 0;
}

void FuncKeyReset(void)
{
	if(gFuncState == 1 || gFuncState == 2 || ta_cfg->funcKeyTimer) FuncKeyInitialize();
	else	gFuncTimer = 0;
}

void InitOperationMode(void)
{
	gFuncState = funcResetState();
	FuncKeyInitialize();
}

int IsMealsMode(void)
{
	return 0;
}

int IsArmMode(void)
{
//	if(sys_cfg->extData[5] & 0x01) return 1;
//	else	return 0;
	return 0;
}

int IsUseLocalIO(void)
{
//	if(sys_cfg->OtherOption & USE_IOMAN) return 0;
//	else	return 1;
	return 1;
}

