int userfsSeekEx(long position)
{
	return fseek(fp2, position, SEEK_SET);
}

long userfsTellEx(void)
{
	return ftell(fp2);
}

int userfsEofEx(void)
{
	return _FsEof(fp2);
}

int userfsSeekAtIndexEx(int index)
{
	unsigned char   buf[USER_EX_RECORD_SZ];
	int     rval, count;

	fseek(fp2, 0L, SEEK_SET);
	count = rval = 0;
	while(1) {
		if(count >= index) break;
		rval = fread(buf, 1, USER_EX_RECORD_SZ, fp2);
		if(rval < USER_EX_RECORD_SZ) break;
		if(buf[0]) count++;
	}
	if(rval >= 0) rval = count;
	return rval;
}

int userfsGetEx(FP_USER *user)
{
	unsigned char	dId[4], buf[USER_EX_RECORD_SZ];
	int		rval;

	userEncodeId(user->id, dId);
	fseek(fp2, 0L, SEEK_SET);
	rval = fsFindRecord(fp2, buf, USER_EX_RECORD_SZ, dId, 1, 3);
	if(rval > 0) userDecodeEx(user, buf+4);
	return rval;
}

// Return -1:System error 0:Full 1:New user/Exist user(Overwrite)
int userfsAddEx(FP_USER *user)
{
	unsigned char	dId[4];
	long	offset;
	int		rval, found;

	userEncodeId(user->id, dId);
	fseek(fp2, 0L, SEEK_SET);
	found = fsFindRecord(fp2, TmpBuf, USER_EX_RECORD_SZ, dId, 1, 3);
	if(found < 0) return -1;
	if(user->userName[0] || user->userExternalId[0]) {
		if(!found) {
			fseek(fp2, 0L, SEEK_SET);
			rval = fsFindNullRecord(fp2, USER_EX_RECORD_SZ);
			if(rval < 0) return rval;
			else if(rval == 0) {
				offset = ftell(fp2) + USER_EX_RECORD_SZ;
				//if(offset > fsGetLimit(f2)) return 0;
			}
		}
		TmpBuf[0] = OT_USER;
		userEncodeId(user->id, TmpBuf + 1);
		userEncodeEx(user, TmpBuf+4);	
		rval = fwrite(TmpBuf, 1, USER_EX_RECORD_SZ, fp2);
		if(rval != USER_EX_RECORD_SZ) return -1;
	} else if(found) {
		TmpBuf[0] = 0x00;
		rval = fwrite(TmpBuf, 1, 1, fp2);
	}
	return 1;
}

/*
int userfsGetExEncoded(long id, unsigned char *data)
{
	unsigned char	dId[4];
	int		rval;

	IDtoPACK3(id, dId);
	rval = fsGetRecords(fp2, data, 1, USER_EX_RECORD_SZ, dId, 3);
	if(rval > 0) rval = USER_EX_RECORD_SZ - 4;
	return rval;
}
*/

// Return -1:System error 0:Full 1:New user 2:Exist user(Overwrite)
int userfsAddExEncoded(long id, unsigned char *data)
{
	unsigned char	buf[USER_EX_RECORD_SZ];
	int		rval;

	buf[0] = OT_USER; IDtoPACK3(id, buf+1); memcpy(buf+4, data, USER_EX_RECORD_SZ-4);
	rval = fsAddRecord(fp2, buf, USER_EX_RECORD_SZ, 3);
	return rval;
}

int userfsReadBulkEx(unsigned char *data, int size)
{
	int		rval;
	
	rval = fsReadRecord(fp2, data, USER_EX_RECORD_SZ, size / USER_EX_RECORD_SZ);
	if(rval > 0) rval *= USER_EX_RECORD_SZ;
	return rval;
}

int userfsAddBulkEx(unsigned char *data, int size)
{
	int		rval;

	fseek(fp2, 0L, SEEK_END);
	rval = fwrite(data, 1, size, fp2);	
	if(rval != size) return -1;
	else	return 0;
}

int userfsRemoveAllEx(void)
{
	fp2 = _RemoveFile(fp2, FN_USREX);
	if(!fp2) return -1;
	else	return 0;	
}

int userfsGetExCount(void)
{
	return fsGetRecordCount(fp2, USER_EX_RECORD_SZ);
}

