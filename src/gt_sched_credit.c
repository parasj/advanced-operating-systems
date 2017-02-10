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

#define SCHED_CREDIT_BURN_PER_TIMESTEP 200 /* 1 per 10ms */
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
	fprintf(stdout, "{\"msg\": \"grant_credits\", \"weight\": %d, \"orig_credits\": %d, \"credits\": %d, \"gid\": %d, \"tid\": %d}\n", ut->sched_weight, ut->remaining_credits, ut->remaining_credits + credits_to_grant, ut->uthread_gid, ut->uthread_tid);
#endif
	assert(ut->remaining_credits < credits_to_grant);
	ut->remaining_credits = credits_to_grant;
}

void burn_credits(uthread_struct_t *ut) {
	unsigned int burned = (int) round(((float) ut->t.last_runtime / SCHED_CREDIT_TIMESTEP) * SCHED_CREDIT_BURN_PER_TIMESTEP);
	ut->remaining_credits -= burned;
#if GTTHREAD_LOG
	fprintf(stdout, "{\"msg\": \"burn_credits\", \"gid\": %d, \"id\": %d, \"orig_credits\": %d, \"credits\": %d, \"runtime_microtime\": %llu}\n", ut->uthread_gid, ut->uthread_tid, burned + ut->remaining_credits, ut->remaining_credits, ut->t.last_runtime);
#endif
}

int credit_accounting(uthread_struct_t *ut) {
	burn_credits(ut);
	calc_priority(ut);

	if (ut->remaining_credits > 0) {
		return SCHED_CREDIT_UNDER;
	} else {
		return SCHED_CREDIT_OVER;
	}
}

/*** Create/Destroy uthread_t ***/
void sched_credit_thread_oninit(uthread_struct_t *ut) {
#if GTTHREAD_LOG
	fprintf(stdout, "{\"msg\": \"sched_credit_thread_oninit\", \"gid\": %d, \"tid\": %d}\n", ut->uthread_gid, ut->uthread_tid);
#endif
	grant_credits(ut);
	calc_priority(ut);
}

void sched_credit_thread_topup(uthread_struct_t *ut) {
#if GTTHREAD_LOG
	fprintf(stdout, "{\"msg\": \"sched_credit_thread_topup\", \"gid\": %d, \"tid\": %d}\n", ut->uthread_gid, ut->uthread_tid);
#endif
	grant_credits(ut);
	ut->topup_counter++;
	calc_priority(ut);
}

void sched_credit_thread_onexit(uthread_struct_t *ut) {
#if GTTHREAD_LOG
	fprintf(stdout, "{\"msg\": \"sched_credit_thread_onexit\", \"gid\": %d, \"tid\": %d, \"total_cpu_time_microtime\": %llu, \"total_real_time_microtime\": %llu, \"n_top_ups\": %d}\n", ut->uthread_gid, ut->uthread_tid, ut->t.total_runtime, getmicroseconds() - ut->t.time_created, ut->topup_counter);
#endif
}