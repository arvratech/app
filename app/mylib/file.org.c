#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include "gactivity.h" 
#include "gapp.h" 
#include "file.h"


/*
struct statvfs {
    unsigned long  f_bsize;		// filesystem block size
    unsigned long  f_frsize;	// fragment size
    fsblkcnt_t     f_blocks;	// size of fs in f_frsize units
    fsblkcnt_t     f_bfree;		// # free blocks
    fsblkcnt_t     f_bavail;	// # free blocks for unprivileged users
    fsfilcnt_t     f_files;		// # inodes
    fsfilcnt_t     f_ffree;		// # free inodes
    fsfilcnt_t     f_favail;	// # free inodes for unprivileged users
    unsigned long  f_fsid;		// filesystem ID
    unsigned long  f_flag;		// mount flags
    unsigned long  f_namemax;	// maximum filename length
};
*/

// block size
int diskAvailableSpace(char *path, unsigned long *psize)
{
	struct statvfs	vfs;
	unsigned long	size;
	int		rval;

	rval = statvfs(path, &vfs);
	if(rval < 9) {
		LOG(1, "diskAvaiableSpace: statvfs(%s)", path);
	} else {
		//size = stat.f_bsize * stat.f_bavail;
printf("DISK: %d %d\n", vfs.f_bsize, vfs.f_bavail);
		size  = vfs.f_bsize;
		if(size > 1024) size >>= 10;
		*psize = size * vfs.f_bavail;
		rval = 0;
	}
	return rval;
}

int fileSize(char *path, unsigned long *psize)
{
	struct stat		buf;
	int		rval;

	printf("fileSize: %s\n", path);
	rval = stat(path, &buf);
	if(rval < 0) {
	printf("fileSize: error\n");  
		LOG(1, "fileSize: stat(%s)", path);
	} else {
		*psize = buf.st_size;
	printf("fileSize: %lu\n", *psize);  
		rval = 0;
	}
	return rval;
}

#include "rtc.h"
void wdPing(void);

int fileCopy(char *srcPath, char *dstPath, void (*callback)(unsigned long size))
{
	FILE	*fp1, *fp2;
	char	temp[4096];
	unsigned long	timer, count;
	int		size, rval;

	fp1 = fopen(srcPath, "r");
	if(!fp1) {
		LOG(1, "fileCopy: fopen(%s)", srcPath);
		return -1;
	}
	fp2 = fopen(dstPath, "w");
	if(!fp2) {
		LOG(1, "fileCopy: fopen(%s)", dstPath);
		fclose(fp1);
		return -1;
	}
	timer = MS_TIMER;
	count = 0;
	while(1) {
		size = fread(temp, 1, 4096, fp1);
		if(size > 0) {
			rval = fwrite(temp, 1, size, fp2);
			if(rval != size) {
				rval = -2;
				break;
			}
			count += rval;
			if(callback) {
				(*callback)(count);
				usleep(5);
			}

		} else {
			if(feof(fp1)) rval = 0;
			else	rval = -1;
			break;
		}
		if((MS_TIMER-timer) >= 1200) {	// 1500
			wdPing();
			timer = MS_TIMER;
		}
	}
	if(rval < 0) {
		LOG(1, "fileCopy: fread()/fwrite()");
		fclose(fp1); fclose(fp2);
		unlink(dstPath);
	} else {
printf("fileCopy OK: %d\n", count);
		fclose(fp1); fclose(fp2);
	}
	return rval;
}


int fileCopyToMem(char *path, unsigned char *buf)
{
	FILE	*fp;
	unsigned char	*p;
	int		rval, count;

	fp = fopen(path, "r");
	if(!fp) {
		LOG(1, "fileCopyToMem: fopen(%s)", path);
		return -1;
	}
	p = buf;
	count = 0;
	while(1) {
		rval = fread(p, 1, 4096, fp);
		if(rval > 0) {
			p += rval;
		} else {
			if(feof(fp)) rval = 0;
			else	rval = -1;
			break;
		}
	}
	fclose(fp);
	if(rval < 0) {
		LOG(1, "fileCopyToMem: fread()");
	} else {
printf("fileCopyToMem OK: %d\n", p - buf);
	}
	return rval;
}

int dirUsage(char *path)
{
	DIR 	*dir;
	struct dirent	*dent;
	struct stat		_statbuf;
	char	filePath[128];
	unsigned long	size;
	int		rval;

	dir = opendir(path);
	if(!dir) {
		LOG(1, "dirUsage: opendir(%s)", path);
		return -1;
	}
	size = 0;
	while((dent = readdir(dir))) {
		if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;
		sprintf(filePath, "%s/%s", path, dent->d_name);
		lstat(filePath, &_statbuf);
		if(S_ISDIR(_statbuf.st_mode)) {
			size += dirUsage(filePath);
		} else {
			rval = stat(filePath, &_statbuf);
			if(rval < 0) {
				LOG(1, "dirUsage: stat(%s)", filePath);
			} else {
				size += _statbuf.st_size;
			}
		}	
	}
	closedir(dir);
	return size;
}

