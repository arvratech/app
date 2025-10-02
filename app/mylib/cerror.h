#ifndef _C_ERROR_H_
#define _C_ERROR_H_

#include <setjmp.h>

extern jmp_buf		_jmpbuf_;

#define TRY			do { int x; x = setjmp(_jmpbuf_); if(x == 0) {
#define CATCH(x)	} else {
#define ETRY		} } while(0)
#define THROW(x)	longjmp(_jmpbuf_, x)


#endif

