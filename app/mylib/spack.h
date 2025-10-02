#ifndef _SPACK_H_
#define _SPACK_H_

#include <arpa/inet.h>
#include <sys/socket.h>


typedef struct _SPACK_ADDR {
	int				ifIndex;
	unsigned char	srcIpAddr[4];
	unsigned short	srcIpPort;	
	unsigned char	dstMacAddr[6];
	unsigned char	dstIpAddr[4];
	unsigned short	dstIpPort;	
	unsigned char	reserve[2];	
} SPACK_ADDR;


int  spackUdpOpen(void);
void spackAddr(struct sockaddr *saddr, int ifIndex, unsigned char *dstMacAddr);
int  spackPayload(unsigned char *buf, unsigned long srcIpAddr, unsigned short srcIpPort,
					unsigned long dstIpAddr, unsigned short dstIpPort, unsigned char *apdu, int apduLen);
int  spackPayload2(unsigned char *buf, unsigned char *srcIpAddr, unsigned short srcIpPort,
					unsigned char *dstIpAddr, unsigned short dstIpPort, unsigned char *apdu, int apduLen);



#endif

