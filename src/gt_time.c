#include <stdio.h>
#include <unistd.h>
#include <linux/unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sched.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <assert.h>
#include <time.h>

#include "gt_include.h"

microtime_t getmicroseconds() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (microtime_t) (tv.tv_sec * (long long) 1000000 + tv.tv_usec);
}

/*** Timekeeper API ***/

void timekeeper_create_uthread(timekeeper_t *t) {
	assert(t->time_created == 0);
    t->time_created = getmicroseconds();
    t->time_destroyed = 0;
    t->last_start = 0;
    t->total_runtime = 0;
}

void timekeeper_start_uthread(timekeeper_t *t) {
    t->last_start = getmicroseconds();
}

void timekeeper_stop_uthread(timekeeper_t *t) {
    assert(t->last_start != 0);
    microtime_t delta = getmicroseconds() - t->last_start;
    t->total_runtime += delta;
    t->last_start = 0;
}

void timekeeper_destroy_uthread(timekeeper_t *t) {
    assert(t->time_destroyed == 0);
	t->time_destroyed = getmicroseconds();
}