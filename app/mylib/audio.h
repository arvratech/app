#ifndef _AUDIO_H_
#define _AUDIO_H_


// ment type definitions
#define MENT_SOFT			0
#define MENT_HARD			1
#define MENT_REPEAT			2
#define MENT_SPECIAL		4

#define MENT_STOP_SOFT		1	
#define MENT_STOP_HARD		2	
#define MENT_STOP_SPECIAL	3	


void audioInit(void);
void audioExit(void);
int  audioPost(unsigned char *msg);
void audioContinue(void);
void audioFlush(void);
int  audioActive(void);
void audioSetAlarm(void);
void audioFlushAlarm(void);


#endif
