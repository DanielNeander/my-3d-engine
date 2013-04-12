/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#include "stdafx.h"

#include "stats.h"

using namespace STAT;

double STAT::PSNR( double sqerr, int samples )
{
	double mse = sqerr/samples;
	double psnr = 10 * log10(1.0 / mse);
	return psnr;
}

double STAT::PSNR( const float* input1, int inputlen, const float* input2 )
{
	double sqerr = SqErr( input1, inputlen, input2 );
	double psnr = PSNR( sqerr, inputlen );
	return psnr;
}

void STAT::Range( float* ranges, const float* input, int inputlen )
{
	ranges[0] = FLT_MAX;
	ranges[1] = -FLT_MAX;
	const float* ip		  = input;
	const float* inputend = input + inputlen;

	while (ip < inputend) {
		const float val = *ip++;
		ranges[0] = min( ranges[0], val );
		ranges[1] = max( ranges[1], val );
	}
}
// get the stats for reference stream 'input1' and an approx stream 'input2'
void STAT::GetAllStats( AllStats* stats, const float* input1, int inputlen, const float* input2 )
{
	stats->m_sqError = SqErr( input1, inputlen, input2 );
	stats->m_mse = stats->m_sqError / inputlen;
	stats->m_psnr = PSNR( stats->m_sqError, inputlen );
	double mean = Mean( input1, inputlen, &stats->m_totalEngr );
}

// get the stats for reference stream 'input1' and an approx stream 'input2'
void STAT::GetAllStats( AllStats* stats, const double* input1, int inputlen, const double* input2 )
{
	stats->m_sqError = SqErr( input1, inputlen, input2 );
	stats->m_mse = stats->m_sqError / inputlen;
	stats->m_psnr = PSNR( stats->m_sqError, inputlen );
	double mean = Mean( input1, inputlen, &stats->m_totalEngr );
}

void STAT::PrintStats( const AllStats& stats )
{
//  printf("Avg Eng Y: %0.2f\n", avg_yR);
	printf("Sqnrm: %3.2f, ", stats.m_totalEngr);
	printf("%2.1f%c ", stats.m_percentEngr, '%');
	printf("Sq err: %0.2f, ", stats.m_sqError);
	printf("MSE: %0.2e, ", stats.m_mse);
	printf("PSNR: %0.2f(dB)\n", stats.m_psnr);
}

