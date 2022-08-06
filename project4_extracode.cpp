#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include <xmmintrin.h>

#define SSE_WIDTH		4

#ifndef SIZE 
#define SIZE 			1000
#endif

#ifndef NUMTRAILS
#define NUMTRAILS       100
#endif

#ifndef NUMT
#define NUMT            4
#endif

#define NUM_ELEMENTS_PER_CORE   SIZE/NUMT

float A[SIZE];
float B[SIZE];
float C[SIZE];

const float RANDOM = 10.0;
unsigned int seed = 0; 


float
SQR( float x )
{
        return x*x;
}

float
Ranf( unsigned int *seedp,  float low, float high )
{
        float r = (float) rand_r( seedp );              // 0 - RAND_MAX

        return( low  +  r * ( high - low ) / (float)RAND_MAX );
}

int
Ranf( unsigned int *seedp, int ilow, int ihigh )
{
        float low = (float)ilow;
        float high = (float)ihigh + 0.9999f;

        return (int)(Ranf(seedp, low,high) );
}

void
SimdMul( float *a, float *b,   float *c,   int len )
{
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;
	register float *pa = a;
	register float *pb = b;
	register float *pc = c;
	for( int i = 0; i < limit; i += SSE_WIDTH )
	{
		_mm_storeu_ps( pc,  _mm_mul_ps( _mm_loadu_ps( pa ), _mm_loadu_ps( pb ) ) );
		pa += SSE_WIDTH;
		pb += SSE_WIDTH;
		pc += SSE_WIDTH;
	}

	for( int i = limit; i < len; i++ )
	{
		c[i] = a[i] * b[i];
	}
}

float Sum(float *A, float *B, int len){
	float sum = 0.0;

	for (int i = 0; i < len; i++){
		sum += A[i] * B[i];
	}

	return sum;
}

/*
float
SimdMulSum( float *a, float *b, int len )
{
	float sum[4] = { 0., 0., 0., 0. };
	int limit = ( len/SSE_WIDTH ) * SSE_WIDTH;
	register float *pa = a;
	register float *pb = b;

	__m128 ss = _mm_loadu_ps( &sum[0] );
	for( int i = 0; i < limit; i += SSE_WIDTH )
	{
		ss = _mm_add_ps( ss, _mm_mul_ps( _mm_loadu_ps( pa ), _mm_loadu_ps( pb ) ) );
		pa += SSE_WIDTH;
		pb += SSE_WIDTH;
	}
	_mm_storeu_ps( &sum[0], ss );

	for( int i = limit; i < len; i++ )
	{
		sum[0] += a[i] * b[i];
	}

	return sum[0] + sum[1] + sum[2] + sum[3];
}
*/
int main()
{
	for (int i = 0; i < SIZE; i++){
		A[i] = Ranf( &seed, -RANDOM, RANDOM);
		B[i] = Ranf( &seed, -RANDOM, RANDOM);	
	}
	
	double max_Sum_Performance = 0.0;
    double max_SimdMul_Performance = 0.0;
    double max_Sum_Thread_Performance = 0.0;

    omp_set_num_threads(NUMT);

	for(int i = 0; i < NUMTRAILS; i++){
		double time_start = omp_get_wtime();
        Sum(A, B, SIZE);
        double time_end = omp_get_wtime();
        double sumPerformance = (double) SIZE / (time_end - time_start) / 1000000.; 
        if(sumPerformance > max_Sum_Performance) 
		max_Sum_Performance = sumPerformance;

		time_start = omp_get_wtime();
        #pragma omp parallel
        {
            int first = omp_get_thread_num() * NUM_ELEMENTS_PER_CORE;
            SimdMul(&A[first], &B[first], &C[first], NUM_ELEMENTS_PER_CORE);
        }
        time_end = omp_get_wtime();
        double simdMulPerformance = (double) SIZE / (time_end - time_start) / 1000000.; 
        if(simdMulPerformance > max_SimdMul_Performance) 
        max_SimdMul_Performance = simdMulPerformance;

        time_start = omp_get_wtime();
        #pragma omp parallel
        {
            Sum(A, B, NUM_ELEMENTS_PER_CORE);
        }
        time_end = omp_get_wtime();
        double mul_sum_thread_performance = (double) SIZE / (time_end- time_start) / 1000000.; 
        if(mul_sum_thread_performance > max_Sum_Thread_Performance) 
        max_Sum_Thread_Performance = mul_sum_thread_performance;
    }
	
	printf("threads = %d, array size = %7d, SIMD speedup = %.2f, Sum speedup = %.2f\n", NUMT ,SIZE,max_SimdMul_Performance/max_Sum_Performance, max_Sum_Thread_Performance/max_Sum_Performance);
	return 0;
}