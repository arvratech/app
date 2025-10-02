#ifndef _ACU_SVR_H_
#define _ACU_SVR_H_


void acuProcessRxCommon(unsigned char *buf, int State);
void acuProcessRequest(void);
void acuProcessSegRequest(void);
void acuProcessResponse(void);
void acuSetOnReceivedRsp(void (*onReceivedRsp)(int, unsigned char *, int));
void acuCallOnReceivedRsp(int status, unsigned char *buf, int len);
void acuProcessResponse2(void);
void acuSetChanged(void);
void acuGetReq(unsigned char *req);


#endif

