#include <gt_include.h>

/*** Utils forward defs ***/

int weight2credits(int weight) {
	float scaled_weight = weight / 256.0; // default_weight
	float clamped_weight = (scaled_weight > 4) ? 4 : (scaled_weight < 1) ? 1 : scaled_weight;
	return (int) (clamped_weight * 30.0); // default_credits
}

void calc_priority(uthread_struct_t *ut) {
	ut->uthread_priority = ut->sched_credits.credits_left > 0 ? SCHED_CREDIT_UNDER : SCHED_CREDIT_OVER;
}

/*** Credit API ***/

void grant_credits(uthread_struct_t *ut) {
	int credits_to_grant = weight2credits(ut->sched_weight);
	assert(ut->credits_left < credits_to_grant);

	ut->sched_credits.credits_left += credits_to_grant;
	calc_priority(ut);
}

void burn_credits(uthread_struct_t *ut) {
	int credits_to_burn = ut->runtime_owed;
	ut->runtime_owed = 0;
	ut->sched_credits.credits_left -= credits_to_burn;
	calc_priority(ut);
}


/*** Create/Destroy uthread_t ***/
void sched_credit_thread_oninit(uthread_struct_t *ut) {
	fprintf(stdout, "sched_credit_thread_oninit g%dt%d\n", uthread_gid, uthread_tid);
	grant_credits(u_new);
}

void sched_credit_thread_onexit(uthread_struct_t *ut) {
	// write stats to file
	fprintf(stdout, "sched_credit_thread_onexit g%dt%d\n", uthread_gid, uthread_tid);
}