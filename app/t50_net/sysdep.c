#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "defs.h"
#include "rtc.h"
#include "syscfg.h"
#include "sysdep.h"

#define MAX_USER_NUM			 100000

#define XFER_BUF_SIZE			1024000		// 1MB
#define XFER_BUF_LONG_SIZE		 256000
#define TEMP_BUF_SIZE			1304576		// 1.274MB
#define TEMP_BUF_LONG_SIZE		 326144
#define MAIN_BUF_SIZE			1024000		// 640KB => 1MB 2012.7.27
#define MAIN_BUF_LONG_SIZE		 256000 


unsigned long _XferBuffer[XFER_BUF_LONG_SIZE];
unsigned long _MainBuffer[XFER_BUF_LONG_SIZE];


int WriteProgram(unsigned char *addr, int nSize)
{
}

int GetMaxUserSize(void)
{
	return MAX_USER_NUM;
}

int GetUserFileSize(void)
{
	return MAX_USER_NUM * 32;
}

int		XferSize;

unsigned char *GetXferBuffer(int XferFile)
{
	return (unsigned char *)_XferBuffer;
}

int GetXferBufferSize(int XferFile)
{
	return XFER_BUF_SIZE;
}

int GetXferSize(int XferFile)
{
	return XferSize;
}

void SetXferSize(int XferFile, int nSize)
{
	XferSize = nSize;
}

unsigned char *GetMainBuffer(void)
{
	return (unsigned char *)_MainBuffer;
}

int GetMainBufferSize(void)
{
	return MAIN_BUF_SIZE;
}
