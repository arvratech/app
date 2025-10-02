#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "msg.h"
#include "uv.h"
#include "dev.h"
#include "psu.h"

/*
 *  input(0 - 40v) 
 *       |
 *       >  470000 ohm 
 *       <
 *       | 
 *       +--------- output(0.0 - 3.35v)   output I = input V / (470000+43000)      
 *       |                                output V = I * R = input V / 513000 * 43000 = input V * 43 / 513 
 *       >   43000 ohm                    input V  = output V * 513 / 43
 *       <
 *       |
 */
static int	acVoltageNow, batVoltageNow;
static unsigned char	poeStatus, acAlarmState, batAlarmState, batStatus, batOnline, batCapacity;
static void (*_OnAcAlarmStateChanged)(void);
static void (*_OnBatAlarmStateChanged)(void);
static void (*_OnBatStatusChanged)(void);
static void (*_OnPoeStatusChanged)(void);
static uv_timer_t	_timerPsu;


void psuInit(void (*onAcAlarmStateChanged)(void), void (*onBatAlarmStateChanged)(void), void (*onBatStatusChanged)(void), void (*onPoeStatusChanged)(void))
{
	_OnAcAlarmStateChanged	= onAcAlarmStateChanged;
	_OnBatAlarmStateChanged	= onBatAlarmStateChanged;
	_OnBatStatusChanged		= onBatStatusChanged;
	_OnPoeStatusChanged		= onPoeStatusChanged;
	acVoltageNow = batVoltageNow = 0xff;
	poeStatus = 0xff;
	acAlarmState = batAlarmState = 0;
	uv_timer_init(uv_default_loop(), &_timerPsu);
}

int psuPoeStatus(void *self)
{
	return (int)poeStatus;
}

void PsuOnPoeStatusChanged(void *self, int status)
{
	int		oldPoeStatus;

	if(status) status = 1;
	oldPoeStatus = poeStatus;
	if(oldPoeStatus != status) {
		poeStatus = status;
printf("PoeStatus changed: %d\n", status);
		if(devSystemStatus(NULL) == SS_OPERATIONAL && _OnPoeStatusChanged) (*_OnPoeStatusChanged)();
	}
}

char *psuPoeStatusName(void *self)
{
	int		val;

	if(poeStatus) val = 5; else val = 0;
	return xmenu_bat_status[val];
}

// in mA
int psuAcVoltageNow(void *self)
{
	return acVoltageNow;
}

static void _OnPsuTimer(uv_timer_t *handle)
{
	acAlarmState = 0;
	if(devSystemStatus(NULL) == SS_OPERATIONAL && _OnAcAlarmStateChanged) (*_OnAcAlarmStateChanged)();
}


void PsuOnAcVoltageNowChanged(void *self, int volt)
{
	int		oldVolt;

	oldVolt = acVoltageNow;
	if(oldVolt != volt) {
		acVoltageNow = volt;
		if(!acAlarmState && volt < 8000) {
			acAlarmState = 1;
			if(devSystemStatus(NULL) == SS_OPERATIONAL && _OnAcAlarmStateChanged) (*_OnAcAlarmStateChanged)();
		} else if(acAlarmState && volt > 9000 && !uv_is_active((uv_handle_t *)&_timerPsu)) {
			uv_timer_start(&_timerPsu, _OnPsuTimer, 1000, 0);
		} else if(volt < 8000 && uv_is_active((uv_handle_t *)&_timerPsu)) {
			uv_timer_stop(&_timerPsu);
		}
	}
}

int psuAcAlarmState(void *self)
{
	return (int)acAlarmState;
}

// in mA
int psuBatVoltageNow(void *self)
{
	return batVoltageNow;
}

void PsuOnBatVoltageNowChanged(void *self, int volt)
{
	int		oldVolt, changed;

	oldVolt = batVoltageNow;
	if(oldVolt != volt) {
		batVoltageNow = volt;
		changed = 0;
		if(!batAlarmState && volt < 3600) {
			batAlarmState = 1; changed = 1;
		} else if(batAlarmState && volt > 3600) {
			batAlarmState = 0; changed = 1;
		}
		if(changed && devSystemStatus(NULL) == SS_OPERATIONAL && _OnBatAlarmStateChanged) (*_OnBatAlarmStateChanged)();
	}
}

int psuBatAlarmState(void *self)
{
	return (int)batAlarmState;
}

int psuBatStatus(void *self)
{
	return (int)batStatus;
}

int psuBatCapacity(void *self)
{
	return (int)batCapacity;
}

char *psuBatStatusName(void *self)
{
	return xmenu_bat_status[batStatus];
}

int psuBatPower(void)
{
	if(psuAcAlarmState(NULL) && !psuPoeStatus(NULL)) return 1;
	else	return 0;
}

static char batStatusStr[5][16] = {
	"Unknown", "Charging", "Discharging", "Not charging", "Full"
};

int PsuOnDataChanged(void)
{
	FILE	*fp;
	char	key[64], val[64];
	unsigned char	oldStatus, oldCapacity;
	int		rval, i, batPresent, volt;

/*
	fp = fopen("/sys/class/power_supply/ac/uevent", "r");
	if(!fp) {
		return -1;
	}
	while(1) {
		rval = file_get_keyvalue(fp, key, val);
		if(rval <= 0) break;
		if(!strcmp(key, "POWER_SUPPLY_VOLTAGE_NOW")) {
			acVolt = n_atoi(val) / 1000;
		}
	}
	fclose(fp);
*/
	fp = fopen("/sys/class/power_supply/battery/uevent", "r");
	if(!fp) {
		return -1;
	}
	oldStatus = batStatus;
	oldCapacity = batStatus;
	while(1) {
		rval = file_get_keyvalue(fp, key, val);
		if(rval <= 0) break;
		if(!strcmp(key, "POWER_SUPPLY_STATUS")) {
			for(i = 1;i < 5;i++)
				if(!strcmp(val, batStatusStr[i])) break;
			if(i < 5) batStatus = i;
			else	  batStatus = 0;
		} else if(!strcmp(key, "POWER_SUPPLY_PRESENT")) {
			batPresent = n_atoi(val);
		} else if(!strcmp(key, "POWER_SUPPLY_VOLTAGE_ONLINE")) {
			batOnline = n_atoi(val);
		} else if(!strcmp(key, "POWER_SUPPLY_VOLTAGE_NOW")) {
			volt = n_atoi(val) / 1000;
		} else if(!strcmp(key, "POWER_SUPPLY_CAPACITY")) {
			batCapacity = n_atoi(val);
		}
	}
	fclose(fp);
	if(!batPresent) {
		batStatus = 0; volt = 0;
	}
//printf("%lu %s %d%% %dmv\n", MS_TIMER, batStatusStr[batStatus], (int)batCapacity, batVoltageNow);
	if(oldStatus != batStatus || oldCapacity != batCapacity) {
		if(devSystemStatus(NULL) == SS_OPERATIONAL && _OnBatStatusChanged) (*_OnBatStatusChanged)();
	}
	PsuOnBatVoltageNowChanged(NULL, volt);
	return 0;
}

