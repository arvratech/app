#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/poll.h>
#include "dirent.h"
//#include "wpa_common.h"
#include "rtc.h"
#include "wpa_ctrl.h"
#include "wpactrl.h"


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
			sprintf(local.sun_path, "%s/" CONFIG_CTRL_IFACE_CLIENT_PREFIX "%d-%d",
						  cli_path, (int) getpid(), counter);
		} else {
			sprintf(local.sun_path, CONFIG_CTRL_IFACE_CLIENT_DIR "/"
						  CONFIG_CTRL_IFACE_CLIENT_PREFIX "%d-%d", (int) getpid(), counter);
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
	if(!strcmp(ctrl_path, "@abstract:")) {
		dest.sun_path[0] = 0;
		strcpy(dest.sun_path + 1, ctrl_path + 10);
	} else {
		strcpy(dest.sun_path, ctrl_path);
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

int wpa_ctrl_recv(int s, char *reply, int replyLen)
{
	int		rval;

wpaloop:
	rval = recv(s, reply, replyLen, 0);
	if(rval < 0) {
		perror("wpa_ctrl_recv: recv");
//		if(errno == EINTR) goto wpaloop;
	}
	return rval;
}

static int	ctrl_s, mon_s;
static char ctrl_sun_path[104], mon_sun_path[104];
static char ctrl_iface[80];
static char ctrl_iface_dir[80];

void wpactrlInit(void)
{
	ctrl_s = mon_s = -1;
	ctrl_sun_path[0] = mon_sun_path[0] = 0;
	ctrl_iface[0] = 0;
	strcpy(ctrl_iface_dir, "/var/run/wpa_supplicant");
}

int wpactrlOpen(const char *ifname)
{
	struct dirent *dent;
	DIR 	*dir;
	struct timeval tv;
	fd_set	rfds;
	char	cfile[128], reply[24], *pos, *pos2;
	int		rval, flen;
	size_t	len;

	if(ifname) {
		if(strcmp(ifname, ctrl_iface)) strcpy(ctrl_iface, ifname);
	} else {
		// if wpa_supplicant is executing, there exist "/var/run/wpa_supplicant/wlan0"
		dir = opendir(ctrl_iface_dir);
		ctrl_iface[0] = 0;
		if(dir) {
			while((dent = readdir(dir))) {
#ifdef _DIRENT_HAVE_D_TYPE
				// Skip the file if it is not a socket.  Also accept DT_UNKNOWN (0) in case
				// the C library or underlying file system does not support d_type.
				if(dent->d_type != DT_SOCK && dent->d_type != DT_UNKNOWN) continue;
#endif
				if(!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) continue;
				printf("Selected interface: %s\n", dent->d_name);
				strcpy(ctrl_iface, dent->d_name);
				break;
			}
			closedir(dir);
		}
	}
	if(!ctrl_iface[0]) {
		printf("can find interface in [%s]\n", ctrl_iface_dir);
		return -1;
	}
	flen = strlen(ctrl_iface_dir) + strlen(ctrl_iface) + 2;
	// cfile = "/var/run/wpa_supplicant/wlan0"
	snprintf(cfile, flen, "%s/%s", ctrl_iface_dir, ctrl_iface);
printf("connecting: [%s]\n", cfile);
	ctrl_s = wpa_ctrl_open(cfile, NULL, ctrl_sun_path);
	if(ctrl_s < 0) {
		return -1;
	}
	mon_s = wpa_ctrl_open(cfile, NULL, mon_sun_path);
	if(mon_s < 0) {
		wpa_ctrl_close(ctrl_s, ctrl_sun_path);
		return -1;
	}
	rval = wpa_ctrl_send(mon_s, "ATTACH", 6);
	if(rval < 0) {
		wpa_ctrl_close(ctrl_s, ctrl_sun_path);
		wpa_ctrl_close(mon_s, mon_sun_path);
		return -1;
	}
	tv.tv_sec = 2;
	tv.tv_usec = 0;
	FD_ZERO(&rfds);
	FD_SET(mon_s, &rfds);
	rval = select(mon_s + 1, &rfds, NULL, NULL, &tv);
	if(FD_ISSET(mon_s, &rfds)) {
		rval = wpa_ctrl_recv(mon_s, reply, 6);
		if(rval == 3 && !memcmp(reply, "OK\n", 3)) rval = 0;
		else	rval = -1;
	} else	rval = -1;
	if(rval < 0) {
		wpa_ctrl_close(ctrl_s, ctrl_sun_path);
		wpa_ctrl_close(mon_s, mon_sun_path);
	}
	return rval;
}

/*
	len = sizeof(buf) - 1;
	if(wpa_ctrl_request(ctrl_conn, "INTERFACES", 10, buf, &len, NULL) >= 0) {
		buf[len] = 0; pos = buf;
printf("INTERFACES\n%s", buf);
		while(*pos) {
			pos2 = strchr(pos, '\n');
			if(pos2) *pos2 = 0;
//			if(strcmp(pos, ctrl_iface)) adapterSelect->addItem(pos);
			if(pos2) pos = pos2 + 1;
			else	 break;
		}
	}
	len = sizeof(buf) - 1;
	if (wpa_ctrl_request(ctrl_conn, "GET_CAPABILITY eap", 18, buf, &len, NULL) >= 0) {
		buf[len] = 0;
printf("CAPABILITY eap\n%s", buf);
		//QStringList types = res.split(QChar(' '));
		//bool wps = types.contains("WSC");
	}
	return 0;
}
*/

void wpactrlClose(void)
{
	struct timeval tv;
	fd_set	rfds;
	char	reply[24];
	int		rval;

	if(mon_s >= 0) {
		rval = wpa_ctrl_send(mon_s, "DETACH", 6);
		if(!rval) {
			tv.tv_sec = 2;
			tv.tv_usec = 0;
			FD_ZERO(&rfds);
			FD_SET(mon_s, &rfds);
			rval = select(mon_s + 1, &rfds, NULL, NULL, &tv);
			if(FD_ISSET(mon_s, &rfds)) {
				rval = wpa_ctrl_recv(mon_s, reply, 6);
			}
		}
		wpa_ctrl_close(mon_s, mon_sun_path);
		mon_s = -1;
	}
	if(ctrl_s >= 0) {
		wpa_ctrl_close(ctrl_s, ctrl_sun_path);
		ctrl_s = -1;
	}
	ctrl_sun_path[0] = mon_sun_path[0] = 0;
	ctrl_iface[0] = 0;
	ctrl_iface_dir[0] = 0;
printf("wpaClose...\n");
}

int wpactrlPipePoll(int pipe_fd, int timeout)
{
	struct pollfd	fds[3];
	unsigned char	buf[8];
	int		rval;

	fds[0].fd = mon_s;
	fds[0].events = POLLIN;
	fds[1].fd = pipe_fd;
	fds[1].events = POLLIN;
	rval = poll(fds, 2, timeout);
	if(rval > 0) {
		rval = 0;
		if(fds[0].revents & POLLIN) rval |= 0x01;
		if(fds[1].revents & (POLLIN | POLLOUT)) rval |= 0x02;
	} else if(rval < 0) {
		if(errno == EINTR) {
printf("wpaPipePoll EINTR...\n");
			rval = 0;
		} else {
			printf("wpaPipePoll() poll error: %s", strerror(errno));
		}
	}
	return rval;
}

int wpactrlRequest(char *cmd, char *reply, int replyLen)
{
	struct timeval tv;
	fd_set	rfds;
	int		rval, recvlen;

	tv.tv_sec = 0; tv.tv_usec = 0;
	FD_ZERO(&rfds);
	FD_SET(ctrl_s, &rfds);
	rval = select(ctrl_s + 1, &rfds, NULL, NULL, &tv);
	if(FD_ISSET(ctrl_s, &rfds)) {
		rval = wpa_ctrl_recv(ctrl_s, reply, replyLen);
if(rval > 0) {
	printf("BEFORE recv=%d\n", rval);
	reply[rval] = 0;
printf("Prev Reply: %d\n%s", recvlen, reply);
}
	}
printf("%lu Tx: [%s]\n", MS_TIMER, cmd);
	rval = wpa_ctrl_send(ctrl_s, cmd, strlen(cmd));
	if(rval < 0) return rval;
	tv.tv_sec = 5; tv.tv_usec = 0;
	FD_ZERO(&rfds);
	FD_SET(ctrl_s, &rfds);
	rval = select(ctrl_s + 1, &rfds, NULL, NULL, &tv);
	if(FD_ISSET(ctrl_s, &rfds)) {
		recvlen = 0;
		rval = wpa_ctrl_recv(ctrl_s, reply, replyLen);
//printf("recv=%d\n", rval);
		while(rval >= 0) {
			recvlen += rval;
			tv.tv_sec = 0; tv.tv_usec = 0;
			FD_ZERO(&rfds);
			FD_SET(ctrl_s, &rfds);
			rval = select(ctrl_s + 1, &rfds, NULL, NULL, &tv);
			if(FD_ISSET(ctrl_s, &rfds)) {
				rval = wpa_ctrl_recv(ctrl_s, reply+recvlen, replyLen-recvlen);
printf("more recv=%d\n", rval);
			} else rval = -2;
		}
		if(rval != -1) {
			reply[recvlen] = 0;
if(recvlen > 256) printf("%lu Rx: %d [.....]\n", MS_TIMER, recvlen);
else printf("%lu Rx: %s", MS_TIMER, reply);
			rval = recvlen;
		}
	} else {
printf("Timeout...\n");
		rval = -2;
	}
	return rval;
}

int wpactrlMoreResponse(char *reply, int replyLen)
{
	struct timeval tv;
	fd_set	rfds;
	int		rval, recvlen;

	tv.tv_sec = 3; tv.tv_usec = 0;
	FD_ZERO(&rfds);
	FD_SET(ctrl_s, &rfds);
	rval = select(ctrl_s + 1, &rfds, NULL, NULL, &tv);
	if(FD_ISSET(ctrl_s, &rfds)) {
		recvlen = 0;
		rval = wpa_ctrl_recv(ctrl_s, reply, replyLen);
printf("more recv=%d\n", rval);
		while(rval >= 0) {
			recvlen += rval;
			tv.tv_sec = 0; tv.tv_usec = 0;
			FD_ZERO(&rfds);
			FD_SET(ctrl_s, &rfds);
			rval = select(ctrl_s + 1, &rfds, NULL, NULL, &tv);
			if(FD_ISSET(ctrl_s, &rfds)) {
				rval = wpa_ctrl_recv(ctrl_s, reply+recvlen, replyLen-recvlen);
printf("more recv=%d\n", rval);
			} else rval = -2;
		}
		if(rval != -1) {
			reply[recvlen] = 0;
printf("more Reply: %d\n%s", recvlen, reply);
			rval = recvlen;
		}
	} else {
printf("more Timeout...\n");
		rval = -2;
	}
	return rval;
}

int wpactrlEventRead(char *buf, int len)
{
	int		rval;

	rval = wpa_ctrl_recv(mon_s, buf, len);
	return rval;
}

