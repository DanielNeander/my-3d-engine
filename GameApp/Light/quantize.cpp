/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/
#include "stdafx.h"
#include <algorithm>
#include <bitset>
#include <xutility>
#include <float.h>
#include <math.h>
#include <malloc.h>
#include <assert.h>
#include <crtdbg.h>

#include "quantize.h"
#include "stats.h"


ScalarQuant::ScalarQuant(
	QKind qkind,			//kind
	int nc,
	const float* scales,	//scales[nc]: the scales for each set of scalars 
	const float* ranges		//ranges[nc][2]: the min..max for each set of scalars
) : m_kind(qkind), m_scales(scales), m_ranges(ranges), m_recon(0)
{
	if (m_recon == 0) {
		m_recon = new float[nc];

		for (int b=0; b < nc; b++) {
			m_recon[b] = m_ranges[2*b+0];
			
			if (qkind == kMidThread) {	//handle midThread
				float binw = 1.f/m_scales[b];
				m_recon[b] += binw/2;
			}
		}
	}
	m_half = (m_kind == kMidRise) ? 0.5f : 0.f;
}

ScalarQuant::~ScalarQuant()
{
	delete [] m_recon;
}

void ScalarQuant::GetReconLevels( float* recon, int nc )
{
	for (int i=0; i < nc; i++) {
		recon[i] = m_recon[i];
	}
}

CodeBook1D::CodeBook1D(int bits) : m_codes(0)
{
	int m_len = (1 << bits);
	m_codes = new Code[m_len];
}

CodeBook1D::~CodeBook1D()
{
	delete [] m_codes;
}

void CopyCodeBook( float* code4, int count, const CodeBook1D& codebook )
{
	assert( count >= codebook.m_len*4 );
	float* cp = code4;

	for (int c=0; c < codebook.m_len; c++) {
		const CodeBook1D::Code& code = codebook.m_codes[c];
		cp[0] = code.m_code;
		cp += 4;
	}
}

LloydMax1D::LloydMax1D( int bits )
:  m_bits(0), m_levels(0), m_totals(0), m_count(0), m_centroids(0), m_mses(0),
   m_outputD(0), m_outputF(0), m_outputCode(0), m_inputLen(0)
{
	Init(bits);
}

LloydMax1D::~LloydMax1D()
{
	Clear();
	delete [] m_outputD;	m_outputD = 0;
	delete [] m_outputF;	m_outputF = 0;
	delete [] m_outputCode; m_outputCode = 0;
}

void LloydMax1D::Clear()
{
	delete [] m_levels;		m_levels = 0;
	delete [] m_centroids;	m_centroids = 0;
	delete [] m_totals;		m_totals = 0;
	delete [] m_count;		m_count = 0;
	delete [] m_mses;		m_mses = 0;
}

void LloydMax1D::Init( int bits )
{
	if (m_bits != bits) {
		Clear();
		m_bins = (1 << bits);
		m_levels	= new double[m_bins];
		m_centroids = new double[m_bins];
		m_totals    = new double[m_bins];
		m_count		= new int[m_bins];
		m_mses		= new double[m_bins];
	}
}

void LloydMax1D::InitInput( int inputLen, Flags flags )
{
	if (m_inputLen != inputLen) {
		if (flags & kFloat) {
			delete [] m_outputF;
			m_outputF = new float[inputLen];
		}
		else {
			delete [] m_outputD;
			m_outputD = new double[inputLen];
		}
		m_outputCode = new U32[inputLen];
		m_inputLen = inputLen;
	}
}

/*
-- RangeAndMean:
*/
class RangeAndMean : public LloydMax1D::InputCallback {
public:
	RangeAndMean(LloydMax1D& lm, double* range, double* mean) : LloydMax1D::InputCallback(lm), m_total(0.f), m_outRange(range), m_mean(mean)
	{
		m_range[0] = FLT_MAX;
		m_range[1] = -FLT_MAX;
	}
	void OneValue( const double val )
	{
		m_total += val;
		m_range[0] = min( m_range[0], val );
		m_range[1] = max( m_range[1], val );
	}
	void Done(int inputlen)
	{
		*m_mean = m_total/inputlen;
		m_outRange[0] = m_range[0];
		m_outRange[1] = m_range[1];
	}
	double	m_total;
	double	m_range[2];
	double*	m_outRange;
	double*	m_mean;
};	//end of RangeAndMean


