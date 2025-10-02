/*
 * wpa_supplicant/hostapd control interface library
 * Copyright (c) 2004-2007, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#include "includes.h"
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include "wpa_ctrl.h"
#include "common.h"

#ifndef CONFIG_CTRL_IFACE_CLIENT_DIR
#define CONFIG_CTRL_IFACE_CLIENT_DIR	"/tmp"
#endif
#ifndef CONFIG_CTRL_IFACE_CLIENT_PREFIX
#define CONFIG_CTRL_IFACE_CLIENT_PREFIX	"wpa_ctrl_"
#endif

static int	counter = 0;


int wpa_ctrl_open(char *ctrl_path, char *cli_path, char *local_sun_path)
{
	struct sockaddr_un local, dest;
	size_t res;
	int		s, rval, flags, trycnt;

	if(!ctrl_path  || !ctrl_path[0]) return -1;
	s = socket(PF_UNIX, SOCK_DGRAM, 0);
	if(s < 0) {
printf("socket error: %s\n", strerror(errno));
		return -1;
	}
	counter++;
	local.sun_family = AF_UNIX;
	trycnt = 0;
	while(trycnt < 2) {
		if(cli_path && cli_path[0] == '/') {
			rval = os_snprintf(local.sun_path, sizeof(local.sun_path), "%s/" CONFIG_CTRL_IFACE_CLIENT_PREFIX "%d-%d",
					  cli_path, (int) getpid(), counter);
		} else {
			rval = os_snprintf(local.sun_path, sizeof(local.sun_path), CONFIG_CTRL_IFACE_CLIENT_DIR "/"
					  CONFIG_CTRL_IFACE_CLIENT_PREFIX "%d-%d", (int) getpid(), counter);
		}
		if(os_snprintf_error(sizeof(local.sun_path), rval)) {
			rval = -1;
			break;
		}
		rval = bind(s, (struct sockaddr *)&local, sizeof(local));
		if(rval < 0 && errno == EADDRINUSE) {
			// getpid() returns unique identifier for this instance of wpa_ctrl, so the existing
			// socket file must have been left by unclean termination of an earlier run.
			//Remove the file and try again.
			unlink(local.sun_path);
		} else	break;
		trycnt++;
	}
	if(rval < 0) {
printf("bind error: %s\n", strerror(errno));
		close(s);
		return -1;
	}
	dest.sun_family = AF_UNIX;
	if(!os_strncmp(ctrl_path, "@abstract:", 10)) {
		dest.sun_path[0] = 0;
		os_strlcpy(dest.sun_path + 1, ctrl_path + 10, sizeof(dest.sun_path) - 1);
	} else {
		rval = os_strlcpy(dest.sun_path, ctrl_path, sizeof(dest.sun_path));
		if(rval >= sizeof(dest.sun_path)) {
printf("dest path too long\n");
			close(s);
			return -1;
		}
	}
	rval = connect(s, (struct sockaddr *)&dest, sizeof(dest));
	if(rval < 0) {
printf("connect error: %s\n", strerror(errno));
		close(s);
		unlink(local.sun_path);
		return -1;
	}
	strcpy(local_sun_path, local.sun_path);
printf("[%s]-[%s] opened\n", local_sun_path, dest.sun_path);
	return s;
}

void wpa_ctrl_close(int s, char *local_sun_path)
{
	if(s >= 0) {
		unlink(local_sun_path);
		close(s);
	}
}

int wpa_ctrl_send(int s, char *cmd, int cmd_len)
{
	int		rval;

	rval = send(s, cmd, cmd_len, 0);
	if(rval < 0) perror("wpa_ctrl_send: send");
	return rval;
}

int wpa_ctrl_recv(int s, char *reply, int reply_len)
{
	int		rval;

	rval = recv(s, reply, reply_len, 0);
	if(rval < 0) perror("wpa_ctrl_recv: recv");
	return rval;
}

