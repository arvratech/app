#ifndef _SC_LIB_H_
#define _SC_LIB_H_


int  scEncodeKey(unsigned char *src_data, unsigned char *dst_data);
int  scDecodeKey(unsigned char *src_data, unsigned char *dst_data);
void scPrintKey(unsigned char *key, char *msg);
int  scValidateKey(unsigned char *key);
int  scGetKey(SYS_CFG *sys_cfg, unsigned char *data);
int  scSetKey(SYS_CFG *sys_cfg, unsigned char *data);
void scSetValidateKey(SYS_CFG *sys_cfg, unsigned char *data);
int  scCompareKey(SYS_CFG *sys_cfg, unsigned char *data);


#endif
