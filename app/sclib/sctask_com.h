#ifndef _SC_TASK_COM_H_
#define _SC_TASK_COM_H_


#define MAX_CARD_SIZE		4

// SC open mode definition
#define SC_ISO14443A		1
#define SC_ISO14443B		2
#define SC_ISO15693			3

// SC state definitions
#define SC_OFF				0
#define SC_IDLE				1
#define SC_REQUEST			2
#define SC_ANTICOLLSION		3
#define SC_READY			4
#define SC_ACTIVE			5
#define SC_FATAL            9

#define SC_TIMER            MS_TIMER

extern unsigned char  scOpenMode, scState, scSubState, scCount, scReaderFormat, scCardType, scCardApp, scCardApp2;
extern unsigned long  scTimer;
extern long           scTimeout;


void scInitData(void);
void scClearData(void);
void scAddUid(unsigned char *uid);
int  scFindUid(unsigned char *uid);
void scSetOnDataDone(void (*onDataDone)(unsigned char *, int));
void scAddData(int format, unsigned char *data, int len);
void scDoneData(void);
int  scExistData(void);
int  scDataIsFull(void);
int  scCardPresence(void);
int  scExtractTransportCardNumber(unsigned char *buf, int len, unsigned char *data);
int  scExtractSpecialTransportCardNumber(unsigned char *buf, int len, unsigned char *data);

void iso14443AActiveTask(void);
void iso14443AExitTask(void);
void iso14443BActiveTask(void);
void iso14443BExitTask(void);
void iso15693ActiveTask(void);
void iso15693ExitTask(void);


#endif
