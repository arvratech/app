#ifndef _PING_H_
#define _PING_H_


int  pingOpen(void);
void pingClose(void);
int  pingSend(unsigned char *ipAddr, int id, int seqNo);
void pingResultText(char *text, int result, unsigned char *ipAddr);


#endif

