int userfsSeekPhoto(long position)
{
	return fsSeek(f3, position, SEEK_SET);
}

long userfsTellPhoto(void)
{
	return fsTell(f3);
}

int userfsEofPhoto(void)
{
	return fsEof(f3);
}

int userfsGetPhoto(FP_USER *user)
{
	unsigned char	dID[4];
	int		rval;

	userEncodeID(user->ID, dID);
	fsSeek(f2, 0L, SEEK_SET);
	rval = fsFindRecord(f3, TmpBuf, USER_PHOTO_RECORD_SZ, dID, 1, 3);
	if(rval > 0) userDecodePhoto(user, TmpBuf+4);
	return rval;
//cprintf("userfsGetEx: QL=%d Width=%d Height=%d %02x-%02x-%02x-%02x\n", (int)user->QuantityLevel, (int)user->Width, (int)user->Height, (int)user->JPEGBuffer[0], (int)user->JPEGBuffer[1], (int)user->JPEGBuffer[2], (int)user->JPEGBuffer[3]);
}

// Return -1:System error 0:Full 1:New user/Exist user(Overwrite)
int userfsAddPhoto(FP_USER *user)
{
	unsigned char	dID[4];
	long	offset;
	int		rval, found;

	userEncodeID(user->ID, dID);
	fsSeek(f3, 0L, SEEK_SET);
	while((rval = fsRead(f3, TmpBuf, USER_PHOTO_RECORD_SZ)) == USER_PHOTO_RECORD_SZ)
		if(TmpBuf[0] && !n_memcmp(dID, TmpBuf+1, 3)) break;
	if(rval < 0) return -1;
	else if(rval >= USER_PHOTO_RECORD_SZ) found = 1;
	else	found = 0;
	if(user->Width && user->Height) {
		if(!found) {
			fsSeek(f3, 0L, SEEK_SET);
			while((rval = fsRead(f3, TmpBuf, USER_PHOTO_RECORD_SZ)) == USER_PHOTO_RECORD_SZ)
				if(!TmpBuf[0]) break;
			if(rval < 0) return rval;
			else if(rval >= USER_PHOTO_RECORD_SZ) found = 1;
			else {
				offset = fsTell(f3) + USER_PHOTO_RECORD_SZ;
				if(offset > fsGetLimit(f3)) return 0;
			}
		}
		if(found) {
			offset = -USER_PHOTO_RECORD_SZ;
			fsSeek(f3, offset, SEEK_CUR);
		}
		TmpBuf[0] = OT_USER;
		userEncodeID(user->ID, TmpBuf + 1);
		userEncodePhoto(user, TmpBuf+4);	
		flashWriteEnable();
		rval = fsWrite(f3, TmpBuf, USER_PHOTO_RECORD_SZ);
		flashWriteDisable();
		if(rval != USER_PHOTO_RECORD_SZ) return -1;
	} else if(found) {
		TmpBuf[0] = 0x00;
		flashWriteEnable();
		rval = fsWrite(f3, TmpBuf, 1);
		flashWriteDisable();
	}
	return 1;
}

int userfsGetPhotoEncoded(long nID, unsigned char *data)
{
	unsigned char	dID[4];
	int		rval;

	IDtoPACK3(nID, dID);
	rval = fsGetRecords(f3, data, 1, USER_PHOTO_RECORD_SZ, dID, 3);
	if(rval > 0) rval = USER_PHOTO_RECORD_SZ - 4;
	return rval;
}

// Return -1:System error 0:Full 1:New user 2:Exist user(Overwrite)
int userfsAddPhotoEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[USER_EX_RECORD_SZ];
	int		rval;

	buf[0] = OT_USER; IDtoPACK3(nID, buf+1); memcpy(buf+4, data, USER_EX_RECORD_SZ-4);
	flashWriteEnable();
	rval = fsAddRecord(f3, buf, USER_PHOTO_RECORD_SZ, 3);
	flashWriteDisable();
	return rval;
}

int userfsReadBulkPhoto(unsigned char *Data, int Size)
{
	int		rval;
	
	rval = fsReadRecord(f3, Data, USER_PHOTO_RECORD_SZ, Size / USER_PHOTO_RECORD_SZ);
	if(rval > 0) rval *= USER_PHOTO_RECORD_SZ;
	return rval;
}

int userfsAddBulkPhoto(unsigned char *Data, int Size)
{
	int		rval;

	fsSeek(f3, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f3, Data, Size);	
	flashWriteDisable();
	if(rval != Size) return -1;
	else	return 0;
}

int userfsRemoveAllPhoto(void)
{
	int		rval;

	fsSeek(f3, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f3, NULL, -1);
	flashWriteDisable();	
	if(rval < 0) return -1;
	else	return 1;
}

int userfsGetPhotoCount(void)
{
	return fsGetRecordCount(f3, USER_PHOTO_RECORD_SZ);
}
