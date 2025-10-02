#ifndef _SC_LIB_H_
#define _SC_LIB_H_


int  scEncodeKey(unsigned char *src_data, unsigned char *dst_data);
int  scDecodeKey(unsigned char *src_data, unsigned char *dst_data);
void scPrintKey(unsigned char *key, char *msg);
int  scValidateKey(unsigned char *key);


#endif
