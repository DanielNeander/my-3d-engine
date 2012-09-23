#ifndef RENDERER_BLENDMATRIXSET_H
#define RENDERER_BLENDMATRIXSET_H

class noRenderSystemDX9;

class noBlendMatrixSet : public hkReferencedObject {

public:
	noBlendMatrixSet() {} 

	static noBlendMatrixSet* (__cdecl *create)(noRenderSystemDX9* context);

	virtual int getNumMatrices() const = 0; 

	/// As the format of the hardware matrix is probably not exactly the same as HKG the
	/// set and get for the matrices is virtual and the storage is left up to the impl.
	virtual void addMatrix(const float* m, int referenceID) = 0;

	/// As the format of the hardware matrix is probably not exactly the same as HKG the
	/// set and get for the matrices is virtual and the storage is left up to the impl.
	virtual void setMatrix(int i, const float* m) = 0;

	/// Wipe the current set (resize to 0)
	virtual void clear() = 0;

	/// Bind the data for this matrix set, ie set these matrices as the current blending set
	virtual bool bind() = 0;

	/// To aid in setup and update from where ever these matrices come from
	/// we store a reference int for each matrix. Has to be the same size as getNumMatrices().
	hkArray<hkInt16> m_referenceIDs;
};


#endif