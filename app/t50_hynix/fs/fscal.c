int calfsSeek(long position)
{
	return fsSeek(f1, position, SEEK_SET);
}

long calfsTell(void)
{
	return fsTell(f1);
}

int calfsEof(void)
{
	return fsEof(f1);
}

int calfsGet(CALENDAR *cal)
{
	unsigned char	dID[4], buf[CALENDAR_RECORD_SZ];
	int		rval;

	calEncodeID(cal->ID, dID);
	fsSeek(f1, 0L, SEEK_SET);
	rval = fsFindRecord(f1, buf, CALENDAR_RECORD_SZ, dID, 1, 1);
	if(rval <= 0) return rval;
	calDecode(cal, buf + 2);
	return 1;
}

int calfsGetEncoded(long nID, unsigned char *data)
{
	unsigned char	dID[4];
	int		i, rval;

	dID[0] = (unsigned char)nID;
	rval = fsGetRecords(f1, data, 1, CALENDAR_RECORD_SZ, dID, 1);
	if(rval > 0) rval *= CALENDAR_BODY_SZ;
if(rval) {
cprintf("calfsGetEncoded: ID=%d\n", nID);
for(i = 0;i < 20;i++) cprintf("%02x-", data[i]);
cprintf("\n");
}
	return rval;
}

// Return -1:System error 0:Full 1:New 2:Exist(Overwrite) 3:Skip(Same)
int calfsAddEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[CALENDAR_RECORD_SZ];
	int		i, rval;

cprintf("calfsAddEncoded: ID=%d\n", nID);
for(i = 0;i < 20;i++) cprintf("%02x-", data[i]);
cprintf("\n");
	buf[0] = OT_CALENDAR; buf[1] = (unsigned char)nID; memcpy(buf+2, data, CALENDAR_BODY_SZ);
	flashWriteEnable();
	rval = fsAddRecord(f1, buf, CALENDAR_RECORD_SZ, 1);
	flashWriteDisable();
	return rval;
}

int calfsReadBulk(unsigned char *Data, int Size)
{
	int		rval;

	rval = fsReadRecord(f1, Data, CALENDAR_RECORD_SZ, Size / CALENDAR_RECORD_SZ);
	if(rval > 0) rval *= CALENDAR_RECORD_SZ;
	return rval;
}

int calfsAddBulk(unsigned char *Data, int Size)
{
	int		rval;

	fsSeek(f1, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f1, Data, Size);	
	flashWriteDisable();
	if(rval != Size) return -1;
	else	return 0;
}

int calfsRemove(long nID)
{
	unsigned char	dID[4];
	int		rval;

	calEncodeID(nID, dID);
	flashWriteEnable();
	rval = fsDeleteRecord(f1, CALENDAR_RECORD_SZ, dID, 1);
	flashWriteDisable();
	return rval;
}

int calfsRemoveAll(void)
{
	int		rval;

	fsSeek(f1, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f1, NULL, -1);
	flashWriteDisable();	
	if(rval < 0) return -1;
	else	return 1;
}

int calfsGetCount(void)
{
	return fsGetRecordCount(f1, CALENDAR_RECORD_SZ);
}
