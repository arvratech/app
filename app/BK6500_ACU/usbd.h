#ifndef __USBD_H__
#define __USBD_H__

extern int	usbd_errno;

int  usbdOpen(void);
int  usbdIsConnected(void);
int  usbdIsReconnected(void);
int  usbdRead(void *buf, int size);
int  usbdWrite(void *buf, int size); 
int  usbdWriteNull(void);
void usbdClearRead(void);
void usbdClearWrite(void);
int  ugetc(void);
int  uputc(int c);
int  uprintf(char *fmt, ...);
int  ugetcEx(int timeout);


#endif /*__USBD_H__*/
