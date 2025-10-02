#include "uv.h"
#include "task.h"
#include <sys/stat.h>
#include <stdio.h>
#include "../mylib/rtc.h"


static int once_cb_called = 0;
static int once_close_cb_called = 0;
static int repeat_cb_called = 0;
static int repeat_close_cb_called = 0;
static int order_cb_called = 0;
static uint64_t start_time;
static uv_timer_t tiny_timer;
static uv_timer_t huge_timer1;
static uv_timer_t huge_timer2;


static void once_close_cb(uv_handle_t* handle)
{
  printf("%u once_close_cb...\n", MS_TIMER);
  ASSERT(handle);
  ASSERT(0 == uv_is_active(handle));

  once_close_cb_called++;
}


static void once_cb(uv_timer_t* handle)
{
  printf("ONCE_CB %d\n", once_cb_called);

  ASSERT(handle != NULL);
  ASSERT(0 == uv_is_active((uv_handle_t*) handle));

  once_cb_called++;

  uv_close((uv_handle_t*)handle, once_close_cb);

  /* Just call this randomly for the code coverage. */
  uv_update_time(uv_default_loop());
}


static void repeat_close_cb(uv_handle_t* handle)
{
  printf("REPEAT_CLOSE_CB\n");

  ASSERT(handle != NULL);

  repeat_close_cb_called++;
}

static void repeat_cb(uv_timer_t* handle) {
  printf("REPEAT_CB\n");

  ASSERT(handle != NULL);
  ASSERT(1 == uv_is_active((uv_handle_t*) handle));

  repeat_cb_called++;

  if (repeat_cb_called == 5) {
    uv_close((uv_handle_t*)handle, repeat_close_cb);
  }
}

static void never_cb(uv_timer_t* handle) {
  FATAL("never_cb should never be called");
}


TEST_IMPL(timer) {
  uv_timer_t once_timers[10];
  uv_timer_t *once;
  uv_timer_t repeat, never;
  unsigned int i;
  int r;

printf("uv_test_timer....\n");
  start_time = uv_now(uv_default_loop());
  ASSERT(0 < start_time);

  /* Let 10 timers time out in 500 ms total. */
  for (i = 0; i < ARRAY_SIZE(once_timers); i++) {
    once = once_timers + i;
    r = uv_timer_init(uv_default_loop(), once);
    ASSERT(r == 0);
    r = uv_timer_start(once, once_cb, i * 50, 0);
    ASSERT(r == 0);
  }

  /* The 11th timer is a repeating timer that runs 4 times */
  r = uv_timer_init(uv_default_loop(), &repeat);
  ASSERT(r == 0);
  r = uv_timer_start(&repeat, repeat_cb, 100, 100);
  ASSERT(r == 0);

  /* The 12th timer should not do anything. */
  r = uv_timer_init(uv_default_loop(), &never);
  ASSERT(r == 0);
  r = uv_timer_start(&never, never_cb, 100, 100);
  ASSERT(r == 0);
  r = uv_timer_stop(&never);
  ASSERT(r == 0);
  uv_unref((uv_handle_t*)&never);

  uv_run(uv_default_loop(), UV_RUN_DEFAULT);

  ASSERT(once_cb_called == 10);
  ASSERT(once_close_cb_called == 10);
  printf("repeat_cb_called %d\n", repeat_cb_called);
  ASSERT(repeat_cb_called == 5);
  ASSERT(repeat_close_cb_called == 1);

  ASSERT(500 <= uv_now(uv_default_loop()) - start_time);

  MAKE_VALGRIND_HAPPY();
  return 0;
}


TEST_IMPL(timer_start_twice) {
  uv_timer_t once;
  int r;

  r = uv_timer_init(uv_default_loop(), &once);
  ASSERT(r == 0);
  r = uv_timer_start(&once, never_cb, 86400 * 1000, 0);
  ASSERT(r == 0);
  r = uv_timer_start(&once, once_cb, 10, 0);
  ASSERT(r == 0);
  r = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
  ASSERT(r == 0);

  ASSERT(once_cb_called == 1);

  MAKE_VALGRIND_HAPPY();
  return 0;
}


