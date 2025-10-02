#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "ble.h"


void bleSetDefault(BLE_CFG *cfg)
{
	unsigned char	*p;

	cfg->config			= 0x00;
	cfg->min_rssi		= -90;
	cfg->interval		= 600;
	cfg->reserve[0]		= 0x00;
	cfg->reserve[1]		= 0x00;
	p = cfg->macFilters;
	*p++ = 0x00; *p++ = 0x07; *p++ = 0x79;
	*p++ = 0x00; *p++ = 0x0d; *p++ = 0x6f;
	*p++ = 0x08; *p++ = 0x6b; *p++ = 0xd7;
	*p++ = 0x90; *p++ = 0xfd; *p++ = 0x9f;
	*p++ = 0xd0; *p++ = 0xcf; *p++ = 0x5e;
	memset(p, 0xff, 15);
}

int bleEncode(BLE_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cfg->config;
	*p++ = cfg->min_rssi;
	SHORTtoBYTE(cfg->interval, p); p += 2;
	*p++ = cfg->reserve[0];
	*p++ = cfg->reserve[1];
	memcpy(p, cfg->macFilters, 30); p += 30;
//printf("bleEncode=[%02x %d %d %02x-%02x-%02x]\n", (int)cfg->config, (int)cfg->min_rssi, (int)cfg->interval, (int)cfg->macFilters[0],  (int)cfg->macFilters[1],  (int)cfg->macFilters[2]);
	return p - (unsigned char *)buf;
}

int bleDecode(BLE_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cfg->config			= *p++;
	cfg->min_rssi		= *p++;
	BYTEtoSHORT(p, &cfg->interval); p += 2;
	cfg->reserve[0]		= *p++;
	cfg->reserve[1]		= *p++;
	memcpy(cfg->macFilters, p, 30); p += 30;
//printf("bleDecode=[%02x %d %d %02x-%02x-%02x]\n", (int)cfg->config, (int)cfg->min_rssi, (int)cfg->interval, (int)cfg->macFilters[0],  (int)cfg->macFilters[1],  (int)cfg->macFilters[2]);
	return p - (unsigned char *)buf;
}

int bleValidate(BLE_CFG *cfg)
{
	return 1;
}

int bleConfig(void *self)
{
	return (int)sys_cfg->ble.config;
}

BOOL bleEnable(void *self)
{
	if(sys_cfg->ble.config & 0x80) return TRUE;
	else	return FALSE;
}

void bleSetEnable(void *self, BOOL enable)
{
	if(enable) sys_cfg->ble.config |= 0x80;
	else	   sys_cfg->ble.config &= 0x7f;
}

BOOL bleEnableBeaconScan(void *self)
{
	if(sys_cfg->ble.config & 0x40) return TRUE;
	else	return FALSE;
}

void bleSetEnableBeaconScan(void *self, BOOL enable)
{
	if(enable) sys_cfg->ble.config |= 0x40;
	else	   sys_cfg->ble.config &= 0xbf;
}

int bleMinBeaconSignal(void *self)
{
	return (int)sys_cfg->ble.min_rssi;
}

void bleSetMinBeaconSignal(void *self, int signal)
{
	sys_cfg->ble.min_rssi = signal;
}

int bleBeaconIntervalTime(void *self)
{
	return (int)sys_cfg->ble.interval;
}

void bleSetBeaconIntervalTime(void *self, int time)
{
	sys_cfg->ble.interval = time;
}

unsigned char *bleGetMacFilter(void *self, int index)
{
	return sys_cfg->ble.macFilters + index*3;
}

void bleSetMacFilter(void *self, int index, unsigned char *data)
{
	memcpy(sys_cfg->ble.macFilters + index*3, data, 3);
}

unsigned char nullmac[4] = { 0xff, 0xff, 0xff, 0xff };

void bleGetMacFilterName(void *self, int index, char *name)
{
	unsigned char	*p;
	int		len;

	p = bleGetMacFilter(self, index);
	if(memcmp(p, nullmac, 3)) bin2hexstr(p, 3, name);
	else	name[0] = 0;
	stoupper(name);
}

char *bleMacFilterName(void *self, int index)
{
	unsigned char	*p;
	int		len;

	p = bleGetMacFilter(self, index);
	if(memcmp(p, nullmac, 3)) bin2hexstr(p, 3, gText);
	else	gText[0] = 0;
	stoupper(gText);
	return gText;
}

void bleSetMacFilterName(void *self, int index, char *name)
{
	unsigned char	data[20];
	int		len;

	len = hexstr2bin(name, data);
	if(len != 3) memcpy(data, nullmac, 3); 	
	bleSetMacFilter(self, index, data);
}

