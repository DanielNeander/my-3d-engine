/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/
#include "stdafx.h"
#include <xutility>
#include <stdio.h>
#include <assert.h>
#include <conio.h>

#include "rconfig.h"
#include "prt.h"
#include "Vector.h"
#include "ray.h"
//#include "triangleMesh.h"
//#include "scene.h"
#include "SH.h"
#include "sampling.h"
#include "mathutils.h"
#include "quantize.h"
#include "stats.h"

#include "GameApp/GameScene.h"

#define USELOG	0
/*
-- File header for the .raw shadow sample file. 
*/
struct RawHeader {
	int		m_vertCount;
	int		m_samples;		//!< # of rays
	U32		m_crc;
};

using namespace MATH;

static size_t WriteRawHeader( FILE* f, const RawHeader& header );
static size_t ReadRawHeader( FILE* f, RawHeader* header );
static size_t WriteRaw( const float* yR, int numsamples, FILE* rawf );

static int kOffset = 0;
static FILE* gLogFile = 0;

PRT::PRT() : vertsize(0), bands(0), coeffs(0), m_ranges(0), m_ranges2(0) {}

PRT::PRT(int numverts, int b) 
: vertsize(numverts), bands(0), coeffs(0), 
  m_scalars(0), m_scalarBytes(0), m_scalarRC48(0), m_scalarRC32(0), m_scalarLM(0),
  m_ranges(0), m_ranges2(0), m_recon(0), m_quanKind(kNull), m_codebook(0), m_stats(0), m_scales(0)
{
	resize( numverts, b );
}

PRT::~PRT() {
	delete [] coeffs;
	delete [] m_scalars;
	delete [] m_scalarBytes;
	delete [] m_scalarRC48;
	delete [] m_scalarRC32;
	delete [] m_scalarLM;
	delete [] m_ranges;
	delete [] m_ranges2;
	delete [] m_scales;
	delete [] m_recon;
	delete [] m_codebook;
	delete [] m_stats;
}

void PRT::reset()
{
	for (int i = 0; i < vertsize*bands; ++i) { //initialize all coeffs to 0 to begin
		coeffs[i].r = 0;
		coeffs[i].g = 0;
		coeffs[i].b = 0;
	}
}

void PRT::resize(int numverts, int b)
{
	if ((numverts != vertsize) || (b != bands)) {
		delete [] coeffs;
	}
	if (b != bands) {
		delete [] m_stats;
		m_stats = new STAT::AllStats[b];
		m_codebook = new CodeBook1D*[b];
		m_ranges2  = new float[b*2];
	}
	coeffs = new RGBCoeff[numverts*b];
	vertsize = numverts;
	bands = b;
}

///returns the size in bytes for our coefficients
size_t PRT::getSizeBytes(bool color) const
{
	if (color) {
		return sizeof(RGBCoeff)*vertsize*bands;
	}
	else {
		return sizeof(float)*vertsize*bands;
	}
}

void PRT::getSamples(double* samples, int b) const
{
	int nb = bands;
	for (int i = 0; i < vertsize; ++i) {
		samples[i] = (double)m_scalars[nb*i + b];
	}
}

float* PRT::convertToScalars(int method, float scale)
{
#if USELOG
	gLogFile = fopen("c:/temp/prt.log", "a+");
#endif
	if (m_scalars == 0) {
		m_scalars	  = new float[vertsize*bands];
	}
	if (m_ranges == 0) {
		m_ranges	  = new float[bands*2];
	}
	if (m_scales == 0) {
		m_scales	  = new float[bands];
	}
	float* op = m_scalars;
	RGBCoeff* cp = coeffs;
	//1: convert to scalar and compute ranges for each term:
	for (int b=0; b < bands; b++) {
		m_ranges[2*b+0] = FLT_MAX;
		m_ranges[2*b+1] = -FLT_MAX;
	}
	const float* shranges = getSHRanges( 100 );

	for (int i = 0; i < vertsize; ++i) {
		for (int b=0; b < bands; b++) {
			float val = cp->r;
			val *= scale;
			*op = val;
			m_ranges[2*b+0] = min( m_ranges[2*b+0], val );
			m_ranges[2*b+1] = max( m_ranges[2*b+1], val );
			++cp;
			++op;
		}
	}
	//2: compute overall range:
	float s0 = FLT_MAX, s1 = -FLT_MAX;
	for (int b=0; b < bands; b++) {
		s0 = min( s0, m_ranges[2*b+0] );
		s1 = max( s1, m_ranges[2*b+1] );
	}
	m_min = s0;	m_max = s1;

	dumpPackMethod( method );
	// pack into runtime formats:
	packScalars( method );

	if (method != -1) {
		assert( m_quanKind != kNull );
	}
	dumpQuantizationKind( m_quanKind );

	float* unpacked = 0, *unpacked2 = 0;

	switch (method) {
		case 0:
			unpacked = unpackScalarsRC72();
		break;
		case 1:
			unpacked = unpackScalarsRC72_9();
		break;
		case 2:
			unpacked = unpackScalarsRC48(m_scalarRC48);
		break;
		case 3:		//Lloyd-Max optimized 48 bit RC
			unpacked = unpackScalarsLM48(m_scalarLM);
		break;
		case 4:
			unpacked = unpackScalarsRC32();
			unpacked2 = unpackScalarsRC32SIMD(unpacked);
		break;
	}
	if (unpacked != 0) {
		STAT::AllStats stats;
		GetAllStats( &stats, m_scalars, vertsize*9, unpacked );
//		printf("Overall: "); PrintStats( stats, stdout );

		collectStats( unpacked );

		delete [] unpacked;
	}
#if USELOG
	fclose(gLogFile);
#endif
	return m_scalars;	//scalars;
}


void PRT::packScalars( int method )
{
	if (m_scalars == 0)
		return;

	switch (method) {
		case 0:		// 1 scale-bias => pack into byte
			packScalarsRC72(kMidRise);		//results in 'm_scalarBytes'
		break;
		case 1:		// 9 scale-bias pairs => pack into byte
			packScalarsRC72_9(kMidRise);	//results in 'm_scalarBytes'
		break;
		case 2:		//48 bit uniform quantized variable bit fields
			if (m_scalarRC48 == 0) {
				m_scalarRC48 = new Byte[vertsize*6];		//use 48bit range code
			}
			packScalarsRC48(m_scalarRC48, m_scalars);
		break;
		case 3:		//Lloyd-Max quantizer
			//Note: currently the shader is only use the LM for p[5..8], the rest is still using regular 48b RC
			if (m_scalarRC48 == 0) {
				m_scalarRC48 = new Byte[vertsize*6];		//use 48bit range code
			}
			packScalarsRC48(m_scalarRC48, m_scalars);
			packScalarsLM48();		//results in 'm_scalarLM'
		break;
		case 4:
			if (m_scalarRC32 == 0) {
				m_scalarRC32 = new Byte[vertsize*4];		//use 32bit range code
			}
			packScalarsRC32(m_scalarRC32);
		break;
		default:
			printf("Unknown packing method %d\n", method);
		break;
	}
}

