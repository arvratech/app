#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "prim.h"
#include "dev.h"
#include "sysprim.h"



int main(int argc, char *argv[])
{
	char	*p, temp[256], temp2[64], temp3[64], signo[16];

	if(argc > 2) {
		printf("Usage: killapp [-signo]\n");
		return 1;
	}
	if(argc == 2) {
		p = argv[1];
		if(p[0] != '-' || n_atoi(p+1) < 1) {
			printf("Usage: killapp [-signo]\n");
			return 1;
		}
		strcpy(signo, p);
	} else {
		strcpy(signo, "-1");	
	}
	ExecPipe("/bin/ps", "ps", NULL); 
	ExecPipeFinal(temp, "/bin/grep", "grep", devFirmwareName(NULL), NULL); 
	p = temp;
	p = read_linestr(p, temp2);
	read_token(temp2, temp3);
	if(p) p = read_linestr(p, temp2);
	if(p) {
printf("[%s]\n", temp3);
		ExecPipeFinal(temp, "/bin/kill", "kill", signo, temp3, NULL); 
		return 0;
	} else {
		printf("%s process not found\n", devFirmwareName(NULL));
		return  1;
	}
}

