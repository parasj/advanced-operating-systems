#include <gt_include.h>

int weight2credits(uthread_struct_t *ut) {
	int weight = ut->sched_weight;
	int allocations[] = {0, 25, 50, 75, 100};
	assert(weight <= 4);
	assert(weight >= 1);

	return allocations[weight];

	// u_new->sched_weight = (weight > 4) ? 4 : (weight < 1) ? 1 : weight; // clamp weights
	// u_new->sched_credits.credits_left = allocations[u_new->sched_weight - 1];
}

int calc_priority(uthread_struct_t *ut) {
	ut->uthread_priority = u_new->sched_credits.credits_left > 0 ? 0 : 1;
	return ut->uthread_priority;
}