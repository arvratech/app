#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "syscfg.h"
#include "exec.h"
#include "sysprim.h"
#include "uv.h"

int		gRequest,  gIOStatus;
unsigned char	gFuncState, gFuncKey, gFuncTimer, gSlaveState, fpmTouch;
unsigned char	slvSystemStatus, sleepMode, sleepTimer;
int		mainLoop, authCount;

void MainShutdown(void);

static uv_loop_t	*loop;
static uv_work_t	_work;

void sigExitHandler(int signo)
{
printf("signal...%d\n", signo);
	restoreExitSignal();
	MainShutdown();
}

int main(int argc, char *argv[])
{
	int		bg, n;

	bg = 0;
	while(1) {
		n = getopt(argc, argv, "bB");
		if(n < 0) break;
		switch(n) {
		case 'b':
		case 'B':
			bg = 1;
			break;
		}
	}
	setExitSignal(bg, sigExitHandler);
	if(bg) {
		n = initDaemon();
		if(n < 0) return 1;
	}
	loop = uv_default_loop();
	uv_init_async_threads();
printf("uv_run...\n");
	uv_run(loop, 0);
printf("Exit.......\n");
	return 0;
}

void MainShutdown(void)
{
	uv_threadpool_cleanup();
	uv_stop(loop);
	uv_async_watcher_send(loop);
}

void MainSvcPost(int type)
{
}

uv_loop_t *MainLoop(void)
{
	return loop;
}

uv_work_t *GetMainWork(void)
{
	return &_work;
}
