#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <signal.h>
#include "lang.h"
#include "lcdc.h"
#include "rtc.h"
#include "prim.h"
#include "spimst.h"
#include "user.h"
#include "syscfg.h"
#include "timer.h"
#include "audio.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "psmem.h"
#include "actprim.h"
#include "wpanet.h"
#include "wpactrl.h"
#include "appact.h"
#include "exec.h"


char	mpDir[20]	= "/mnt/nand1-2/video";
char	mpList[8]	= "mplist";
char	mpPath[24]	= "/mnt/nand1-1/mplayer";
char	mpName[8]	= "mplayer";

int		mpFd = -1;
int		mpState;

void mpOpen(void)
{
	mpFd = ExecOnly(mpPath, mpName, "-slave", "-idle", "-quiet", NULL);
printf("runPlayer: fd=%d\n", mpFd);
//	write(mpFd, "loop 0\n", 7);
	write(mpFd, "set_property loop 0\n", 20);
	mpState = MP_IDLE;
}

void mpClose(void)
{
	if(mpState > MP_NULL) {
		write(mpFd, "quit\n", 5);
		close(mpFd);
		usleep(3000);
		mpFd = -1;
		mpState = MP_NULL;
printf("Quit...\n");
	}
}

int mpOpenRun(void)
{
	DIR 	*dir;
	struct dirent *dent;
	char	*array[16], path[8][64];
	int		i, j, count;

	dir = opendir(mpDir);
	if(!dir) {
		if(errno == ENOENT) printf("not found MP4 directory\n");
		else	printf("MP4 directory open error\n");
		return -1;
	}
	count = 0;
	while((dent = readdir(dir))) {
		if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;
		sprintf(path[count], "%s/%s", mpDir, dent->d_name);
		count++;
		if(count > 7) break;
	}
	closedir(dir);
	if(count == 1) {
		mpFd = ExecOnly(mpPath, mpName, "-slave", "-quiet", path[0], "-loop", "0", NULL);
	} else if(count > 1) {
		i = 0;
		array[i++] = mpName; 
		array[i++] = "-slave";
		array[i++] = "-quiet";
		array[i++] = "{";
		for(j = 0;j < count;j++) array[i++] = path[j];
		array[i++] = "}";
		array[i++] = "-loop";
		array[i++] = "0";
		array[i] = NULL;
		mpFd = ExecOnlyWithArray(mpPath, array);
	} else {
		mpFd = -1;
	}
	if(mpFd >= 0) mpState = MP_RUN;
	return mpFd;
}	

int mpRun(void)
{
	DIR 	*dir;
	FILE	*fp;
	struct dirent *dent;
	char	temp[128];
	int		count;

	dir = opendir(mpDir);
	if(!dir) {
		if(errno == ENOENT) printf("not found video directory\n");
		else	printf("video directory open error\n");
		return -1;
	}
	fp = fopen(mpList, "w");
	if(!fp) {
		printf("%s open error\n", mpList);
		closedir(dir);
		return -1;
	}
	count = 0;
	while((dent = readdir(dir))) {
		if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;
		sprintf(temp, "%s/%s\n", mpDir, dent->d_name);
		fwrite(temp, 1, strlen(temp), fp);
		count++;
	}
	fclose(fp);
	closedir(dir);
	sprintf(temp, "loadlist %s\n", mpList);
	write(mpFd, temp, strlen(temp));
	mpState = MP_RUN;
printf("Run...\n");
	return count;
}

void mpStop(void)
{
	write(mpFd, "stop\n", 5);
	mpState = MP_IDLE;
}

void mpPause(void)
{
	write(mpFd, "pause\n", 6);
	mpState = MP_PAUSE;
printf("Pause...\n");
}

void mpResume(void)
{
	write(mpFd, "pause\n", 6);
	mpState = MP_RUN;
printf("Resume...\n");
}

int mpGetState(void)
{
	return (int)mpState;
}

int mpCount(void)
{
	DIR 	*dir;
	struct dirent *dent;
	char	temp[128];
	int		count;

	dir = opendir(mpDir);
	if(!dir && errno == ENOENT) {
		dirCreate(mpDir);
		dir = opendir(mpDir);
	}
	if(!dir) {
		printf("video directory open error\n");
		return -1;
	}
	count = 0;
	while((dent = readdir(dir))) {
		if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;
printf("%d [%s]\n", count, dent->d_name);
		count++;
	}
	closedir(dir);
	return count;
}

int mpGetFile(int index, char *fileName)
{
	DIR 	*dir;
	FILE	*fp;
	struct dirent *dent;
	int		count, found;

	dir = opendir(mpDir);
	if(!dir) {
		if(errno == ENOENT) printf("not found video directory\n");
		else	printf("video directory open error\n");
		return -1;
	}
	count = found = 0;
	while((dent = readdir(dir))) {
		if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;
		if(count == index) { 
			sprintf(fileName, "%s", dent->d_name);
			found = 1;
			break;
		}
		count++;
	}
	closedir(dir);
	return found;
}

int mpGetFileSize(char *fileName, unsigned long *fileSize)
{
    struct stat sb;
	char	temp[128];
	int		rval;


	sprintf(temp, "%s/%s", mpDir, fileName);
	rval = stat(temp, &sb);
	if(rval < 0) {	
		perror("stat error");
		return -1;
    }
	*fileSize = (long)sb.st_size;
//printf("Last file access:         %s", ctime(&sb.st_atime));
//printf("Last file modification:   %s", ctime(&sb.st_mtime));
	return 0;
}

int mpDeleteFile(char *fileName)
{
	char	temp[128];
	int		rval;

	sprintf(temp, "%s/%s", mpDir, fileName);
	rval = remove(temp);
	if(rval < 0) perror("remove error");
	return rval;
}

