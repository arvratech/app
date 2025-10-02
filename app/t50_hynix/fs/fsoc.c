int ocfsSeek(long position)
{
	return fsSeek(f4, position, SEEK_SET);
}

long ocfsTell(void)
{
	return fsTell(f4);
}

int ocfsEof(void)
{
	return fsEof(f4);
}

int ocfsGet(OUTPUT_COMMAND *oc)
{
	unsigned char	dID[4], buf[OUTPUT_COMMAND_RECORD_SZ];
	int		rval;

	ocEncodeID(oc->ID, dID);
	fsSeek(f4, 0L, SEEK_SET);
	rval = fsFindRecord(f4, buf, OUTPUT_COMMAND_RECORD_SZ, dID, 1, 1);
	if(rval <= 0) return rval;
	ocDecode(oc, buf + 2);
	return 1;
}

int ocfsGetEncoded(long nID, unsigned char *data)
{
	unsigned char	dID[4];
	int		rval;

	dID[0] = (unsigned char)nID;
	rval = fsGetRecords(f4, data, 1, OUTPUT_COMMAND_RECORD_SZ, dID, 1);
	if(rval > 0) rval = OUTPUT_COMMAND_RECORD_SZ - 2;
	return rval;
}

// Return -1:System error 0:Full 1:New 2:Exist(Overwrite) 3:Skip(Same)
int ocfsAddEncoded(long nID, unsigned char *data)
{
	unsigned char	buf[OUTPUT_COMMAND_RECORD_SZ];
	int		rval;

	buf[0] = OT_OUTPUT_COMMAND; buf[1] = (unsigned char)nID; memcpy(buf+2, data, OUTPUT_COMMAND_BODY_SZ);
	flashWriteEnable();
	rval = fsAddRecord(f4, buf, OUTPUT_COMMAND_RECORD_SZ, 1);
	flashWriteDisable();
	return rval;
}

int ocfsReadBulk(unsigned char *Data, int Size)
{
	int		rval;

	rval = fsReadRecord(f4, Data, OUTPUT_COMMAND_RECORD_SZ, Size / OUTPUT_COMMAND_RECORD_SZ);
	if(rval > 0) rval *= OUTPUT_COMMAND_RECORD_SZ;
	return rval;
}

int ocfsAddBulk(unsigned char *Data, int Size)
{
	int		rval;

	fsSeek(f4, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f4, Data, Size);	
	flashWriteDisable();
	if(rval != Size) return -1;
	else	return 0;
}

int ocfsRemove(long nID)
{
	unsigned char	dID[4];
	int		rval;

	ocEncodeID(nID, dID);
	flashWriteEnable();
	rval = fsDeleteRecord(f4, OUTPUT_COMMAND_RECORD_SZ, dID, 1);
	flashWriteDisable();
	return rval;
}

int ocfsRemoveAll(void)
{
	int		rval;

	fsSeek(f4, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f4, NULL, -1);
	flashWriteDisable();	
	if(rval < 0) return -1;
	else	return 1;
}

int ocfsGetCount(void)
{
	return fsGetRecordCount(f4, OUTPUT_COMMAND_RECORD_SZ);
}
