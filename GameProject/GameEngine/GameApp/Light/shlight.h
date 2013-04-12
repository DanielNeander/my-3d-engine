/******************************************************************************
"Practical Methods for Precomputed Radiance Transfer Using Spherical Harmonics"

Jerome Ko - submatrix@gmail.com
Manchor Ko - man961.great@gmail.com
Matthias Zwicker - mzwicker@cs.ucsd.edu

******************************************************************************/

#ifndef SHLIGHT_H
#define SHLIGHT_H

//#include "Vector.h"

class Shader;

class SHLight {
public:
	SHLight( const float* coeffs, int bands );
	~SHLight();

	void	Init(const float* coeffs, int bands);

	//void	bindToShader( Shader* vertexShader ) const;

	const float* GetLi() const { return m_Lcoeffs; }
	const float* GetRedMatrix() const { return m_redMatrix.ToFloatPtr(); }
	const float* GetGrnMatrix() const { return m_greenMatrix.ToFloatPtr(); }
	const float* GetBluMatrix() const { return m_blueMatrix.ToFloatPtr(); }
public:
	float*		m_Lcoeffs;
	int			m_bands;

	noMat4	m_redMatrix;
	noMat4	m_greenMatrix;
	noMat4	m_blueMatrix;
};	//end of SHLight

SHLight* LoadAndProject( const char* filename, int bands );
void DumpLi( const float* thecoeffs, int bands );

#endif
