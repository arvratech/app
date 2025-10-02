/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/* This file contains both the uv__async internal infrastructure and the
 * user-facing uv_async_t functions.
 */

#include "uv.h"
#include "internal.h"
//#include "atomic-ops.h"	// commented and belows inserted by KDK
#define __LINUX_ARM_ARCH__	9
#define __KERNEL__			1
//#include <asm-generic/atomic.h>
#ifdef _A20
//#include <asm/atomic.h>
#else
#include <asm-arm/atomic.h>
#include "/home/w55fa92_t35/linux-2.6.35.4/arch/arm/include/asm/atomic.h"
#endif
#include <errno.h>
#include <stdio.h>  /* snprintf() */
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int uv_async_init(uv_loop_t *loop, uv_async_t *handle, uv_async_cb async_cb)
{
  int		err;

	uv__handle_init(loop, (uv_handle_t*)handle, UV_ASYNC);
	handle->async_cb = async_cb;
	handle->pending = 0;
	QUEUE_INSERT_TAIL(&loop->async_handles, &handle->queue);
	uv__handle_start(handle);
	return 0;
}

int uv_async_send(uv_async_t *handle)
{
	int		val;

  /* Do a cheap read first. */
//  if (ACCESS_ONCE(int, handle->pending) != 0)
//    return 0;

//  if (cmpxchgi(&handle->pending, 0, 1) == 0)	changed by KDK
//  val = atomic_cmpxchg(&handle->pending, 0, 1);
// if(val == 0) uv_async_watcher_send(handle->loop);
	handle->pending = 1;
	uv_async_watcher_send(handle->loop);
	return 0;
}

void uv__async_close(uv_async_t *handle)
{
	QUEUE_REMOVE(&handle->queue);
	uv__handle_stop(handle);
}

static void uv__async_io(uv_loop_t *loop, uv__io_t *w, unsigned int events)
{
	char	buf[32];
	ssize_t	r;
	QUEUE	*q, queue;
	uv_async_t *h;
	int		val;

	assert(w == &loop->async_io_watcher);
	while(1) {
		r = read(w->fd, buf, sizeof(buf));
		//if(r == sizeof(buf)) continue;
		if(r == 0) break;
		else if(r < 0) {
			if(errno == EAGAIN || errno == EWOULDBLOCK) break;
			if(errno != EINTR) abort();
		}
	}
	QUEUE_MOVE(&loop->async_handles, &queue);
	while(!QUEUE_EMPTY(&queue)) {
		q = QUEUE_HEAD(&queue);
		h = QUEUE_DATA(q, uv_async_t, queue);
		QUEUE_REMOVE(q);
		QUEUE_INSERT_TAIL(&loop->async_handles, q);
//		if(cmpxchgi(&h->pending, 1, 0) == 0)	changed by KDK
//		val = atomic_cmpxchg(&h->pending, 1, 0);
//val = __sync_bool_compare_and_swap(&h->pending, 1, 0);
		val = h->pending; h->pending = 0;
		if(val && h->async_cb) h->async_cb(h);
	}
}

int uv_async_watcher_send(uv_loop_t *loop)
{
	const void	*buf;
	static const uint64_t val = 1;
	ssize_t	len;
	int		fd, rval;

	fd = loop->async_wfd;
	buf = &val;
	len = sizeof(val);
	do
		rval = write(fd, buf, len);
	while(rval < 0 && errno == EINTR);
	if(rval == len) return 0;
	if(rval < 0)
		if(errno == EAGAIN || errno == EWOULDBLOCK) return -1;
	abort();
}

static int uv__async_eventfd(void);

int uv__async_watcher_start(uv_loop_t *loop)
{
	int		rval, fd, pipefd[2];

//printf("uv__async_watcher_start...loop=%x\n", loop);
	if(loop->async_io_watcher.fd != -1) return 0;
	rval = uv__async_eventfd();
	if(rval >= 0) {
		pipefd[0] = pipefd[1] = rval;
	} else if(rval == UV_ENOSYS) {
		rval = uv__make_pipe(pipefd, UV__F_NONBLOCK);
#if defined(__linux__)
		/* Save a file descriptor by opening one of the pipe descriptors as
		* read/write through the procfs.  That file descriptor can then
		* function as both ends of the pipe.
		*/
		if(rval == 0) {
			char	buf[32];
			int		fd;

			snprintf(buf, sizeof(buf), "/proc/self/fd/%d", pipefd[0]);
			fd = uv__open_cloexec(buf, O_RDWR);
			if(fd >= 0) {
				uv__close(pipefd[0]);
				uv__close(pipefd[1]);
				pipefd[0] = pipefd[1] = fd;
			}
		}
#endif
	}
//printf("uv__async_watcher_start...rval=0x%x\n", rval);
	if(rval < 0) return rval;
	uv__io_init(&loop->async_io_watcher, uv__async_io, pipefd[0]);
	uv__io_start(loop, &loop->async_io_watcher, POLLIN);
	loop->async_wfd = pipefd[1];

	return 0;
}

int uv__async_watcher_fork(uv_loop_t *loop)
{
	if(loop->async_io_watcher.fd == -1) /* never started */
		return 0;
	uv__async_watcher_stop(loop);
	return uv__async_watcher_start(loop);
}

void uv__async_watcher_stop(uv_loop_t *loop)
{
	if(loop->async_io_watcher.fd == -1) return;
	if(loop->async_wfd != -1) {
		if(loop->async_wfd != loop->async_io_watcher.fd)
		uv__close(loop->async_wfd);
		loop->async_wfd = -1;
	}
	uv__io_stop(loop, &loop->async_io_watcher, POLLIN);
	uv__close(loop->async_io_watcher.fd);
	loop->async_io_watcher.fd = -1;
}

static int uv__async_eventfd(void)
{
#if defined(__linux__)
	static int no_eventfd2, no_eventfd;
	int		fd;

	if(no_eventfd2) goto skip_eventfd2;
	fd = uv__eventfd2(0, UV__EFD_CLOEXEC | UV__EFD_NONBLOCK);
	if(fd != -1) return fd;
	if(errno != ENOSYS) return UV__ERR(errno);
	no_eventfd2 = 1;

skip_eventfd2:
	if(no_eventfd) goto skip_eventfd;
	fd = uv__eventfd(0);
	if(fd != -1) {
		uv__cloexec(fd, 1);
		uv__nonblock(fd, 1);
		return fd;
	}
	if(errno != ENOSYS) return UV__ERR(errno);
	no_eventfd = 1;

skip_eventfd:

#endif

	return UV_ENOSYS;
}

