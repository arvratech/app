#ifndef _BLE_H_
#define _BLE_H_


typedef struct _BLE_CFG {
	unsigned char	config;
	signed char		min_rssi;
	unsigned short	interval;
	unsigned char	reserve[2];
	unsigned char	macFilters[30];
} BLE_CFG;


void bleSetDefault(BLE_CFG *cfg);
int  bleEncode(BLE_CFG *cfg, void *buf);
int  bleDecode(BLE_CFG *cfg, void *buf);
int  bleValidate(BLE_CFG *cfg);

int  bleConfig(void *self);
BOOL bleEnable(void *self);
void bleSetEnable(void *self, BOOL enable);
BOOL bleEnableBeaconScan(void *self);
void bleSetEnableBeaconScan(void *self, BOOL enable);
int  bleMinBeaconSignal(void *self);
void bleSetMinBeaconSignal(void *self, int signal);
int  bleBeaconIntervalTime(void *self);
void bleSetBeaconIntervalTime(void *self, int time);
unsigned char *bleGetMacFilter(void *self, int index);
void bleSetMacFilter(void *self, int index, unsigned char *data);
void bleGetMacFilterName(void *self, int index, char *name);
char *bleMacFilterName(void *self, int index);
void bleSetMacFilterName(void *self, int index, char *name);


#endif