static void copy1set( float* out, const float* input, int len, int bands )
{
	const float* end = input + len*bands;
	while (input != end) {
		*out = *input;
		input += bands;
		++out;
	}
}

void PRT::collectStats( const float* unpacked )
{
	float* reference = new float[vertsize];
	float* unpack1   = new float[vertsize];
	float* energies  = (float*)_alloca(sizeof(float)*bands);
	float* ranges	 = (float*)_alloca(sizeof(float)*bands*2);
	STAT::AllStats* stats = (STAT::AllStats*)_alloca(sizeof(STAT::AllStats)*bands);

	for (int b=0; b < bands; b++) {
		STAT::AllStats* stat = stats + b;
		copy1set( reference, m_scalars+b, vertsize, bands );
		copy1set( unpack1,   unpacked+b,  vertsize, bands );

		GetAllStats( stat, reference, vertsize, unpack1 );
		STAT::Range( ranges + 2*b, unpack1, vertsize );
		energies[b] = stat->m_totalEngr;
	}
	float totalEnergy = 0.f;
	for (int b=0; b < bands; b++) {
		totalEnergy += energies[b];
	}
	for (int b=0; b < bands; b++) {
		STAT::AllStats* stat = stats + b;
		stat->m_percentEngr = energies[b]*100/totalEnergy;
	}
	for (int b=0; b < bands; b++) {
		printf("band[%d]: ",b);
		printf("(%2.3f,%2.3f) ", ranges[2*b+0], ranges[2*b+1]);
//		PrintStats( *(stats + b), stdout );
	}
	delete [] reference;
	delete [] unpack1;
}

void PRT::dumpPackMethod( int method )
{
	const char* methodStr[] = {
		"unpacked floats",								//-prt -1
		"9 bytes range coded with shared scale-bias",	//-prt 0
		"9 bytes range coded with custom scale-bias",	//-prt 1
		"48 bits (8,6,6,6,6)+(4,4,4,4) range coded",	//-prt 2
		"48 bits range coded with Lloyd-Max",			//-prt 3
		"32 bits (5,4,4,4,3,3,3,3,3) range coded",		//-prt 4
	};
	const char* prtstr = (method <= 4) ? methodStr[method+1] : "unknown";
	_cprintf("PRT compression method '%s' =>\n", prtstr);
}

void PRT::dumpQuantizationKind( QKind qkind )
{
	const char* kindStr[] = {
		"null",
		"mid-rise quantizer",							//kMidRise
		"mid-thread quantizer",							//kMidThread
	};
	const char* kndstr = (qkind <= 2) ? kindStr[qkind] : "unknown";
	_cprintf("PRT quantization method '%s'.\n", kndstr);
}

//scale factors to map 'range' to 0..<1 << bits - 1>
void PRT::computeScales(Byte bitsBudgets[], QKind qkind )
{
	if (m_recon == 0) {
		m_recon = new float[bands];
	}
	for (int b=0; b < bands; b++) {
		float range = m_ranges[2*b+1] - m_ranges[2*b+0];
		float scale = (float)(1 << bitsBudgets[b]);
		if (qkind == kMidRise)
			scale -= 1.f;		//odd# of levels
		scale /= range;
		m_scales[b] = scale;	//scale factor to map 'range' to 0..<1 << bits - 1>
		m_recon[b]  = m_ranges[2*b+0];
		
		if (qkind == kMidThread) {	//handle midThread
			float binw = 1.f/scale;
			m_recon[b] += binw/2;
		}
	}
	m_quanKind = qkind;
}

void PRT::updateScalar( int b, const double* quantized )
{
	float* sp = &m_scalars[b];
	const double* qp = quantized;
	for (int i = 0; i < vertsize; ++i) {
		*sp = (float)*quantized++;
		sp += bands;
	}
}

void PRT::allocCodeBook( Byte bitbudgets[] )
{
	for (int b=0; b < bands; b++) {
		int bitbudget = bitbudgets[b];
		m_codebook[b] = new CodeBook1D(bitbudget);
	}
}

// binds the PRT scalar coefficients using our 9 byte uniform quantized range-coded shader:
void PRT::packScalarsRC72(QKind qkind)
{
	Byte BitsBudgets[] = { 8, 8, 8, 8, 8, 8, 8, 8, 8 };
	computeScales(BitsBudgets, qkind);

	if (m_scalarBytes == 0) {
		m_scalarBytes = new Byte[vertsize*bands];
		float s0 = m_min, s1 = m_max;

		float valueRange = s1 - s0;
		m_scale = 255.f/valueRange;
		unsigned char* ob = m_scalarBytes;
		float* op = m_scalars;
		for (int i = 0; i < vertsize; ++i) {
			for (int b=0; b < bands; b++) {
				*ob = MATH::round((*op - s0) * m_scale);
				++ob;
				++op;
			}
		}
		m_quanKind = qkind;
	}
}

float* PRT::unpackScalarsRC72()
{
	float* scalars = new float[vertsize*bands];
	float scale = 1.f/m_scale;
	const unsigned char* ob = m_scalarBytes;
	float* op = scalars;
	for (int i = 0; i < vertsize*bands; ++i) {
		*op = m_min + *ob * scale;
		++ob;
		++op;
	}
	for (int b=0; b < bands; b++) {
		printf("Band[%d]: min = %1.3f, max =  %1.3f\n", b, m_ranges[2*b], m_ranges[2*b+1]); 
	}
	return scalars;
}

// binds the PRT scalar coefficients using our 9 byte uniform quantized range-coded with individual scale-bias:
void PRT::packScalarsRC72_9(QKind qkind)
{
	Byte BitsBudgets[] = { 8, 8, 8, 8, 8, 8, 8, 8, 8 };
	computeScales(BitsBudgets, qkind);
	
	if (m_scalarBytes == 0) {
		m_scalarBytes = new Byte[vertsize*bands];

		unsigned char* ob = m_scalarBytes;
		float* op = m_scalars;
		for (int i = 0; i < vertsize; ++i) {
			for (int b=0; b < bands; b++) {
				float bias = m_ranges[2*b+0];		//bias
				float valueRange = (m_ranges[2*b+1] - bias);
				float scale		 = m_scales[b];
				*ob = MATH::round((*op - bias) * scale);
				++ob;
				++op;
			}
		}
		for (int b=0; b < bands; b++) {
			printf("Band[%d]: min = %1.3f, max =  %1.3f\n", b, m_ranges[2*b], m_ranges[2*b+1]); 
		}
	}
}

float* PRT::unpackScalarsRC72_9()
{
	float* scalars = new float[vertsize*bands];
	float scale = 1.f/m_scale;
	const unsigned char* ob = m_scalarBytes;
	float* op = scalars;
	for (int i = 0; i < vertsize; ++i) {
		for (int b=0; b < bands; b++) {
			float bias  = m_ranges[2*b+0];		//bias
			float scale	= 1/m_scales[b];
			*op = bias + *ob * scale;
			++ob;
			++op;
		}
	}
	return scalars;
}

