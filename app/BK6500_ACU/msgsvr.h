#ifndef _MSG_SVR_H_
#define _MSG_SVR_H_


int  IsCommand(unsigned char *buf);
int  IsResult(unsigned char *buf);
int  GetDeviceID(unsigned char *buf);
int  GetCommand(unsigned char *buf);
int  GetResult(unsigned char *buf);
int  CodeCommand(unsigned char *buf, int Address, int Command, int DataSize);
int  CodeResult(unsigned char *buf, int Result, int DataSize);


#endif	/* _MSG_SVR_H_ */