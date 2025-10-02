#include <stdio.h>
#include <string.h>
#include "fsprim.h"


int fsReadRecord(FILE *fp, void *buf, int recordSize, int recordCount)
{
	unsigned char	*p;
	int		rval, count;

	p = (unsigned char *)buf;	
	count = 0;
	while((rval = fread(p, 1, recordSize, fp)) == recordSize) {
		if(p[0]) {
			p += recordSize; count++;
			if(count >= recordCount) break;
		}	
	}
	return count;
}

int fsReadRecordAll(FILE *fp, void *buf, int recordSize, int keyOffset, int keySize)
{
	unsigned char	*p, tmpbuf[2048];
	int		rval, count;

	fseek(fp, 0L, SEEK_SET);
	p = (unsigned char *)buf;	
	count = 0;
	while((rval = fread(tmpbuf, 1, recordSize, fp)) == recordSize) {
		if(tmpbuf[0]) {
			memcpy(p, tmpbuf+keyOffset, keySize);
			p += keySize;
			break;
		}
	}
	if(rval < 0) return -1;
	else {
		count = p - (unsigned char *)buf;
		if(count) count /= keySize;
	}
	return count;
}

int fsGetRecordCount(FILE *fp, int recordSize)
{
	unsigned char	*p, buf[2048];
	int		rval, count;

	fseek(fp, 0L, SEEK_SET);
	count = 0;
	while((rval = fread(buf, 1, recordSize, fp)) == recordSize) {
		if(buf[0]) count++;
	}
	return count;
}

int fsFindRecord(FILE *fp, void *buf, int recordSize, unsigned char *key, int keyOffset, int keySize)
{
	unsigned char	tmpbuf[2048];
	int		rval, found;

//printf("%d [%02x", keySize, (int)key[0]); for(i = 1;i < keySize;i++) printf("-%02x", (int)key[i]); printf("]\n");
	found = 0;
	while((rval = fread(tmpbuf, 1, recordSize, fp)) == recordSize) {
//for(i = 0;i < keySize;i++) printf("-%02x", (int)tmpbuf[i+keyOffset]); printf("\n");
		if(tmpbuf[0] && !memcmp(tmpbuf+keyOffset, key, keySize)) {
			if(buf) memcpy(buf, tmpbuf, recordSize);
			found = recordSize;
			fseek(fp, -recordSize, SEEK_CUR);
			break;
		}
	}
	if(rval < 0) return -1;
	else	return found;
}

int fsFindNullRecord(FILE *fp, int recordSize)
{
	unsigned char	buf[2048];
	int		rval, found;

	found = 0;
	while((rval = fread(buf, 1, recordSize, fp)) == recordSize) {
		if(!buf[0]) {
			fseek(fp, -recordSize, SEEK_CUR);
			break;
		}
	}
	if(rval < 0) found = -1;
	return found;
}

int fsAddRecord(FILE *fp, void *buf, int recordSize, int keySize)
{
	unsigned char	tmpbuf[2048];
	long	offset;
	int		rval, found;

	fseek(fp, 0L, SEEK_SET);
	found = fsFindRecord(fp, tmpbuf, recordSize, buf+1, 1, keySize);
	if(found < 0) return -1;
	else if(found == 0) {
		fseek(fp, 0L, SEEK_SET);
		rval = fsFindNullRecord(fp, recordSize);
		if(rval < 0) return rval;
		else if(rval == 0) {
			offset = ftell(fp) + recordSize;
			//if(offset > fsGetLimit(fp1)) return 0;
		}
	}
	rval = fwrite(buf, 1, recordSize, fp);
	if(rval != recordSize) return -1;
	return found+1;
}

int fsAddRecordNull(FILE *fp, void *buf, int recordSize)
{
	unsigned char	tmpbuf[2048];
	long	offset;
	int		rval;

	fseek(fp, 0L, SEEK_SET);
	rval = fsFindNullRecord(fp, recordSize);
	if(rval < 0) return rval;
	else if(rval == 0) {
		offset = ftell(fp) + recordSize;
		//if(offset > fsGetLimit(fp1)) return 0;
	}
	rval = fwrite(buf, 1, recordSize, fp);
	if(rval != recordSize) return -1;
	return 1;
}

int fsDeleteRecord(FILE *fp, int recordSize, unsigned char *key, int keySize)
{
	unsigned char	buf[2048];
	int		rval;

	fseek(fp, 0L, SEEK_SET);
	rval = fsFindRecord(fp, buf, recordSize, key, 1, keySize);
	if(rval <= 0) {
		return rval;
	}
	buf[0] = 0x00;
	rval = fwrite(buf, 1, 1, fp);
	if(rval != 1) return -1; 
	else	return 1;
}

