#ifndef _ACT_PRIM_H_
#define _ACT_PRIM_H_

#include "cgrect.h"
#include "gactivity.h"


GACTIVITY *activityAlloc(void);
void activityRelease(GACTIVITY *act);
void InitializeActivitys(void);
GACTIVITY *CreateActivity(void (*onCreate)(GACTIVITY *));
void DestroyActivity(GACTIVITY *act);
void DestroyActivityForResult(GACTIVITY *act, int resultCode, char *intent);
void DestroyDialogActivity(GACTIVITY *act);
void DestroyDialogActivityForResult(GACTIVITY *act, int resultCode, char *intent);
void MoveHomeActivity(void);


#endif

