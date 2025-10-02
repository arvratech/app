#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs_obj.h"
#include "prim.h"
#include "pno.h"


#include "pnocfg.c"


void pnoCopy(PHONE_NO *pnoD, PHONE_NO *pnoS)
{
	memcpy(pnoD, pnoS, sizeof(PHONE_NO));	
}

int pnoCompare(PHONE_NO *pno1, PHONE_NO *pno2)
{
	unsigned char	buf1[64], buf2[64];
	int		rval;

	pnoEncode(pno1, buf1);
	pnoEncode(pno2, buf2);
	rval = n_memcmp(buf1+1, buf2+1, 8);
	return rval;
}

#include "cotask.h"
#include "fsuser.h"

long pnoGetNewID(long StartID, long MinID, long MaxID)
{
	PHONE_NO	*pno, _pno;
	long	nID;

	pno = &_pno;
	if(StartID <= MinID || StartID > MaxID) StartID = MinID;
	nID = StartID;
	while(1) {
		pno->ObjectID = nID;
//cprintf("NewUserID [%s] %d\n", user->ID, userfsGet(user));	 
		if(pnofsGet(pno) <= 0) return nID;
		nID++;
		if(nID > MaxID) nID = MinID;
		if(!(nID & 0xf)) taskYield();
	} while(nID != StartID) ;
	return 0L;
}
