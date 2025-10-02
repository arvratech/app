#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "syscfg.h"
#include "oc.h"


void ocSetDefault(OUTPUT_COMMAND *oc)
{
	oc->repeatCount	= 0;
	oc->onTime			= 0;
	oc->offTime		= 0;
}

int ocEncode(OUTPUT_COMMAND *oc, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = oc->repeatCount;
	*p++ = oc->onTime;
	*p++ = oc->offTime;
 	return p - (unsigned char *)buf;
}

int ocDecode(OUTPUT_COMMAND *oc, void *buf)
{
	unsigned char	*p, c;

	p = (unsigned char *)buf;
	oc->repeatCount	= *p++;
	oc->onTime		= *p++;
	oc->offTime		= *p++;
 	return p - (unsigned char *)buf;
}

int ocValidate(OUTPUT_COMMAND *oc)
{
	return 1;
}

int ocGetEnable(int ocID)
{
	int		enable;

	if(ocID < 2 || sys_cfg->outputCommands[ocID-2].onTime) enable = 1;
	else	enable = 0;
	return enable;
} 

void ocSetEnableDefault(int ocID)
{
	OUTPUT_COMMAND		*cfg;

	if(ocID >= 2) {
		cfg = &sys_cfg->outputCommands[ocID-2];
		cfg->repeatCount	= 1;
		cfg->onTime			= 10;
		cfg->offTime		= 10;
	}
}

int ocGetRepeatCount(int ocID)
{
	int		val;

	if(ocID >= 2) val = sys_cfg->outputCommands[ocID-2].repeatCount;
	else	val = -1;
	return val;
}

void ocSetRepeatCount(int ocID, int repeatCount)
{
	if(ocID >= 2) sys_cfg->outputCommands[ocID-2].repeatCount = repeatCount;
}

int ocGetOnTime(int ocID)
{
	int		val;

	if(ocID >= 2) val = sys_cfg->outputCommands[ocID-2].onTime;
	else	val = -1;
	return val;
}

void ocSetOnTime(int ocID, int onTime)
{
	if(ocID >= 2) sys_cfg->outputCommands[ocID-2].onTime = onTime;
}

int ocGetOffTime(int ocID)
{
	int		val;

	if(ocID >= 2) val = sys_cfg->outputCommands[ocID-2].offTime;
	else	val = -1;
	return val;
}

void ocSetOffTime(int ocID, int offTime)
{
	if(ocID >= 2) sys_cfg->outputCommands[ocID-2].offTime = offTime;
}

int ocsGetIDList(unsigned char *idList)
{
	int		i, count;

	idList[0] = 0;
	idList[1] = 0;
	count = 2;
	for(i = 0;i < MAX_OC_SZ-1;i++)
		if(sys_cfg->outputCommands[i].onTime) {
			idList[count] = i + 2;
			count++;
		}
	return count;
}

int ocsGetIndex(unsigned char *idList, int count, int ocID)
{
	int		i;

	for(i = 0;i < count;i++)
		if(idList[i] == ocID) break;
	if(i >= count) i = -1;
	return i;
}
