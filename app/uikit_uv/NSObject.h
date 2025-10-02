#ifndef _NSOBJECT_H
#define _NSOBJECT_H

#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include "NSEnum.h"


extern void *NSClass;	// cls = alloc(NSClass); init(cls, type, "name", super, size, sel, meth, ... 0);
extern void *NSObject;	// obj = alloc(NSObject); init(obj)


int  alloc(void *class, void *buffer);
void init(void *self, ...);

void *class(void *self);
void *superclass(void *self);
BOOL isKindOfClass(void *self, void *class);
BOOL isMemberOfClass(void *self, void *class);
char *className(void *self);
int  sizeOf(void *self);


#endif
