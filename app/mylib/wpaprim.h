#ifndef _WPA_PRIM_H_
#define _WPA_PRIM_H_


char *wpaStateName(int wpaState);
char *wpaNetworkStateName(int state);
char *wpaSecurityName(int security);
void wpaSecuredName(int security, char *name);
int  wpaFrequency2Channel(int freq);


#endif

