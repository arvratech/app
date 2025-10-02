#ifndef _WIEGAND_H_
#define _WIEGAND_H_


#define WIEGAND_26		1
#define WIEGAND_34		2


void wiegandOpen(int port);
void wiegandClose(int port);
int  wiegandPeek(int port);
int  wiegandRead(int port, unsigned char *pData);
int  wiegandGetKey(int port);
void wiegand26Write(unsigned char *pData);
void wiegand34Write(unsigned char *pData);
void wiegand98Write(unsigned char *pData);
int  wiegandCheck26Data(unsigned char *data);
void wiegandReverse26Data(unsigned char *data);
int  wiegandCheck34Data(unsigned char *data);
int  wiegandCheck35Data(unsigned char *data);
int  wiegandCheck37Data(unsigned char *data);
int  wiegandCheck130Data(unsigned char *data);
int  wiegandCheck66Data(unsigned char *data);

void wiegand26Write(unsigned char *pData);
void wiegand34Write(unsigned char *pData);
void wiegand98Write(unsigned char *pData);
void wiegandWriteTest(void);


#endif	/* _WIEGAND_H_ */
