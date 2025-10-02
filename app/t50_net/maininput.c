#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include "defs.h"
#include "prim.h"
#include "uv.h"
#include "dev.h"
#include "hw.h"
#include "siocred.h"
#include "slvmlib.h"
#include "slvm.h"
#include "sctask_com.h"
#include "maininput.h"

int		pipeMain[2];
uv_poll_t	_poll_pipe, _poll_sio;


void *MainLoop(void)
{
	return uv_default_loop();
}

int MainOpenPipe(void)
{
	int		rval;

	rval = pipeOpen(pipeMain);
	if(rval < 0) {
		return rval;
	}
	uv_poll_init(MainLoop(), &_poll_pipe, pipeMain[0]);
	scSetOnDataDone(MainPostPipe);
	return 0;
}

void MainClosePipe(void)
{
	uv_poll_stop(&_poll_pipe);
	pipeClose(pipeMain);
}

void MainOnReadPipe(void)
{
	unsigned char	buf[256];
	int		rval;

	rval = pipeRead(_poll_pipe.io_watcher.fd, buf);
	if(rval > 0) slvmCredential(buf, rval);
}

static void _MainOnPollPipe(uv_poll_t *handle, int status, int events)
{
//printf("### onMainPollPipe ###\n");
	//fd = handle->io_watcher.fd;
	MainOnReadPipe();
}

void MainStartPipe(void)
{
	uv_poll_start(&_poll_pipe, UV_READABLE, _MainOnPollPipe);
}

void MainPostPipe(unsigned char *buf, int length)
{
	pipeWrite(pipeMain[1], buf, length);
}

int MainOpenSio(void)
{
	int		fd;
	
	fd = sioOpen(19200L);
	if(fd < 0) return -1;
	uv_poll_init(MainLoop(), &_poll_sio, fd);
	return 0;
}

void MainCloseSio(void)
{
	uv_poll_stop(&_poll_sio);
	close(_poll_sio.io_watcher.fd);
}

void MainOnReadSio(void)
{
	unsigned char	buf[128];
	void	*cr;
	int		rval, fd;

	fd = _poll_sio.io_watcher.fd;
	rval = read(fd, buf, 128);
	cr = crsGet(0);
	if(rval > 0 && crIsEnableHynixEmvReader(cr)) sioOnRead(fd, buf, rval);
}

static void _MainOnPollSio(uv_poll_t *handle, int status, int events)
{
//printf("### onMainPollSio ### %d\n", rval);
	//fd = handle->io_watcher.fd;
	MainOnReadSio();
}

void MainStartSio(void)
{
	uv_poll_start(&_poll_sio, UV_READABLE, _MainOnPollSio);
}

