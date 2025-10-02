int schefsSeek(long position)
{
	return fsSeek(f2, position, SEEK_SET);
}

long schefsTell(void)
{
	return fsTell(f2);
}

int schefsEof(void)
{
	return fsEof(f2);
}

int schefsGet(SCHEDULE *sche)
{
	unsigned char	dID[4], buf[SCHEDULE_RECORD_SZ];
	int		rval;

	scheEncodeID(sche->ID, dID);
	fsSeek(f2, 0L, SEEK_SET);
	rval = fsFindRecord(f2, buf, SCHEDULE_RECORD_SZ, dID, 1, 1);
	if(rval <= 0) return rval;
	scheDecode(sche, buf + 2);
	return 1;
}

int schefsGetEncoded(long nID, unsigned char *data)
{
	unsigned char	dID[4];
	int		rval;

	dID[0] = (unsigned char)nID;
	rval = fsGetRecords(f2, data, 1, SCHEDULE_RECORD_SZ, dID, 1);
	if(rval > 0) rval = SCHEDULE_RECORD_SZ - 2;
	return rval;
}

// Return -1:System error 0:Full 1:New 2:Exist(Overwrite) 3:Skip(Same)
int schefsAddEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[SCHEDULE_RECORD_SZ];
	int		rval;

	buf[0] = OT_SCHEDULE; buf[1] = (unsigned char)nID; memcpy(buf+2, data, SCHEDULE_BODY_SZ);
	flashWriteEnable();
	rval = fsAddRecord(f2, buf, SCHEDULE_RECORD_SZ, 1);
	flashWriteDisable();
	return rval;
}

int schefsReadBulk(unsigned char *Data, int Size)
{
	int		rval;

	rval = fsReadRecord(f2, Data, SCHEDULE_RECORD_SZ, Size / SCHEDULE_RECORD_SZ);
	if(rval > 0) rval *= SCHEDULE_RECORD_SZ;
	return rval;
}

int schefsAddBulk(unsigned char *Data, int Size)
{
	int		rval;

	fsSeek(f2, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f2, Data, Size);	
	flashWriteDisable();
	if(rval != Size) return -1;
	else	return 0;
}

int schefsRemove(long nID)
{
	unsigned char	dID[4];
	int		rval;

	scheEncodeID(nID, dID);
	flashWriteEnable();
	rval = fsDeleteRecord(f2, SCHEDULE_RECORD_SZ, dID, 1);
	flashWriteDisable();
	return rval;
}

int schefsRemoveAll(void)
{
	int		rval;

	fsSeek(f2, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f2, NULL, -1);
	flashWriteDisable();	
	if(rval < 0) return -1;
	else	return 1;
}

int schefsGetCount(void)
{
	return fsGetRecordCount(f2, SCHEDULE_RECORD_SZ);
}