TEST_IMPL(timer_init) {
  uv_timer_t handle;

  ASSERT(0 == uv_timer_init(uv_default_loop(), &handle));
  ASSERT(0 == uv_timer_get_repeat(&handle));
  ASSERT(0 == uv_is_active((uv_handle_t*) &handle));

  MAKE_VALGRIND_HAPPY();
  return 0;
}


static void order_cb_a(uv_timer_t *handle) {
  ASSERT(order_cb_called++ == *(int*)handle->data);
}


static void order_cb_b(uv_timer_t *handle) {
  ASSERT(order_cb_called++ == *(int*)handle->data);
}


TEST_IMPL(timer_order) {
  int first;
  int second;
  uv_timer_t handle_a;
  uv_timer_t handle_b;

  first = 0;
  second = 1;
  ASSERT(0 == uv_timer_init(uv_default_loop(), &handle_a));
  ASSERT(0 == uv_timer_init(uv_default_loop(), &handle_b));

  /* Test for starting handle_a then handle_b */
  handle_a.data = &first;
  ASSERT(0 == uv_timer_start(&handle_a, order_cb_a, 0, 0));
  handle_b.data = &second;
  ASSERT(0 == uv_timer_start(&handle_b, order_cb_b, 0, 0));
  ASSERT(0 == uv_run(uv_default_loop(), UV_RUN_DEFAULT));

  ASSERT(order_cb_called == 2);

  ASSERT(0 == uv_timer_stop(&handle_a));
  ASSERT(0 == uv_timer_stop(&handle_b));

  /* Test for starting handle_b then handle_a */
  order_cb_called = 0;
  handle_b.data = &first;
  ASSERT(0 == uv_timer_start(&handle_b, order_cb_b, 0, 0));

  handle_a.data = &second;
  ASSERT(0 == uv_timer_start(&handle_a, order_cb_a, 0, 0));
  ASSERT(0 == uv_run(uv_default_loop(), UV_RUN_DEFAULT));

  ASSERT(order_cb_called == 2);

  MAKE_VALGRIND_HAPPY();
  return 0;
}


static void tiny_timer_cb(uv_timer_t* handle) {
  ASSERT(handle == &tiny_timer);
  uv_close((uv_handle_t*) &tiny_timer, NULL);
  uv_close((uv_handle_t*) &huge_timer1, NULL);
  uv_close((uv_handle_t*) &huge_timer2, NULL);
}


TEST_IMPL(timer_huge_timeout) {
  ASSERT(0 == uv_timer_init(uv_default_loop(), &tiny_timer));
  ASSERT(0 == uv_timer_init(uv_default_loop(), &huge_timer1));
  ASSERT(0 == uv_timer_init(uv_default_loop(), &huge_timer2));
  ASSERT(0 == uv_timer_start(&tiny_timer, tiny_timer_cb, 1, 0));
  ASSERT(0 == uv_timer_start(&huge_timer1, tiny_timer_cb, 0xffffffffffffLL, 0));
  ASSERT(0 == uv_timer_start(&huge_timer2, tiny_timer_cb, (uint64_t) -1, 0));
  ASSERT(0 == uv_run(uv_default_loop(), UV_RUN_DEFAULT));
  MAKE_VALGRIND_HAPPY();
  return 0;
}


static void huge_repeat_cb(uv_timer_t* handle) {
  static int ncalls;

  if (ncalls == 0)
    ASSERT(handle == &huge_timer1);
  else
    ASSERT(handle == &tiny_timer);

  if (++ncalls == 10) {
    uv_close((uv_handle_t*) &tiny_timer, NULL);
    uv_close((uv_handle_t*) &huge_timer1, NULL);
  }
}


TEST_IMPL(timer_huge_repeat) {
  ASSERT(0 == uv_timer_init(uv_default_loop(), &tiny_timer));
  ASSERT(0 == uv_timer_init(uv_default_loop(), &huge_timer1));
  ASSERT(0 == uv_timer_start(&tiny_timer, huge_repeat_cb, 2, 2));
  ASSERT(0 == uv_timer_start(&huge_timer1, huge_repeat_cb, 1, (uint64_t) -1));
  ASSERT(0 == uv_run(uv_default_loop(), UV_RUN_DEFAULT));
  MAKE_VALGRIND_HAPPY();
  return 0;
}