//pack register 0 for RC48
void inline PackR0( U32& r0, const float* scalars, const float* scales, const float* ranges, QKind qkind )
{
	U32   s[5];

	ScalarQuant quant( qkind, 5, scales, ranges );
	quant.Quantize( s, 5, scalars );

	r0 =  (s[0] << 24) & 0xff000000;
	r0 |= (s[1] << 18) & 0x00fc0000;
	r0 |= (s[2] << 12) & 0x0003f000;
	r0 |= (s[3] <<  6) & 0x00000fc0;
	r0 |= (s[4] <<  0) & 0x0000003f;
}

//pack register 1 for RC48
void inline PackR1( U32& r1, const float* scalars, const float* scales, const float* ranges, QKind qkind )
{
	U32   s[4];

	ScalarQuant quant( qkind, 4, scales, ranges );
	quant.Quantize( s, 4, scalars );

	r1 =  (s[0] << 28) & 0xf0000000;
	r1 |= (s[1] << 24) & 0x0f000000;
	r1 |= (s[2] << 20) & 0x00f00000;
	r1 |= (s[3] << 16) & 0x000f0000;
}

// binds the PRT scalar coefficients using our 48 bit uniform quantized range-coded shader:
void PRT::packScalarsRC48(Byte* orc, const float* scalars)
{
	QKind qkind = kMidRise;	//kMidThread; kMidRise
	Byte BitsBudgets[] = { 8, 6, 6, 6, 6, 4, 4, 4, 4 };
	computeScales(BitsBudgets, qkind);

	const float* is = scalars;		//input: scalars
	for (int i = 0; i < vertsize; ++i) {
		U32 r0 = 0, r1 = 0;
		PackR0( r0, is, m_scales, m_ranges, qkind );
		orc[0] = (r0 >> 24) & 0xff;
		orc[1] = (r0 >> 16) & 0xff;
		orc[2] = (r0 >>  8) & 0xff;
		orc[3] = (r0 >>  0) & 0xff;
		orc += 4;
		PackR1( r1, is+5, m_scales+5, m_ranges + 2*5, qkind );
		orc[0] = (r1 >> 24) & 0xff;
		orc[1] = (r1 >> 16) & 0xff;
		orc += 2;

		is += bands;
	}
}

void inline Pack1RC32( U32& r0, int bands, const float* scalars, const float* scales, const float* ranges, QKind qkind )
{
	U32	s[9];
	U32	r;

	ScalarQuant quant( qkind, 9, scales, ranges );
	quant.Quantize( s, bands, scalars );

	r =  (s[0] << 27) & 0xf8000000;		//prt[0] - 5 bits (b31..b27)
	r |= (s[1] << 23) & 0x07800000;		//prt[1] - 4 bits (b26..b23)
	r |= (s[2] << 19) & 0x00780000;		//prt[2] - 4 bits (b22..b19)
	r |= (s[3] << 15) & 0x00078000;		//prt[3] - 4 bits (b18..b15)
	r |= (s[4] << 12) & 0x00007000;		//prt[4] - 3 bits (b14..b12)
	r |= (s[5] <<  9) & 0x00000e00;		//prt[5] - 3 bits (b11..b09)
	r |= (s[6] <<  6) & 0x000001c0;		//prt[6] - 3 bits (b08..b06)
	r |= (s[7] <<  3) & 0x00000038;		//prt[7] - 3 bits (b05..b03)
	r |= (s[8] <<  0) & 0x00000007;		//prt[8] - 3 bits (b02..b00)

	r0 = r;
}
// binds the PRT scalar coefficients using our 48 bit uniform quantized range-coded shader:
void PRT::packScalarsRC32(Byte* orc)
{
	QKind qkind = kMidThread;	//kMidThread kMidRise;
	Byte BitsBudgets[] = { 5, 4, 4, 4, 3, 3, 3, 3, 3 };
	computeScales(BitsBudgets, qkind);
	
	const float* is = m_scalars;		//input: scalars
	for (int i = 0; i < vertsize; ++i) {
		U32 r0 = 0;
		Pack1RC32( r0, bands, is, m_scales, m_ranges, qkind );
		orc[0] = (r0 >> 24) & 0xff;
		orc[1] = (r0 >> 16) & 0xff;
		orc[2] = (r0 >>  8) & 0xff;
		orc[3] = (r0 >>  0) & 0xff;
		orc += 4;

		is += bands;
	}
}

//simulates the 4 byte to 4 float conversion in shader
void inline fourB2F( float r0[4], const Byte* ob )
{
	r0[0] = (float)ob[0];
	r0[1] = (float)ob[1];
	r0[2] = (float)ob[2];
	r0[3] = (float)ob[3];
}

float* PRT::unpackScalarsRC32()
{
	float* scalars = new float[vertsize*bands];
	float* recon = (float*)_alloca(sizeof(float)*bands);
	const Byte* ob = m_scalarRC32;
	float* op = scalars;
	//0: initialize our reconstruction levels:
	for (int b=0; b < bands; b++) {
		recon[b] = m_recon[b];		//m_ranges[2*b];	
	}
	//here we 1st use bit ops to simulate the logic inside a shader. 
	//we purposely do not use the convenience of 32 bit ops. Then
	//we use floating ops to perform the same operation.
	for (int i = 0; i < vertsize; ++i) {
		float r0[4];	fourB2F( r0, ob );	//simulates byte->float4 conversion
		float t, tb, lhs, rhs;
		
		//p[0]: bit ops
		U32 b0 = ob[0];
		b0 >>= 3;			//right shift 3 bits
		tb = (float)b0/m_scales[0];
		tb += recon[0];
		//p[0]: float ops
		t = r0[0]/8.f;		//right shift 3 bits
		lhs = fract(t);		//lhs for next field
		t = floorf(t)/m_scales[0];
		t += recon[0];
		op[0] = t;

		//p[1]: bit ops
		U32 b1 = (ob[0] & 0x7) << 1;		//extract low 3b + high 1b
		b1 += (ob[1] >> 7);
		tb = (float)b1/m_scales[1]; 
		tb += recon[1];
		//p[1]: float ops
		lhs *= 2.f * 8.f;		//left shift 1 + the right shift above
		t = r0[1]/128.f;		//right shift 7 bits
		rhs = fract(t);			//lhs for next field
		t = (lhs + floorf(t))/m_scales[1];
		t += recon[1];
		op[1] = t;
		lhs = rhs;
		//p[2]: bit ops
		U32 b2 = (ob[1] & 0x78) >> 3;		//extract b6..3 
		tb = (float)b2/m_scales[2];
		tb += recon[2];
		//p[2]: float ops
		lhs *= 16.f;			//left shift 4 to get the high 4 bits of the shifted low 7 bits
		rhs = fract(lhs);		//lhs for next field
		t = (floorf(lhs))/m_scales[2];
		t += recon[2];
		op[2] = t;
		lhs = rhs;
		//p[3]: bit ops
		U32 b3 = (ob[1] & 0x7) << 1;	//extract b2..0 of ob[1]
		b3 += (ob[2] >> 7);				//get b7 of ob[2]
		tb = (float)b3/m_scales[3]; 
		tb += recon[3];
		//p[3]: float ops
		lhs *= 8.f * 2.f;				//left shift 3 to get the high 3 bits of the shifted low 7 bits
		t = r0[2]/128.f;		//right shift 7 bits
		rhs = fract(t);			//lhs for next field
		t = (lhs + floorf(t))/m_scales[3];
		t += recon[3];
		op[3] = t;
		lhs = rhs;
		//p[4]: bit ops
		U32 b4 = (ob[2] & 0x70) >> 4;	//extract b6..4 of ob[1]
		tb = (float)b4/m_scales[4]; 
		tb += recon[4];
		//p[4]: float ops
		lhs *= 8.f;				//left shift 3 to get the high 3 bits of the shifted
		rhs = fract(lhs);		//lhs for next field
		t = (floorf(lhs))/m_scales[4];
		t += recon[4];
		op[4] = t;
		lhs = rhs;
		//p[5]: bit ops
		U32 b5 = (ob[2] & 0xe) >> 1;	//extract b3..1 of ob[1]
		tb = (float)b5/m_scales[5]; 
		tb += recon[5];
		//p[5]: float ops
		lhs *= 8.f;				//left shift 3 to get the high 3 bits of the shifted
		rhs = fract(lhs);		//lhs for next field
		t = (floorf(lhs))/m_scales[5];
		t += recon[5];
		op[5] = t;
		lhs = rhs;
		//p[6]: bit ops
		U32 b6 = (ob[2] & 0x1) << 2;	//extract b0 of ob[2]
		b6 += (ob[3] >> 6) & 0x3;		//extract high 2 bits of ob[3]
		tb = (float)b6/m_scales[6]; 
		tb += recon[6];
		//p[6]: float ops
		lhs *= 2.f * 4.f;		//left shift 1 to get the high 1 bits of the shifted
		t = r0[3]/64.f;			//right shift 6 bits to get the high 2 bits
		rhs = fract(t);			//lhs for next field
		t = (lhs + floorf(t))/m_scales[6];
		t += recon[6];
		op[6] = t;
		lhs = rhs;
		//p[7]: float ops
		lhs *= 8.f;				//left shift 8 to get the high 8 bits of the shifted
		rhs = fract(lhs);		//lhs for next field
		t = (floorf(lhs))/m_scales[7];
		t += recon[7];
		op[7] = t;
		lhs = rhs;
		//p[8]: float ops
		lhs *= 8.f;				//left shift 8 to get the high 8 bits of the shifted
		t = (floorf(lhs))/m_scales[8];
		t += recon[8];
		op[8] = t;

		op += bands;
		ob += 4;
	}
	return scalars;
}

