#ifndef _SC_LIB_H_
#define _SC_LIB_H_


int  scAuthKey(int block, unsigned char *uid, unsigned char *Key);
int  scReadData(int block, unsigned char *buf, int size, unsigned char *uid, unsigned char *SecondaryKey, unsigned char *NewKey);
int  scWriteData(int block, unsigned char *buf, int size, unsigned char *uid, unsigned char *SecondaryKey, unsigned char *NewKey);
int  scFormatCard(unsigned char *uid, unsigned char *SecondaryKey, unsigned char *NewKey);
void scSetKeyDefault(void);
int  scValidateKey(unsigned char *key, int len);
int  scGetKey(SYS_CFG *sys_cfg, unsigned char *data);
int  scSetKey(SYS_CFG *sys_cfg, unsigned char *data);
void scSetValidateKey(unsigned char *data, int len);
int  scCompareKey(unsigned char *data);
int  scEncodeKey(unsigned char *src_data, unsigned char *dst_data);
int  scDecodeKey(unsigned char *src_data, unsigned char *dst_data);
void scPrintKey(unsigned char *key, char *msg);


#endif	/* _SC_LIB_H_ */