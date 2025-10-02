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


void runWpaSupplicant(void)
{
	char	temp[512];

	ExecPipeFinal(temp, "/sbin/insmod", "insmod", "wifi/8188eu.ko", NULL); 
printf("insmod [%s]\n", temp);
	ExecPipeFinal(temp, "/mnt/nand1-1/wifi/wpa_supplicant", "wpa_supplicant", "-iwlan0", "-Dwext", "-C/var/run/wpa_supplicant", "-B", NULL); 
printf("wpa_supplicant [%s]\n", temp);
}

void stopWpaSupplicant(void)
{
	char	*p, temp[256], temp2[64];
	int		pid;

	ExecPipe("/bin/ps", "ps", NULL); 
	ExecPipeFinal(temp, "/bin/grep", "grep", "wpa_supplicant", NULL); 
	p = read_token(temp, temp2);
	pid = n_atoi(temp2);
	if(pid > 1) {
		ExecPipeFinal(temp, "/bin/kill", "kill", temp2, NULL); 
	}
}

void runScriptFile(char *scriptFile)
{
	char	temp[256];

	ExecPipeFinal(temp, "/bin/sh", "sh", scriptFile, NULL); 
printf("%s", temp);
}

#ifdef _A20
char	mpDir[20]	= "video";
#else
char	mpDir[20]	= "../nand1-2/video";
#endif

int mpFileCount(void)
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
		count++;
	}
	closedir(dir);
	return count;
}

void mpArrangeFile(void)
{
	DIR 	*dir;
	struct dirent *dent;
	char	srcPath[128], dstPath[128];
	int		len, found;

	while(1) {
		dir = opendir(mpDir);
		if(!dir) {
			if(errno == ENOENT) printf("not found video directory\n");
			else	printf("video directory open error\n");
			break;
		}
		found = 0;
		while((dent = readdir(dir))) {
			if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;
			len = strlen(dent->d_name);
			if(dent->d_name[len-1] == '_') {
				mpFullPath(srcPath, dent->d_name);
				strcpy(dstPath, srcPath);
				len = strlen(dstPath);
				dstPath[len-1] = 0;
printf("mpArrageFile #########\n");
printf("srcPath: %s\n", srcPath);
printf("dstPath: %s\n", dstPath);
				rename(srcPath, dstPath);
				sync();
				found = 1;
				break;
			}
		}
		closedir(dir);
		if(!found) break;
	}	
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

void mpFullPath(char *path, char *fileName)
{
	sprintf(path, "%s/%s", mpDir, fileName);
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

void mpDeleteAll(void)
{
	DIR 	*dir;
	struct dirent *dent;
	char	path[64];

	sprintf(path, "%s/*", mpDir);
//	ExecPipeFinal(temp, "/bin/rm", "rm", path, NULL); 

	dir = opendir(mpDir);
	if(!dir) {
		if(errno == ENOENT) printf("not found video directory\n");
		else	printf("video directory open error\n");
		return -1;
	}
	while((dent = readdir(dir))) {
		if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;
		sprintf(path, "%s/%s", mpDir, dent->d_name);
		remove(path);
	}
	closedir(dir);
}