static void fourB2V4( noVec4& v4, const Byte* ob )
{
	v4.x = (float)ob[0];
	v4.y = (float)ob[1];
	v4.z = (float)ob[2];
	v4.w = (float)ob[3];
}

static bool Check( const float* reference, const float* pp0, int nf )
{
	for (int i=0; i < nf; i++) {
		if (fabs(pp0[i] - reference[i]) > 0.000001f) {
			assert(0);
			return false;
		}
	}
	return true;
}


float* PRT::unpackScalarsRC32SIMD(const float* reference)
{
	float* scalars = new float[vertsize*bands];
	float* recon = (float*)_alloca(sizeof(float)*bands);
	const Byte* ob = m_scalarRC32;
	float* op = scalars;
	//0: initialize our reconstruction levels:
	for (int b=0; b < bands; b++) {
		recon[b] = m_recon[b];		//m_ranges[2*b];
	}
	//rshft0/lshft0 is used to decode p0..p3
	noVec4 rshft0(   1/8.f, 1/128.f, 1/8.f, 1/128.f );		//prt0.xyyz * rshft0
	noVec4 lshft0( 8.f*2.f,    16.f,  16.f,     2.f );
	//rshft1/lshft1 is used to decode p6..p8
	noVec4 rshft1(   1/2.f,  1/64.f,  1/64.f,  1/8.f );			//prt0.zwww * rshft1
	noVec4 lshft1( 4.f*2.f,     1.f,     8.f,   8.f );
	//			  p6(2)    p6(1..0) p7(2..0) p8(2..0)
	noVec4 scale0 = noVec4(1.f/m_scales[0],1.f/m_scales[1],1.f/m_scales[2],1.f/m_scales[3]);
	noVec4 scale1 = noVec4(1.f/m_scales[4],1.f/m_scales[5],0,0);
	noVec4 scale2 = noVec4(1.f/m_scales[5],1.f/m_scales[6],1.f/m_scales[7],1.f/m_scales[8]);
	noVec4 bias0  = noVec4(recon[0],recon[1],recon[2],recon[3]);
	noVec4 bias1  = noVec4(recon[4],recon[5],0,0);
	noVec4 bias2  = noVec4(recon[5],recon[6],recon[7],recon[8]);

	for (int i = 0; i < vertsize; ++i) {
		noVec4 prt0;	fourB2V4( prt0, ob );	//simulates byte->float4 conversion
		float t;

		noVec4 p03;	//p[0..3]
		noVec4 lhs3;
		
		p03 = xyyz(prt0).Multiply(rshft0);	// rshft(1/8.f,1/8.f,0,0) for now we are only interested in .xy
									//.x: p0.p1(3..1), .y: p1(0).p2(3..0)+p3(3..1), .z: p3(0).p4(2..0)+p5(2..0)+p6(3)

		lhs3.x = fract(p03.x);	//.x: p1(3..1)
		lhs3.y = fract(p03.y);	//.y: p2(3..0) + p3(3..1)
		lhs3.w = fract(p03.w);	//.w: p4(2..0)
		//lhs3 = lhs3 * lshft0;			// lshft(        0, 16.*4., 4.*16., 64.);
			
		//p[0..3]:
		noVec4 pp0;
		pp0.x = floor(p03.x);		//p0(4..0)
		pp0.y = lhs3.x * 8.f*2.f;	//p1(3..1) compensate rshft + shift to correct significant place
		pp0.y += floor(p03.y);		//p1(0)
		lhs3.y *= 16.f;
		pp0.z = floor(lhs3.y);		//p2(3..0)
		t = fract(lhs3.y) * 8.f*2.f;	//p3(3..1)
		t += floor(p03.w);			//p3(0)
		pp0.w = t;					//p3(3..0)

		noVec4 p0 = pp0.Multiply(scale0) + bias0;
		noVec4 p4, p58;	//p[4], p[5..8]

		//p[4..5]
		lhs3.w *= 8.f;
		p4.x = floor(lhs3.w);
		p4.x = p4.x * scale1.x + bias1.x;
		p4.y = fract(lhs3.w) * 8.f;		//p5(2..0)
		p4.y = floor(p4.y);
		p4.y = p4.y * scale1.y + bias1.y;
		//p[6..8]:
		p58 = zwww(prt0).Multiply(rshft1);
		lhs3.x = fract(p58.x);	//.x: .p6(2)
		lhs3.y = (p58.y);	//.y: .p6(1..0)
		lhs3.z = fract(p58.z);	//.z: .p7(2..0)
		lhs3.w = fract(p58.w);	//.w: .p8(2..0)
		lhs3 = lhs3.Multiply(lshft1);
		lhs3.x = floor(lhs3.x);	//.x: .p6(2)
		lhs3.y = floor(lhs3.y);	//.y: .p6(1..0)
		lhs3.z = floor(lhs3.z);	//.z: .p7(2..0)
		lhs3.w = floor(lhs3.w);	//.w: .p8(2..0)
		lhs3.y += lhs3.x;		//p6
		p58 = lhs3 .Multiply(scale2) + bias2;

		Check( reference+0, &p0.x, 4 );
		Check( reference+4, &p4.x, 2 );
		Check( reference+6, &p58.y, 2 );
		op += bands;
		reference += bands;
		ob += 4;
	}
	return scalars;
}

