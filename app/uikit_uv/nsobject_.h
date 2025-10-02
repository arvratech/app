#ifndef _NSOBJECT__H
#define _NSOBJECT__H


struct NSObject {
	struct NSClass *isa;
};

struct NSClass {
	struct NSObject	_;			// class' description 
	char			*name;		// class' name 
	struct NSClass	*super;		// class' super class
	int				size;		// class' object's size
	void (*izer)(void *self, va_list *app);
	void *(*dtor)(void *self);
};


void izer(void * self, va_list *app);
void super_izer(void *class, void *self, va_list *app);
void *dtor(void *self);
void *super_dtor(void *class, void *self);


#endif

