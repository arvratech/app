#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "uvlib.h"


void uvTimerInit(uv_timer_t *timer)
{
	uv_timer_init(uv_default_loop(), timer);	
}

