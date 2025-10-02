#ifndef _PJSUA_LIB_H_
#define _PJSUA_LIB_H_


#include "pjsua-lib/pjsua.h"


int  pjsuaInit(void (*on_call_state)(pjsua_call_id call_id, pjsip_event *e), void (*on_media_state)(pjsua_call_id call_id));
void pjsuaExit(void);
int  pjsuaMakeCall(char *url);
int  pjsuaHangupCall(void);
int  pjsuaCallDuration(void);


#endif

