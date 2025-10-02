#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "unitcfg.h"
#include "almcfg.h"


void actSetDefault(ALARM_ACTION_CFG *cfg)
{
	int		i;

	cfg->AlarmOutput = 0xff;
	for(i = 0;i < MAX_OUTPUT_COMMAND_SZ;i++) cfg->OutputCommands[i] = 0xff;	// OutputCommand:255=Null
}

int actEncode(ALARM_ACTION_CFG *cfg, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	*p++ = cfg->AlarmOutput;
	for(i = 0;i < MAX_OUTPUT_COMMAND_SZ;i++) *p++ = cfg->OutputCommands[i];
	return p - (unsigned char *)buf;
}

int actDecode(ALARM_ACTION_CFG *cfg, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	cfg->AlarmOutput = *p++;
	for(i = 0;i < MAX_OUTPUT_COMMAND_SZ;i++) cfg->OutputCommands[i] = *p++;
	return p - (unsigned char *)buf;
}

int actValidate(ALARM_ACTION_CFG *cfg)
{
	return 1;
}

int actGetCodedSize(void)
{
	return 13;
}

int ocEncodeID(long nID, void *buf)
{
	IDtoPACK1(nID, buf);
	return 1;
}

int ocDecodeID(long *pID, void *buf)
{
	PACK1toID(buf, pID);
	return 1;
}

int ocValidateID(long nID)
{
	if(nID >= 0 && nID < MAX_OC_SZ) return 1;
	else	return 0;
}

void ocSetDefault(OUTPUT_COMMAND_CFG *cfg)
{
	int		i;
	
	cfg->RepeatCount = 0;
	for(i = 0;i < MAX_OUTPUT_CADENCE_SZ;i++) {
		cfg->OutputCadences[i][0] = 0;		//OnTime
		cfg->OutputCadences[i][1] = 0;		//OffTime
	}
}

int ocEncode(OUTPUT_COMMAND_CFG *cfg, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	*p++ = cfg->RepeatCount;
	for(i = 0;i < MAX_OUTPUT_CADENCE_SZ;i++) {
		*p++ = cfg->OutputCadences[i][0];	//OnTime
		*p++ = cfg->OutputCadences[i][1];	//OffTime
	}
	return p - (unsigned char *)buf;
}

int ocDecode(OUTPUT_COMMAND_CFG *cfg, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	cfg->RepeatCount = *p++;
	for(i = 0;i < MAX_OUTPUT_CADENCE_SZ;i++) {
		cfg->OutputCadences[i][0] = *p++;
		cfg->OutputCadences[i][1] = *p++;
	}
	return p - (unsigned char *)buf;
}

int ocValidate(OUTPUT_COMMAND_CFG *cfg)
{
	return 1;
}
