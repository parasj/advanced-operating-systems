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

#include "gt_include.h"

#define SCHED_CREDIT_BURN_PER_TIMESTEP 25
#define SCHED_CREDIT_TIMESTEP KTHREAD_VTALRM_USEC // 100ms

#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))

/*** Utils forward defs ***/

int weight2credits(int weight) {
	assert(weight <= 4 && weight >= 1);
	int allocations[] = {25, 50, 75, 100};
	return allocations[weight - 1];
}

void calc_priority(uthread_struct_t *ut) {
	// ut->uthread_priority = ut->remaining_credits > 0 ? SCHED_CREDIT_UNDER : SCHED_CREDIT_OVER;`
}

/*** Credit API ***/

void grant_credits(uthread_struct_t *ut) {
	int credits_to_grant = weight2credits(ut->sched_weight);
#if GTTHREAD_LOG
	fprintf(stderr, "grant_credits %d %d %d g%dt%d\n", ut->sched_weight, ut->remaining_credits, ut->remaining_credits + credits_to_grant, ut->uthread_gid, ut->uthread_tid);
#endif
	assert(ut->remaining_credits < credits_to_grant);
	ut->remaining_credits = credits_to_grant;
}

void burn_credits(uthread_struct_t *ut) {
	unsigned int burned = (int) round(((float) ut->t.last_runtime / SCHED_CREDIT_TIMESTEP) * SCHED_CREDIT_BURN_PER_TIMESTEP);
	ut->remaining_credits -= burned;
#if GTTHREAD_LOG
	fprintf(stderr, "burn_credits g%dt%d %d credits (now %d) based on %llu us runtime\n", ut->uthread_gid, ut->uthread_tid, burned, ut->remaining_credits, ut->t.last_runtime);
#endif
}

int credit_accounting(uthread_struct_t *ut) {
#if GTTHREAD_LOG	
	fprintf(stderr, "credit_accounting g%dt%d has %d\n", ut->uthread_gid, ut->uthread_tid, ut->remaining_credits);
#endif
	if (ut->remaining_credits >= SCHED_CREDIT_BURN_PER_TIMESTEP) {
		burn_credits(ut);
		calc_priority(ut);
		return SCHED_CREDIT_UNDER;
	} else {
		return SCHED_CREDIT_OVER;
	}
}

/*** Create/Destroy uthread_t ***/
void sched_credit_thread_oninit(uthread_struct_t *ut) {
#if GTTHREAD_LOG
	fprintf(stderr, "sched_credit_thread_oninit g%dt%d\n", ut->uthread_gid, ut->uthread_tid);
#endif
	grant_credits(ut);
	calc_priority(ut);
}

void sched_credit_thread_topup(uthread_struct_t *ut) {
#if GTTHREAD_LOG
	fprintf(stderr, "sched_credit_thread_topup g%dt%d\n", ut->uthread_gid, ut->uthread_tid);
#endif
	grant_credits(ut);
	calc_priority(ut);
}

void sched_credit_thread_onexit(uthread_struct_t *ut) {
	// write stats to file
	fprintf(stderr, "%d\t%d\t%llu\t%llu\n", ut->uthread_gid, ut->uthread_tid, ut->t.total_runtime, getmicroseconds() - ut->t.time_created);
#if GTTHREAD_LOG
	fprintf(stderr, "sched_credit_thread_onexit g%dt%d\n", ut->uthread_gid, ut->uthread_tid);
#endif
}