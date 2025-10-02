#ifndef _SC_MIFARE_H
#define _SC_MIFARE_H


/*! Size of the intermal MiFare transmit buffer. FIXME: Should be reduced to 18 */
#define MAX_MIFARE_TX_BUF_SZ            64
#define MAX_MIFARE_TX_RAWBUF_SZ         72      // ((MAX_MIFARE_TX_BUF_SZ * 9) + 7) / 8

#define MIFARE_AUTH_KEY_A                   0x60
#define MIFARE_AUTH_KEY_B                   0x61
#define MIFARE_READ_BLOCK                   0x30
#define MIFARE_WRITE_BLOCK                  0xA0
#define MIFARE_ACK                          0x0A
#define MIFARE_NACK_NOT_ALLOWED             0x04
#define MIFARE_NACK_TRANSMISSION_ERROR      0x04


void mifareOpen(void);
void mifareClose(void);
int  mifareCiplerActive(void);
int  mifareAuthenticate(int auth_mode, unsigned char *key, unsigned char *uid, int addr);
int  mifareReadBlock(int addr, unsigned char *buf);
int  mifareWriteBlock(int addr, unsigned char *buf);
void mifareHalt(void);
int  mifareRead(int block, unsigned char *buf, int size, unsigned char *uid, unsigned char *key);
int  mifareWrite(int block, unsigned char *buf, int size, unsigned char *uid, unsigned char *key);


#endif