/*
-- Centroid:
*/
class Centroid : public LloydMax1D::InputCallback {
public:
	Centroid(LloydMax1D& lm, int bins);
	~Centroid();
	void Start(int inputlen);
	void OneValue( const double val );
	void Done(int inputlen);
	void Dump();
	bool IsEmpty(int b) const { return m_count[b] == 0; }
public:
	int				m_numbins;
	double*			m_bins;
	int*			m_count;
	float			m_total;
};	//end of Centroid

Centroid::Centroid(LloydMax1D& lm, int bins) : LloydMax1D::InputCallback(lm), m_numbins(bins)
{
	m_bins  = new double[bins];
	m_count = new int[bins];
}

Centroid::~Centroid()
{
	delete [] m_bins;
	delete [] m_count;
}

void Centroid::Start(int inputlen)
{
	for (int b=0; b < m_numbins; b++) {
		m_bins[b] = 0.;
		m_count[b] = 0;
	}
}

void Centroid::OneValue(const double val)
{
	int bin = m_lm.Bin(val);
	m_count[bin]++;
	m_bins[bin] += val;
}

void Centroid::Done(int inputlen)
{
	for (int b=0; b < m_numbins; b++) {
		if (!IsEmpty(b))
			m_bins[b] /= m_count[b];
	}
	m_total = (float)inputlen;
}

void Centroid::Dump()
{
	printf("Centroids:\n");
	for (int b=0; b < m_numbins; b++) {
		double uniform = m_lm.m_centroids[b];
		float percentage = (m_count[b]/m_total) * 100.f;
		printf("bin[%2d]: %2.6f, %2.6f, %2.3f\n", b, m_bins[b], uniform, percentage);
	}
}

bool LloydMax1D::CheckLevels()
{
	const_leveliterator begin = Begin();
	const_leveliterator end   = End() - 1;
	bool result = true;
	while (begin != end) {
		if (*begin > *(begin+1)) {
			result = false;
			break;
		}
		++begin;
	}
	assert(result);
	assert( m_levels[m_bins-1] > m_range[1] );	//this must hold for Quantize to work

	return result;
}

void LloydMax1D::AdjustLevelEnd()
{
	m_levels[m_bins-1] = m_range[1] + DBL_EPSILON;	//make sure the last level is > m_range[1]
}

void LloydMax1D::RestoreLevelEnd()
{
	m_levels[m_bins-1] = m_range[1];
}

void LloydMax1D::GenUniformLevels()
{
	double step = (m_range[1] - m_range[0]) / m_bins;
	double* upper = m_levels;		//our decision levels;
	double level = m_range[0] + step;

	for (int i=0; i < m_bins; i++) {
		*upper++ = level;
		level += step;
	}
	ComputeCentroids();

	AdjustLevelEnd();
}

/// computes all the centroids given the current decision levels
void LloydMax1D::ComputeCentroids()
{
	RestoreLevelEnd();			//remove eps from end
	double l0 = m_range[0];

	for (int i=0; i < m_bins; i++) {
		m_centroids[i] = (m_levels[i] + l0)/2.;
		l0 = m_levels[i];
	}
	AdjustLevelEnd();			//add eps back to the last decision level
}

int	LloydMax1D::Bin( double val ) const {
	const_leveliterator loc = std::lower_bound(Begin(), End(), val );
	int bin = loc - Begin();
	assert( bin < m_bins );
#if 0
	int bn = NextBin(bin), bp = PrevBin(bin);
	double q0  = m_centroids[bin];
	double q1  = m_centroids[bn];
	double q_1 = m_centroids[bp];

	double err0 = abs(val - q0);
	double err1 = abs(val - q1);
	double err_1 = abs(val - q_1);
	if (err1 < err0) {
		bin = bn;
		err0 = err1;
	}
	if (err_1 < err0) {
		return bp;
	}
#endif
	return bin;
}

double LloydMax1D::Quantize( double val, int* code )
{
	int bin = Bin(val);
	double l1 = m_levels[bin], l0 = (bin != 0 ? m_levels[bin-1] : 0.);

	m_totals[bin] += val;
	m_count[bin]  += 1;
	double qval = m_centroids[bin];
	double err = (val - qval);	//*(val - qval);
	m_mses[bin]   += err*err;
	*code = bin;

	return qval;
}

void LloydMax1D::InitPass()
{
	for (int b=0; b < m_bins; b++) {
		m_totals[b] = 0.;
		m_count[b]  = 0;
		m_mses[b]   = 0.;
	}
	//compute new decision levels:
	for (int b=0; b < m_bins-1; b++) {
		double l = (m_centroids[b] + m_centroids[b+1])/2;
		m_levels[b] = l;
	}
	AdjustLevelEnd();
	CheckLevels();
}