int dirCreate(char *path)
{
	struct stat		buf;
	int		rval;

	rval = stat(path, &buf);
	if(rval < 0) {
		rval = mkdir(path, 0755);
		if(rval < 0) LOG(1, "dirCreate: mkdir(%s)", path);
	} else {
		rval = 1;
	}
	return rval;
}

int dirDelete(char *path)
{
	DIR 	*dir;
	struct dirent	*dent;
	struct stat		_statbuf;
	char	filePath[128];
	int		rval;

	dir = opendir(path);
	if(!dir) {
		LOG(1, "dirDelete: opendir(%s)", path);
		return -1;
	}
	while((dent = readdir(dir))) {
		if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;
		sprintf(filePath, "%s/%s", path, dent->d_name);
		lstat(filePath, &_statbuf);
		if(S_ISDIR(_statbuf.st_mode)) {
			rval = dirDelete(filePath);
		} else {
			rval = unlink(filePath);
		}
	}
	closedir(dir);
	return rval;
}

int dirCopy(char *srcPath, char *dstPath, void (*callback)(unsigned long size))
{
	DIR 	*dir;
	struct dirent	*dent;
	struct stat		_statbuf;
	char	srcFile[128], dstFile[128];
	unsigned long	count, val;
	int		rval;

printf("dirCopy srcDir: %s\n", srcPath);
printf("dirCopy dstDir: %s\n", dstPath);
	rval = dirCreate(dstPath);
	if(rval < 0) return rval;
	else if(rval > 0) dirDelete(dstPath);
	dir = opendir(srcPath);
	if(!dir) {
		LOG(1, "dirCopy: opendir(%s)", srcPath);
		return -1; 
	}
	rval = 0; count = 0;
	while((dent = readdir(dir))) {
		if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;
		sprintf(srcFile, "%s/%s", srcPath, dent->d_name);
		sprintf(dstFile, "%s/%s", dstPath, dent->d_name);
		lstat(srcFile, &_statbuf);
		if(S_ISDIR(_statbuf.st_mode)) {
			rval = dirCopy(srcFile, dstFile, callback);
			if(rval < 0) break;
		} else {
			rval = fileCopy(srcFile, dstFile, NULL);
			if(rval < 0) break;
			rval = fileSize(srcFile, &val);
			if(!rval) count += val;
			if(callback) (*callback)(count);
		}
	}
	closedir(dir);
	return rval;
}

int dirCopyAppend(char *srcPath, char *dstPath, void (*callback)(unsigned long size))
{
	DIR 	*dir;
	struct dirent	*dent;
	struct stat		_statbuf;
	char	srcFile[128], dstFile[128];
	unsigned long	count, val;
	int		rval;

printf("dirCopy srcDir: %s\n", srcPath);
printf("dirCopy dstDir: %s\n", dstPath);
	rval = dirCreate(dstPath);
	if(rval < 0) return rval;
	dir = opendir(srcPath);
	if(!dir) {
		LOG(1, "dirCopy: opendir(%s)", srcPath);
		return -1; 
	}
	rval = 0; count = 0;
	while((dent = readdir(dir))) {
		if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;
		sprintf(srcFile, "%s/%s", srcPath, dent->d_name);
		sprintf(dstFile, "%s/%s", dstPath, dent->d_name);
		lstat(srcFile, &_statbuf);
		if(S_ISDIR(_statbuf.st_mode)) {
			rval = dirCopy(srcFile, dstFile, callback);
			if(rval < 0) break;
		} else {
			rval = fileCopy(srcFile, dstFile, NULL);
			if(rval < 0) break;
			rval = fileSize(srcFile, &val);
			if(!rval) count += val;
			if(callback) (*callback)(count);
		}
	}
	closedir(dir);
	return rval;
}

static unsigned long _copySize;

void TaskCopying(unsigned long size)
{
	unsigned char	msg[12];
	long long	val;

	val = size;
	val *= 100;
	msg[0] = GM_USER; msg[1] = val / _copySize;
	appPostMessage(msg);
}

void TaskSetCopySize(unsigned long size)
{
	_copySize = size;
} 

#include	<stdarg.h>

void LOG(int errnum, ...)
{
	va_list	args;
	char	*p, *fmt, buf[256];
	int		status, len;

	va_start(args, errnum);
	fmt = va_arg(args, char *);
	vsprintf(buf, fmt, args);
	va_end(args);
	p = buf + strlen(buf);
	if(errnum) {
		sprintf(p, " %d: ", errno); p += strlen(p);
		strcpy(p, strerror(errno));
    }
	printf("%s\n", buf);
}

