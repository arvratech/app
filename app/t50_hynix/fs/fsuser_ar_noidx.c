int userfsSeekAccessRights(long position)
{
	return fsSeek(f8, position, SEEK_SET);
}

long userfsTellAccessRights(void)
{
	return fsTell(f8);
}

int userfsEofAccessRights(void)
{
	return fsEof(f8);
}

int userfsGetAccessRights(FP_USER *user)
{
	unsigned char	dID[4], buf[USER_AR_RECORD_SZ];
	int		rval;

	userEncodeID(user->ID, dID);
	fsSeek(f8, 0L, SEEK_SET);
	rval = fsFindRecord(f8, buf, USER_AR_RECORD_SZ, dID, 1, 3);
	if(rval > 0) userDecodeAccessRights(user, buf+4);
	return rval;
}
/*
// Return -1:System error 0:Full 1:New user/Exist user(Overwrite)
int userfsAddAccessRights(FP_USER *user)
{
	unsigned char	dID[4];
	long	offset;
	int		rval, found;

	userEncodeID(user->ID, dID);
	fsSeek(f8, 0L, SEEK_SET);
	found = fsFindRecord(f8, TmpBuf, USER_AR_RECORD_SZ, dID, 1, 3);
	if(found < 0) return -1;
	if(user->AccessRights == 0xffffff) {
		if(!found) {
			fsSeek(f8, 0L, SEEK_SET);
			rval = fsFindNullRecord(f8, USER_AR_RECORD_SZ);
			if(rval < 0) return rval;
			else if(rval == 0) {
				offset = fsTell(f8) + USER_AR_RECORD_SZ;
				if(offset > fsGetLimit(f8)) return 0;
			}
		}
		TmpBuf[0] = OT_USER;
		userEncodeID(user->ID, TmpBuf + 1);
		userEncodeAccessRights(user, TmpBuf+4);	
		flashWriteEnable();
		rval = fsWrite(f8, TmpBuf, USER_AR_RECORD_SZ);
		flashWriteDisable();
		if(rval != USER_AR_RECORD_SZ) return -1;
	} else if(found) {
		TmpBuf[0] = 0x00;
		flashWriteEnable();
		rval = fsWrite(f8, TmpBuf, 1);
		flashWriteDisable();
	}
	return 1;
}
*/
int userfsGetAccessRightsEncoded(long nID, unsigned char *data)
{
	unsigned char	dID[4];
	int		rval;

	IDtoPACK3(nID, dID);
	rval = fsGetRecords(f8, data, 1, USER_AR_RECORD_SZ, dID, 3);
	if(rval > 0) rval = USER_AR_RECORD_SZ - 4;
	return rval;
}

// Return -1:System error 0:Full 1:New user 2:Exist user(Overwrite)
int userfsAddAccessRightsEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[USER_AR_RECORD_SZ];
	int		rval;

	buf[0] = OT_USER; IDtoPACK3(nID, buf+1); memcpy(buf+4, data, USER_AR_RECORD_SZ-4);
	flashWriteEnable();
	rval = fsAddRecord(f8, buf, USER_AR_RECORD_SZ, 3);
	flashWriteDisable();
	return rval;
}

int userfsReadBulkAccessRights(unsigned char *Data, int Size)
{
	int		rval;
	
	rval = fsReadRecord(f8, Data, USER_AR_RECORD_SZ, Size / USER_AR_RECORD_SZ);
	if(rval > 0) rval *= USER_AR_RECORD_SZ;
	return rval;
}

int userfsAddBulkAccessRights(unsigned char *Data, int Size)
{
	int		rval;

	fsSeek(f8, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f8, Data, Size);	
	flashWriteDisable();
	if(rval != Size) return -1;
	else	return 0;
}

int userfsRemoveAllAccessRights(void)
{
	int		rval;

	fsSeek(f8, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f8, NULL, -1);
	flashWriteDisable();	
	if(rval < 0) return -1;
	else	return 1;
}

int userfsGetAccessRightsCount(void)
{
	return fsGetRecordCount(f8, USER_AR_RECORD_SZ);
}
