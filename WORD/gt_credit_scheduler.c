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
#include <time.h>

#include "gt_include.h"
#define NUM_THREADS 128

extern int init_credits(int inx) ;
extern void burn_credits() ;
extern int get_priority(uthread_struct_t *u_elem) ;
extern void check_UNDER_and_init_credits() ;



extern int init_credits(int inx)
{
		if(inx < 8 || (inx > 31 && inx < 40) || (inx > 63 && inx < 72) || (inx > 95 && inx < 104))
		 {return 25 ;}
		else if ((inx >7 && inx < 16)|| (inx >39 && inx < 48)|| (inx >71 && inx < 80)|| (inx >103 && inx < 112))
		 {return 25 ;}
		else if ((inx > 15 && inx < 24) ||(inx > 47 && inx < 56)|| (inx > 79 && inx < 88) || (inx > 111 && inx < 120))
		{return 75 ;}
		else {return 100 ; }
}

#if 1
extern int get_priority(uthread_struct_t *u_elem)
{
	if (u_elem->credits > 0 )
		return UTHREAD_PRI_UNDER ;	
	else{
	   return UTHREAD_PRI_OVER ;	
	}
		
}

#endif
#if 0
extern int get_priority(uthread_struct_t *u_elem)
{
	u_elem->uthread_priority = u_elem->credits ;
		
}
#endif

extern void burn_credits(uthread_struct_t *u_obj)
{
 		if ( u_obj->credits > 0 ){
   		u_obj->credits -= 25 ; 
   		//printf("updated credits: %d\n", u_obj->credits );	
   		}  	
    }
#if 0
extern void burn_credits()
{
    int inx ;
    uthread_struct_t u_objs[NUM_THREADS];
    uthread_struct_t *u_obj;
    for (inx = 0 ; inx < NUM_THREADS ; inx ++)
    {
 		u_obj = &u_obj[inx] ;
 		if ( u_obj->credits <= 0 )
        	return;
   		u_obj->credits -= 25 ; 
   		printf("updated credits: %d\n", u_obj->credits );	  	
    }
    
}

extern void check_UNDER_and_init_credits()
{
	uthread_struct_t u_objs[NUM_THREADS];		//??
	uthread_struct_t *u_obj ;
	int count_UNDER = 0 ;
	int inx ;
	for (inx = 0 ; inx < NUM_THREADS ; inx ++)
	{	
		u_obj = &u_obj[inx] ;
		if(!u_obj->uthread_state & (UTHREAD_DONE | UTHREAD_CANCELLED)){
			if (u_obj-> uthread_priority =UTHREAD_PRI_UNDER) {
				count_UNDER ++ ;
			}
		}
	}
	if (count_UNDER == 0) {
		for (inx = 0 ; inx < NUM_THREADS ; inx ++){
			u_obj = &u_obj[inx] ;
			if(!u_obj->uthread_state & (UTHREAD_DONE | UTHREAD_CANCELLED)){
				u_obj->credits = init_credits(u_obj->uthread_tid) ;
			}
		}
	}
	return ;

}

#endif