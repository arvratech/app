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
	unsigned char	buf[USER_AR_RECORD_SZ];
	int		rval;

	rval = _HashIndexGetAR(user->ID, buf);
	if(rval > 0) userDecodeAccessRights(user, buf + 4);
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
	unsigned char	buf[USER_AR_RECORD_SZ];
	int		rval;

	rval = _HashIndexGetAR(nID, buf);
	if(rval > 0) {
		rval = USER_AR_RECORD_SZ - 4;
		memcpy(data, buf+4, rval);
	}
	return rval;
}

// Return -1:System error 0:Full 1:New user 2:Exist user(Overwrite)
int userfsAddAccessRightsEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[USER_AR_RECORD_SZ];
	long	position;
	int		rval, idx;
	
	rval = _HashIndexGetAR(nID, buf);
	if(rval > 0) {
		if(n_memcmp(buf+4, data, USER_AR_RECORD_SZ-4)) {
			idx = rval - 1;
			_HashIndexRemoveAR(idx, nID);
			memcpy(buf+4, data, USER_AR_RECORD_SZ-4);
			position = idx << USER_AR_RECORD_SHIFT;
			fsSeek(f8, position, SEEK_SET);
			flashWriteEnable();
			rval = fsWrite(f8, buf, USER_AR_RECORD_SZ);
			flashWriteDisable();
			if(rval != USER_AR_RECORD_SZ) rval = -1;
			else {
				_HashIndexAddAR(idx, nID);
				rval = 2;
//cprintf("Overwrite Added(%d) Index=%d\n", nID, idx);
			}
		} else {
			rval = 2;
		}
	} else if(!rval) {
		buf[0] = OT_USER; IDtoPACK3(nID, buf+1); memcpy(buf+4, data, USER_AR_RECORD_SZ-4);
		flashWriteEnable();
		rval = fsAddRecordNull(f8, buf, USER_AR_RECORD_SZ);
		flashWriteDisable();
		if(rval > 0) {
			idx = (fsTell(f8) - USER_AR_RECORD_SZ) >> USER_AR_RECORD_SHIFT;
			_HashIndexAddAR(idx, nID);
			rval = 1;
//cprintf("New Added(%d) Index=%d\n", nID, idx);	
		}
	}
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
	unsigned char	*p;
	long	nID;
	int		rval, i, idx, count;

	fsSeek(f8, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f8, Data, Size);	
	flashWriteDisable();
	if(rval != Size) return -1;
	idx = (fsTell(f8) - Size) >> USER_AR_RECORD_SHIFT;
	count = Size >> USER_AR_RECORD_SHIFT; 
cprintf("Index=%d Count=%d\n", idx, count);
if(idx+count > MAX_USER_SZ) {
	cprintf("ERROR....................................\n");
	return -1;
}
	for(i = 0, p = Data;i < count;i++, idx++, p += USER_AR_RECORD_SZ) {
		userDecodeID(&nID, p+1);
		_HashIndexAddAR(idx, nID);
	}
	return 1;
}

int userfsRemoveAllAccessRights(void)
{
	int		rval;

	fsSeek(f8, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f8, NULL, -1);
	flashWriteDisable();
	memset(a_hashtbl, 0, MAX_HASH_SZ<<1);
	if(rval < 0) return -1;
	else	return 1;
}

int userfsGetAccessRightsCount(void)
{
	return fsGetRecordCount(f8, USER_AR_RECORD_SZ);
}