//pack register 1 for LM48 (p[5..8]: 4,4,4,4)
void inline PackR1LM( U32& r1, const Byte* codes )
{
	r1 =  (codes[0] << 28) & 0xf0000000;
	r1 |= (codes[1] << 24) & 0x0f000000;
	r1 |= (codes[2] << 20) & 0x00f00000;
	r1 |= (codes[3] << 16) & 0x000f0000;
}


// override the last two bytes of each 48b code by our quantizer output
static void CopyCodes(Byte* orc, const U32* lmcodes, int len, int outstride)
{
	for (int i = 0; i < len; ++i) {
		orc[0] = *lmcodes;
		orc += outstride;
		++lmcodes;
	}
}

// binds the PRT scalar coefficients using our Lloyd-Max 48 bit uniform quantized range-coded shader:
void PRT::packScalarsLM48()
{
	if (m_scalarLM == 0) {
		m_scalarLM = new Byte[vertsize*6];		//use 48bit range code
		Byte bitbudgets[] = {8, 6,6,6,6, 4,4,4,4};
		allocCodeBook( bitbudgets );
		m_quanKind = kMidRise;

		LloydMax1D lmquant( 4 );
		double* prtsamples = new double[vertsize];
		Byte* outcodes = new Byte[vertsize*4];	

		//1: apply Lloyd-Max to L5..8
		lmquant.Init( 4 );
		for (int b=5; b <= 8; b++) {
			getSamples( prtsamples, b );
			const double* quantized = lmquant.Process( prtsamples, vertsize, 10 );
			lmquant.getCodeBook( m_codebook[b] );
			const U32* lmcodes = lmquant.getOutPutCodes();	//get the Lloyd-Max quantizer output codes
			CopyCodes( outcodes + (b-5), lmcodes, vertsize, 4 );
			//updateScalar( b, quantized );
			m_stats[b] = lmquant.m_stats;
		}
		//2: apply Lloyd-Max to L0
		lmquant.Init( 8 );
		getSamples( prtsamples, 0 );
		const double* quantized0 = lmquant.Process( prtsamples, vertsize, 10 );
		lmquant.getCodeBook( m_codebook[0] );
		//updateScalar( 0, quantized0 );
		m_stats[0] = lmquant.m_stats;

		//3: apply Lloyd-Max to L1..4
		lmquant.Init( 6 );
		for (int b=1; b <= 4; b++) {
			getSamples( prtsamples, b );
			const double* quantized = lmquant.Process( prtsamples, vertsize, 10 );
			lmquant.getCodeBook( m_codebook[b] );
			//updateScalar( b, quantized );
			m_stats[b] = lmquant.m_stats;
		}
		for (int b=0; b < 9; b++) {
			const STAT::AllStats& stat = m_stats[b];
			printf("Band[%d] - energy: %8.1f, MSE: %f, PSNR: %f\n", b, stat.m_totalEngr, stat.m_mse, stat.m_psnr );
		}
		// pack the quantized scalars into 48 bit form in 'm_scalarLM':
		packScalarsRC48( m_scalarLM, m_scalars );

		// override the last two bytes of each 48b code by our quantizer output
		Byte* orc = m_scalarLM + 4;			//1st 4 bytes are already handled by packScalarsRC48
		const Byte* outc = outcodes;
		for (int i = 0; i < vertsize; ++i) {
			U32 r1 = 0;
			PackR1LM( r1, outc );
			orc[0] = (r1 >> 24) & 0xff;
			orc[1] = (r1 >> 16) & 0xff;
			orc += 6;
			outc += 4;
		}
		delete [] outcodes;
		delete [] prtsamples;
	}
}

const Byte* PRT::getPackedSclars() 
{ 
	if (m_scalarBytes == 0) {
		packScalars(0);
	}
	return m_scalarBytes; 
}

void inline twoB2F( float r0[2], const Byte* ob )
{
	r0[0] = (float)ob[0];
	r0[1] = (float)ob[1];
}
float* PRT::unpackScalarsRC48(const Byte* ob)
{
	float* scalars = new float[vertsize*bands];
	//float scale = 1.f/m_scale;
	float* op = scalars;
	//here we 1st use bit ops to simulate the logic inside a shader. 
	//we purposely do not use the convenience of 32 bit ops. Then
	//we use floating ops to perform the same operation.
	for (int i = 0; i < vertsize; ++i) {
		float r0[4];	fourB2F( r0, ob );		//simulates byte->float4 conversion
		float r1[2];	twoB2F ( r1, ob+4 );	//simulates byte->float4 conversion
		float t, tb, lhs, rhs;
		
		op[0] = r0[0]/m_scales[0] + m_ranges[2*0];

		//p[1]: bit ops
		U32 b = ob[1];
		b >>= 2;			//right shift 2 bits
		tb = (float)b/m_scales[1];
		tb += m_ranges[2*1];
		//p[1]: float ops
		t = r0[1]/4.f;		//right shift 2 bits
		lhs = fract(t);		//lhs for next field
		t = floorf(t)/m_scales[1];
		t += m_ranges[2*1];
		op[1] = t;

		//p[2]: bit ops
		b = (ob[1] & 0x3) << 4;
		U32 rhsb = (ob[2] >> 4);
		rhsb += b;
		//p[2]: float ops
		lhs *= 16.f * 4.f;		//left shift 4 + the right shift above
		t = r0[2]/16.f;			//right shift 4 bits
		rhs = fract(t);
		t = (lhs + floorf(t))/m_scales[2];
		t += m_ranges[2*2];
		op[2] = t;
		lhs = rhs;
		//p[3]: bit ops
		b = (ob[2] & 0x0f) << 2;
		rhsb = (ob[3] >> 6);
		rhsb += b;
		//p[3]: float ops
		lhs *= 4.f * 16.f;		//left shift 2 + the right shift above
		t = r0[3]/64.f;			//right shift 2 bits
		rhs = fract(t);
		t = (lhs + floorf(t))/m_scales[3];
		t += m_ranges[2*3];
		op[3] = t;
		lhs = rhs;
		//p[4]: float ops
		lhs *= 64.f;		//left shift 0 + the right shift above
		t = lhs/m_scales[4];
		t += m_ranges[2*4];
		op[4] = t;

		//p[5]: float ops
		noVec4 r58(r1[0],r1[1],r1[0],r1[1]);
		
		r58 /= 16.f;
		t = floorf(r58.x)/m_scales[5];
		t +=  m_ranges[2*5];
		op[5] = t;
		t = floorf(r58.y)/m_scales[7];
		t +=  m_ranges[2*7];
		op[7] = t;

		r58.z = fract(r58.z) * 16.f;
		t = r58.z/m_scales[6];
		t +=  m_ranges[2*6];
		op[6] = t;
		r58.w = fract(r58.w) * 16.f;
		t = r58.w/m_scales[8];
		t +=  m_ranges[2*8];
		op[8] = t;

		op += 9;

		ob += 6;
	}
	return scalars;
}