static unsigned int timer_run_once_timer_cb_called;


static void timer_run_once_timer_cb(uv_timer_t* handle) {
  timer_run_once_timer_cb_called++;
}


TEST_IMPL(timer_run_once) {
  uv_timer_t timer_handle;

  ASSERT(0 == uv_timer_init(uv_default_loop(), &timer_handle));
  ASSERT(0 == uv_timer_start(&timer_handle, timer_run_once_timer_cb, 0, 0));
  ASSERT(0 == uv_run(uv_default_loop(), UV_RUN_ONCE));
  ASSERT(1 == timer_run_once_timer_cb_called);

  ASSERT(0 == uv_timer_start(&timer_handle, timer_run_once_timer_cb, 1, 0));
  ASSERT(0 == uv_run(uv_default_loop(), UV_RUN_ONCE));
  ASSERT(2 == timer_run_once_timer_cb_called);

  uv_close((uv_handle_t*) &timer_handle, NULL);
  ASSERT(0 == uv_run(uv_default_loop(), UV_RUN_ONCE));

  MAKE_VALGRIND_HAPPY();
  return 0;
}


TEST_IMPL(timer_null_callback) {
  uv_timer_t handle;

  ASSERT(0 == uv_timer_init(uv_default_loop(), &handle));
  ASSERT(UV_EINVAL == uv_timer_start(&handle, NULL, 100, 100));

  MAKE_VALGRIND_HAPPY();
  return 0;
}


static void once1_cb(uv_timer_t *handle)
{
  printf("%u timer callback...\n", MS_TIMER);
  ASSERT(handle);
  ASSERT(0 == uv_is_active((uv_handle_t*) handle));
printf("uv_close...\n");
  uv_close((uv_handle_t*)handle, once_close_cb);
  /* Just call this randomly for the code coverage. */
  uv_update_time(uv_default_loop());
}

/*
int main(int argc, char *argv[])
{
  uv_timer_t once;
  int	rval;

printf("%u uv_test_timer....\n", MS_TIMER);
//	start_time = uv_now(uv_default_loop());
//	ASSERT(0 < start_time);
	rval = uv_timer_init(uv_default_loop(), &once);
    rval = uv_timer_start(&once, once1_cb, 3000, 0);
	uv_run(uv_default_loop(), UV_RUN_DEFAULT);

printf("end....\n");
  return 0;
}
*/

static uv_thread_t thread;
static uv_mutex_t mutex;

static uv_prepare_t prepare;
static uv_async_t async;

static volatile int async_cb_called;
static int prepare_cb_called;
static int close_cb_called;


static void thread_task(void *arg)
{
	int		rval;

	while(1) {
		sleep(5);
printf("%u uv_async_send....\n", MS_TIMER);
		rval = uv_async_send(&async);
		sleep(0);
	}
}

static void close_cb(uv_handle_t* handle)
{
	ASSERT(handle != NULL);
}

void async_cb(uv_async_t* handle)
{
printf("%u aync_cb...\n", MS_TIMER);
  ASSERT(handle == &async);
 // uv_close((uv_handle_t*)&async, close_cb);
// uv_close((uv_handle_t*)&prepare, close_cb);
}

void prepare_cb(uv_prepare_t* handle)
{
	 int	rval;

	prepare_cb_called++;
printf("%u prepare_cb...\n", MS_TIMER);
	ASSERT(handle == &prepare);
	if(prepare_cb_called == 1) {
		rval = uv_thread_create(&thread, thread_task, NULL);
printf("thread_create=%d\n", rval);
		ASSERT(rval == 0);
//		rval = uv_thread_join(&thread);
	}
}

int main(int argc, char *argv[])
{
  int	rval;

	prepare_cb_called = 0;
	rval = uv_prepare_init(uv_default_loop(), &prepare);
	ASSERT(rval == 0);
	rval = uv_prepare_start(&prepare, prepare_cb);
	ASSERT(rval == 0);
//	uv_prepare_stop(&prepare);
	rval = uv_async_init(uv_default_loop(), &async, async_cb);
	ASSERT(rval == 0);
	rval = uv_run(uv_default_loop(), UV_RUN_DEFAULT);
printf("exit....\n");
	ASSERT(rval == 0);
	return 0;
}
