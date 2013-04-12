/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef PRT_H
#define PRT_H

#include <stdio.h>

#include "prtfw.h"		//Scene
#include "basictypes.h"
#include "imagePRT.h"		//Pixel
#include "quantize.h"		//Pixel

typedef Pixel RGBCoeff;

class GameScene;
class Mesh;

namespace STAT
{
	class AllStats;
}

class CodeBook1D;


class PRT {
public:
	PRT();
	~PRT();
	void reset();
	PRT(int numverts, int b);

	void resize(int numverts, int b);
	int getNumVerts() const { return vertsize; }
	int getBands() const { return bands; }	
	void projectShadow( const GameScene& thescene, int numsamples, Mesh* mesh, U32 crc );
	RGBCoeff* getCoeffList(int vertindex) { return coeffs+(vertindex*bands); }
	void	getSamples(double* samples, int b) const; 
	float	calcSHCoeff(int l, int m, float theta, float phi);
	//compute the ranges for the SH coefficients. Place results into 'm_ranges2'
	float*  getSHRanges(int numsamples);
	void	dumpStats();
	void nullQKind() { m_quanKind = kNull; }

	int		getCoeffNum() { return bands; }
	size_t	getSizeBytes(bool color) const;

	float*	convertToScalars( int method, float scale = 1.0f );
	void	packScalars( int method );
	//update the original scalars - used only in Lloyd-Max iteration
	void	updateScalar( int b, const double* quantized );

	void	allocCodeBook( Byte bitbudgets[] );
	void	computeScales( Byte bitbudgets[], QKind qkind = kMidThread );

	float*	unpackScalarsRC72();
	float*	unpackScalarsRC72_9();
	float*	unpackScalarsRC48(const Byte* ob);
	float*	unpackScalarsLM48(const Byte* ob);
	float*  unpackScalarsRC32();
	float*  unpackScalarsRC32SIMD(const float* reference);

	//get the original PRT scalars:
	const float* getScalars() const { return m_scalars; }
	//get the range coded PRT scalars:
	const Byte* getScalarsRC(int method) const {
		switch (method) {
			case 0:
			case 1:
				return m_scalarBytes;
			break;
			case 2:
				return m_scalarRC48;
			break;
			case 3:
				return m_scalarLM;
			break;
			case 4:
				return m_scalarRC32;
			break;
			default:
				return 0;
			break;
		}
	}
	const Byte* getPackedSclars();

	//void	RenderGL(const GLContext& context);

	/// bind the locations within 'vshader' for our coefficients:
	//int		bindVertexShader( Shader* vshader, int packmethod );

protected:
	void	dumpPackMethod( int method );
	void	dumpQuantizationKind( QKind qkind );
	void	collectStats( const float* unpacked );

	// binds the PRT scalar coefficients using our 9 byte uniform quantized range-coded shader:
	void	packScalarsRC72(QKind qkind);
	// binds the PRT scalar coefficients using our 9 byte uniform quantized range-coded with individual scale-bias:
	void	packScalarsRC72_9(QKind qkind);
	// binds the PRT scalar coefficients using our 48 bit uniform quantized range-coded shader:
	void	packScalarsRC48(Byte* orc, const float* scalars);
	// binds the PRT scalar coefficients using our 32 bit uniform quantized range-coded shader:
	void	packScalarsRC32(Byte* orc);
	// binds the PRT scalar coefficients using our Lloyd-Max 48 bit uniform quantized range-coded shader:
	void	packScalarsLM48();

	// binds the uniform constants for our 9 byte uniform quantized range-coded with individual scale-bias:
	//int		bindVertexShaderRC72_9( Shader* vshader );
	// binds the uniform constants for our 48 bit range-coded shader:
	//int		bindVertexShaderRC48( Shader* vshader );
	// binds the uniform constants for our 32 bit range-coded shader:
	//int		bindVertexShaderRC32( Shader* vshader );
	// binds the uniform constants for our Lloyd-Max 48 bit range-coded shader:
	//int		bindVertexShaderLM48( Shader* vshader );
private:
	RGBCoeff* coeffs; //linear array of coeffs for ALL verts (bands*numverts)
	float* m_scalars;
	Byte* m_scalarBytes;
    float m_scale;		//scale for compressed m_scalarBytes
	float m_min;
	float m_max;

	Byte* m_scalarRC48;	//48 bit range coded scalars
	Byte* m_scalarRC32;	//32 bit range coded
	Byte* m_scalarLM;	//48 bit range coded scalars with Lloyd-Max

	float*	m_ranges;
	float*	m_scales;
	float*	m_ranges2;
	float*	m_recon;

	CodeBook1D**	m_codebook;
	QKind	m_quanKind;

	int vertsize; //number of UNIQUE vertices in object
	int bands; //number of SH coeffs being used (ie 9,16,25,etc)
	STAT::AllStats* m_stats;
};

inline noVec4 xyyz( const noVec4& v ) { return noVec4(v.x,v.y,v.y,v.z); }
inline noVec4 xyyy( const noVec4& v ) { return noVec4(v.x,v.y,v.y,v.y); }
inline noVec4 zwww( const noVec4& v ) { return noVec4(v.z,v.w,v.w,v.w); }


/// 
bool WritePRTCoefficients( const char* filename, PRT* prt, U32 crc );
bool ReadPRTCoefficients( const char* filename, PRT* prt, U32 crc );

#endif