#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <sys/stat.h>
#include "log.h"


void liblog(const char *func, int errnum, ...)
{
	va_list	args;
	char	*p, *fmt, buf[256];
	int		status, len;

	p = buf;
	strcpy(p, func); p += strlen(func); *p++ = ':'; *p++ = ' ';
	va_start(args, errnum);
	fmt = va_arg(args, char *);
	vsprintf(p, fmt, args);
	va_end(args);
	p += strlen(p);
	if(errnum) sprintf(p, " %d: %s", errno, strerror(errnum));
	printf("%s\n", buf);
}

