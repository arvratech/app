#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "prim.h"
#include "defs_obj.h"
#include "unitcfg.h"
#include "acar.h"


int arEncodeID(long nID, void *buf)
{
	IDtoPACK3(nID, (unsigned char *)buf);
	return 3;
}

int arDecodeID(long *pID, void *buf)
{
	PACK3toID((unsigned char *)buf, pID);
	return 3;
}

int arValidateID(long nID)
{
	if(nID >= 2 && nID < 0xffffff) return 1;
	else	return 0;
}

int arEncode(ACCESS_RIGHTS *ar, void *buf)
{
	unsigned char	*p;
	int		size;

	p = (unsigned char *)buf;
	memcpy(p, ar->Accompaniment, 4); p += 4;
	*p++ = ar->DefaultTimeRange;
	size = ar->Count << 1;
	memcpy(p, ar->AccessRules, size);
	size = p - (unsigned char *)buf;
	if(size < ACCESS_RIGHTS_BODY_SZ) memset(p, 0xff, ACCESS_RIGHTS_BODY_SZ-size);
	return ACCESS_RIGHTS_BODY_SZ;
}

int arDecode(ACCESS_RIGHTS *ar, void *buf)
{
	unsigned char	*p, c;
	int		i;

	p = (unsigned char *)buf;
	memcpy(ar->Accompaniment, p, 4); p += 4;
	ar->DefaultTimeRange = *p++;
	for(i = 0;i < 27;i++) {
		c = *p++;
		if(c == 0xff) break;
		ar->AccessRules[i][0] = c;
		ar->AccessRules[i][1] = *p++;
	}
	ar->Count = i;
	return ACCESS_RIGHTS_BODY_SZ;
}

int arValidate(ACCESS_RIGHTS *ar)
{
	int		i, rval;

	i = ar->Accompaniment[0];
	if(i != 0xff && i != OT_USER && i != OT_ACCESS_RIGHTS) return 0;
	if(ar->DefaultTimeRange == 0xff) return 0;
	rval = 0;
	for(i = 0;i < ar->Count;i++) {
		rval = aruleValidate(ar->AccessRules[i]); break;
		if(!rval) break;
	}
	return rval;
}

// [0]Location.ObjectType [1]Location.ObjectID [2]TimeRange.ObjectID
int aruleValidate(unsigned char *arule)
{
	int		ID;
	
	ID = arule[0] & 0x7f;
	if(ID > unitsGetMaxPointID() || arule[1] == 0xff) return 0;
	return 1;
}