float* PRT::unpackScalarsLM48(const Byte* ob)
{
	float* scalars = new float[vertsize*bands];
	//float scale = 1.f/m_scale;
	float* op = scalars;

	float codes[16*4];
	//get the reconstruction levels (codebook):
	for (int s=5; s < 9; s++) {
		const CodeBook1D& codebook = *m_codebook[s];
		CopyCodeBook( codes + (s-5), 64, codebook );
	}
	//here we 1st use bit ops to simulate the logic inside a shader. 
	//we purposely do not use the convenience of 32 bit ops. Then
	//we use floating ops to perform the same operation.
	for (int i = 0; i < vertsize; ++i) {
		float r0[4];	fourB2F( r0, ob );		//simulates byte->float4 conversion
		float r1[2];	twoB2F ( r1, ob+4 );	//simulates byte->float4 conversion
		float t, lhs, rhs;
		
		op[0] = r0[0]/m_scales[0] + m_ranges[2*0];

		//p[1]: float ops
		t = r0[1]/4.f;		//right shift 2 bits
		lhs = fract(t);		//lhs for next field
		t = floorf(t)/m_scales[1];
		t += m_ranges[2*1];
		op[1] = t;
		//p[2]: float ops
		lhs *= 16.f * 4.f;		//left shift 4 + the right shift above
		t = r0[2]/16.f;			//right shift 4 bits
		rhs = fract(t);
		t = (lhs + floorf(t))/m_scales[2];
		t += m_ranges[2*2];
		op[2] = t;
		lhs = rhs;
		//p[3]: float ops
		lhs *= 4.f * 16.f;		//left shift 2 + the right shift above
		t = r0[3]/64.f;			//right shift 2 bits
		rhs = fract(t);
		t = (lhs + floorf(t))/m_scales[3];
		t += m_ranges[2*3];
		op[3] = t;
		lhs = rhs;
		//p[4]: float ops
		lhs *= 64.f;		//left shift 0 + the right shift above
		t = lhs/m_scales[4];
		t += m_ranges[2*4];
		op[4] = t;

		//p[5]: float ops
		noVec4 r58(r1[0],r1[1],r1[0],r1[1]);		//r58 = r1.xyxy;
		U32 code;

		r58 /= 16.f;
		code = r58.x;
		t = codes[4*code + 0];		//recon[int(r58.x)].x;
		//t = floorf(r58.x)/m_scales[5];
		//t += m_ranges[2*5];
		op[5] = t;

		code = r58.y;
		t = codes[4*code + 2];		//recon[int(r58.y)].z;
		//t = floorf(r58.y)/m_scales[7];
		//t +=  m_ranges[2*7];
		op[7] = t;

		r58.z = fract(r58.z) * 16.f;
		code = r58.z;
		t = codes[4*code + 1];		//recon[int(r58.z)].y;
		//t = r58.z/m_scales[6];
		//t +=  m_ranges[2*6];
		op[6] = t;
		r58.w = fract(r58.w) * 16.f;
		code = r58.w;
		t = codes[4*code + 3];		//recon[int(r58.w)].w;
		//t = r58.w/m_scales[8];
		//t +=  m_ranges[2*8];
		op[8] = t;

		op += 9;

		ob += 6;
	}
	return scalars;
}
	// binds the uniform constants for our 9 byte uniform quantized range-coded with individual scale-bias:
//int	PRT::bindVertexShaderRC72_9( Shader* vshader )
//{
//	float scale0[4] = {1.f/m_scales[0],1.f/m_scales[1],1.f/m_scales[2],0.};
//	float scale1[4] = {1.f/m_scales[3],1.f/m_scales[4],1.f/m_scales[5],0.};
//	float scale2[4] = {1.f/m_scales[6],1.f/m_scales[7],1.f/m_scales[8],0.};
//	float bias0[4]  = {m_ranges[2*0],m_ranges[2*1],m_ranges[2*2],0};
//	float bias1[4]  = {m_ranges[2*3],m_ranges[2*4],m_ranges[2*5],0};
//	float bias2[4]  = {m_ranges[2*6],m_ranges[2*7],m_ranges[2*8],0};
//
//	vshader->SetUniformVec3( "scale0", scale0, 3 );
//	vshader->SetUniformVec3( "scale1", scale1, 3 );
//	vshader->SetUniformVec3( "scale2", scale2, 3 );
//	vshader->SetUniformVec3( "bias0",  bias0,  3 );
//	vshader->SetUniformVec3( "bias1",  bias1,  3 );
//	vshader->SetUniformVec3( "bias2",  bias2,  3 );
//
//	return 0;
//}

