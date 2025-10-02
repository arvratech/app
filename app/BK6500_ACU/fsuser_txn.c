int userfsSeekTransaction(long position)
{
	return fsSeek(f4, position, SEEK_SET);
}

int userfsReadTransaction(FP_USER *user)
{
	unsigned char	buf[USER_TXN_RECORD_SZ];
	int		rval;

	while((rval = fsRead(f4, buf, USER_TXN_RECORD_SZ)) == USER_TXN_RECORD_SZ)
		if(buf[0]) break;
	if(rval < 0) return -1;
	else if(rval < USER_TXN_RECORD_SZ) return 0;
	else {
		if(user) {
			userDecodeID(&user->ID, buf+1);
			userDecodeTransaction(user, buf+4);
		}
		return 1;
	}
}

int userfsGetTransaction(FP_USER *user)
{
	unsigned char	dID[4], buf[USER_TXN_RECORD_SZ];
	int		rval;

	userEncodeID(user->ID, dID);
	fsSeek(f4, 0L, SEEK_SET);
	rval = fsFindRecord(f4, buf, USER_TXN_RECORD_SZ, dID, 1, 3);
	if(rval > 0) userDecodeTransaction(user, buf+4);
	return rval;
}

// Return -1:System error 0:Full 1:New user 2:Exist user(Overwrite)
int userfsAddTransaction(FP_USER *user)
{
	unsigned char	buf[USER_TXN_RECORD_SZ];
	int		rval;

	buf[0] = OT_USER;
	userEncodeID(user->ID, buf+1);
	userEncodeTransaction(user, buf+4);
	flashWriteEnable();
	rval = fsAddRecord(f4, buf, USER_TXN_RECORD_SZ, 3);
	flashWriteDisable();
//cprintf("%d AddTxn: ID=%d AP=%d ZoneTo=%d\n", rval, user->ID, (int)user->AccessPoint, (int)user->ZoneTo);
	return rval;
}	

int userfsRemoveTransaction(long nID)
{
	unsigned char	dID[4];
	int		rval;

	userEncodeID(nID, dID);
	flashWriteEnable();
	rval = fsDeleteRecord(f4, USER_TXN_RECORD_SZ, dID, 3);
	flashWriteDisable();
	return rval;
}

int userfsRemoveAllTransaction(void)
{
	int		rval;

	fsSeek(f4, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f4, NULL, -1);
	flashWriteDisable();	
	if(rval < 0) return -1;
	else	return 1;
}
