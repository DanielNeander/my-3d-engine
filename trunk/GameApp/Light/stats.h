/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef STATS_H
#define STATS_H

namespace STAT
{
	//compute the mean-square error between 'input1' and 'mean'
	template <typename T>
	double Mean( const T* input, int inputlen, double* sqenergy = 0, double* variance = 0 )
	{
		T mean = 0., energy = 0.;
		const T* ip		  = input;
		const T* inputend = input + inputlen;
		while (ip < inputend) {
			T v = *ip++;
			mean += v;
			energy += v*v;
		}
		mean /= inputlen;
		if (sqenergy) {
			*sqenergy = energy;
		}
		if (variance != 0) {
			double var = 0.;
			ip = input;
			while (ip < inputend) {
				T v = *ip++;
				v -= mean;
				var += v*v;
			}
			var /= 2. * (inputlen) * (inputlen-1);	// 2*(n*(n-1))
			*variance = var;
		}
		return mean;
	}
	//compute the sum-of-square error between 'input1' and 'input2'
	template <typename T>
	double SqErr( const T* input1, int inputlen, const T* input2 )
	{
		T mse = 0.;
		const T* ip1	   = input1;
		const T* ip2	   = input2;
		const T* input1end = input1 + inputlen;
		while (ip1 < input1end) {
			T d = (*ip1 - *ip2);
			mse += d*d;
			++ip1;
			++ip2;
		}
		return (double)mse;
	}
	template <typename T>
	double MSE( const T* input1, int inputlen, const T* input2 )
	{
		double sqerr = SqErr( input1, inputlen, input2 );
		return sqerr/inputlen;
	}

	class AllStats {
	public:
		AllStats() : m_totalEngr(0), m_percentEngr(0), m_variance(0), m_sqError(0), m_mse(0), m_psnr(0) {}
		double		m_totalEngr;	//!< sum of signal-squared
		float		m_percentEngr;
		double		m_variance;		//!< signal's variance
		double		m_sqError;		//!< sum of sq-error
		double		m_mse;			//!< mean-square-error
		double		m_psnr;			//!< PSNR
	};

	double PSNR( double sqerr, int samples );

	double PSNR( const float* input1, int inputlen, const float* input2 );
	
	void Range( float* range, const float* input, int inputlen );

	// get the stats for reference stream 'input1' and an approx stream 'input2'
	void GetAllStats( AllStats* stats, const float* input1, int inputlen, const float* input2 );
	// get the stats for reference stream 'input1' and an approx stream 'input2'
	void GetAllStats( AllStats* stats, const double* input1, int inputlen, const double* input2 );
	void PrintStats( const AllStats& stats );
};

#endif
