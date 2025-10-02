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

void _userDecodePhoto(FP_USER *user, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	p += 5;		// skip flag & UserID
	p += 2;
	user->QuantityLevel = *p++;
	BYTEtoSHORT(p, &user->Width); p += 2;
	BYTEtoSHORT(p, &user->Height); p += 2;
	memcpy(user->JPEGBuffer, p, USER_PHOTO_RECORD_SZ-12);
	user->JPEGSize = USER_PHOTO_RECORD_SZ-12;
}

int userfsGetPhoto(FP_USER *user)
{
	unsigned char	dID[8], buf[USER_PHOTO_RECORD_SZ];
	char	uid[12];
	int		rval, found;

	sprintf(uid, "%d", user->ID);
//	userEncodeID(user->ID, dID);
	string2bcd(uid, 8, dID);
printf("ID=%d [%02x-%02x-%02x]\n", user->ID, (int)dID[0], (int)dID[1], (int)dID[2]);
	fseek(fp3, 0L, SEEK_SET);
	found = 0;
	while((rval = fread(buf, 1, USER_PHOTO_RECORD_SZ, fp3)) == USER_PHOTO_RECORD_SZ) {
printf("rval=%d\n", rval);
		if(buf[0]) {
printf("[%02x-%02x-%02x]\n", (int)buf[1], (int)buf[2], (int)buf[3]);
			_userDecodePhoto(user, buf);
			if(!memcmp(buf+1, dID, 4)) {
				found = 1; 	
				break;
			}
		}
	}
if(found) printf("userfsGetPhoto: QL=%d Width=%d Height=%d %02x-%02x-%02x-%02x\n", (int)user->QuantityLevel, (int)user->Width, (int)user->Height, (int)user->JPEGBuffer[0], (int)user->JPEGBuffer[1], (int)user->JPEGBuffer[2], (int)user->JPEGBuffer[3]);
	if(rval < 0) return -1;
	else	return found;
}

