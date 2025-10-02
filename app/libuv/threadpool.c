/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
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

#include "uv-common.h"

#if !defined(_WIN32)
# include "internal.h"
#endif

#include <stdlib.h>

#define MAX_THREADPOOL_SZ		4

static uv_once_t once = UV_ONCE_INIT;
static uv_cond_t cond;
static uv_mutex_t mutex;
static unsigned int idle_threads;
static uv_thread_t	threads[MAX_THREADPOOL_SZ];
static QUEUE exit_message;
static QUEUE wq;


static void uv__cancelled(struct uv__work *w)
{
	abort();
}

/* To avoid deadlock with uv_cancel() it's crucial that the worker
 * never holds the global mutex and the loop-local mutex at the same time.
 */
static void worker(void* arg)
{
	struct uv__work	*w;
	QUEUE	*q;

	uv_sem_post((uv_sem_t*)arg);
	arg = NULL;
	while(1) {
		uv_mutex_lock(&mutex);
		while(QUEUE_EMPTY(&wq)) {
			idle_threads += 1;
			uv_cond_wait(&cond, &mutex);
			idle_threads -= 1;
		}
		q = QUEUE_HEAD(&wq);
		if(q == &exit_message) uv_cond_signal(&cond);
		else {
			QUEUE_REMOVE(q);
			QUEUE_INIT(q);	// Signal uv_cancel() that the work req is executing.
		}
		uv_mutex_unlock(&mutex);

		if(q == &exit_message) break;
		w = QUEUE_DATA(q, struct uv__work, wq);
		w->work(w);

		uv_mutex_lock(&w->loop->wq_mutex);
		w->work = NULL;		// Signal uv_cancel() that the work req is done executing.
		QUEUE_INSERT_TAIL(&w->loop->wq, &w->wq);
		uv_async_watcher_send(w->loop);
		uv_mutex_unlock(&w->loop->wq_mutex);
	}
}

static void post(QUEUE *q)
{
	uv_mutex_lock(&mutex);
	QUEUE_INSERT_TAIL(&wq, q);
	if(idle_threads > 0) uv_cond_signal(&cond);
	uv_mutex_unlock(&mutex);
}

#ifndef _WIN32
/*
UV_DESTRUCTOR(static void cleanup(void))
{
printf("destructor...\n");
//	post(&exit_message);
//	for(i = 0;i < MAX_THREADPOOL_SZ;i++)
//		if(uv_thread_join(&threads[i])) abort();
//	uv_mutex_destroy(&mutex);
//	uv_cond_destroy(&cond);
}
*/
// appended by KDK
void uv_threadpool_cleanup(void)
{
	post(&exit_message);
/*
	for(i = 0;i < MAX_THREADPOOL_SZ;i++) {
		rval = uv_thread_join(&threads[i]);
		if(rval) {
			printf("%d thread_join() error: %s\n", i, strerror(-rval));
			break;
		}
	}
printf("threadpool_cleanup=%d\n", i);
	uv_mutex_destroy(&mutex);
	uv_cond_destroy(&cond);
printf("threadpool_cleanup end...\n");
*/
}

#endif

static void init_threads(void)
{
	const char	*val;
	uv_sem_t sem;
	int		i;

	if(uv_cond_init(&cond)) abort();
	if(uv_mutex_init(&mutex)) abort();
	QUEUE_INIT(&wq);
	if(uv_sem_init(&sem, 0)) abort();
	for(i = 0;i < MAX_THREADPOOL_SZ;i++)
		if(uv_thread_create(&threads[i], worker, &sem)) abort();
	for(i = 0;i < MAX_THREADPOOL_SZ;i++)
		uv_sem_wait(&sem);
	uv_sem_destroy(&sem);
}

#ifndef _WIN32
static void reset_once(void)
{
	uv_once_t child_once = UV_ONCE_INIT;
	memcpy(&once, &child_once, sizeof(child_once));
}
#endif

static void init_once(void)
{
#ifndef _WIN32
  /* Re-initialize the threadpool after fork.
   * Note that this discards the global mutex and condition as well
   * as the work queue.
   */
	if(pthread_atfork(NULL, NULL, &reset_once)) abort();
#endif
	init_threads();
}

void uv_init_async_threads(void)
{
	uv_once(&once, init_once);
}

void uv__work_submit(uv_loop_t* loop, struct uv__work *w, void (*work)(struct uv__work *w),
							void (*done)(struct uv__work *w, int status))
{
	w->loop = loop;
	w->work = work;
	w->done = done;
	post(&w->wq);
}

