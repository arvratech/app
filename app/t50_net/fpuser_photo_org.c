int userfsSeekPhoto(long position)
{
	return fseek(fp3, position, SEEK_SET);
}

long userfsTellPhoto(void)
{
	return ftell(fp3);
}

int userfsEofPhoto(void)
{
	return feof(fp3);
}

int userfsGetPhoto(FP_USER *user)
{
	unsigned char	dID[8], buf[8];
	unsigned short	length;
	int		rval, size, found;

	string2bcd(user->userID, 8, dID);
	fseek(fp3, 0L, SEEK_SET);
	found = 0;
	while((rval = fread(buf, 1, 8, fp3)) == 8) {
		BYTEtoSHORT(buf, &length);
		size = length + 2048 - (length & 2047);
		rval = fread(user->jpegBuffer, 1, size-8, fp3);
		if(rval < size - 8) break;
		if(buf[2] && !memcmp(dID, buf+3, 4) {
			user->jpegSize = length - 8;
			found = 1;
			break;
		}
	}
	if(rval < 0) return -1;
	else	return found;
}

int userfsAddPhoto(FP_USER *user)
{
	unsigned char	dID[8], buf[8];
	unsigned short	length;
	int		rval, size, found;
}

