#ifndef _BIO_H_
#define _BIO_H_

#include "biocfg.h"

extern volatile unsigned char  biPV[];
extern unsigned long  biTimer[];
extern int			  biTimeout[];


typedef struct _BINARY_INPUT {
	unsigned char	ID;
	unsigned char	reserve[3];
	unsigned char	ChangeOfStateTime[6];
	unsigned char	AlarmStateTime[6];
	BINARY_INPUT_CFG *cfg;
} BINARY_INPUT;

typedef struct _BINARY_OUTPUT {
	unsigned char	ID;
	unsigned char	PresentValue;	
	unsigned char	ChangeOfStateTime[6];
	BINARY_OUTPUT_CFG *cfg;
} BINARY_OUTPUT;


void bisInit(BINARY_INPUT *g_bis, int size);
void bisSet(BINARY_INPUT *g_bis);
BINARY_INPUT *bisGetAt(int biID);
void bisSetPutCallBack(void (*PutCallBack)());
void biReset(int biID);
int  biGetAlarmInhibit(int biID);
void biLocalPVChanged(int biID, int PV);
void biProcessLocal(int biID);
void biRemotePVChanged(int biID);
int  biAlarmChanged(int biID);
int  biEncodeStatus(int biID, unsigned char *Buffer);
void biNotifyStatus(int biID);
int  bisEncodeStatus(unsigned char *Buffer);
void bosInit(BINARY_OUTPUT *g_bos, int size);
void bosSet(BINARY_OUTPUT *g_bos);
BINARY_OUTPUT *bosGetAt(int boID);
void bosSetPutCallBack(void (*PutCallBack)());
void boReset(int boID);
int  boGetPresentValue(int boID);
void boSetPresentValue(int boID, int PresentValue);
int  boEncodeStatus(int boID, unsigned char *Buffer);
void boNotifyStatus(int boID);
int  bosEncodeStatus(unsigned char *Buffer);


#endif	/* _BIO_H_ */