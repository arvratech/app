#ifndef _PREF_H_
#define _PREF_H_

#include "prefspec.h"
#include "gactivity.h"


// Tag definitions
#define TAG_PREF_BACK				200
#define TAG_PREF_TABLE				201
#define TAG_PREF_CHILD_VIEW			202
#define TAG_PREF_TITLE				203
#define TAG_PREF_CELL_CONTROL		300
#define TAG_PREF_OK_BTN				395
#define TAG_PREF_CANCEL_BTN			396
#define TAG_PREF_USER_BTN			397
#define TAG_PREF_CELL_BTN		    400	

// Activity request code definitions
#define ACT_PREF_ADD				97
#define ACT_PREF_EDIT				98
#define ACT_PREF_DELETE				99
#define ACT_PREF					100
#define ACT_PREF_SWITCH				200

// Activity result code definitions
#define RESULT_PREF_DELETED			RESULT_FIRST_USER
#define RESULT_PREF_EDITED			RESULT_FIRST_USER+1


void PrefCreate(int requestCode, void *ps);
void PrefRecreate(int requestCode, void *ps);
void PrefButtonCreate(int requestCode, void *ps);
void PrefReloadData(void *act);
void PrefReload(void *act);
void PrefReloadRowAtIndex(void *act, int index);


#endif
