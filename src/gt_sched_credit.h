#ifndef __GT_SCHED_CREDIT_H
#define __GT_SCHED_CREDIT_H

#define SCHED_CREDIT_UNDER 0
#define SCHED_CREDIT_OVER 1

extern int credit_accounting(uthread_struct_t *ut);
extern void sched_credit_thread_oninit(uthread_struct_t *ut);
extern void sched_credit_thread_onexit(uthread_struct_t *ut);


#endif