#include <stdio.h>
#include "rtc.h"
#include "log.h"
#include "as3911.h"
#include "sc_iso14443a.h"
#include "cardcrypto.h"
#include "sctask_com.h"
#include "sccfg.h"
#include "aes.h"
#include "sctask.h"

// Task State definitions
#define	TS_NULL			0
#define	TS_IDLE			1
#define	TS_ACTIVE		2
#define TS_EXIT			3

#define MAX_TASK_SZ		3

static unsigned char  scTaskState, scTaskIndex, scTaskCount, scSleep, scRfoNormalLevel, scCardPresences[MAX_TASK_SZ];
static void (*scActiveTasks[MAX_TASK_SZ])(void);
static void (*scExitTasks[MAX_TASK_SZ])(void);


int scOpen(void)
{
	unsigned char	val;
	int		rval;
	
printf("%lu scOpen...\n", MS_TIMER);
	as3911OpenCom();
	rval = as3911GetReg(AS3911_REG_IC_IDENTITY, &val);		// 0x0b: AS3991B r3.3
	if(rval < 0) {
printf("scOpen: as3911GetReg() error\n");
		return -1;
	}
printf("scOpen: IC_ID=0x%02x\n", (int)val);
	rval = as3911Open();
	if(rval) {
printf("scOpen: as3911Open() error\n");
		as3911Close();
		as3911CloseCom();
		return -1;
	}
	scRfoNormalLevel = CrRfoNormalLevel();
	as3911SetRfoNormalLevel((int)scRfoNormalLevel);
	scInitData();
	CryptoInit();
	return 0;
}

void scClose(void)
{
	scInitData();
	as3911Close();
	as3911CloseCom();
printf("%lu scClose...\n", MS_TIMER);
}

static void _ToIdleTask(void)
{
	scTaskState = TS_IDLE;
}

static void _NextExitTask(void)
{
	while(1) {
		scTaskIndex++;
		if(scTaskIndex >= scTaskCount || scCardPresences[scTaskIndex]) break;
	}
	if(scTaskIndex < scTaskCount) {
		scTaskState = TS_EXIT; scState = SC_OFF;
	} else {
		_ToIdleTask();
	}
}

static void _NextActiveTask(void)
{
	if(scTaskState == TS_IDLE) scTaskIndex = 0;
	else {
		scCardPresences[scTaskIndex] = scCardPresence();
		scTaskIndex++;
	}
	if(scTaskIndex < scTaskCount) {
		scTaskState = TS_ACTIVE; scState = SC_OFF;
	} else {
		scDoneData();
		scTaskIndex = 0xff;
		_NextExitTask();
	}
}

void scTaskInit(void)
{
	scTaskState = TS_NULL;
	scSleep = 0;
}

void scTaskOpen(void)
{
	scTaskState = TS_NULL;
	scSleep = 0;
}

void scTaskClose(void)
{
	if(scTaskState) {
		scClose();
		scTaskState = TS_NULL;
		CrSetScReader(0);		
	}
}
void scTaskEnterSleepMode(void)
{
printf("scTask EnterSleepMode...\n");
	scSleep = 1;
}

void scTaskWakeupSleepMode(void)
{
printf("scTask WakeupSleepMode...\n");
	scSleep = 0;
}

void _scTaskSetup(void);

void scTask(void)
{
	int		val;

	switch(scTaskState) {
	case TS_ACTIVE:	
		(*scActiveTasks[scTaskIndex])();
		if(!scState) _NextActiveTask();
 		else if(scState == SC_FATAL) {
			scClose();
			scTaskState = TS_NULL;
			CrSetScReader(0);
		}	
		break;
	case TS_EXIT:
		(*scExitTasks[scTaskIndex])();
		if(!scState) _NextExitTask();
 		else if(scState == SC_FATAL) {
			scClose();
			scTaskState = TS_NULL;
			CrSetScReader(0);
		}
		break;
	case TS_IDLE:
		if(scSleep) {
			if(scOpenMode) {
				iso14443AClose(); scOpenMode = 0;
			}
		} else {
			if(scOpenMode) {
				iso14443AClose(); scOpenMode = 0;
			}
			ms_sleep(90);
			val = CrScReaderEnable();
			if(val) {
				_scTaskSetup();
				_NextActiveTask();
			} else {
				scClose();
				scTaskState = TS_NULL;
				CrSetScReader(0);
			}
		}
		break;
	default:
		val = CrScReaderEnable();
		if(val) {
			val = scOpen();
printf("### scOpen=%d ###\n", val);
			if(val) {
				ms_sleep(5000);
			} else {
				scOpenMode = 0;
				_ToIdleTask();
				CrSetScReader(1);
			}
		} else {
printf("### scReaderEnable=0\n");
			ms_sleep(500);
		}
	}	
}

void _scTaskSetup(void)
{
	scReaderFormat = CrScReaderFormat();
	if(scReaderFormat == 2) {
		scCardType = CrIso7816FormatCardType();
		scCardApp  = CrIso7816FormatCardApp();
//printf("cardApp=%d\n", (int)scCardApp);
		if(scCardType == 2) {
			scActiveTasks[0] = iso14443AActiveTask;
			scActiveTasks[1] = iso14443BActiveTask;
			scExitTasks[0] = iso14443AExitTask;
			scExitTasks[1] = iso14443BExitTask;
			scTaskCount = 2;
		} else if(scCardType == 1) {
			scActiveTasks[0] = iso14443BActiveTask;
			scExitTasks[0] = iso14443BExitTask;
			scTaskCount = 1;
		} else {
			scCardType = 0;
			scActiveTasks[0] = iso14443AActiveTask;
			scExitTasks[0] = iso14443AExitTask;
			scTaskCount = 1;
		}
	} else if(scReaderFormat == 1) {
		scCardType = CrBlockFormatCardType();
		scCardApp  = CrBlockFormatCardApp();
		if(scCardType == 1) {
			scActiveTasks[0] = iso15693ActiveTask;
			scExitTasks[0] = iso15693ExitTask;
			scTaskCount = 1;
		} else {
			scCardType = 0;
			scActiveTasks[0] = iso14443AActiveTask;
			scExitTasks[0] = iso14443AExitTask;
			scTaskCount = 1;
		}
	} else {
		scReaderFormat = 0;
		scCardType = CrUidFormatCardType();
		if(scCardType == 1) {
			scActiveTasks[0] = iso15693ActiveTask;
			scExitTasks[0] = iso15693ExitTask;
			scTaskCount = 1;
		} else {
			scCardType = 0;
			scActiveTasks[0] = iso14443AActiveTask;
			scExitTasks[0] = iso14443AExitTask;
			scTaskCount = 1;
		}
	}
	if(scRfoNormalLevel != CrRfoNormalLevel()) {
		scRfoNormalLevel = CrRfoNormalLevel();	
		as3911SetRfoNormalLevel((int)scRfoNormalLevel);
	}
}

