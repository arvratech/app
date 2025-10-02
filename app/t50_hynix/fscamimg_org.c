// 55-AA-55-AA-sh-sl-EVT(16)-rs-ql-wh-wl-hh-hl-Data......
// sh-sl(Size) = 28 + DataSize   wh-wl(Width)   hh-hl(Height)
int fsAddCamImage(unsigned char *data, int size)
{
	unsigned short	sVal;
	int		rval, page;

	data[0] = 0x55; data[1] = 0xaa; data[2] = 0x55; data[3] = 0xaa;
	sVal = (unsigned short)size;
	SHORTtoBYTE(sVal, data+4);
	page = flashGetBytesPerPage();
//cprintf("fsAddCamImage=%d %d %02x-%02x-%02x-%02x\n", size, size-28, (int)data[28], (int)data[29], (int)data[30], (int)data[31]);
	size = (size + page - 1) / page * page;
	fsSeek(f7, 0L, SEEK_END);
	flashWriteEnable();
	rval = fsWrite(f7, data, size);	
	flashWriteDisable();
	if(rval != size) return -1;
	else	return 0;
}

int fsReadCamImage(unsigned char *data)
{
	unsigned short	sVal;
	int		rval, size, page;
	
	page = flashGetBytesPerPage();
	while(1) {
		rval = fsReadFirstPage(f7, data);
		if(rval <= 0 || rval >= page && data[0] == 0x55 && data[1] == 0xaa && data[2] == 0x55 && data[3] == 0xaa) break;
		fsSeek(f7, 0L, SEEK_SET);
		flashWriteEnable();
		rval = fsShift(f7, NULL, rval);
		flashWriteDisable();
	}
	if(rval < page) return rval;
	BYTEtoSHORT(data+4, &sVal);
	size = sVal;
	if(size > page) rval = fsRead(f7, data+page, size-page);
//cprintf("fsReadCamImage=%d %d %02x-%02x-%02x-%02x\n", size, size-28, (int)data[28], (int)data[29], (int)data[30], (int)data[31]);
	return size;
}

int fsShiftCamImage(int size)
{
	int		rval;

	rval = flashGetBytesPerPage();
	size = (size + rval - 1) / rval * rval;
	fsSeek(f7, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f7, NULL, size);
	flashWriteDisable();
//cprintf("fsShiftCamImage=%d\n", rval);
	return rval;
}

int fsGetCamImageSize(void)
{
	return (int)fsFileLength(f7);
}

int fsRemoveAllCamImage(void)
{
	int		rval;

	fsSeek(f7, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f7, NULL, -1);
	flashWriteDisable();
	if(rval < 0) return -1;
	else	return 1;
}

