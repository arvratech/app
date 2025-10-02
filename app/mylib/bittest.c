#include <stdio.h>
#include <string.h>

void main(int argc, char *argv[])
{
	unsigned char	s[100], d[100];
	int		i;

	memset(s, 0xff, 100);

	memset(d, 0x00, 100);
	bitcpy(d, 19, s, 17, 20); 
	for(i = 0;i < 6;i++) printf("%02x-", d[i]); printf("\n");
	memset(d, 0x00, 100);
	bitcpy(d, 19, s, 17, 2); 
	for(i = 0;i < 6;i++) printf("%02x-", d[i]); printf("\n");
	bitcpy(d, 19, s, 17, 9); 
	for(i = 0;i < 6;i++) printf("%02x-", d[i]); printf("\n");
}

