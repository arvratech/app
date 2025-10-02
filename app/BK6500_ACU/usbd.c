int	usbd_errno;
 
 
int usbdOpen(void)
{
	return 0;
}

int usbdIsConnected(void)
{
	return 0;
}

int usbdIsReconnected(void)
{
	return 0;
}
 
int usbdWriteNull(void)
{
	return 0;
}

int ugetc(void)
{
	return -1;
}
	
int uputc(int c)
{
	return 0;
}

void usbdClearRead(void)
{
}

void usbdClearWrite(void)
{
}

#include <stdio.h>
#include <stdarg.h>

int uprintf(char *fmt, ...)
{
	return 0;
}

int ugetcEx(int timeout)
{
	return -1;
}