///compute centroid for all the samples within each partition
void LloydMax1D::UpdateCentroids()
{
	double mse = 0.;

	for (int b=0; b < m_bins; b++) {
		if (m_count[b] != 0) {
			double pmean = m_totals[b]/m_count[b];
			m_centroids[b] = pmean;
			mse += m_mses[b];
			m_mses[b] /= m_count[b];
		}
	}
	mse /= m_inputLen;

	//int nb = NextBin(0);
	//while (nb != m_bins) {
	
		//nb = NextBin(nb+1);
	//}
}

///returns the reconstruction data
void LloydMax1D::GetReconstruction( float* centroids, float* scales ) const
{
	int maxsym = m_bins-1;		//maximum representable symbol
	float l0 = (float)m_range[0];		//lower bound of decision level

	for (int b=0; b < m_bins; b++) {
		centroids[b] = (float)m_centroids[b];
		double width = (m_levels[b] - l0);
		double scale = (double)maxsym/width;
		scales[b] = (float)scale;
		l0 = (float)m_levels[b];
	}
}

void LloydMax1D::getCodeBook( CodeBook1D* codebook ) const
{
	float* centroids = (float*)_alloca( sizeof(float)* m_bins );
	float* scales	 = (float*)_alloca( sizeof(float)* m_bins );
	GetReconstruction( centroids, scales );
	CodeBook1D::Code* codes = codebook->m_codes;

	for (int b=0; b < m_bins; b++) {
		codes->m_code  = *centroids++;
		codes->m_scale = *scales++;
		++codes;
	}
	codebook->m_len = m_bins;
}

//one iteration of Lloyd-Max
double LloydMax1D::Iterate( STAT::AllStats* stats, const double* input, int inputlen )
{
	InitPass();
	QuantizeSamples<double>( m_outputD, m_outputCode, input, inputlen );
	UpdateCentroids();	//compute new centroids

	GetAllStats( stats, input, inputlen, m_outputD );

	return stats->m_mse;
}

//one iteration of Lloyd-Max
double LloydMax1D::Iterate( STAT::AllStats* stats, const float* input, int inputlen )
{
	InitPass();
	QuantizeSamples<float>( m_outputF, m_outputCode, input, inputlen );
	UpdateCentroids();	//compute new centroids

	GetAllStats( stats, input, inputlen, m_outputF );

	return stats->m_mse;
}

#if 0
const float* LloydMax1D::Process( const float* input, int inputlen, int iterations, double tolerance )
{
	InitInput( inputlen, kFloat );
	double range, mean;
	RangeAndMean meanrange( &range, &mean );
	ProcessInput( input, inputlen, meanrange );

	m_range[0] = range[0];	m_range[1] = range[1];
	m_mean = mean;
	GenUniformLevels();

	double mse = 0., mse2;
	int tried = 0;
	STAT::AllStats stats;

	mse2 = Iterate( &stats, input, inputlen );	++tried;
	m_orgStats = stats;		//record stats of results without Lloyd-Max

	while ((tried < iterations) && (abs(mse2-mse) > tolerance)) {
		m_stats = stats;
		mse = mse2;
		printf("  iteration (%d): %f\n", tried, mse);
		mse2 = Iterate( &stats, input, inputlen );
		++tried;
	}
	m_stats = stats;

	return m_outputF;
}
#endif

const double* LloydMax1D::Process( const double* input, int inputlen, int iterations, double tolerance )
{
	InitInput( inputlen, kDouble );
	
	RangeAndMean meanrange( *this, m_range, &m_mean );
	ProcessInput<double>( input, inputlen, meanrange );

	GenUniformLevels();

	Centroid centroid( *this, m_bins );
	ProcessInput<double>( input, inputlen, centroid );
	centroid.Dump();

	double mse = 0., mse2;
	int tried = 0;
	STAT::AllStats stats;

	mse2 = Iterate( &stats, input, inputlen );	++tried;
	m_orgStats = stats;		//record stats of results without Lloyd-Max
	double improve = tolerance*2;

	while ((tried < iterations) /*&& (abs(mse2-mse) > tolerance)*/) {
		m_stats = stats;
		mse = mse2;
		printf("  iteration (%d): mse: %f, PSNR: %f\n", tried, mse, stats.m_psnr);
		mse2 = Iterate( &stats, input, inputlen );
		improve = abs(mse2 - mse)/mse2;		//% improvement
		++tried;
	}
	m_stats = stats;

	return m_outputD;
}
