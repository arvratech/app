#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <signal.h>
#include "defs.h"
#include "NSEnum.h"
#include "lang.h"
#include "lcdc.h"
#include "rtc.h"
#include "prim.h"
#include "user.h"
#include "hw.h"
#include "cf.h"
#include "syscfg.h"
#include "dev.h"
#include "netsvc.h"
#include "mod.h"


void modSubdevConfBuffer(int subdevId, unsigned char *buf, int size)
{
}

void modSubdevUnconfBuffer(int subdevId, unsigned char *buf, int size)
{
}

