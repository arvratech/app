#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs_obj.h"
#include "prim.h"
#include "fsar.h"
#include "acap.h"


#include "acarcfg.c"


// Return	-1:Null  0:False  1:True
int arEvaluate(long arID, int apID, unsigned char *ctm)
{
	ACCESS_RIGHTS	*ar, _ar;
	ACCESS_POINT	*ap;
	unsigned char	*p, c;
	long	scheID;
	int		i, rval, ID;
	
//cprintf("AccessRights: arID=%ld apID=%d\n", arID, apID);
	if(arID < 2) return (int)arID;
	ar = &_ar;
	ar->ID = arID;
	rval = arfsGet(ar);
	if(rval <= 0) return 0;
	ap = apsGetAt(apID);
	rval = 0;
	for(i = 0;i < ar->Count;i++) {
		p = ar->AccessRules[i];
		c = *p; ID = c & 0x7f;
//cprintf("AcessRulses[%d] ID=%d %02x\n", i, c, (int)p[1]);
		if(c & 0x80) {
			if(ap->cfg->ZoneTo == ID) rval = 1;
		} else if(ID == apID) rval = 1;
		if(rval) break;
	}
	if(rval) PACK1toID(p+1, &scheID);
	else	 PACK1toID(&ar->DefaultTimeRange, &scheID);
//if(i < ar->Count) cprintf("AR=%ld AccessRules[%d]=%02x-%02x-%02x ScheID=%d\n", ar->ID, i, (int)p[0], (int)p[1], (int)p[2], scheID);
//else	cprintf("AR=%ld DefaultTimeRange=%02x SchID=%d\n", ar->ID, (int)ar->DefaultTimeRange, scheID);
	if(scheID >= 0) rval = schePresentValue(scheID, ctm);
	else	rval = 0;
	return rval;
}
