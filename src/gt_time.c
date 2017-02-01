#include <gt_include.h>

microtime_t getmicroseconds() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return (microtime_t) (tv.tv_sec * (long long) 1000000 + tv.tv_usec);
}

/*** Timekeeper API ***/

void timekeeper_create_uthread(timekeeper_t *t) {
	assert(t->time_created == NULL);
    t->time_created = getmicroseconds();
    t->time_destroyed = NULL;
    t->last_start = NULL;
    t->total_runtime = NULL;
}

void timekeeper_start_uthread(timekeeper_t *t) {
	assert(t->last_start == NULL);
    t->last_start = getmicroseconds();
}

void timekeeper_stop_uthread(timekeeper_t *t) {
    assert(t->last_start != NULL);
	microtime_t delta = getmicroseconds() - t->last_start;
    t->total_runtime += delta;
    t->last_start = NULL;

    printf("%llu us CPU time elapsed\n", delta);
}

void timekeeper_destroy_uthread(timekeeper_t *t) {
    assert(t->time_destroyed == NULL);
	t->time_destroyed = getmicroseconds();
    
    char str[1000] = {0};
    timekeeper_csv(str, t);
    fprintf(stdout, "%s\n", str);
}

void timekeeper_csv(char *str, timekeeper_t *t) {
    sprintf(str, "created=%llu\tdestroyed=%llu\ttotal_runtime=%llu", t->time_created, t->time_destroyed, t->total_runtime);
}