int fsAddEvent(unsigned char *data, int size)
{
	int		rval;

	fsSeek(f5, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f5, data, size);	
	flashWriteDisable();
//cprintf("fsWriteEvent=%d\n", rval);
	if(rval != size) return -1;
	else	return 0;
}

int fsReadEvent(unsigned char *data)
{
	int		rval;
	
	rval = fsReadFirstPage(f5, data);
//if(rval) cprintf("fsReadEvent=%d\n", rval);
	return rval;
}

int fsReadNextEvent(unsigned char *data)
{
	int		rval, size;

	size = flashGetBytesPerPage();
	rval = fsRead(f5, data, size);
	return rval;
}

int fsShiftEvent(int size)
{
	int		rval;

	fsSeek(f5, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f5, NULL, size);
	flashWriteDisable();
	return rval;
}

int fsGetEventSize(void)
{
	return (int)fsFileLength(f5);
}

int fsRemoveAllEvent(void)
{
	int		rval;

	fsSeek(f5, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f5, NULL, -1);
	flashWriteDisable();
	if(rval < 0) return -1;
	else	return 1;
}
