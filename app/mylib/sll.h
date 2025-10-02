#ifndef _SLL_H_
#define _SLL_H_


#define SLL_SYSERR		0xfff7	// GetLastError()
#define SLL_ERROR		0xfff8	// other error
#define SLL_TIMEOUT		0xfff9	// time_out


extern int sll_errno;


int  sllInitClient(void);
void sllSetAddr(struct sockaddr *saddr, int ifIndex, unsigned char *macAddr);
void sllGetAddr(struct sockaddr *saddr, int *ifIndex, unsigned char *macAddr);
void sllEncodePacket(unsigned char *srcAddr, unsigned short srcPort, unsigned char *dstAddr, unsigned short dstPort, void *buf, int length);
int sllDecodePacket(unsigned char *srcAddr, unsigned short *srcPort, unsigned char *dstAddr, unsigned short *dstPort, void *buf, int length);
void sllError(char *s);
char *sllStrError();
void _sllError(int errnum, ...);


#endif
