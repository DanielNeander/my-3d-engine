#ifndef RENDERER_BLENDMATRIXSET_DX9_H
#define RENDERER_BLENDMATRIXSET_DX9_H

#include "noBlendMatrixSet.h"

class noRenderSystemDX9;

class noBlendMatrixSetDX9 : public noBlendMatrixSet {

public:

	static noBlendMatrixSet* createBlendMatrixSetDX9(noRenderSystemDX9* context)
	{
		return new noBlendMatrixSetDX9(context);
	}	

	virtual int getNumMatrices() const { return m_matrices.getSize(); }

	virtual void addMatrix(const float* m, int referenceID);

	virtual void setMatrix(int i, const float* m);

	virtual bool bind();	

	virtual void clear() { m_matrices.setSize(0); m_referenceIDs.setSize(0); }

protected:
	noBlendMatrixSetDX9(noRenderSystemDX9* context);

	noRenderSystemDX9* m_context;
	hkArray<D3DMATRIX> m_matrices;
	int m_maxMatrices;



};



#endif