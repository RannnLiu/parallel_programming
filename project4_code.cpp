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

	for(int i = 0; i < NUMTRAILS; i++){
		double time_start = omp_get_wtime();
        Sum(A, B, SIZE);
        double time_end = omp_get_wtime();
        double sumPerformance = (double) SIZE / (time_end - time_start) / 1000000.; 
        if(sumPerformance > max_Sum_Performance) 
		max_Sum_Performance = sumPerformance;

		time_start = omp_get_wtime();
        SimdMul(A, B, C, SIZE);
        time_end = omp_get_wtime();
        double simdMulPerformance = (double) SIZE / (time_end - time_start) / 1000000.; 
        if(simdMulPerformance > max_SimdMul_Performance) 
		max_SimdMul_Performance = simdMulPerformance;

	}
	
	printf("array size = %7d, max_Sum_Performance = %.2f, max_SimdMul_Performance = %.2f, speedup = %.2f\n", SIZE, max_Sum_Performance, max_SimdMul_Performance, max_SimdMul_Performance/max_Sum_Performance);
	
	return 0;
}