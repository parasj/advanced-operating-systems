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
#include <string.h>

#include "gt_include.h"
#include "math.h"


#define ROWS 512
#define COLS ROWS
// #define SIZE COLS

#define NUM_CPUS 2
#define NUM_GROUPS NUM_CPUS
#define PER_GROUP_COLS (SIZE/NUM_GROUPS)

#define NUM_THREADS 128
#define PER_THREAD_ROWS (SIZE/NUM_THREADS)
// #define SCHED_YIELD

/* A[SIZE][SIZE] X B[SIZE][SIZE] = C[SIZE][SIZE]
 * Let T(g, t) be thread 't' in group 'g'. 
 * T(g, t) is responsible for multiplication : 
 * A(rows)[(t-1)*SIZE -> (t*SIZE - 1)] X B(cols)[(g-1)*SIZE -> (g*SIZE - 1)] */
int SIZE = 256 ;
int temp_array[128];
int _C[128][128][128] ;

typedef struct matrix
{
	int m[ROWS][ROWS];

	int rows;
	int cols;
	unsigned int reserved[2];
	gt_spinlock_t matrix_lock;
} matrix_t;


typedef struct __uthread_arg
{
	matrix_t *_A, *_B, *_C;
	unsigned int reserved0;

	unsigned int tid;
	unsigned int gid;
	int start_row; /* start_row -> (start_row + PER_THREAD_ROWS) */
	int start_col; /* start_col -> (start_col + PER_GROUP_COLS) */
	int end_row;
	int end_col;

	int credits ;
	int scheduler_mode ;
}uthread_arg_t;
	
struct timeval tv1;

static void generate_matrix(matrix_t *mat, int val, int size)
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
	int start_row, end_row;
	int start_col, end_col;
	unsigned int cpuid;
	struct timeval tv2;

#define ptr ((uthread_arg_t *)p)

	i=0; j= 0; k=0;

#ifdef SCHED_YIELD	
	if(!(gt_spin_lock_defined(&ptr->_C->matrix_lock)))
	{
		printf("\n YIELD \n");
		yield();
	}
#endif
	start_row = 0;//ptr->start_row;
	end_row = ptr->end_row;//(ptr->start_row + PER_THREAD_ROWS);

#ifdef GT_GROUP_SPLIT
	start_col = ptr->start_col;
	end_col = (ptr->start_col + PER_THREAD_ROWS);
#else
	start_col = 0;//ptr->start_col;
	end_col = ptr->end_col;
#endif
#ifdef GT_THREADS
	cpuid = kthread_cpu_map[kthread_apic_id()]->cpuid;
	// fprintf(stderr, "\nThread(id:%d, group:%d, cpu:%d) started",ptr->tid, ptr->gid, cpuid);
#else
	// fprintf(stderr, "\nThread(id:%d, group:%d) started",ptr->tid, ptr->gid);
#endif

	for(i = start_row; i < end_row; i++)
		for(j = start_col; j < end_col; j++)
			for(k = 0; k < end_col; k++)
				ptr->_C->m[i][j] += ptr->_A->m[i][k] * ptr->_B->m[k][j];

#ifdef GT_THREADS
	//fprintf(stderr, "\nThread(id:%d, group:%d, cpu:%d) finished (TIME : %lu s and %lu us)",
			//ptr->tid, ptr->gid, cpuid, (tv2.tv_sec - tv1.tv_sec), (tv2.tv_usec - tv1.tv_usec));
#else
	gettimeofday(&tv2,NULL);
	//fprintf(stderr, "\nThread(id:%d, group:%d) finished (TIME : %lu s and %lu us)",
			//ptr->tid, ptr->gid, (tv2.tv_sec - tv1.tv_sec), (tv2.tv_usec - tv1.tv_usec));
#endif

#ifdef SCHED_YIELD		
	gt_spin_unlock(&(ptr->_C->matrix_lock));
#endif

#undef ptr
	return 0;
}

matrix_t A, B, C;

static void init_matrices(int size)
{
	generate_matrix(&A, 1, size);
	generate_matrix(&B, 1, size);
	generate_matrix(&C, 0, size);

	return;
}

long sum (long *array, int start, int end)
{
	int i;
	long sum_value = 0;
	for(i = start; i<end; i++)
	sum_value += array[i];

	return sum_value;
}
		
