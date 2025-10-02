#ifndef _SVR_H_
#define _SVR_H_


void svrLog(unsigned char *buf, int svrSize, int txflg);
int  CheckPacketHead(unsigned char *buf, int size);
int  CheckPacketTrail(unsigned char *buf, int size);
int  CheckPacket(unsigned char *buf, int size);
int  SockReadPacket(int sock, unsigned char *buf);
int  svrWritePacket(int sock, unsigned char *buf, int size);
int  svrSendResult(int sock, unsigned char *buf, int Result, int DataSize);
int  svrSendDump(int sock, unsigned char *buf, int size);


#endif
