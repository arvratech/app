#ifndef _TFTPC_H_
#define _TFTPC_H_


int  tftpcGet(char *local, char *remote, char *target, int estimatedSize, void (*callback)(int));
int  tftpcStatus(void);
int  tftpcResult(void);
int  tftpcDownloadSize(void);
int  tftpcTransferSize(void);
char *tftpcErrorString(void);


#endif

