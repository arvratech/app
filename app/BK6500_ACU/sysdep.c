#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
////////////////////////////////
#include "system_hdr.h"
////////////////////////////////
#include "defs.h"
#include "..\BootLoader\flash.h"
#include "..\BootLoader\flashprim.h"
#include "..\BootMonitor\bmoncmd.h"
#include "rtc.h"
#include "syscfg.h"
#include "sysdep.h"

#define MAX_USER_NUM			   65000
#ifdef BIO_FP
#define MAX_TMPL_NUM			   20000
#define MAX_FPIDENTIFY_NUM		    4000
#else
#define MAX_TMPL_NUM			   20000
#define MAX_FPIDENTIFY_NUM		   12000		// 2018.7.24 6000 => 12000
#endif
#define MAX_PHONE_NO_NUM		    4096
#define MAX_CAM_IMAGE_SIZE		16384000		// 16MB

#define XFER_BUF_SIZE			 1400000		// 1.4
#define XFER_BUF_LONG_SIZE		  350000

#define MAIN_BUF_SIZE			 1024000		// 640KB => 1MB 2012.7.27
#define MAIN_BUF_LONG_SIZE		  256000 


unsigned long _XferBuffer[XFER_BUF_LONG_SIZE];
// Long Alignment 를 위해 unsigned char => unsigned long 선언으로 변경(2011.1.18)
// unsigned char 선언시 Short Alignment 되어 SDI 함수같은 Long access에서 Pabort 발생함
unsigned long _MainBuffer[MAIN_BUF_LONG_SIZE];
//unsigned long _TempBuffer[TEMP_BUF_LONG_SIZE];


void GetMACAddress(unsigned char *addr)
{
	FlashReadMACAddress(addr);
}

int GetAppProgramStartBlock(void)
{
	return 2;
}

//	Return  1:OK  0:Write error  <0:Validate error
int WriteAppProgram(unsigned char *buf, int nSize)
{
	int		rval, startBlock, blockSize;

	startBlock = GetAppProgramStartBlock();
	blockSize = _GetAppProgramBlockSize();
	rval = WriteProgram(startBlock, blockSize, buf, nSize);
	return rval;
}

/*
//	Return  1:OK  0:Write error  <0:Validate error
int WriteProgram(unsigned char *addr, int nSize)
{
	unsigned char	bi_tbl[100];
	unsigned long	data[5];
	int		i, c, block, rval, StartBlock, BlockSize;

cprintf("%lu WriteProgram...%d\n", DS_TIMER, nSize<<2);
	rval = GetProgramType(addr);	// 0:Normal Program 1:Boot Monitor Program
	if(rval < 0) {
		cprintf("WriteProgram: Data header error(Non-Program)");
		return rval;
	}
	BlockSize = flashGetBytesPerPage() * flashGetPagesPerBlock();
	BlockSize = ((nSize << 2) + BlockSize - 1) / BlockSize;
	if(rval) {
		if(BlockSize > 1) return -2;
		StartBlock = 1;
	} else {
		if(BlockSize > 10) return -2;
		StartBlock = GetProgramStartBlock();
	}
	rval = FlashFormatProgramBlocks(StartBlock, BlockSize, bi_tbl);
	for(i = 0, block = StartBlock;i < BlockSize;i++, block++) {
		c = bi_tbl[i];
		if(c) {
			cprintf("%lu WriteProgram: ", DS_TIMER);
			if(c == 1)		cprintf("Block erase error: %d", block);
			else if(c == 2)	cprintf("Bad block: %d", block);
			else if(c == 3)	cprintf("Bad block erase error: %d", block);
			else if(c == 4)	cprintf("Unknown block: %d", block);
			else if(c == 5)	cprintf("Unknown block erase error: %d", block);
			else if(c == 6)	cprintf("Read spare error: %d", block);
			else			cprintf("Read spare error, erase error: %d", block);
			cputch('\n');
		}
	}
	rval = FlashWriteProgram(StartBlock, addr, nSize);
	if(rval < 0) {
		cprintf("%lu WriteProgram: Flash write error\n", DS_TIMER);
		rval = 0;
	} else {
		rval = FlashValidateProgram(StartBlock, addr, nSize);
		if(rval < 0) {
			cprintf("%ld WriteProgram: ", DS_TIMER);
			if(rval == -1) cprintf("Validate checksum error");
			else if(rval == -2) cprintf("Validate header error(Non-Program)");
			else if(rval == -3) cprintf("Validate length error");
			else	cprintf("Validate data inconsistency"); 
			cputch('\n');
		} else {			
			cprintf("%ld WriteProgram OK: %d\n", DS_TIMER, nSize<<2);
			rval = 1;
		}
	}
	return rval;
}
*/

int GetUserStartBlock(void)
{
//	int		a, b, c;
//a = flashGetBytesPerPage(); b = flashGetPagesPerBlock(); c = flashGetBlocksPerDevice();
//cprintf("Bytes=%d Pages=%d Block=%d Block=%dKB Total=%dMB\n", a, b, c, a*b/1024, a*b*c/1024/1024);
	return 12;
}

int GetMaxUserSize(void)
{
	return MAX_USER_NUM;
}

int GetMaxFPTemplateSize(void)
{
	return MAX_TMPL_NUM;
}

int GetMaxFPIdentifySize(void)
{
	return MAX_FPIDENTIFY_NUM;
}

int GetMaxPhoneNoSize(void)
{
	return MAX_PHONE_NO_NUM;
}

int GetUserFileSize(void)
{
	return MAX_USER_NUM * 32;
}

int GetFPTemplateFileSize(void)
{
	return MAX_TMPL_NUM * STORED_TMPL_DOUBLE_SZ;	// Bug fix 2024.6.20 512=>1024
}

int GetPhoneNoFileSize(void)
{
	return MAX_PHONE_NO_NUM * 16;
}

int GetCamImageFileSize(void)
{
	return MAX_CAM_IMAGE_SIZE;	
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
