#include<stdio.h>
#include<stdlib.h>
#include<omp.h>
#include<time.h>
#include<math.h>

#ifndef NUMT
#define NUMT		    2
#endif

#ifndef NUMNODES
#define NUMNODES	200
#endif

#ifndef NUMTRIES
#define NUMTRIES	10
#endif

#define XMIN     -1.
#define XMAX      1.
#define YMIN     -1.
#define YMAX      1.

#define N	0.70

float Height( int , int );	// function prototype

int main( int argc, char *argv[ ] )
{
    //. . .

	// the area of a single full-sized tile:
	// (not all tiles are full-sized, though)

	float fullTileArea = (  ( ( XMAX - XMIN )/(float)(NUMNODES-1) )  *
				( ( YMAX - YMIN )/(float)(NUMNODES-1) )  );

	float maxPerformance = 0.0;

	// sum up the weighted heights into the variable "volume"
	
    float sum_vol = 0.0;

	// using an OpenMP for loop and a reduction:
	omp_set_num_threads(NUMT);

	//?????
	for(int t = 0; t < NUMTRIES; t++){
		//float sum_vol = 0.;
		double time0 = omp_get_wtime();
		
		#pragma omp parallel for default(none) shared(fullTileArea) reduction(+:sum_vol)
		for( int i = 0; i < NUMNODES*NUMNODES; i++ )
		{
			int iu = i % NUMNODES;
			int iv = i / NUMNODES;
			float z = Height( iu, iv );
			float vol = fullTileArea;
			//. . .
			//corner, edge, middle
			if (((iu == NUMNODES-1)||(iu == 0)) && ((iv == NUMNODES-1)||(iv == 0))) //corner
				vol = z * fullTileArea * 0.25;
			else if(((iu != NUMNODES-1)&&(iu != 0)) && ((iv != NUMNODES-1) && (iv != 0))) //middle
				vol = z * fullTileArea;
			else
				vol = z * fullTileArea * 0.5; //edge
			sum_vol += vol * 2.0;
		}
		double time1 = omp_get_wtime();
		float megaTrialsPerSecond = (float) NUMNODES * NUMNODES / (time1 - time0)/ 1000000.; 
		if(megaTrialsPerSecond > maxPerformance)
            maxPerformance = megaTrialsPerSecond;
		
	}
	float avg_vol = sum_vol / NUMTRIES;
	
	printf("%2d threads, %d trials nodes: %5d; maxPerformance = %2.2lf, Average Volume = %f\n", NUMT, NUMTRIES, NUMNODES, maxPerformance, avg_vol);
    return 0;
}

float
Height( int iu, int iv )	// iu,iv = 0 .. NUMNODES-1
{
	float x = -1.  +  2.*(float)iu /(float)(NUMNODES-1);	// -1. to +1.
	float y = -1.  +  2.*(float)iv /(float)(NUMNODES-1);	// -1. to +1.

	float xn = pow( fabs(x), (double)N );
	float yn = pow( fabs(y), (double)N );
	float r = 1. - xn - yn;
	if( r <= 0. )
	        return 0.;
	float height = pow( r, 1./(float)N );
	return height;
}
