#ifndef _MY_CRC_H_
#define _MY_CRC_H_


void crc8Init(void);
void crc16Init(void);
int  crc8Check(unsigned char *buf, int length);
int  crc16Check(unsigned char *buf, int length);


#endif

