#ifndef WPA_LIB_H
#define WPA_LIB_H

#include "wpa_ctrl.h"


void wpactrlInit(void);
int  wpactrlOpen(const char *ifname);
void wpactrlClose(void);
int  wpactrlPipePoll(int pipe_fd, int timeout);
int  wpactrlRequest(char *cmd, char *reply, int replyLen);
int  wpactrlMoreResponse(char *reply, int replyLen);
int  wpactrlEventRead(char *buf, int len);


#endif

