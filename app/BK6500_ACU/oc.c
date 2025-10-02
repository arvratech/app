#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "prim.h"
#include "fsar.h"

#include "occfg.c"


int ocEncode(OUTPUT_COMMAND *oc, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	*p++ = oc->RepeatCount;
	for(i = 0;i < oc->Count;i++) {
		*p++ = oc->OutputCadences[i][0];	//OnTime
		*p++ = oc->OutputCadences[i][1];	//OffTime
	}
	i = p - (unsigned char *)buf;
	if(i < OUTPUT_COMMAND_BODY_SZ) memset(p, 0xff, OUTPUT_COMMAND_BODY_SZ-i);		
	return OUTPUT_COMMAND_BODY_SZ;
}

int ocDecode(OUTPUT_COMMAND *oc, void *buf)
{
	unsigned char	*p, c;
	int		i;

	p = (unsigned char *)buf;
	oc->RepeatCount = *p++;
	for(i = 0;i < MAX_OUTPUT_CADENCE_SZ;i++) {
		c = *p++;
		if(c == 0xff) break;
		oc->OutputCadences[i][0] = c;
		oc->OutputCadences[i][1] = *p++;
	}
	oc->Count = i;
	return OUTPUT_COMMAND_BODY_SZ;
}

int ocValidate(OUTPUT_COMMAND *oc)
{
	return 0;
}