static int uv__work_cancel(uv_loop_t *loop, uv_req_t *req, struct uv__work *w)
{
	int	cancelled;

	uv_mutex_lock(&mutex);
	uv_mutex_lock(&w->loop->wq_mutex);

	cancelled = !QUEUE_EMPTY(&w->wq) && w->work != NULL;
	if(cancelled) QUEUE_REMOVE(&w->wq);

	uv_mutex_unlock(&w->loop->wq_mutex);
	uv_mutex_unlock(&mutex);

	if(!cancelled) return UV_EBUSY;

	w->work = uv__cancelled;
	uv_mutex_lock(&loop->wq_mutex);
	QUEUE_INSERT_TAIL(&loop->wq, &w->wq);
	uv_async_watcher_send(loop);
	uv_mutex_unlock(&loop->wq_mutex);

	return 0;
}

// modified by KDK
// void uv__work_done(uv_async_t *handle)
// ==> call uv__run_work in the uv_run loop
void uv__run_work(uv_loop_t *loop)
{
	struct uv__work *w;
	QUEUE	*q, wq;
	int		err;

	uv_mutex_lock(&loop->wq_mutex);
	QUEUE_MOVE(&loop->wq, &wq);
	uv_mutex_unlock(&loop->wq_mutex);
	while(!QUEUE_EMPTY(&wq)) {
		q = QUEUE_HEAD(&wq);
		QUEUE_REMOVE(q);
		w = container_of(q, struct uv__work, wq);
		err = (w->work == uv__cancelled) ? UV_ECANCELED : 0;
		w->done(w, err);
	}
}

static void uv__queue_work(struct uv__work* w)
{
	uv_work_t *req;

	req = container_of(w, uv_work_t, work_req);
	req->work_cb(req);
}

static void uv__queue_done(struct uv__work* w, int err)
{
	uv_work_t *req;

	req = container_of(w, uv_work_t, work_req);
	uv__req_unregister(req->loop, req);
	if(req->after_work_cb == NULL) return;
	req->after_work_cb(req, err);
}

int uv_queue_work(uv_loop_t *loop, uv_work_t *req, uv_work_cb work_cb, uv_after_work_cb after_work_cb)
{
	if(work_cb == NULL) return UV_EINVAL;
	uv__req_init(loop, req, UV_WORK);
	req->loop = loop;
	req->work_cb = work_cb;
	req->after_work_cb = after_work_cb;
	uv__work_submit(loop, &req->work_req, uv__queue_work, uv__queue_done);
	return 0;
}

int uv_cancel(uv_req_t *req)
{
	struct uv__work *wreq;
	uv_loop_t *loop;

	switch(req->type) {
	case UV_FS:
		loop =  ((uv_fs_t *)req)->loop;
		wreq = &((uv_fs_t *)req)->work_req;
		break;
	case UV_GETADDRINFO:
		loop =  ((uv_getaddrinfo_t *)req)->loop;
		wreq = &((uv_getaddrinfo_t *)req)->work_req;
		break;
	case UV_GETNAMEINFO:
		loop =  ((uv_getnameinfo_t *)req)->loop;
		wreq = &((uv_getnameinfo_t *)req)->work_req;
		break;
	case UV_WORK:
		loop =  ((uv_work_t *)req)->loop;
		wreq = &((uv_work_t *)req)->work_req;
		break;
	default:
		return UV_EINVAL;
	}
	return uv__work_cancel(loop, req, wreq);
}

// Belows are writed newly by KDK

void uv__loop_work_submit(uv_loop_t* loop, struct uv__work *w, void (*done)(struct uv__work *w, int err))
{
	w->loop = loop;
	w->work = NULL;
	w->done = done;
	uv_mutex_lock(&loop->wq_mutex);
	QUEUE_INSERT_TAIL(&loop->wq, &w->wq);
	uv_async_watcher_send(loop);
	uv_mutex_unlock(&loop->wq_mutex);
}

static void uv__loop_done(struct uv__work* w, int err)
{
	uv_work_t *req;

	req = container_of(w, uv_work_t, work_req);
	req->after_work_cb(req, err);
}

static void uv__loop_work(struct uv__work* w, int err)
{
	uv_work_t *req;

	req = container_of(w, uv_work_t, work_req);
	req->work_cb(req);
	if(req->after_work_cb) uv__loop_work_submit(req->loop, &req->work_req, uv__loop_done);
}

int uv_queue_loop_work(uv_loop_t *loop, uv_loop_t *wloop, uv_work_t *req,
								 uv_work_cb work_cb, uv_after_work_cb after_work_cb)
{
	if(work_cb == NULL) return UV_EINVAL;
	uv__req_init(loop, req, UV_WORK);
	req->loop = loop;
	req->work_cb = work_cb;
	req->after_work_cb = after_work_cb;
	uv__loop_work_submit(wloop, &req->work_req, uv__loop_work);
	return 0;
}

