#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "syscfg.h"
#include "psmem.h"


static unsigned long _memBuffer[16000];
static unsigned char *memBuffer;


void PSInitBuffer(void)
{
	memBuffer = (unsigned char *)_memBuffer;
}

void *PSalloc(int size)
{
	void	*p;

	p = memBuffer;
	memBuffer += size;
	return p;
}

void PSfree(void *buffer)
{
	memBuffer  = buffer;
}

