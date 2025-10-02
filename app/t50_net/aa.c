#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "msg.h"
#include "syscfg.h"
#include "dev.h"
#include "aa.h"

static ALARM_ACTION	_AlarmActions[MAX_AA_SZ];


void aasInit(void)
{
	ALARM_ACTION	*aa;
	int		i;

	for(i = 0, aa = _AlarmActions;i < MAX_AA_SZ;i++, aa++) {
		aa->id		= i;
		aa->cfg		= &sys_cfg->alarmActions[i];
	}
}

void *aasGet(int id)
{
	ALARM_ACTION	*aa;

	if(id >= 0 && id < MAX_AA_SZ) {
		aa = &_AlarmActions[id];
		if(!aa->cfg->enable) aa = NULL;
	} else	aa = NULL;
	return (void *)aa;
}

void *aasAt(int index)
{
	ALARM_ACTION	*aa;
	int		i, count;

	for(i = count = 0, aa = _AlarmActions;i < MAX_AA_SZ;i++, aa++)
		if(aa->cfg->enable)  {
			if(count == index) break;
			count++;
		}
	if(i >= MAX_AA_SZ) aa = NULL;
	return (void *)aa;
}

int aasIndexFor(int id)
{
	ALARM_ACTION	*aa;
	int		i, index;

	for(i = index = 0, aa = _AlarmActions;i < MAX_AA_SZ;i++, aa++)
		if(aa->cfg->enable)  {
			if(aa->id == id) break;
			index++;
		}
	if(i >= MAX_AA_SZ) index = -1;
	return index;
}

void *aasAdd(void *self)
{
	ALARM_ACTION	*aa = self;
	ALARM_ACTION	*aaf;
	int		i;

	for(i = 0, aaf = _AlarmActions;i < MAX_AA_SZ;i++, aaf++)
		if(!aaf->cfg->enable) break;
	if(i < MAX_AA_SZ) {
		memcpy(aaf->cfg, aa->cfg, sizeof(ALARM_ACTION_CFG));
		aaf->cfg->enable = 1;
	} else	aaf = NULL;
	return aaf;
}

void aasRemove(int id)
{
	ALARM_ACTION	*aa;

	if(id >= 0 && id < MAX_AA_SZ) {
		aa = aasGet(id);
		aaSetDefault(aa);
	}
}

void aasRemoveAt(int index)
{
	ALARM_ACTION	*aa;
	int		i, count;

	for(i = count = 0, aa = _AlarmActions;i < MAX_AA_SZ;i++, aa++)
		if(aa->cfg->enable)  {
			if(count == index) break;
			count++;
		}
	if(i < MAX_AA_SZ) aaSetDefault(aa);
}

int aasGetList(void *objs[])
{
	ALARM_ACTION	*aa;
	int		i, count;

	count = 0;
	for(i = count = 0;i < MAX_AA_SZ;i++) {
		aa = aasGet(i);
		if(aa) objs[count++] = aa;
	}
	return count;
}

int aasCount(void)
{
	ALARM_ACTION	*aa;
	int		i, count;

	for(i = count = 0, aa = _AlarmActions;i < MAX_AA_SZ;i++, aa++)
		if(aa->cfg->enable) count++;
	return count;
}

void _AaSetDefault(ALARM_ACTION_CFG *cfg)
{
	ACTION_COMMAND	*ac;
	int		i;

	cfg->enable		= 0;
	ac = cfg->actionCommands;
	for(i = 0;i < MAX_ACTION_COMMAND_SZ;i++, ac++)
		aaacSetEnable(ac, 0);
}

int _AaEncode(ALARM_ACTION_CFG *cfg, void *buf)
{
	ACTION_COMMAND	*ac;
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	*p++ = cfg->enable;
	ac = cfg->actionCommands;
	for(i = 0;i < MAX_ACTION_COMMAND_SZ;i++, ac++) {
		*p++ = ac->enable;
		*p++ = ac->onTime;
		*p++ = ac->offTime;
		*p++ = ac->repeatCount;
	}
 	return p - (unsigned char *)buf;
}

