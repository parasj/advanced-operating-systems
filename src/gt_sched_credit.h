#ifndef __GT_SCHED_CREDIT_H
#define __GT_SCHED_CREDIT_H

#define SCHED_CREDIT_UNDER 0
#define SCHED_CREDIT_OVER 1

extern void grant_credits(uthread_struct_t *ut);
extern void burn_credits(uthread_struct_t *ut);

extern void sched_uthread_create_log(uthread_struct_t *ut);

#endif
