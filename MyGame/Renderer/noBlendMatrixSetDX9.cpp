#include "DXUT.h"
#include <Common/Base/Container/LocalArray/hkLocalArray.h>
#include "noRenderSystemDX9.h"
#include "noBlendMatrixSetDX9.h"


void noBlendMatrixSetDX9::addMatrix( const float* m, int referenceID )
{
	D3DMATRIX* newM = m_matrices.expandBy(1);
	hkgMat4Copy( (float*)newM, m );
	m_referenceIDs.pushBack((hkInt16)referenceID);
	HK_ASSERT(0x0, m_referenceIDs.getSize() == m_matrices.getSize());
}

void noBlendMatrixSetDX9::setMatrix( int i, const float* m )
{
	if ((i < 0) || (i > m_matrices.getSize()) )
		return;

	hkgMat4Copy( (float*)&m_matrices[i], m);	
}

bool noBlendMatrixSetDX9::bind()
{
	LPDIRECT3DDEVICE9  m_pDevice = m_context->GetD3DDevice();
	//if (m_context->getCurrentVertexShader()) // Have a shader bound.
	if (1)
	{ 
		int nb = m_matrices.getSize()<m_maxMatrices? m_matrices.getSize() : m_maxMatrices;

		hkLocalArray<float> transposedMatrices( (nb * 12) + 4 ); // +4 so we can use it directly to transpose 16 elems as we go, overwiting the last row
		transposedMatrices.setSizeUnchecked( (nb*12) + 4);

		// then we need to place these matrices in the shader. 
		for (int m=0; m < nb; ++m)
		{	
			float* t = &transposedMatrices[m*12];
			hkgMat4Transpose( t, (const float*)&m_matrices[m]);
		}
		// one big set rather than n small ones
		const int startReg = 0; // XX assumed in all hkg bone shaders
		m_pDevice->SetVertexShaderConstantF(startReg, transposedMatrices.begin(), 3*nb ); 

	}
	else
	{
		for (int m=0; m < m_matrices.getSize(); ++m)
		{
			m_pDevice->SetTransform( D3DTS_WORLDMATRIX(m), &m_matrices[m] );
		}
	}

	return true;	
}

noBlendMatrixSetDX9::noBlendMatrixSetDX9( noRenderSystemDX9* context ) 
	: m_context(context)
{

}