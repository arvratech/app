#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "sysprim.h"
#include "gactivity.h"
#include "gapp.h"
#include "prefspec.h"
#include "viewprim.h"
#include "actprim.h"
#include "pref.h"
#include "file.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "file.h"
#include "cf.h"
#include "cfprim.h"
#include "appact.h"
#include "exec.h"


void AdmVfOnCreate(GACTIVITY *act);
void AdmVfOnDestroy(GACTIVITY *act);
void AdmVfOnClick(GACTIVITY *act, void *view);
void AdmVfOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmVfOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
int  AdmVfOnNumberOfRowsInTableView(void *self, void *tblv);
void *AdmVfOnCellForRowAtIndex(void *self, void *tblv, int index);
void AdmVfOnDidSelectRowAtIndex(void *self, void *tblv, int index);

static void	*bufAddress;


void AdmVf(int requestCode, void *pss)
{
	GACTIVITY	*act;

printf("AdmVf\n");
	act = CreateActivity(AdmVfOnCreate);
	activitySetPsContext(act, pss);
	activitySetTimeout(act, 30);
	appStartActivity(act, NULL, 0);
}

void AdmVfOnCreate(GACTIVITY *act)
{
	void	*wnd, *tblv, *v, *ps; 

	ps = act->psContext;
	activitySetOnDestroy(act, AdmVfOnDestroy);
	activitySetOnClick(act, AdmVfOnClick);
	activitySetOnAppMessage(act, AdmVfOnAppMessage);
	activitySetOnActivityResult(act, AdmVfOnActivityResult);
	tblv = AdmActInitWithTableView(act, PStitle(ps));
	wnd = activityWindow(act);
	UIsetOnNumberOfRowsInTableView(tblv, AdmVfOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, AdmVfOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, AdmVfOnDidSelectRowAtIndex);
	bufAddress = ViewAllocBuffer();
	mpArrangeFile();
	UIreloadData(tblv);
}

void AdmVfOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

void AdmVfOnClick(GACTIVITY *act, void *view)
{
	int		tag, count;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_PREF_BACK:		DestroyActivity(act); break;
	}
}

void AdmVfOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
}

static int _rowCount;

int AdmVfOnNumberOfRowsInTableView(void *self, void *tblv)
{
	_rowCount = mpFileCount() + 1;
	return _rowCount;
}

void *AdmVfOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell;
	char	title[64];
	int		rval;

	rval = mpGetFile(index, title);
	if(rval <= 0) { 
		cell = AdmInitTableViewCell(tblv, UITableViewCellStyleDefault, UITableViewCellAccessoryTypeNone, xmenu_video[0]);
		CellSetImageAlphaMask(cell, "add.bmp");
	} else {
		cell = AdmInitTableViewCell(tblv, UITableViewCellStyleDefault, UITableViewCellAccessoryTypeNone, title);
	}
	return cell;
}

static int	_onResultCount;

void AdmVfOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell;
	char	*p, fileName[64], temp[256];
	long	size;
	int		rval;
	
	cell = UIcellForRowAtIndex(tblv, index);
	if(cell) {
		if(index >= 0 && index < _rowCount-1) {
			p = temp;
			strcpy(fileName, CellTitle(cell));
			sprintf(p, "title=%s\n", fileName); p += strlen(p);
			rval = mpGetFileSize(fileName, &size);
			sprintf(p, "%s=%ld\n", admFileSizeTitle(), size); p += strlen(p);
			sprintf(p, "button=%s\n", admCancelTitle()); p += strlen(p);
			sprintf(p, "button=%s\n", admDeleteTitle()); p += strlen(p);
			FieldAlertDialog(ACT_PREF+index, temp);
		} else if(index == _rowCount-1) {
			_onResultCount = 0;
			sprintf(temp, "%s=%s\n", CellTitle(cell), GetPromptMsg(M_ARE_YOU_REALLY));  
			AlertDialog(ACT_PREF+index, temp, TRUE);
		}
	}
}

void *VideoCopyTask(void *arg);

void AdmVfOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	void	*wnd, *tblv, *cell;
	pthread_t	_thread;
	unsigned char	msg[12];
	char	temp[128];
	int		rval, index;

	wnd = activityWindow(act);
	tblv = UIviewWithTag(wnd, TAG_PREF_TABLE);
	index = requestCode - ACT_PREF;
printf("AdmVfOnActivityResult...index=%d request=%d result=%d\n", index, requestCode, resultCode);
	if(index >= 0 && index < _rowCount-1 && resultCode == RESULT_FIRST_USER+1) {
printf("delete....%d\n");
		cell = UIcellForRowAtIndex(tblv, index);
		mpDeleteFile(CellTitle(cell));
		UIremoveChildAll(tblv);
		ViewFreeBuffer(bufAddress);
		UIreloadData(tblv);
	} else if(index == _rowCount-1 && resultCode == RESULT_OK) {
printf("resultCount=%d\n", _onResultCount);
		if(_onResultCount == 0) {
			rval = mountUsb();
			if(rval) {
				cell = UIcellForRowAtIndex(tblv, index);
				AlertResultMsg(CellTitle(cell), R_FAILED);
			} else {
				cell = UIcellForRowAtIndex(tblv, index);
				act = (GACTIVITY *)ProgressDialog(ACT_PREF+_rowCount-1, CellTitle(cell), "");
				pthread_create(&_thread, NULL, VideoCopyTask, (void *)act);
			}
		} else {
			umountUsb();
			UIremoveChildAll(tblv);
			ViewFreeBuffer(bufAddress);
			UIreloadData(tblv);
		}
		_onResultCount++;
	}
}


extern char    usbAppDir[];
extern char    usbBootDir[];
extern char    tgtAppDir[];
extern char    tgtBootDir[];

void *VideoCopyTask(void *arg)
{
	GACTIVITY	*act;
	DIR 	*dir;
	struct dirent *dent;
	unsigned char	msg[12];
	char	srcPath[64], dstPath[64], srcFile[128], dstFile[128];
	char	temp[80];
	unsigned long	size;
	int		rval;

	act = (GACTIVITY *)arg;
	usleep(250);
	sprintf(srcPath, "%s/video", usbAppDir);
	sprintf(dstPath, "%s/video", tgtAppDir);
printf("copy: [%s] =>[%s]\n", srcPath, dstPath);
	dir = opendir(srcPath);
	if(!dir) {
		msg[0] = GM_USER+2; msg[1] = 1;
		appPostMessage(msg);
		return (void *)0; 
	}
	rval = 0;
	while((dent = readdir(dir))) {
		if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;
		sprintf(srcFile, "%s/%s", srcPath, dent->d_name);
		sprintf(dstFile, "%s/%s", dstPath, dent->d_name);
//printf("[%s] =>[%s]\n", srcFile, dstFile);
		rval = fileSize(srcFile, &size);
		if(rval < 0) break;
		TaskSetCopySize(size);
		activitySetParam1(act, dstFile+5);
		msg[0] = GM_USER+1; msg[1] = 0;
		appPostMessage(msg);
		usleep(30);
		rval = fileCopy(srcFile, dstFile, TaskCopying);
		if(rval < 0) break;
	}
	closedir(dir);
	sync();
	msg[0] = GM_USER+2;
	if(rval < 0) msg[1] = 2;
	else	msg[1] = 0;
	appPostMessage(msg);
	return (void *)0;
}

