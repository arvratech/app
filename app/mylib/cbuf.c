//====================================================================
// File Name : cbuf.c
// Function  : S3C2410 Test Main Menu
// Program   : Kang DaeKyu
// Date      : December 1, 2003
// Version   : 1.0
// History
//   0.0 : Programming start (February 20,2002) -> SOP
//====================================================================

#include <string.h>
#include "cbuf.h"

/*
 * Circular Buffer Structure:
 *
 *	Each circular buffer has 8 bytes of overhead, and 2^n bytes of data area,
 *	of which 2^n - 1 bytes are used to hold data at any time; the additional
 *	data area byte provides a separator for the tail index and the head.
 *	Therefore, when allocating space for a buffer, you will need 2^n + 8 bytes.
 *	following structure:
 *		1 byte: byte of lock status: bit 0: write lock
 *									 bit 1: read lock
 *									 bits 2-15: unused
 *		2 byte: unused
 *		3,4 byte: short word of head index of data         
 *		5,6 byte: short word of tail index of data
 *		7,8 byte: short word of 2^n - 1 mask
 *		9th byte: start of 2^n byte data area for 2^n - 1 bytes of data
 *					for port A write buffer defined as follows:
 *			(head == tail)						: buffer is empty
 *			(head + data length) & mask == tail : buffer is full
 *			(head - 1) & mask = tail			
 */

#define CBUF_HEAD(cbuf)  (*(volatile unsigned short *)(cbuf+2))
#define CBUF_TAIL(cbuf)  (*(volatile unsigned short *)(cbuf+4))
#define CBUF_MASK(cbuf)  (*(volatile unsigned short *)(cbuf+6))


void cbufInit(void *cbuf, int DataSize)	// DataSize must be  2^n - 1
{
	unsigned char	*p;

	p = (unsigned char *)cbuf;
	memset(p, 0, 6);
	CBUF_MASK(p) = DataSize;
}

int cbufLength(void *cbuf)
{
	unsigned char	*p;

	p = (unsigned char *)cbuf;
	return (int)CBUF_MASK(p);
}

int cbufUsed(void *cbuf)
{
	unsigned char	*p;
	int		val, mask;

	p = (unsigned char *)cbuf;	
	mask = CBUF_MASK(p);
	val = CBUF_TAIL(p) - CBUF_HEAD(p) + mask + 1;
	return val & mask;
}

int cbufFree(void *cbuf)
{
	unsigned char	*p;
	int		val, mask;

	p = (unsigned char *)cbuf;	
	mask = CBUF_MASK(p);
	val = CBUF_HEAD(p) - CBUF_TAIL(p) + mask;
	return val & mask;
}

void cbufFlush(void *cbuf)
{
	memset(cbuf, 0, 6);
}

int cbufEmpty(void *cbuf)
{
	unsigned char	*p;

	p = (unsigned char *)cbuf;
	if(CBUF_HEAD(p) == CBUF_TAIL(p)) return 1;
	else	return 0;
}

int cbufWrlock(void *cbuf)
{
	unsigned char	*p;

	p = (unsigned char *)cbuf;
	if(p[0] & 0x01) return 0;
	else {
		p[0] |= 0x01;
		return 1;
	}	
}
		
void cbufWrunlock(void *cbuf)
{
	unsigned char	*p;

	p = (unsigned char *)cbuf;
	p[0] &= 0xfe;	
}

int cbufRdlock(void *cbuf)
{
	unsigned char	*p;

	p = (unsigned char *)cbuf;
	if(p[0] & 0x02) return 0;
	else {	
		p[0] |= 0x02;
		return 1;
	}
}
		
void cbufRdunlock(void *cbuf)
{
	unsigned char	*p;

	p = (unsigned char *)cbuf;
	p[0] &= 0x0d;	
}

int cbufPeek(void *cbuf)
{
	unsigned char	*p;	
	int		c, head;
	
	p = (unsigned char *)cbuf;
	head = CBUF_HEAD(p);
	if(head == CBUF_TAIL(p)) c = -1;	// empty
	else	c = *(p + 8 + head);
	return c;
}

int cbufGetch(void *cbuf)
{
	unsigned char	*p;
	int		c, head;
	
	p = (unsigned char *)cbuf;
	head = CBUF_HEAD(p);
	if(head == CBUF_TAIL(p)) c = -1;	// empty
	else {
		c = *(p + 8 + head);
		head++;
		CBUF_HEAD(p) = head & CBUF_MASK(p);
	}
	return c;
}

int cbufPutch(void *cbuf, int c)
{
	unsigned char	*p;	
	int		tail, mask;
	
	p = (unsigned char *)cbuf;
	tail = CBUF_TAIL(p);
	mask = CBUF_MASK(p);
	if(((CBUF_HEAD(p) - 1) & mask) == tail) return 0;	// full
	*(p + 8 + tail) = c;
	tail++;
	CBUF_TAIL(p) = tail & mask;
	return 1;
}

int cbufGet(void *cbuf, unsigned char *buf, int len)
{
	unsigned char	*p;	
	int		i, head, tail, mask;
	
	p = (unsigned char *)cbuf;
	head = CBUF_HEAD(p);
	tail = CBUF_TAIL(p);
	mask = CBUF_MASK(p);
	i = 0;
	while(head != tail && i < len) {
		buf[i] = *(p + 8 + head);
		i++; head++;
		head &= mask;
	}
	CBUF_HEAD(p) = head;
	return i;
}

int cbufPut(void *cbuf, unsigned char *buf, int len)
{
	unsigned char	*p;	
	int		i, tail, mask, end;

	p = (unsigned char *)cbuf;
	tail = CBUF_TAIL(p);
	mask = CBUF_MASK(p);
	end = (CBUF_HEAD(p) - 1) & mask;
	i = 0;
	while(tail != end && i < len) {
		*(p + 8 + tail) = buf[i];
		i++; tail++;
		tail &= mask;
	}
	CBUF_TAIL(p) = tail;
	return i;
}

/*
int cbuf_print(void *cbuf)
{
	unsigned char	*p;	
	int		head, tail;
	
	p = (unsigned char *)cbuf;
	head = CBUF_HEAD(p);
	tail = CBUF_TAIL(p);
c1printf("head=%d tail=%d c=%d\n", head, tail, (tail-head+CBUF_MASK(p)+1)&CBUF_MASK(p));
}
*/
