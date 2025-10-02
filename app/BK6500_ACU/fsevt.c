#define MAX_EVTBUF_SZ		512

static int		evtBufSize;
static unsigned char	evtBuf[MAX_EVTBUF_SZ];


int fsOpenEvent(void)
{
	evtBufSize = 0;
	return 0;
}

int fsCloseEvent(void)
{
//printf("fsCloseEvent: %d\n", fsEventSize()); 
	fsFlushEvent();
	return 0;
}

int fsAddEvent(unsigned char *data)
{
	int		rval, size;

	memcpy(evtBuf+evtBufSize, data, 16);
	evtBufSize += 16;
	if(evtBufSize >= MAX_EVTBUF_SZ) {
		fsSeek(f5, 0L, SEEK_END);
		size = evtBufSize;
		flashWriteEnable();
		rval = fsWrite(f5, evtBuf, size);	
		flashWriteDisable();
		if(rval != size) return -1;
		evtBufSize = 0;
	}
	return 0;
}

int fsReadEvent(unsigned char *data)
{
	int		rval;
	
	rval = fsReadFirstPage(f5, data);
	if(rval <= 0) {
		rval = evtBufSize;
		if(rval > 0) memcpy(data, evtBuf, rval);
	}
	return rval;
}

int fsShiftEvent(int size)
{
	int		rval;

	if(fsFileLength(f5) > 0) {
		fsSeek(f5, 0L, SEEK_SET);
		flashWriteEnable();
		rval = fsShift(f5, NULL, size);
		flashWriteDisable();
	} else {
		if(size < evtBufSize) {
			memcpy(evtBuf, evtBuf+size, evtBufSize-size);
			evtBufSize -= size;
		} else {
			evtBufSize = 0;
		}
		rval = size;
	}
	return  size;
}

int fsFlushEvent(void)
{
	int		rval, size;

	size = evtBufSize;
	if(size > 0) {
		fsSeek(f5, 0L, SEEK_END);
		flashWriteEnable();
		rval = fsWrite(f5, evtBuf, size);	
		flashWriteDisable();
		if(rval != size) return -1;		
	}
	evtBufSize = 0;	
	return 0;
}

int fsEventSize(void)
{
	return fsFileLength(f5) + evtBufSize;
}

int fsRemoveAllEvent(void)
{
	int		rval;

	evtBufSize = 0;
	fsSeek(f5, 0L, SEEK_SET);
	flashWriteEnable();
	rval = fsShift(f5, NULL, -1);
	flashWriteDisable();
	if(rval < 0) return -1;
	else	return 1;
}
