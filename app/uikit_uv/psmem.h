#ifndef _PS_MEM_H_
#define _PS_MEM_H_


void PSInitBuffer(void);
void *PSalloc(int size);
void PSfree(void *buffer);


#endif
