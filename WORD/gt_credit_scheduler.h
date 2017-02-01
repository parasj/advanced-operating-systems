#ifndef __GT_CREDIT_SCHEDULER_H
#define __GT_CREDIT_SCHEDULER_H


#define UTHREAD_PRI_UNDER     0      /* time-share w/ credits */
#define UTHREAD_PRI_OVER      1      /* time-share w/o credits */

#define DEFAULT_UTHREAD_PRIORITY 16


extern int init_credits(int inx) ;
extern void burn_credits() ;
extern int get_priority(uthread_struct_t *u_elem) ;
extern void check_UNDER_and_init_credits() ;


#endif