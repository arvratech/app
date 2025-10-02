static int		evtFileHead, evtFileSize, evtBufSize;
static unsigned char	evtBuf[512];


int fsOpenEvent(void)
{
	evtFileHead = evtBufSize = 0;
	fseek(fp5, 0L, SEEK_END);
	evtFileSize = ftell(fp5);
printf("fsOpenEvent: fileSize=%d\n", evtFileSize); 
	return 0;
}

void wdPing(void);

int _FsTruncateEvent(int offset)
{
	unsigned char	buf[4096];
	unsigned long	timer;
	long	srcp, dstp;
	int		rval, size;

printf("_FsTruncateEvtFile...%d\n", offset);
	fsFlushEvent();
	dstp = 0; srcp = offset;
	timer = MS_TIMER;
	while(1) {
		size = evtFileSize - srcp;
		if(size <= 0) {
			rval = 0;
			break;
		}
		if(size > 4096) size = 4096;
		fseek(fp5, srcp, SEEK_SET);
		rval = fread(buf, 1, size, fp5);
		if(rval != size) {
			rval = -1;
			break;	
		}
		srcp += size; 
		fseek(fp5, dstp, SEEK_SET);
		rval = fwrite(buf, 1, size, fp5);
		if(rval != size) {
			rval = -1;
			break;	
		}
		dstp += size; 
		if((MS_TIMER-timer) >= 1500) {
			wdPing();
			timer = MS_TIMER;
		}
	}
	if(rval < 0) {
printf("_FsTruncateEvent() error: evtFileHead=%d evtFileSize=%d %d %d\n", evtFileHead, evtFileSize, srcp, dstp); 
	} else {
		evtFileSize -= offset; evtFileHead = 0;
		fsync(fileno(fp5));
		//ftruncate(fileno(fp5), evtFileSize);
		fclose(fp5);
		truncate(FN_EVT, evtFileSize);
		fp5 = _OpenFile(FN_EVT);
printf("_FsTruncateEvent() OK: evtFileSize=%d %d %d\n", evtFileSize, srcp, dstp); 
	}
	return rval;
}

int fsCloseEvent(void)
{
printf("fsCloseEvent: %d\n", fsEventSize());
	fsFlushEvent();
	if(evtFileHead > 0) {
		_FsTruncateEvent(evtFileHead);
	}
	sync();
	return 0;
}

int fsAddEvent(unsigned char *data)
{
	int		rval, size;

	memcpy(evtBuf+evtBufSize, data, 16);
	evtBufSize += 16;
	if(evtBufSize >= 512) {
		fseek(fp5, 0L, SEEK_END);
		size = evtBufSize;
		rval = fwrite(evtBuf, 1, size, fp5);
printf("fsAddEvent; fwrite=%d\n", rval);
		if(rval != size) return -1;
		evtFileSize += size; evtBufSize = 0;
		fsync(fileno(fp5));
/*
		if(evtFileSize > 32000000) {
			if(evtFileHead > 3200000) size  = evtFileHead;
			else	size = 3200000;
			_FsTruncateEvent(size);
		}
*/
	}
	return 0;
}

int fsReadEvent(unsigned char *data)
{
	long	seekp;
	int		rval, size, mask, offset;
	
	if(evtFileHead < evtFileSize) {
		size = evtFileSize - evtFileHead;
		if(size > 512) size = 512;
		offset = evtFileHead + 512;
		mask = 511;
		offset &= ~mask;
		if(evtFileHead + size > offset) size = offset - evtFileHead;
		seekp = evtFileHead;
		fseek(fp5, seekp, SEEK_SET);
		rval = fread(data, 1, size, fp5);
	} else {
		rval = evtBufSize;
		if(rval > 0) memcpy(data, evtBuf, rval);
	}
	return rval;
}

int fsShiftEvent(int size)
{
	long	seekp;
	int		rval;

	if(evtFileHead < evtFileSize) {
		evtFileHead += size;
		if(evtFileHead >= evtFileSize) {
			fsync(fileno(fp5));
			//rval = ftruncate(fileno(fp5), 0);
			fp5 = _RemoveFile(fp5, FN_EVT);
			evtFileHead = evtFileSize = 0;
			sync();
		}
	} else {
		if(size < evtBufSize) {
			memcpy(evtBuf, evtBuf+size, evtBufSize-size);
			evtBufSize -= size;
		} else {
			evtBufSize = 0;
		}
	}
	return  size;
}

int fsFlushEvent(void)
{
	int		rval, size;

	size = evtBufSize;
	if(size > 0) {
		fseek(fp5, 0L, SEEK_END);
		rval = fwrite(evtBuf, 1, size, fp5);
printf("fsFlushEvent: fwrite=%d\n", rval);
		if(rval != size) return -1;
		evtFileSize += size; evtBufSize = 0;
		fsync(fileno(fp5));
	}
	return 0;
}

int fsEventSize(void)
{
	return evtFileSize - evtFileHead + evtBufSize;
}

int fsRemoveAllEvent(void)
{
	int		rval;

	if(evtFileSize > 0) {
		fsync(fileno(fp5));
		//rval = ftruncate(fileno(fp5), 0); // not working
		fp5 = _RemoveFile(fp5, FN_EVT);
	} else  rval = 0;
	evtFileHead = evtFileSize = evtBufSize = 0;
	sync();
	return rval;
}

