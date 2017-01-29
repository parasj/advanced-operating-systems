#ifndef __GT_UTHREAD_H
#define __GT_UTHREAD_H
#include <time.h>
#include <setjmp.h>
#include <ucontext.h>
#include <signal.h>
/* User-level thread implementation (using alternate signal stacks) */

typedef unsigned int uthread_t;
typedef unsigned int uthread_group_t;

/* uthread states */
#define UTHREAD_INIT 0x01
#define UTHREAD_RUNNABLE 0x02
#define UTHREAD_RUNNING 0x04
#define UTHREAD_CANCELLED 0x08
#define UTHREAD_DONE 0x10

#define CREDITS_BURN_TSLICE 10000  // us
#define YIELD_TSLICE 30000  // us

/* uthread struct : has all the uthread context info */
typedef struct uthread_struct
{
	
	int uthread_state; /* UTHREAD_INIT, UTHREAD_RUNNABLE, UTHREAD_RUNNING, UTHREAD_CANCELLED, UTHREAD_DONE */
	int uthread_priority; /* uthread running priority */
	int cpu_id; /* cpu it is currently executing on */
	int last_cpu_id; /* last cpu it was executing on */
	
	uthread_t uthread_tid; /* thread id */
	uthread_group_t uthread_gid; /* thread group id  */
	int (*uthread_func)(void*);
	void *uthread_arg;

	void *exit_status; /* exit status */
	int reserved1;
	int reserved2;
	int reserved3;

	int scheduler_mode ;  // 1 = credit scheduler, 0 = priority scheduler

	int credits;				
	long running_time;
	long life_time;
	long last_time ;
	long now ;
	long last_time_over ;
	long now_over ;
	struct timespec start_time, end_time, create_time;

	int cycle_count ;
	
	sigjmp_buf uthread_env; /* 156 bytes : save user-level thread context*/
	stack_t uthread_stack; /* 12 bytes : user-level thread stack */
	TAILQ_ENTRY(uthread_struct) uthread_runq;
} uthread_struct_t;

struct __kthread_runqueue;
extern void uthread_schedule(uthread_struct_t * (*kthread_best_sched_uthread)(struct __kthread_runqueue *));

typedef struct uthread_stats_holder {
	long running_time[128];
	long life_time[128];
} final_uthread_stats;
final_uthread_stats stats;

extern void yield();
#endif
