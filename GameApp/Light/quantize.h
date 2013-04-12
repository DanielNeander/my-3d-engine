/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef QUANTIZE_H
#define QUANTIZE_H

#include <malloc.h>

#include "basictypes.h"
#include "stats.h"


enum QKind { kNull = 0, kMidRise = 1, kMidThread = 2 };

/*!
-- ScalarQuant - a scalar quantization that supports mid-rise and mid-thread.
*/
class ScalarQuant {
public:
	ScalarQuant(
		QKind qkind,			//kind
		int nc,					//# of sets scalars to quantize
		const float* scales,	//scales[nc]: the scales for each set of scalars 
		const float* ranges		//ranges[nc][2]: the min..max for each set of scalars
	);
	~ScalarQuant();

	void Quantize(
		U32* output,			//output[nc]: quantizer output
		int nc,					//# of coefficient sets
		const float* scalars	//scalars[nc]: a set of scalars to be quantized
	)
	{
		float* p = (float*)_alloca( sizeof(float)*nc );
		for (int i=0; i < nc; i++) {
			float delta = scalars[i] - m_ranges[2*i];	//- bias[b]
			p[i] = delta * m_scales[i];					//* scale[b]
			output[i] =	(U32)floor(p[i] + m_half);		//mid-thread //round
		}
	}
	void GetReconLevels( float* recon, int nc );
public:
	QKind			m_kind;
	const float*	m_scales;
	const float*	m_ranges;
	float			m_half;
	float*			m_recon;
};	//end of ScalarQuant

/*!
-- CodeBook1D - codebook for a 1D signal.
*/
class CodeBook1D {
public:
	class Code {
	public:
		float	m_code;		//!< the reconstruction level
		float	m_scale;	//!< scale factor to convert the quantized samples to unsigned bitfield
	};
	CodeBook1D(int bits);
	~CodeBook1D();

public:
	int			m_len;			//!< # of code words in the code book
	Code*		m_codes;		//!< the reconstruction levels + scale factors to convert the quantized samples
};	//end of CodeBook1D

void CopyCodeBook( float* code4, int count, const CodeBook1D& codebook );

/*!
-- LloydMax optimal uniform quantizer for a 1D signal.
*/
class LloydMax1D {
public:
	enum Flags { kDouble = 1, kFloat = 2 };

	LloydMax1D( int bits );
	~LloydMax1D();
	
	void Init( int bits );
	void InitInput( int inputLen, Flags flags );
	void Clear();

	//const float*  Process( const float* input, int inputlen,  int iterations, double tolerance = 0.001f );
	const double* Process( const double* input, int inputlen, int iterations, double tolerance = 0.001 );

	double Quantize( double val, int* code );
	//maps 'val' to a bin
	int	Bin( double val ) const;

	///generate an uniform quantizer
	void GenUniformLevels();
	/// computes all the centroids given the current decision levels
	void ComputeCentroids();

	///returns the centroids/reconstruction levels:
	const double* GetCentroids() const { return m_centroids; }
	///returns the decision levels
	const double* GetLevels() const { return m_levels; }
	const U32* getOutPutCodes() const { return m_outputCode; }

	void	getCodeBook( CodeBook1D* codebook ) const;

	/*!
	-- InputCallback: callback for ProcessInput
	*/
	class InputCallback {
	public:
		InputCallback( LloydMax1D& lm ) : m_lm(lm) {}

		virtual void Start(int inputlen) {};
		virtual void OneValue( const double val ) = 0;
		virtual void Done(int inputlen) {};

		LloydMax1D&		m_lm;
	};
protected:
	typedef const double* const_leveliterator;

	const_leveliterator Begin() const { return m_levels; }
	const_leveliterator End() const { return m_levels + m_bins; }

	template <typename T>
	static void ProcessInput( const T* input, int inputlen, InputCallback& callback )
	{
		const T* ip		  = input;
		const T* inputend = input + inputlen;
		callback.Start(inputlen);
		while (ip < inputend) {
			T val = *ip;
			callback.OneValue( val );
			++ip;
		}
		callback.Done(inputlen);
	}

	///one iteration of Lloyd-Max
	double	Iterate( STAT::AllStats* stats, const double* input, int inputlen);
	///one iteration of Lloyd-Max
	double	Iterate( STAT::AllStats* stats, const float* input, int inputlen );
	void	InitPass();
	///compute centroid for all the samples within each partition
	void	UpdateCentroids();
	///returns the reconstruction data
	void	GetReconstruction( float* centroids, float* scales ) const;

	//quantize all the samples in'input1' and place in 'output'
	template <typename T>
	void QuantizeSamples( T* output, U32* codes, const T* input, int inputlen )
	{
		const T* ip	= input;
		T* op = output;
		U32* oc = codes;
		const T* inputend = input + inputlen;
		while (ip < inputend) {
			int code;
			T val = *ip;
			T qval = (T)Quantize(val, &code);
			*op++ = qval;
			*oc++ = code;
			++ip;
		}
	}
	/// returns the next non-empty bin
	int		NextBin( int b ) const {
		++b;
		return (b != m_bins) ? b : 0;
	}
	int		PrevBin( int b ) const {
		return (b != 0) ? b-1 : m_bins-1;
	}
	bool	ValidBin(int b) { return (b < m_bins) && (b >= 0); }
	bool	CheckLevels();
	void	AdjustLevelEnd();
	void	RestoreLevelEnd();

public:

	int		m_bits;			//!< # of bits to use
	int		m_bins;			//!< 2^m_bits
	int		m_inputLen;		//!< # of input samples
	double	m_mean;

	double*	m_levels;		//!< decision levels
	double* m_totals;		//!< sum total for each bin
	int*	m_count;
	double*	m_centroids;	//!< the current centroids
	double* m_mses;			//!< per partition MSEs

	double	m_range[2];
	double*	m_outputD;		//!< quantized output
	float*	m_outputF;		//!< quantized output
	U32*	m_outputCode;

	STAT::AllStats m_stats;
	STAT::AllStats m_orgStats;	//!< stats of the original input
};	//end of LloydMax

#endif