int _AaDecode(ALARM_ACTION_CFG *cfg, void *buf)
{
	ACTION_COMMAND	*ac;
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	cfg->enable		= *p++;
	ac = cfg->actionCommands;
	for(i = 0;i < MAX_ACTION_COMMAND_SZ;i++, ac++) {
		ac->enable		= *p++;
		ac->onTime		= *p++;
		ac->offTime		= *p++;
		ac->repeatCount	= *p++;
	}
 	return p - (unsigned char *)buf;
}

int _AaValidate(ALARM_ACTION_CFG *cfg)
{
	return 1;
}

int aaId(void *self)
{
	ALARM_ACTION	*aa = self;

	return (int)aa->id;
}

void aaSetDefault(void *self)
{
	ALARM_ACTION	*aa = self;

	_AaSetDefault(aa->cfg);
}

void aaCopy(void *d, void *s)
{
	ALARM_ACTION	*daa, *saa;

	daa = d; saa = s;
	if(saa->cfg) memcpy(daa->cfg, saa->cfg, sizeof(ALARM_ACTION_CFG));
	daa->id = saa->id;
}

void *aaActionCommandAtIndex(void *self, int index)
{
	ALARM_ACTION	*aa = self;

	return &aa->cfg->actionCommands[index];
}

int aaEnableAtIndex(void *self, int index)
{
	ALARM_ACTION	*aa = self;

	return (int)aa->cfg->actionCommands[index].enable;
}

void aaSetEnableAtIndex(void *self, int index, int enable)
{
	ALARM_ACTION	*aa = self;

	aa->cfg->actionCommands[index].enable = enable;
}

int aaOnTimeAtIndex(void *self, int index)
{
	ALARM_ACTION	*aa = self;

	return (int)aa->cfg->actionCommands[index].onTime;
}

void aaSetOnTimeAtIndex(void *self, int index, int onTime)
{
	ALARM_ACTION	*aa = self;

	aa->cfg->actionCommands[index].onTime = onTime;
}

int aaOffTimeAtIndex(void *self, int index)
{
	ALARM_ACTION	*aa = self;

	return (int)aa->cfg->actionCommands[index].offTime;
}

void aaSetOffTimeAtIndex(void *self, int index, int offTime)
{
	ALARM_ACTION	*aa = self;

	aa->cfg->actionCommands[index].offTime = offTime;
}

int aaRepeatCountAtIndex(void *self, int index)
{
	ALARM_ACTION	*aa = self;

	return (int)aa->cfg->actionCommands[index].repeatCount;
}

void aaSetRepeatCountAtIndex(void *self, int index, int repeatCount)
{
	ALARM_ACTION	*aa = self;

	aa->cfg->actionCommands[index].repeatCount = repeatCount;
}

int aaacEnable(void *self)
{
	ACTION_COMMAND	*ac = self;

	return (int)ac->enable;
}

void aaacSetEnable(void *self, int enable)
{
	ACTION_COMMAND	*ac = self;

	ac->enable = enable;
	if(ac->enable) {
		ac->onTime = ac->offTime = 10; ac->repeatCount = 1;
	} else {
		ac->onTime = ac->offTime = 0; ac->repeatCount = 0;
	}
}

int aaacOnTime(void *self)
{
	ACTION_COMMAND	*ac = self;

	return (int)ac->onTime;
}

void aaacSetOnTime(void *self, int onTime)
{
	ACTION_COMMAND	*ac = self;

	ac->onTime = onTime;
}

int aaacOffTime(void *self)
{
	ACTION_COMMAND	*ac = self;

	return (int)ac->offTime;
}

void aaacSetOffTime(void *self, int offTime)
{
	ACTION_COMMAND	*ac = self;

	ac->offTime = offTime;
}

int aaacRepeatCount(void *self)
{
	ACTION_COMMAND	*ac = self;

	return (int)ac->repeatCount;
}

void aaacSetRepeatCount(void *self, int repeatCount)
{
	ACTION_COMMAND	*ac = self;

	ac->repeatCount = repeatCount;
}

