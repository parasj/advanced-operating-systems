#include <gt_include.h>

/*** Utils forward defs ***/

int weight2credits(int weight) {
	float scaled_weight = weight / 256.0; // default_weight
	float clamped_weight = (scaled_weight > 4) ? 4 : (scaled_weight < 1) ? 1 : scaled_weight;
	return (int) (clamped_weight * 30.0); // default_credits
}

void calc_priority(uthread_struct_t *ut) {
	ut->uthread_priority = ut->remaining_credits > 0 ? SCHED_CREDIT_UNDER : SCHED_CREDIT_OVER;
}

/*** Credit API ***/

void grant_credits(uthread_struct_t *ut) {
	int credits_to_grant = weight2credits(ut->sched_weight);
	assert(ut->remaining_credits < credits_to_grant);
	ut->remaining_credits = credits_to_grant;
}

void burn_credits(uthread_struct_t *ut) {
	ut->remaining_credits -= SCHED_CREDIT_BURN_PER_TIMESTEP;
}

int credit_accounting(uthread_struct_t *ut) {
	if (ut->remaining_credits >= SCHED_CREDIT_BURN_PER_TIMESTEP) {
		burn_credits(ut);
		calc_priority(ut);
		return SCHED_CREDIT_UNDER;
	} else {
		grant_credits(ut);
		calc_priority(ut);
		return SCHED_CREDIT_OVER;
	}
}

/*** Create/Destroy uthread_t ***/
void sched_credit_thread_oninit(uthread_struct_t *ut) {
	fprintf(stdout, "sched_credit_thread_oninit g%dt%d\n", uthread_gid, uthread_tid);
	grant_credits(u_new);
	calc_priority(ut);
}

void sched_credit_thread_onexit(uthread_struct_t *ut) {
	// write stats to file
	// todo
	fprintf(stdout, "sched_credit_thread_onexit g%dt%d\n", uthread_gid, uthread_tid);
}