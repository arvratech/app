#ifndef _ARP_PING_H_
#define _ARP_PING_H_


int  spackArpOpen(void);
void spackArpClose(int fd);
int  spackArpSend(int fd, char *ifName, unsigned char *sMacAddr, unsigned char *sIpAddr, unsigned char *tIpAddr);
int  spackArpRecv(int fd, char *ifName, unsigned char *sMacAddr, unsigned char *sIpAddr, unsigned char *tMacAddr, unsigned char *tIpAddr);


#endif

