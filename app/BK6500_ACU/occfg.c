#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "oc.h"


int ocEncodeID(long nID, void *buf)
{
	IDtoPACK1(nID, buf);
	return 1;
}

int ocDecodeID(long *pID, void *buf)
{
	PACK1toID(buf, pID);
	return 1;
}

int ocValidateID(long nID)
{
	if(nID >= 0 && nID < 0xff) return 1;
	else	return 0;
}
