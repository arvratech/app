#ifndef _NET_SVC_H_
#define _NET_SVC_H_


// NET Service
typedef struct _NET_SVC {
	unsigned char	type;
	unsigned char	invokeId;
	unsigned char	serviceChoice;
	unsigned char	error;
	unsigned char	errorClass;
	unsigned char	errorCode;
	unsigned short	dataLength;
	unsigned short	svcLength;
	unsigned char	svcWork;
	unsigned char	svcPending;
	unsigned char	pduPending;
	unsigned char	objectType;
	unsigned char	propertyID;
	unsigned char	wrState;
	unsigned char	reserve[2];
	long			position;
	unsigned char	*data;
} NET_SVC;


void netsvcInit(NET_SVC *svc, int svcType, unsigned char *buf);
void netsvcResponding(NET_SVC *svc, unsigned char *buf, int length);
void netsvcRequesting(NET_SVC *svc, unsigned char *buf, int length);


#endif

