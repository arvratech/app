#define CAMEVT_BUF_SZ	819200		// 4915200
static int	camevtFileHead, camevtFileSize, camevtBufSize;
static unsigned char	camevtBuf[CAMEVT_BUF_SZ];


int fsOpenCamEvt(void)
{
	unsigned char	buf[512];
	int		rval, page;

	page = 512;
	camevtFileHead = 0;
	fseek(fp7, 0L, SEEK_SET);
	while(1) {
		rval = fread(buf, 1, page, fp7);
		if(rval < page || buf[0] == 0x55 && buf[1] == 0xaa && buf[2] == 0x55 && buf[3] == 0xaa) break;
		camevtFileHead += page;
	}
	fseek(fp7, 0L, SEEK_END);
	camevtFileSize = ftell(fp7);
printf("fsOpenCamEvt: fileSize=%d start=%d\n", camevtFileSize, camevtFileHead);
	if(camevtFileHead >= camevtFileSize) {
		rval = ftruncate(fileno(fp7), 0);
		camevtFileHead = camevtFileSize = 0;
	}
	camevtBufSize = 0;
}

int fsCloseCamEvt(void)
{
	fsFlushCamEvt();
	return 0;
}

// 55-AA-55-AA-sz-sz-sz-sz-EVT(16)-JPEG data......
// sz(size) = jpegBufLength + 24
int fsAddCamEvt(unsigned char *evtRecord, unsigned char *jpegBuf, int jpegBufLength)
{
	unsigned char	*p;
	unsigned long	lval;
	int		rval, size, bsize, page, i;

	page = 512;
	size = jpegBufLength + 24;
	bsize = (size + page - 1) / page * page;
	if(camevtBufSize + bsize > CAMEVT_BUF_SZ) {
		fseek(fp7, 0L, SEEK_END);
		rval = fwrite(camevtBuf, 1, camevtBufSize, fp7);
printf("fsAddCamEvt; fwrite=%d\n", rval);
		if(rval != camevtBufSize) return -1;
		camevtFileSize += camevtBufSize;
		camevtBufSize = 0;
	}
printf("fsAddCamEvt: %d jpegLen=%d\n", camevtBufSize, jpegBufLength);
	p = camevtBuf + camevtBufSize;
	p[0] = 0x55; p[1] = 0xaa; p[2] = 0x55; p[3] = 0xaa;
	lval = (unsigned long)size; LONGtoBYTE(lval, p+4);
	memcpy(p+8, evtRecord, 16);
	memcpy(p+24, jpegBuf, size-24);
	camevtBufSize += bsize;
	return 0;
}

int fsReadCamEvt(unsigned char *buf)
{
	unsigned char	*p, tmp[512];
	unsigned long	lval;
	long	seekp;
	int		rval, size, count, page;

	page = 512;
	if(camevtFileSize > camevtFileHead) {
		seekp = camevtFileHead;
		fseek(fp7, seekp, SEEK_SET);
		while(1) {
			rval = fread(tmp, 1, page, fp7);
			if(rval < page) {
printf("fsReadCamEvt: format error...rval=%d\n", rval);
				rval = ftruncate(fileno(fp7), 0);
				camevtFileHead = camevtFileSize = 0;
				return 0;
			}
			if(tmp[0] == 0x55 && tmp[1] == 0xaa && tmp[2] == 0x55 && tmp[3] == 0xaa) break;
printf("fsReadCamEvt: format error...offset=%d\n", camevtFileHead);
			camevtFileHead += page;
		}
		rval = page - 8;
		memcpy(buf, tmp+8, rval);
		BYTEtoLONG(tmp+4, &lval); size = lval - 8;
		if(size > rval) {
			count = size - rval;
			rval = fread(buf+rval, 1, count, fp7);
			if(rval != count) {
printf("fsReadCamEvt: fread error...%d/%d\n", rval, count);
				rval = ftruncate(fileno(fp7), 0);
				camevtFileHead = camevtFileSize = 0;
				size = 0;
			}
		}
	} else {
		p = camevtBuf; size = camevtBufSize;
		if(size > 0) {
			if(p[0] == 0x55 && p[1] == 0xaa && p[2] == 0x55 && p[3] == 0xaa) {
				BYTEtoLONG(p+4, &lval); size = lval - 8;
				memcpy(buf, p+8, size);
			} else {
printf("fsReadCamEvt: buf format error...%u\n", lval);
				camevtBufSize = 0;
				size = 0;
			}
		}
	}
//printf("fsReadCamEvt=%d\n", size);
	return size;
}

int fsShiftCamEvt(void)
{
	unsigned char	buf[512];
	unsigned long	lval;
	long	seekp;
	int		c, rval, size, page;

	page = 512;
	if(camevtFileSize > camevtFileHead) {
		seekp = camevtFileHead;
		fseek(fp7, seekp, SEEK_SET);
		rval = fread(buf, 1, page, fp7);
		if(rval < page) {
printf("fsShiftCamEvt: format error...eof\n");
			rval = ftruncate(fileno(fp7), 0);
			camevtFileHead = camevtFileSize = 0;
			size = 0;	
		} else if(buf[0] == 0x55 && buf[1] == 0xaa && buf[2] == 0x55 && buf[3] == 0xaa) {
			BYTEtoLONG(buf+4, &lval); size = lval;
			size = (size + page - 1) / page * page;
			buf[0] = buf[1] = buf[2] = buf[3] = 0xff;
			fseek(fp7, seekp, SEEK_SET);
			rval = fwrite(buf, 1, 4, fp7);
			camevtFileHead += size;
			if(camevtFileHead >= camevtFileSize) {
				rval = ftruncate(fileno(fp7), 0);
				camevtFileHead = camevtFileSize = 0;
			}
		} else {
printf("fsShiftCamEvt: format error...%d\n", camevtFileHead);
			size = 0;	
		}
	} else {
		BYTEtoLONG(camevtBuf+4, &lval); size = lval;
		size = (size + page - 1) / page * page;
		if(camevtBufSize > size) {
			camevtBufSize -= size;
			memcpy(camevtBuf, camevtBuf+size, camevtBufSize);
		} else {
			camevtBufSize = 0;
		}
	}
	return size;
}

int fsFlushCamEvt(void)
{
	int		rval, size;

	size = camevtBufSize;
	if(size > 0) {
		fseek(fp7, 0L, SEEK_END);
		rval = fwrite(camevtBuf, 1, size, fp7);
printf("fsFlushCamEvt: fwrite=%d\n", rval);
		if(rval != size) return -1;
		camevtFileSize += size;
		camevtBufSize = 0;
	}
	return 0;
}

int fsCamEvtSize(void)
{
	return camevtFileSize - camevtFileHead + camevtBufSize;
}

int fsRemoveAllCamEvt(void)
{
	int		rval;

	if(camevtFileSize > 0) rval = ftruncate(fileno(fp7), 0);
	else	rval = 0;
	camevtFileHead = camevtFileSize = camevtBufSize = 0;
	return rval;
}

