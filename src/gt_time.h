#ifndef __GT_TIME_H
#define __GT_TIME_H

typedef unsigned long long microtime_t;

typedef struct timekeeper
{
	microtime_t time_created;
    microtime_t time_destroyed;

    microtime_t last_start;
    microtime_t last_runtime;

	microtime_t total_runtime;
} timekeeper_t;

extern microtime_t getmicroseconds();
extern void timekeeper_create_uthread(timekeeper_t *ut);
extern void timekeeper_start_uthread(timekeeper_t *t);
extern void timekeeper_stop_uthread(timekeeper_t *t);
extern void timekeeper_destroy_uthread(timekeeper_t *t);
extern void timekeeper_csv(char *str, timekeeper_t *t);

#endif