// binds the uniform constants for our 48 bit range-coded shader:
//int	PRT::bindVertexShaderRC48( Shader* vshader )
//{
//	/*if (m_scalarRC48)*/ {
//		float scales[9], ranges[9];
//		//compute the scale and bias used for decompress
//		for (int s=0; s < 9; s++) {
//			scales[s] = 1.f/m_scales[s];
//			//ranges[s] = m_ranges[2*s];		//bias
//			ranges[s] = m_recon[s];
//			{
//				//ranges[s] += scales[s]/2;		//mid-point of bin
//			}
//		}
//		//SIMD version
//		//set up the constants for decompress:
//		vshader->SetUniformLocation( "scale0", scales+0, 1 );
//		vshader->SetUniformnoVec4	   ( "scales", scales+1, 4 );
//		vshader->SetUniformnoVec4	   ( "scales2", scales+5, 4 );
//		vshader->SetUniformLocation( "bias0", ranges+0, 1 );
//		vshader->SetUniformnoVec4	   ( "bias", ranges+1, 4 );
//		vshader->SetUniformnoVec4	   ( "bias2", ranges+5, 4 );
//
//		noVec4 rshft;		//for right shift to extract low bits of each term
//		noVec4 lshft;		//for left shift to extract high order bits
//
//		rshft = noVec4( scales[0], 1/4.f, 1/16.f, 1/64.f );
//
//		lshft = noVec4(         0, 
//									16.*4.,				//left shift 4 + the right shift above
//											4.*16.,		//left shift 2 + the right shift above
//													64.	//left shift 0 + the right shift above
//						);
//		vshader->SetUniformnoVec4( "rshft", &rshft.x, 4 );
//		vshader->SetUniformnoVec4( "lshft", &lshft.x, 4 );
//
//		noVec4 shft4(   1/16.f,  16.f,  0.f,  0.f );				//1/16, 16, 0, 0
//		vshader->SetUniformnoVec4( "shft4", &shft4.x, 4 );
//	}
//	return 0;
//}
//
//// binds the uniform constants for our 32 bit range-coded shader:
//int PRT::bindVertexShaderRC32( Shader* vshader )
//{
//	if (m_scalarRC32) {
//		float scales[9], ranges[9];
//		//compute the scale and bias used for decompress
//		for (int s=0; s < 9; s++) {
//			scales[s] = 1.f/m_scales[s];
//			//ranges[s] = m_ranges[2*s];		//bias
//			ranges[s] = m_recon[s];
//			//reconstruction levels - mid-point of each bin
//			{
//				//ranges[s] += scales[s]/2;
//			}
//		}
//		float scale0[4] = {1.f/m_scales[0],1.f/m_scales[1],1.f/m_scales[2],1.f/m_scales[3]};
//		float scale1[4] = {1.f/m_scales[4],1.f/m_scales[5],0,0};
//		float scale2[4] = {1.f/m_scales[5],1.f/m_scales[6],1.f/m_scales[7],1.f/m_scales[8]};
//		float bias0[4]  = {ranges[2*0],ranges[2*1],ranges[2*2],ranges[2*3]};
//		float bias1[4]  = {ranges[2*4],ranges[2*5],0,0};
//		float bias2[4]  = {ranges[2*5],ranges[2*6],ranges[2*7],ranges[2*8]};
//
//		//set up the constants for decompress:
//		vshader->SetUniformnoVec4	  ( "scale0", scale0, 4 );
//		vshader->SetUniformnoVec4	  ( "scale1", scale1, 4 );
//		vshader->SetUniformnoVec4	  ( "scale2", scale2, 4 );
//		vshader->SetUniformnoVec4	  ( "bias0",  bias0, 4 );
//		vshader->SetUniformnoVec4	  ( "bias1",   bias1, 4 );
//		vshader->SetUniformnoVec4	  ( "bias2",  bias2, 4 );
//
//		//rshft0/lshft0 is used to decode p0..p3
//		noVec4 rshft0;		//for right shift to extract low bits of each term
//		noVec4 lshft0;		//for left shift to extract high order bits
//
//		rshft0 = noVec4(   1/8.f,   1/128.f,  1/8.f,  1/128.f );			//prt0.xyyz * rshft0
//		lshft0 = noVec4( 8.f*2.f,      16.f,   16.f,     2.f );
//		noVec4 lshft01 ( 8.f*2.f,      16.f,    8.f, 0 );
//
//		//rshft1/lshft1 is used to decode p6..p8
//		noVec4 rshft1(   1/2.f,  1/64.f,  1/64.f,  1/8.f );				//prt0.zwww * rshft1
//		noVec4 lshft1( 4.f*2.f,     1.f,     8.f,   8.f );
//		//			     p6(2)    p6(1..0) p7(2..0) p8(2..0)
//
//		vshader->SetUniformnoVec4( "rshft0", &rshft0.x, 4 );
//		vshader->SetUniformnoVec4( "lshft0", &lshft0.x, 4 );
//		vshader->SetUniformnoVec4( "lshft01", &lshft01.x, 4 );
//		vshader->SetUniformnoVec4( "rshft1", &rshft1.x, 4 );
//		vshader->SetUniformnoVec4( "lshft1", &lshft1.x, 4 );
//	}
//	return 0;
//}
//
//// binds the uniform constants for our Lloyd-Max 48 bit range-coded shader:
//int	PRT::bindVertexShaderLM48( Shader* vshader )
//{
//	//temporary build this upon the RC48 - the shifting factors are the same
//	bindVertexShaderRC48(vshader);
//
//	if (m_scalarLM) {
//		float codes[16*4];
//		//get the reconstruction levels (codebook):
//		for (int s=5; s < 9; s++) {
//			const CodeBook1D& codebook = *m_codebook[s];
//			CopyCodeBook( codes + (s-5), 64, codebook );
//		}
//		//set up the constants for decompress:
//		vshader->SetUniformnoVec4Array( "recon58", codes, 16*4 );
//	}
//	return 0;
//}
//
///// bind the locations within 'vshader' for our coefficients:
//int	PRT::bindVertexShader( Shader* vshader, int packmethod )
//{
//	if (vshader) {
//		//ideally we should get the list of uniforms from the shader source/compiled program itself
//		vshader->BindAttribLocation( 0, "prt0" );		//prt coefficients 0..3
//		vshader->BindAttribLocation( 1, "prt1" );		//prt coefficients 4..7
//		vshader->BindAttribLocation( 2, "prt2" );		//prt coefficent 8
//
//		switch (packmethod) {
//			case 0:
//			{
//				float scale[4] = {1.f/m_scale,1.f/m_scale,1.f/m_scale,1.f/m_scale};
//				float bias[4]  = {m_min,m_min,m_min,m_min};
//				vshader->SetUniformnoVec4( "scale", scale, 4 );
//				vshader->SetUniformnoVec4( "bias",  bias, 4 );
//			}
//			break;
//			case 1:
//			{
//				return bindVertexShaderRC72_9(vshader);
//			}
//			break;
//			case 2:		//uniform 48 bit range-coded
//				return bindVertexShaderRC48(vshader);
//			break;
//			case 3:		//Lloyd-Max 48 bit range-coded
//				return bindVertexShaderLM48(vshader);
//			break;
//			case 4:
//				return bindVertexShaderRC32(vshader);
//			break;
//			default:
//				printf("**Error: unknown PRT pack method %d\n", packmethod);
//			break;
//		}
//	}
//	return 0;
//}
//
////unpacked => 9 floats
//void PRT::RenderGL(const GLContext& context)
//{
//	const Shader* vshader = context.m_vShader;		// vertex shader
//	if (vshader) {
//		GLuint loc0 = vshader->m_attribLocs[0];	//( "prt0" );
//		glEnableVertexAttribArray( loc0 );
//		glVertexAttribPointer( loc0, 3, GL_FLOAT, false, sizeof(float)*9, m_scalars);
//		GLuint loc1 = vshader->m_attribLocs[1];	//( "prt1" );
//		glEnableVertexAttribArray( loc1 );
//		glVertexAttribPointer( loc1, 3, GL_FLOAT, false, sizeof(float)*9, m_scalars + 3);
//		GLuint loc2 = vshader->m_attribLocs[2];	//( "prt2" );
//		glEnableVertexAttribArray( loc2 );
//		glVertexAttribPointer( loc2, 3, GL_FLOAT, false, sizeof(float)*9, m_scalars + 6);
//	}
//}

float** CopySampleLocations( const Spheresample* samples, int numsamples )
{
	float** angle = new float*[numsamples];
	const Spheresample* sp = samples;
	for(int i=0; i < numsamples; i++) {
		float* ang = angle[i] = new float[2];
		ang[0] = sp->theta;
		ang[1] = sp->phi;
		++sp;
	}
	return angle;
}

//compute the ranges for the SH coefficients. Place results into 'm_ranges2'
float* PRT::getSHRanges(int numsamples)
{
	//generate numsamples samples on sphere
	Spheresample* samples = new Spheresample[numsamples];
	float* pdfs = new float[numsamples];	//array of pdfs, 1 per sample
	float normfactor;

	GenerateSphericalSamples( samples, pdfs, numsamples, kOffset, &normfactor );
	float* shcoeffs = calcSH(bands, numsamples, samples, true, m_ranges2);

	printf("SH ranges:\n");
	float scale = 1.f/(4* noMath::PI);
	for (int b=0; b < bands; b++) {
		printf(" Band[%d]: min = %1.3f, max =  %1.3f", b, m_ranges2[2*b]*scale, m_ranges2[2*b+1]*scale);
		printf("\n");
	}
	delete [] samples;
	delete [] pdfs;
	delete [] shcoeffs;

	return m_ranges2;
}

