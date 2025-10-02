#ifndef _WPA_CTRL_H_
#define _WPA_CTRL_H_

#include "wpanetcfg.h"

void WpaInit(void *context, void (*onConnected)(void *, void *), void (*onDisconnected)(void *, void *), void(*onInterfaceDisabled)(void *));
void WpaThreadInit(void *(*ThreadMain)(void *));
void *WpaTask(void *arg);
void WpaSetScanning(int scan);
void WpaCtrlRequest(char *cmd);
int  wpaWpaState(void);
unsigned char *wpaBssid(void);
int  wpaStateIsConnected(void);
void wpaSetScanning(int enable);
void wpaAddNetwork(WPA_NET_CFG *wpanet_cfg);
void wpaSelectNetwork(WPA_NET_CFG *wpanet_cfg);
void wpaRemoveNetwork(WPA_NET_CFG *wpanet_cfg);
int  wpaStatus(void);


#endif

