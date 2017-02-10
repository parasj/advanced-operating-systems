#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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

#define NUM_CPUS 2
#define NUM_GROUPS NUM_CPUS

#define NTRIALS (4*4*8)

typedef struct matrix
{
	int m[256][256]; // MAX SIZE

	int rows;
	int cols;
	unsigned int reserved[2];
} matrix_t;


typedef struct __uthread_arg
{
	matrix_t *_A, *_B, *_C;
	unsigned int reserved0;

	unsigned int tid;
	unsigned int gid;

	unsigned int credits;
} uthread_arg_t;
	
microtime_t tv1;

static void generate_matrix(matrix_t *mat, int size, int val)
{
	int i,j;
	mat->rows = size;
	mat->cols = size;
	for(i = 0; i < mat->rows;i++)
		for( j = 0; j < mat->cols; j++ )
		{
			mat->m[i][j] = val;
		}
	return;
}

static void * uthread_mulmat(void *p) {
	int i, j, k;
	microtime_t time_end;

#define ptr ((uthread_arg_t *)p)
	i=0; j= 0; k=0;

	for(i = 0; i < ptr->_A->rows; i++) {
		for(j = 0; j < ptr->_B->rows; j++) {
			for(k = 0; k < ptr->_C->rows; k++) {
				ptr->_C->m[i][j] += ptr->_A->m[i][k] * ptr->_B->m[k][j];
			}
		}
	
		
		if (i == 64)
			gt_yield();
	}

	time_end = getmicroseconds();
	fprintf(stderr, "{'msg': 'trial_complete', 'tid': %d, 'gid': %d, 'trial_time': %llu, 'mat_size': %d, 'credits': %d}\n", ptr->tid, ptr->gid, time_end - tv1, ptr->_A->rows, ptr->credits * 25);

#undef ptr
	return 0;
}

uthread_arg_t uargs[NTRIALS];
uthread_t utids[NTRIALS];
microtime_t cputimes[NTRIALS];
microtime_t realtimes[NTRIALS];
matrix_t A[NTRIALS];
matrix_t B[NTRIALS];
matrix_t C[NTRIALS];

static void usage(char *argv[]) {
	fprintf(stderr, "usage: %s [0 | 1] where 0 represents the O(1) scheduler and 1 represents using the credit scheduler (default is O(1) scheduler)\n", argv[0]);
}

int main(int argc, char *argv[])
{
	sched_strategy_t scheduler;
	int mat_size, weight, calc_tid, trialid = 0;
	uthread_arg_t *uarg;

	if (argc == 2 && atoi(argv[1]) == 1) {
		scheduler = UTHREAD_CREDIT;
		// fprintf(stdout, "{\"msg\": \"sched_strategy\", \"sched_strategy\": \"UTHREAD_CREDIT\"}\n");
	} else if (argc == 2 && atoi(argv[1]) == 0 || argc == 1) {
		scheduler = UTHREAD_O1;
		// fprintf(stdout, "{\"msg\": \"sched_strategy\", \"sched_strategy\": \"UTHREAD_O1\"}\n");
	} else {
		usage(argv);
		exit(1);
	}

	gtthread_app_init();
	gt_set_sched_strategy(scheduler);

	trialid = 0;

	int i;
	for (weight = 4; weight >= 1; --weight) {
		for (mat_size = 32; mat_size <= 256; mat_size <<= 1) {
			for (i = 0; i < 8; ++i) {
				generate_matrix(&A[trialid], mat_size, 1);
				generate_matrix(&B[trialid], mat_size, 1);
				generate_matrix(&C[trialid], mat_size, 0);
				trialid++;
			}
		}
	}

	trialid = 0;
	tv1 = getmicroseconds();

	int iter;

	for (weight = 4; weight >= 1; --weight) {
		for (mat_size = 32; mat_size <= 256; mat_size <<= 1) {
			for (iter = 0; iter < 8; ++iter) {
				uarg = &uargs[trialid];
				uarg->_A = &A[trialid];
				uarg->_B = &B[trialid];
				uarg->_C = &C[trialid];

				uarg->credits = weight;

				uarg->tid = trialid;
				uarg->gid = 0;

				uthread_create(&utids[trialid], uthread_mulmat, uarg, uarg->gid, weight, &cputimes[trialid], &realtimes[trialid]);
				// fprintf(stdout, "\n{\"msg\": \"trial_scheduled\", \"trial_id\": %d, \"mat_size\": %d, \"weight\": %d}\n", trialid, mat_size, weight);
				++trialid;
			}
		}
	}

	gtthread_app_exit();

	trialid = 0;
	for (weight = 4; weight >= 1; --weight) {
		for (mat_size = 32; mat_size <= 256; mat_size <<= 1) {
			for (iter = 0; iter < 8; ++iter) {
				fprintf(stderr, "{\"size\": %d, \"wgt\" %d, \"iteration"\: %d, \"rtime\" %llu, \"ctime\" %llu\n", mat_size, weight, iter, cputimes[trialid], realtimes[trialid]);
				trialid++;
			}
		}
	}

	return(0);
}