void PRT::dumpStats()
{
	printf("PRT ranges:\n");
	for (int b=0; b < bands; b++) {
		printf(" Band[%d]: min = %1.3f, max =  %1.3f", b, m_ranges[2*b], m_ranges[2*b+1]);
		printf("\n");
	}
}

/// round up to next power of 2:
U32 RoundUpPow2(U32 v) {
    v--;
    v |= (v >> 1);
    v |= (v >> 2);
    v |= (v >> 4);
    v |= (v >> 8);
    v |= (v >> 16);
    return v+1;
}

//almost .5f = .5f - 1e^(number of exp bit)
const double kDoubleMagicRoundEps = (.5-1.4e-11);

/// fast round of double 'val' to int:
inline int Round2Int(double val) {
    return int (val+kDoubleMagicRoundEps);
}

/// round up to next square:
inline U32 RoundUp2Sq(U32 v) {
    float r = sqrtf((float)v);
    int   ri = Round2Int(r);
    return (ri*ri);
}

void PRT::projectShadow( const GameScene& thescene, int numsamples, Mesh* mesh, U32 crc )
{
	Ray shadowray;
	

	HitInfo minhit;
	float H; //cos term as in Sloan/Kautz/Snyder
	float grayness; //the visibility convoluted with brdf (V*H) for a sample
	float* shcoeffs; //all coeffs for all samples (numbands * numsamples)
	noVec3 basis[3]; //basis vectors at vertex
	numsamples = RoundUp2Sq(numsamples);

	//numsamples = 900;
	//initialize all coeffs to 0 to begin
	reset();
	//generate numsamples samples on sphere
	Spheresample* samples = new Spheresample[numsamples];
	float* pdfs = new float[numsamples];	//array of pdfs, 1 per sample
	float * yR  = new float[numsamples];	//the Vs for LS fit
	float normfactor;

	GenerateSphericalSamples( samples, pdfs, numsamples, kOffset, &normfactor );

	float** angle = CopySampleLocations( samples, numsamples );

	kOffset += numsamples;
	kOffset %= 1217;	//97;
	shcoeffs = calcSH(bands, numsamples, samples, true, m_ranges2); //precompute all the SH-coeffs away

	M2Mesh* m2mesh = (M2Mesh*)pMesh;

	for (int i = 0; i < vertsize; ++i) {
		//shoot x sample shadow rays
		//convolute with brdf (H)
		//project sample (V*H) onto SH basis
		//sum sample contribution into coeffs[i*bands]
		//Divide each coefficient by number of samples and multiply by weights
		//const PosNrm& aposnrm = posnrms[i];
		noVec3 pos = m2mesh->m2_->vertDatas[i].pos;
		noVec3 norm = m2mesh->m2_->vertDatas[i].normal;

		basis[0] = norm;
		OrthoBasis(basis[0], basis[1], basis[2]); //create basis around vertex
		RGBCoeff* coeffentry = getCoeffList(i);
		shadowray.o = pos;	//ray origin is at unique vertex
		shadowray.o += norm * 0.0001f;
		for (int j = 0; j < numsamples; ++j) {
			shadowray.nextID(); //increment rayID for mailboxing
			noVec3 thesample = noVec3(samples[j].x, samples[j].y, samples[j].z); //sample in world space
			//generate one uniform sample on the hemisphere
			float u1 = rand() / (float)RAND_MAX;
			int offset = kOffset + i;
			u1 = VanDerCorput(offset, u1);
			float u2 = (i + 0.5f)/numsamples;	//Sobol2(offset, 0);
			//SampleHemisphereUniform(&thesample, basis, u1, u2, &pdf);
			//pdfs[j] = pdf;
			yR[j] = 0.;
			//WorldToLocal( &thesample, thesample, basis );	//rotate sample into tangent space
			shadowray.d = thesample; //generate shadow ray direction
			H = shadowray.d * norm;
			if (H > 0) { //only care about samples in upper hemisphere
				if (!thescene.Trace(minhit, shadowray, 0.0001, MIRO_TMAX)) { //not occluded
					for (int k = 0; k < bands; ++k) {
						RGBCoeff& coeff = coeffentry[k];
						grayness = H * shcoeffs[j*bands + k]; //project onto SH basis
						grayness /= pdfs[j];
						coeff.r += grayness; //sum up contribution
						coeff.g += grayness;
						coeff.b += grayness;

						float v = shcoeffs[j*bands + k];
					}
					yR[j] = H;
				}
				else {
				}
			}
		} //end for (int j = 0; j < numsamples; ++j)

		for (int k = 0; k < bands; ++k) { //weight coeffs properly
			RGBCoeff& coeff = coeffentry[k];
			coeff *= normfactor;
		}
		float eng_xR = (float) 0.0;
		for (int j = 0; j < bands; j++)
			eng_xR += (coeffentry[j].r * coeffentry[j].r);
		//printf("Projected Square Norm: %0.4f\n", eng_xR);
		_cprintf("\r%d",i);
	}

	delete [] samples;
	delete [] pdfs;
	delete [] yR;
}

bool WritePRTCoefficients( const char* filename, PRT* prt, U32 crc )
{
	int nv = prt->getNumVerts();
	int bands = prt->getBands();
	FILE* f = fopen( filename, "wb" );

	fwrite( &crc, sizeof(U32), 1, f );
	fwrite( &nv, sizeof(int), 1, f );
	fwrite( &bands, sizeof(int), 1, f );

	for (int v=0; v < nv; v++) {
		const RGBCoeff* coeffs = prt->getCoeffList(v);
		for (int c=0; c < bands; c++) {
			const RGBCoeff* coeff = coeffs + c;
			fwrite( coeff, sizeof(RGBCoeff), 1, f );
		}
	}
	fclose(f);

	return (f != 0);
}

bool ReadPRTCoefficients( const char* filename, PRT* prt, U32 crc )
{
	int nv = prt->getNumVerts();
	int bands = prt->getBands();
	int orgnv;
	FILE* f = fopen( filename, "rb" );
	if (f == 0)
		return false;
	U32 orgcrc;
	int orgbands;

	fread( &orgcrc, sizeof(U32), 1, f );
	fread( &orgnv, sizeof(int), 1, f );
	fread( &orgbands, sizeof(int), 1, f );
	if ((crc != orgcrc) || (nv != orgnv) || (bands != orgbands)) {
		return false;
	}
	for (int v=0; v < nv; v++) {
		RGBCoeff* coeffs = prt->getCoeffList(v);
		for (int c=0; c < bands; c++) {
			RGBCoeff* coeff = coeffs + c;
			fread( coeff, sizeof(RGBCoeff), 1, f );
		}
	}
	fclose(f);

	return true;
}

