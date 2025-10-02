#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "NSObject.h"
#include "nsobject_.h"


static void NSClass_izer(void *self, va_list *app)
{
	struct NSClass *cls = self;
	typedef void (*voidf)();	// generic function pointer
	voidf		selector, method;
	va_list		ap;
	int			offset;

//printf("NSClass_izer...[%s]\n", className(cls));
	offset = offsetof(struct NSClass, izer);
	cls->name	= va_arg(*app, char *);
	cls->super	= va_arg(*app, struct NSClass *);
	cls->size	= va_arg(*app, int);
	assert(cls->super);
	memcpy((char *)cls + offset, (char *)cls->super + offset, sizeOf(cls->super) - offset);
#ifdef va_copy
	va_copy(ap, *app);
#else
	ap = *app;
#endif
	while((selector = va_arg(ap, voidf))) {
		method = va_arg(ap, voidf);
		if(selector == (voidf)izer) *(voidf *)&cls->izer = method;
		else if(selector == (voidf)dtor) *(voidf *)&cls->dtor = method;
	}
#ifdef va_copy
    va_end(ap);
#endif
}

static void *NSClass_dtor(void *self)
{
	struct NSClass *cls = self;

	fprintf(stderr, "%s: cannot destroy class\n", className(cls));
	return 0;
}

static void NSObject_izer(void *self, va_list *app)
{
}

static void *NSObject_dtor(void *self)
{
	return self;
}

// Initialization
static struct NSClass object[] = {
	{ { object + 1 },
//	  "NSObject", object, sizeof(struct NSObject), NSObject_izer, NSObject_dtor
	  "NSObject", NULL, sizeof(struct NSObject), NSObject_izer, NSObject_dtor
	},
	{ { object + 1 },
	  "NSClass", object, sizeof(struct NSClass), NSClass_izer, NSClass_dtor
	}
};

void *NSObject = object;
void *NSClass = object + 1;

void izer(void *self, va_list *app)
{
	struct NSClass *cls = class(self);

//printf("izer...self=%x cls=%x %s\n", (unsigned int)self, (unsigned int)cls, className(self));
	assert(cls && cls->izer);
	cls->izer(self, app);
}

void super_izer(void *class, void *self, va_list *app)
{
	struct NSClass *cls = class;
	struct NSClass *supercls;

	supercls = cls->super;
	assert(supercls && supercls->izer);
	supercls->izer(self, app);
}

void *dtor(void *self)
{
	struct NSClass *cls = class(self);

	assert(cls->dtor);
	return cls->dtor(self);
}

void *super_dtor(void *class, void *self)
{
	struct NSClass *cls = class;
	struct NSClass *supercls;

	supercls = cls->super;
	assert(supercls && supercls->dtor);
	return supercls->dtor(self);
}

//	object management and selectors
int alloc(void *class, void *buffer)
{
	struct NSClass *cls = class;
	struct NSObject *obj;

	assert(cls && cls->size);
	obj = (struct NSObject *)buffer;
	obj->isa = cls;
	return cls->size;
}

void init(void *self, ...)
{
	struct NSObject *obj = self;
	va_list ap;

	va_start(ap, self);
	izer(obj, &ap);
	va_end(ap);
}

void *class(void *self)
{
	struct NSObject *obj = self;

	assert(obj && obj ->isa);
	return obj->isa;
}

void *superclass(void *self)
{
	struct NSObject *obj = self;
	struct NSClass *cls;

	cls = obj->isa;
	assert(cls);
	return cls->super;
}

// Returns a Boolean value that indicates whether the receiver is an instance of given class
// or an instance of any class that inherits from that class.
BOOL isKindOfClass(void *self, void *class)
{
	struct NSObject *obj = self;
	struct NSClass *cls;

	cls = obj->isa;
	while(cls) {
		if(cls == class) break;
		cls = cls->super;
	}
	if(cls) return TRUE;
	else	return FALSE;
}

// Returns a Boolean value that indicates whether the receiver is an instance of a given class.
BOOL isMemberOfClass(void *self, void *class)
{
	struct NSObject *obj = self;

	if(obj->isa == class) return TRUE;
	else	return FALSE;
}

char *className(void *self)
{
	struct NSObject *obj = self;
	struct NSClass	*cls;

	assert(obj && obj ->isa);
	cls = obj->isa;
	return cls->name;
}

int sizeOf(void * self)
{
	struct NSObject *obj = self;
	struct NSClass	*cls;

	assert(obj && obj ->isa);
	cls = obj->isa;
	return cls->size;
}

