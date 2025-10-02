#ifndef _WG_LIB_H_
#define _WG_LIB_H_


int  wgEncode(unsigned char *rawData, int bitLen, unsigned char *encData);
void wgReverse(unsigned char *src, unsigned char *dst, int len);
int  wgDecode(unsigned char *encData, int bitLen, unsigned char *rawData);
int  wgDecode35Bits(unsigned char *encData, unsigned char *rawData);
 

#endif

