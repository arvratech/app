#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "actprim.h"
#include "gapp.h"


#define MAX_ACTIVITY_SZ		12

static GACTIVITY	_gActs[MAX_ACTIVITY_SZ];


GACTIVITY *activityAlloc(void)
{
	GACTIVITY	*a;
	int		i;

	for(i = 0, a = _gActs;i < MAX_ACTIVITY_SZ;i++, a++)
		if(!a->state) break;
//printf("activityAlloc=%d\n", i);
	if(i < MAX_ACTIVITY_SZ) a->state = 1;
	else	a = (GACTIVITY *)0;
	return a;
}

void activityRelease(GACTIVITY *act)
{
	GACTIVITY	*a;
	int		i;

	for(i = 0, a = _gActs;i < MAX_ACTIVITY_SZ;i++, a++)
		if(a == act) a->state = 0;
}

void InitializeActivitys(void)
{
	GACTIVITY	*a;
	int		i;

	for(i = 0, a = _gActs;i < MAX_ACTIVITY_SZ;i++, a++) {
		a->state = 0;
	}
}

GACTIVITY *CreateActivity(void (*onCreate)(GACTIVITY *))
{
	GACTIVITY	*a;

	a = activityAlloc();
	if(a)  {
		activityInit(a);
		activitySetOnCreate(a, onCreate);
	}
	return a;
}

void DestroyActivity(GACTIVITY *act)
{
	appDestroyActivity(act);
}

void DestroyActivityForResult(GACTIVITY *act, int resultCode, char *intent)
{
	activitySetResult(act, resultCode, intent);
	appDestroyActivity(act);
}

void DestroyDialogActivity(GACTIVITY *act)
{
	if(act->onDismiss) (*act->onDismiss)(act);
	else	appDestroyActivity(act);
}

void DestroyDialogActivityForResult(GACTIVITY *act, int resultCode, char *intent)
{
	activitySetResult(act, resultCode, intent);
	if(act->onDismiss) (*act->onDismiss)(act);
	else	appDestroyActivity(act);
}

void MoveHomeActivity(void)
{
	GACTIVITY	*act;

	act = appCurrentActivity();
	appDestroyActivityAffinity(act);
}

