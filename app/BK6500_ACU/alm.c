#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "unit.h"
#include "sche.h"
#include "evt.h"
#include "event.h"
#include "acad.h"
#include "iodev.h"
#include "bio.h"
#include "user.h"
#include "cr.h"
#include "alm.h"

#include "almcfg.c"


static ALARM_ACTION	*acts;


void actsInit(ALARM_ACTION *g_acts, int size)
{
	ALARM_ACTION	*act;
	int		i;

	actsSet(g_acts);
	for(i = 0, act = g_acts;i < size;i++, act++) {
		act->ID		= i;
		act->cfg	= &sys_cfg->AlarmActions[i];
		actReset(i);
	}
}

void actsSet(ALARM_ACTION *g_acts)
{
	acts = g_acts;
}

ALARM_ACTION *actsGetAt(int ID)
{
	return &acts[ID];
}

void actReset(int ID)
{
	ALARM_ACTION	*act;

	act = &acts[ID];
	act->PresentValue = 0xff;
}

void actSetPresentValue(int ID, int PresentValue)
{
	ALARM_ACTION	*act;
	int		val, boID;

	act = actsGetAt(ID);
	act->PresentValue = (unsigned char)PresentValue;
	act->Count = 0;
	if(act->PresentValue >= MAX_OUTPUT_COMMAND_SZ) act->OutputCommand = 0;
	else	act->OutputCommand = act->cfg->OutputCommands[PresentValue];
	boID = actGetAlarmOutput(ID);
	if(act->OutputCommand == 0 || act->OutputCommand == 1) {
cprintf("%ld actSetPresentValue: ID=%d PV=%d ocID=%d boID=%d \n", DS_TIMER, ID, PresentValue, (int)act->OutputCommand, boID);
		boSetPresentValue(boID, (int)act->OutputCommand);
		act->OutputTimeout = 0;
	} else {
cprintf("%ld actSetPresentValue: ID=%d PV=1 ocID=%d boID=%d \n", DS_TIMER, ID, (int)act->OutputCommand, boID);
		val = sys_cfg->OutputCommands[act->OutputCommand-2].OutputCadences[0][0];
//		if(!val) act->OutputTimeout = 0;
		boSetPresentValue(boID, 1);
		act->OutputTimeout = rtcDS2Tick(val);
		act->OutputTimer = TICK_TIMER;
		act->Loop = 0;
	}
}

int actGetOutputCommand(int ID, int Index)
{
	return (int)sys_cfg->AlarmActions[ID].OutputCommands[Index];
}

void actProcess(int ID)
{
	ALARM_ACTION	*act;
	OUTPUT_COMMAND_CFG	*oc_cfg;
	int		val, cnt, boID;

	act = actsGetAt(ID);
	if(act->OutputTimeout && (TICK_TIMER-act->OutputTimer) >= act->OutputTimeout) {
		oc_cfg = &sys_cfg->OutputCommands[act->OutputCommand-2];
		act->Loop++;
		boID = actGetAlarmOutput(ID);
		if(act->Loop & 1) {	// Off Time
cprintf("%ld actProcess: ID=%d OFF\n", DS_TIMER, ID);
			boSetPresentValue(boID, 0);
			val = oc_cfg->OutputCadences[act->Loop>>1][1];
			if(val) {
				act->OutputTimeout = rtcDS2Tick(val); act->OutputTimer = TICK_TIMER;
			} else	act->OutputTimeout = 0;
		} else {			// On Time
			cnt = act->Loop >> 1;
			if(cnt < 4) val = oc_cfg->OutputCadences[cnt][0];
			else	val = 0;
			if(!val) {
				act->Loop = 0; act->Count++;
				val = oc_cfg->OutputCadences[0][0];
			}
			if(val && (!oc_cfg->RepeatCount || act->Count < oc_cfg->RepeatCount)) {
cprintf("%ld actProcess: ID=%d ON\n", DS_TIMER, ID);
				boSetPresentValue(boID, 1);
				act->OutputTimeout = rtcDS2Tick(val); act->OutputTimer = TICK_TIMER;
			} else	act->OutputTimeout = 0;
		}
	}
}

int ocExist(long nID)
{
	int		enable;

	enable = 0;
	if(nID >= 0 && nID < MAX_OC_SZ) {
		if(nID < 2) enable = 1;
		else	enable = sys_cfg->OutputCommands[nID-2].OutputCadences[0][0];
	}
	return enable;
}

int ocGetEncoded(long nID, unsigned char *buf)
{
	OUTPUT_COMMAND_CFG	*oc_cfg;
	int		rval;

	oc_cfg = &sys_cfg->OutputCommands[nID-2];
	if(!oc_cfg->OutputCadences[0][0]) rval = 0;
	else	rval = ocEncode(oc_cfg, buf);
	return rval;
}

int ocAddEncoded(long nID, unsigned char *buf)
{
	OUTPUT_COMMAND_CFG	*oc_cfg, _tmp_cfg;
	unsigned char	tmpbuf[32];
	int		rval, size;

	oc_cfg = &sys_cfg->OutputCommands[nID-2];
	size = ocEncode(oc_cfg, tmpbuf);
	if(!n_memcmp(buf, tmpbuf, size)) rval = 1;
	else {
		memcpy(&_tmp_cfg, oc_cfg, sizeof(OUTPUT_COMMAND_CFG));
		ocDecode(oc_cfg, buf);
		rval = syscfgWrite(sys_cfg);
		if(!rval) rval = 1;
		else	memcpy(oc_cfg, &_tmp_cfg, sizeof(OUTPUT_COMMAND_CFG)); 
	}
	return rval;
}

int ocRemove(long nID)
{
	OUTPUT_COMMAND_CFG	*oc_cfg, _tmp_cfg;
	int		rval;

	oc_cfg = &sys_cfg->OutputCommands[nID-2];
	if(!oc_cfg->OutputCadences[0][0]) rval = 0;
	else {
		memcpy(&_tmp_cfg, oc_cfg, sizeof(OUTPUT_COMMAND_CFG));
		ocSetDefault(oc_cfg);
		rval = syscfgWrite(sys_cfg);
		if(!rval) rval = 1;
		else	memcpy(oc_cfg, &_tmp_cfg, sizeof(OUTPUT_COMMAND_CFG)); 
	}
	return rval;
}

int ocRemoveAll(void)
{
	OUTPUT_COMMAND_CFG	_tmp_cfg[MAX_OC_SZ-2];
	int		i, rval, size;

	size = sizeof(OUTPUT_COMMAND_CFG) * (MAX_OC_SZ-2);
	memcpy(_tmp_cfg, sys_cfg->OutputCommands, size);
	for(i = 2;i < MAX_OC_SZ;i++)
		ocSetDefault(&sys_cfg->OutputCommands[i-2]);
	if(!n_memcmp(sys_cfg->OutputCommands, _tmp_cfg, size)) rval = 1;
	else {
		rval = syscfgWrite(sys_cfg);
		if(!rval) rval = 1;
		else	memcpy(sys_cfg->OutputCommands, _tmp_cfg, size);
	}
	return rval;
}
