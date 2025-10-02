int userfsSeekEx(long position)
{
	return fsSeek(f2, position, SEEK_SET);
}

long userfsTellEx(void)
{
	return fsTell(f2);
}

int userfsEofEx(void)
{
	return fsEof(f2);
}

int userfsGetEx(FP_USER *user)
{
	unsigned char	dID[4], buf[USER_EX_RECORD_SZ];
	int		rval;

	userEncodeID(user->ID, dID);
	fsSeek(f2, 0L, SEEK_SET);
	rval = fsFindRecord(f2, buf, USER_EX_RECORD_SZ, dID, 1, 3);
	if(rval > 0) userDecodeEx(user, buf+4);
	return rval;
}

// Return -1:System error 0:Full 1:New user/Exist user(Overwrite)
int userfsAddEx(FP_USER *user)
{
	unsigned char	dID[4];
	long	offset;
	int		rval, found;

	userEncodeID(user->ID, dID);
	fsSeek(f2, 0L, SEEK_SET);
	found = fsFindRecord(f2, TmpBuf, USER_EX_RECORD_SZ, dID, 1, 3);
	if(found < 0) return -1;
	if(user->UserName[0] || user->UserExternalID[0]) {
		if(!found) {
			fsSeek(f2, 0L, SEEK_SET);
			rval = fsFindNullRecord(f2, USER_EX_RECORD_SZ);
			if(rval < 0) return rval;
			else if(rval == 0) {
				offset = fsTell(f2) + USER_EX_RECORD_SZ;
				if(offset > fsGetLimit(f2)) return 0;
			}
		}
		TmpBuf[0] = OT_USER;
		userEncodeID(user->ID, TmpBuf + 1);
		userEncodeEx(user, TmpBuf+4);	
		flashWriteEnable();
		rval = fsWrite(f2, TmpBuf, USER_EX_RECORD_SZ);
		flashWriteDisable();
		if(rval != USER_EX_RECORD_SZ) return -1;
	} else if(found) {
		TmpBuf[0] = 0x00;
		flashWriteEnable();
		rval = fsWrite(f2, TmpBuf, 1);
		flashWriteDisable();
	}
	return 1;
}

int userfsGetExEncoded(long nID, unsigned char *data)
{
	unsigned char	dID[4];
	int		rval;

	IDtoPACK3(nID, dID);
	rval = fsGetRecords(f2, data, 1, USER_EX_RECORD_SZ, dID, 3);
	if(rval > 0) rval = USER_EX_RECORD_SZ - 4;
	return rval;
}

// Return -1:System error 0:Full 1:New user 2:Exist user(Overwrite)
int userfsAddExEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[USER_EX_RECORD_SZ];
	int		rval;

	buf[0] = OT_USER; IDtoPACK3(nID, buf+1); memcpy(buf+4, data, USER_EX_RECORD_SZ-4);
	flashWriteEnable();
	rval = fsAddRecord(f2, buf, USER_EX_RECORD_SZ, 3);
	flashWriteDisable();
	return rval;
}

int userfsReadBulkEx(unsigned char *Data, int Size)
{
	int		rval;
	
	rval = fsReadRecord(f2, Data, USER_EX_RECORD_SZ, Size / USER_EX_RECORD_SZ);
	if(rval > 0) rval *= USER_EX_RECORD_SZ;
	return rval;
}

int userfsAddBulkEx(unsigned char *Data, int Size)
{
	int		rval;

	fsSeek(f2, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f2, Data, Size);	
	flashWriteDisable();
	if(rval != Size) return -1;
	else	return 0;
}

int userfsRemoveAllEx(void)
{
	int		rval;

	fsSeek(f2, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f2, NULL, -1);
	flashWriteDisable();	
	if(rval < 0) return -1;
	else	return 1;
}

int userfsGetExCount(void)
{
	return fsGetRecordCount(f2, USER_EX_RECORD_SZ);
}
