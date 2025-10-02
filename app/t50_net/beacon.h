#ifndef _BEACON_H_
#define _BEACON_H_


typedef struct _BEACON {
	unsigned char	state;
	signed char		rssi;
	unsigned char	addr[6];
	unsigned char	ctm[4];
	unsigned long	timestamp;
	void			*queue[2];
} BEACON;


void beaconsInit(void (*timerCb)(uv_timer_t *));
void beaconsTimer(int timeout);
int  beaconsScan(void);
int  beaconsAdd(int *pIndex, unsigned char *addr, unsigned long timestamp, unsigned char *ctm);
int  beaconsReadAtIndex(BEACON *beacon, int index);
int  beaconsCount(void);


#endif