float standard_deviation(long *array, int start, int end, float mean)
{
	int i;
	long sd = 0;
	long temp = 0;
	for(i=start; i<end; i++)
	temp += (array[i] - mean)*(array[i] - mean);

	sd = (float) sqrt(temp/32);

	return sd;
}

void print_time()
{
	float mean_life_time[16] ;
	float mean_running_time[16] ;
	float sd_life_time[16] ;
	float sd_running_time[16] ;

	printf("MEAN OF LIFE TIME\n");

	int start_count ;
	int  end_count ;
	int i ;

	for(i = 0 ; i < 16 ; i ++ ){
		start_count = i * 8 ;
		end_count = (i+1) * 8 ; 
		mean_life_time[i] = (float) sum(stats.life_time, start_count, end_count)/8;
		mean_running_time[i] = (float) sum(stats.running_time, start_count, end_count)/8;
	}

	for(i = 0 ; i < 16 ; i ++ ){
		printf("\n Mean life time of group %d = %f usec\n", i, mean_life_time[i]);
	}

	printf("MEAN OF RUNNING TIME\n");

	for(i = 0 ; i < 16 ; i ++ ){
		printf("\n Mean running time of group %d = %f usec\n", i, mean_running_time[i]);
	}

    printf("\nSTANDARD DEVIATION OF LIFE TIME\n");

    for(i = 0 ; i < 16 ; i ++ ){
		start_count = i * 8 ;
		end_count = (i+1) * 8 ; 
		sd_life_time[i] = (float) standard_deviation(stats.life_time, start_count, end_count, mean_life_time[i]);
		sd_running_time[i] = (float) standard_deviation(stats.life_time, start_count, end_count, mean_running_time[i]);
	}

	for(i = 0 ; i < 16 ; i ++ ){
		printf("\n SD life time of group %d = %f usec\n", i, sd_life_time[i]);
	}

    printf("\nSTANDARD DEVIATION OF RUNNING TIME\n");

    for(i = 0 ; i < 16 ; i ++ ){
		printf("\n SD running time of group %d = %f usec\n", i, sd_running_time[i]);
	}

	printf("\n\n");

	return;
}

void init_size(int inx, int* SIZE)
{
	if(inx < 32)
	{
		*SIZE = 32 ;
	}
	else if (inx > 31 && inx < 64)
	{
		*SIZE = 64 ;
	}
	else if (inx > 63 && inx < 96)
	{
		*SIZE = 128 ;
	}
	else
	{
		*SIZE = 256 ;
	}

}


uthread_arg_t uargs[NUM_THREADS];
uthread_t utids[NUM_THREADS];

int main(int argc, char *argv[])
{	
	// printf("1 = credit scheduler, 0 = priority scheduler :\n");
	int mode ;
	int comp ;
	
	char str[1] ;
	strcpy(str,"1") ;
	comp = strcmp(argv[1],str) ;
	
	if (argc != 2){
		printf("out of range\n" );
	}else{
		if (comp == 0){
			mode = 1;
			printf("credit scheduler mode\n");
		}else{
			mode = 0 ;
			printf("priority scheduler mode\n");
		}
	}


	uthread_arg_t *uarg;
	int inx;
	SIZE = 256 ;

	gtthread_app_init();

	init_matrices(SIZE);

	gettimeofday(&tv1,NULL);

	for(inx=0; inx<NUM_THREADS; inx++)
	{
		uarg = &uargs[inx];
		uarg->_A = &A;
		uarg->_B = &B;
		uarg->_C = &C;
		init_size(inx, &SIZE) ;
		uarg->credits = init_credits(inx) ;

		uarg->tid = inx;
		uarg->gid = (inx % NUM_GROUPS);
		uarg->start_row = (inx * PER_THREAD_ROWS);
		
#ifdef GT_GROUP_SPLIT
		/* Wanted to split the columns by groups !!! */
		uarg->start_col = (uarg->gid * PER_GROUP_COLS);
#endif
		uarg->start_row = 0;
		uarg->end_row = SIZE;
		uarg->start_col = 0;
		uarg->end_col = SIZE;
		uarg->scheduler_mode = mode ;

		uthread_create(&utids[inx], uthread_mulmat, uarg, uarg->gid, uarg->credits, uarg->scheduler_mode);
	}

	gtthread_app_exit();

	print_time() ;
	// print_matrix(&C);
	// fprintf(stderr, "********************************");
	return(0);
}
