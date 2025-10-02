#include <stdio.h>
#include <string.h>
#include "lcdc.h"
#include "cgtext.h"


int textLineCount(char *string)
{
	char	*s;
	int		c, len, cnt;

	s = string; cnt = 0;
	while(c) {
		len = 0;
		while(1) {
			c = *s++;
			if(!c || c == '\n') break;
			len++;
		}
		if(c || len) cnt++;
		if(!c) break;
	}
	return cnt;
}

int textInsertCharAtPosition(char *string, int textPosition, char *st)
{
	char	*s, *s0, temp[128];
	int		c, len;

//printf("[%s] %d [%s]\n", string, textPosition, st);
	len = 0; s = string;
	while((*s) && len < textPosition) { 
		c = *s++; len++;
		if(c & 0x80) {
			c <<= 8; c |= *s++; len++;
		}
	}
	strcpy(temp, string);
	s = string + len;
	strcpy(s, st); s += strlen(s);
	strcpy(s, temp+len);
	return len + strlen(st);
}

int textDeleteCharBelowPosition(char *string, int textPosition)
{
	char	*s, *d;
	int		c, len, w;

	len = 0; s = string;
	while((*s) && len < textPosition) { 
		c = *s++; len++; w = 1;
		if(c & 0x80) {
			c <<= 8; c |= *s++; len++; w = 2;
		}
	}
	if(len > 0) {
		d = s - w;
		do {
			c = *s++; *d++ = c;	
		} while(c) ;
		len -= w;
	}
	return len;
}

int textDeleteCharAbovePosition(char *string, int textPosition)
{
	char	*s, *d;
	int		c, len, w;

	len = 0; s = string;
	while((*s) && len < textPosition) { 
		c = *s++; len++;
		if(c & 0x80) {
			c <<= 8; c |= *s++; len++;
		}
	}
	if(*s) {
		if(c & 0x80) w = 2; else w = 1;
		d = s + w;
		do {
			c = *d++; *s++ = c;	
		} while(c) ;
	}
	return len;
}

