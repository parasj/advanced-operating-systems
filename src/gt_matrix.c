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

#define NTRIALS (4*4)

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
}uthread_arg_t;
	
struct timeval tv1;

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

#if 0
static void print_matrix(matrix_t *mat)
{
	int i, j;

	for(i=0;i<SIZE;i++)
	{
		for(j=0;j<SIZE;j++)
			printf(" %d ",mat->m[i][j]);
		printf("\n");
	}

	return;
}

static void * uthread_mulmat(void *p)
{
	int i, j, k;
	unsigned int cpuid;
	struct timeval tv2;

#define ptr ((uthread_arg_t *)p)

	i=0; j= 0; k=0;

	start_row = ptr->start_row;
	end_row = (ptr->start_row + PER_THREAD_ROWS);

#ifdef GT_GROUP_SPLIT
	start_col = ptr->start_col;
	end_col = (ptr->start_col + PER_THREAD_ROWS);
#else
	start_col = 0;
	end_col = SIZE;
#endif

#ifdef GT_THREADS
	cpuid = kthread_cpu_map[kthread_apic_id()]->cpuid;
	fprintf(stderr, "Thread(id:%d, group:%d, cpu:%d) started\n",ptr->tid, ptr->gid, cpuid);
#else
	fprintf(stderr, "Thread(id:%d, group:%d) started\n",ptr->tid, ptr->gid);
#endif

	for(i = start_row; i < end_row; i++)
		for(j = start_col; j < end_col; j++)
			for(k = 0; k < SIZE; k++)
				ptr->_C->m[i][j] += ptr->_A->m[i][k] * ptr->_B->m[k][j];

#ifdef GT_THREADS
	fprintf(stderr, "Thread(id:%d, group:%d, cpu:%d) finished (TIME : %lu s and %lu us)\n",
			ptr->tid, ptr->gid, cpuid, (tv2.tv_sec - tv1.tv_sec), (tv2.tv_usec - tv1.tv_usec));
#else
	gettimeofday(&tv2,NULL);
	fprintf(stderr, "Thread(id:%d, group:%d) finished (TIME : %lu s and %lu us)\n",
			ptr->tid, ptr->gid, (tv2.tv_sec - tv1.tv_sec), (tv2.tv_usec - tv1.tv_usec));
#endif

#undef ptr
	return 0;
}
#endif

static void * uthread_mulmat(void *p) {
	int i, j, k;
	usleep(100000);
	for(i = 0; i < ((uthread_arg_t *)p)->_A->rows; i++) {
		for(j = 0; j < ((uthread_arg_t *)p)->_A->cols; j++)
			for(k = 0; k < ((uthread_arg_t *)p)->_A->rows; k++)
				((uthread_arg_t *)p)->_C->m[i][j] += ((uthread_arg_t *)p)->_A->m[i][k] * ((uthread_arg_t *)p)->_B->m[k][j];
			if (i == 16)
				gt_yield();
	}
	return 0;
}

uthread_arg_t uargs[NTRIALS];
uthread_t utids[NTRIALS];
matrix_t A[NTRIALS];
matrix_t B[NTRIALS];
matrix_t C[NTRIALS];

static void usage(char *argv[]) {
	fprintf(stderr, "usage: %s [0 | 1] where 0 represents the O(1) scheduler and 1 represents using the credit scheduler (default is O(1) scheduler)\n", argv[0]);
}

int main(int argc, char *argv[])
{
	int scheduler, mat_size, weight, calc_tid, trialid = 0;
	uthread_arg_t *uarg;

	if (argc == 2 && atoi(argv[1]) == 1) {
		scheduler = UTHREAD_CREDIT;
		fprintf(stderr, "scheduler strategy: UTHREAD_CREDIT\n");
	} else if (argc == 2 && atoi(argv[1]) == 0 || argc == 1) {
		scheduler = UTHREAD_O1;
		fprintf(stderr, "scheduler strategy: UTHREAD_O1\n");
	} else {
		usage(argv);
		exit(1);
	}

	gtthread_app_init();

	for (mat_size = 32; mat_size <= 256; mat_size <<= 1) {
		for (weight = 1; weight <= 4; ++weight) {
			calc_tid = mat_size * 10 + weight; // tid = {MATSIZE}{WEIGHT}, eg 321 means 32x32 matrix with weight 1

			generate_matrix(&A[trialid], mat_size, 2);
			generate_matrix(&B[trialid], mat_size, 4);
			generate_matrix(&C[trialid], mat_size, 0);

			uarg = &uargs[trialid];
			uarg->_A = &A[trialid];
			uarg->_B = &B[trialid];
			uarg->_C = &C[trialid];

			uarg->tid = calc_tid;
			uarg->gid = (trialid % NUM_GROUPS); // use trialid since weight isn't necessarily uniform

			uthread_create(&utids[trialid], uthread_mulmat, uarg, uarg->gid, UTHREAD_CREDIT, weight);
			fprintf(stderr, "scheduled trial %d: %d\n", trialid, calc_tid);
			++trialid;
		}
	}

	gtthread_app_exit();

	return(0);
}
