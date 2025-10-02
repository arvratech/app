#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "NSEnum.h"
#include "defs.h"
#include "syscfg.h"
#include "sysprim.h"
#include "slvm.h"

int  MainInit(void);
void MainExit(void);
void MainTask(void);
void UpdateWallpaper(void);
void MainToShutdown(int reason);


void sigExitHandler(int signo)
{
printf("signal...%d\n", signo);
	if(signo == SIGHUP) {
		UpdateWallpaper();
	} else {
		restoreExitSignal();
		slvmResetMaster(1);
		MainToShutdown(G_USER_SHUTDOWN);
	}
}

int main(int argc, char *argv[])
{
	char	fileName[16];
	int		n, bg;

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
	remove("core");
	sprintf(fileName, "%s.out", devFirmwareName(NULL));
	remove(fileName);
	sync();
	if(bg) {
		n = initDaemon();
		if(n < 0) return 1;
	}
	syscfgInitialize();
	n = MainInit();
	if(n == 0) {
		MainTask();
		MainExit();
	}
	n = devShutdownReason(NULL);
printf("### shutdownReason=%d ###\n", n);
    if(n == G_WARM_RESTART || n == G_COLD_RESTART) {
		ExecForeground("/sbin/reboot", "reboot", "-f", NULL);
	} else if(n == G_POWER_FAIL || n == G_POWER_OFF) {
		ExecForeground("/sbin/poweroff", "poweroff", "-f", NULL);
	}
	return 0;
}

__attribute__((destructor)) static void mainExit(void)
{
printf("destructor: mainExit...\n");
}

int jpegSaveFile(char *fileName, int width, int height, void *yuvBuffer)
{
	return 0;
}

/*
#include <ctype.h>

__const unsigned short int **__ctype_b_loc (void);

__const unsigned short int **__ctype_b (void)
{
	return __ctype_b_loc();
}
*/

/*ctype_b.c
 *
 * This file has been added to compensate for a bug in
 * version 2.3.2 of the glibc library for RH8.
 */

#define attribute_hidden
#define CTYPE_EXTERN_INLINE /* Define real functions for accessors.  */
#include <ctype.h>
/*
#include <locale/localeinfo.h>

__libc_tsd_define (, CTYPE_B)
__libc_tsd_define (, CTYPE_TOLOWER)
__libc_tsd_define (, CTYPE_TOUPPER)


#include <shlib-compat.h>
*/

#define b(t,x,o) (((const t *) __ctype_##x) + o)
//extern const char _nl_C_LC_CTYPE_class[] attribute_hidden;
//extern const char _nl_C_LC_CTYPE_toupper[] attribute_hidden;
//extern const char _nl_C_LC_CTYPE_tolower[] attribute_hidden;
const unsigned short int *__ctype_b = b (unsigned short int, b_loc, 128);
const __int32_t *__ctype_tolower = b (__int32_t, tolower_loc, 128);
const __int32_t *__ctype_toupper = b (__int32_t, toupper_loc, 128);

