#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <signal.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "queue.h"
#include "uv.h"
#include "beacon.h"


#define MAX_BEACON_SZ	128

static BEACON	beaconPools[MAX_BEACON_SZ];
static void		*beaconq[2];
static uv_timer_t	*timerBeacons, _timerBeacons;
static void (*beaconsTimerCb)(uv_timer_t *);
void *MainLoop(void);


void beaconsInit(void (*timerCb)(uv_timer_t *))
{
	BEACON	*beacon;
	int		i;
	
	for(i = 0, beacon = beaconPools;i < MAX_BEACON_SZ;i++, beacon++) 
		beacon->state = 0;
	QUEUE_INIT(&beaconq);
	timerBeacons = &_timerBeacons;
	uv_timer_init((uv_loop_t *)MainLoop(), timerBeacons);
	beaconsTimerCb = timerCb;
	beaconsTimer(5000);
}

void beaconsTimer(int timeout)
{
	uv_timer_start(timerBeacons, beaconsTimerCb, timeout, 0);
}

int beaconsScan(void)
{
	BEACON	*beacon;
	QUEUE	queue;
	QUEUE	*q;
	int		timeout, count;

	count = 0;
	timeout = bleBeaconIntervalTime(NULL) * 1000;
	QUEUE_MOVE(&beaconq, &queue);
	while(!QUEUE_EMPTY(&queue)) {
		q = QUEUE_HEAD(&queue);
		beacon = QUEUE_DATA(q, BEACON, queue);
		QUEUE_REMOVE(q);
		if((MS_TIMER-beacon->timestamp) > timeout) {
printf("Remove: [...%02x-%02x]\n", (int)beacon->addr[4], (int)beacon->addr[5]);
			beacon->state = 0;
			//QUEUE_INI(q);
			count++;
		} else {
			QUEUE_INSERT_TAIL(&beaconq, q);
		}
	}
	return count;
}

// -1:Full  0:Exist  1:New
int beaconsAdd(int *pIndex, unsigned char *addr, unsigned long timestamp, unsigned char *ctm)
{
	BEACON	*beacon;
	QUEUE	*q;
	int		i;

	i = 0;
	QUEUE_FOREACH(q, &beaconq) {
		beacon = QUEUE_DATA(q, BEACON, queue);
		if(!memcmp(beacon->addr, addr+1, 6)) {
			beacon->rssi = addr[0];
			beacon->timestamp = timestamp;
			memcpy(beacon->ctm, ctm+3, 3);
			*pIndex = i;
			return 0;
		}
		i++;
	}
	for(i = 0, beacon = beaconPools;i < MAX_BEACON_SZ;i++, beacon++) 
		if(!beacon->state) break;
	if(i >= MAX_BEACON_SZ) i = -1;
	else {
		beacon->rssi = addr[0];
		memcpy(beacon->addr, addr+1, 6);
		beacon->timestamp = timestamp;
		memcpy(beacon->ctm, ctm+3, 3);
		beacon->state = 1;
		QUEUE_INSERT_TAIL(&beaconq, &beacon->queue);
		*pIndex = i;
		i = 1;
	}
	return i;
}

int beaconsReadAtIndex(BEACON *beacon, int index)
{
	QUEUE	*q;
	int		i, found;

	i = found = 0;
	QUEUE_FOREACH(q, &beaconq) {
		if(i == index) {
			found = 1;
			break;
		}
		i++;
	}
	if(found) memcpy(beacon, QUEUE_DATA(q, BEACON, queue), sizeof(BEACON));
	return found;
}

int beaconsCount(void)
{
	QUEUE	*q;
	int		count;

	count = 0;
	QUEUE_FOREACH(q, &beaconq) {
		count++;
	}
	return count;
}

