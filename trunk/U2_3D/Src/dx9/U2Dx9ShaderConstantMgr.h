/************************************************************************
module	:	U2Dx9ShaderConstantMgr
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_DX9_SHADERCONSTANTMGR_H
#define U2_DX9_SHADERCONSTANTMGR_H

#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>

class U2Dx9Renderer;

class U2_3D U2Dx9ShaderConstantMgr : public U2RefObject
{
public:
	virtual ~U2Dx9ShaderConstantMgr();


	inline bool SetVertexShaderConstantB(unsigned int uiStartRegister,
		const BOOL* pbConstantData, unsigned int uiBoolCount, 
		bool bSave);
	inline bool SetVertexShaderConstantB(unsigned int uiStartRegister,
		const BOOL* pbConstantData, unsigned int uiBoolCount);
	inline bool GetVertexShaderConstantB(unsigned int uiStartRegister,
		BOOL* pbConstantData, unsigned int uiBoolCount);
	inline bool RestoreVertexShaderConstantB(unsigned int uiStartRegister, 
		unsigned int uiBoolCount);
	inline bool SetVertexShaderConstantF(unsigned int uiStartRegister,
		const float* pfConstantData, unsigned int uiFloat4Count, 
		bool bSave);
	inline bool SetVertexShaderConstantF(unsigned int uiStartRegister,
		const float* pfConstantData, unsigned int uiFloat4Count);
	inline bool GetVertexShaderConstantF(unsigned int uiStartRegister,
		float* pfConstantData, unsigned int uiFloat4Count);
	inline bool RestoreVertexShaderConstantF(unsigned int uiStartRegister, 
		unsigned int uiFloat4Count);
	inline bool SetVertexShaderConstantI(unsigned int uiStartRegister,
		const int* piConstantData, unsigned int uiInt4Count, 
		bool bSave);
	inline bool SetVertexShaderConstantI(unsigned int uiStartRegister,
		const int* piConstantData, unsigned int uiInt4Count);
	inline bool GetVertexShaderConstantI(unsigned int uiStartRegister,
		int* piConstantData, unsigned int uiInt4Count);
	inline bool RestoreVertexShaderConstantI(unsigned int uiStartRegister, 
		unsigned int uiInt4Count);

	inline bool SetPixelShaderConstantB(unsigned int uiStartRegister,
		const BOOL* pbConstantData, unsigned int uiBoolCount, 
		bool bSave);
	inline bool SetPixelShaderConstantB(unsigned int uiStartRegister,
		const BOOL* pbConstantData, unsigned int uiBoolCount);
	inline bool GetPixelShaderConstantB(unsigned int uiStartRegister,
		BOOL* pbConstantData, unsigned int uiBoolCount);
	inline bool RestorePixelShaderConstantB(unsigned int uiStartRegister, 
		unsigned int uiBoolCount);
	inline bool SetPixelShaderConstantF(unsigned int uiStartRegister,
		const float* pfConstantData, unsigned int uiFloat4Count, 
		bool bSave);
	inline bool SetPixelShaderConstantF(unsigned int uiStartRegister,
		const float* pfConstantData, unsigned int uiFloat4Count);
	inline bool GetPixelShaderConstantF(unsigned int uiStartRegister,
		float* pfConstantData, unsigned int uiFloat4Count);
	inline bool RestorePixelShaderConstantF(unsigned int uiStartRegister, 
		unsigned int uiFloat4Count);
	inline bool SetPixelShaderConstantI(unsigned int uiStartRegister,
		const int* piConstantData, unsigned int uiInt4Count, 
		bool bSave);
	inline bool SetPixelShaderConstantI(unsigned int uiStartRegister,
		const int* piConstantData, unsigned int uiInt4Count);
	inline bool GetPixelShaderConstantI(unsigned int uiStartRegister,
		int* piConstantData, unsigned int uiInt4Count);
	inline bool RestorePixelShaderConstantI(unsigned int uiStartRegister, 
		unsigned int uiInt4Count);


	void	Commit();

	static U2Dx9ShaderConstantMgr* Create(U2Dx9Renderer* pRenderer, 
		const D3DCAPS9& d3dCaps);


private:
	U2Dx9ShaderConstantMgr(U2Dx9Renderer* pRenderer, 
		const D3DCAPS9& d3dCaps);

	// Float Registers
	float* m_pFVSConstants;
	float* m_pSavedFVSConstants;
	float* m_pFPSConstants;
	float* m_pSavedFPSConstants;

	unsigned int m_uiFirstDirtyFVSReg;
	unsigned int m_uiFirstCleanFVSReg; // One past last dirty
	unsigned int m_uiFirstDirtyFPSReg;
	unsigned int m_uiFirstCleanFPSReg; // One past last dirty

	uint32 m_uNumFVSConstants;
	uint32 m_uNumFPSConstants;

	// Int Registers
	int* m_pIVSConstants;
	int* m_pSavedIVSConstants;
	int* m_pIPSConstants;
	int* m_pSavedIPSConstants;

	unsigned int m_uiFirstDirtyIVSReg;
	unsigned int m_uiFirstCleanIVSReg; // One past last dirty
	unsigned int m_uiFirstDirtyIPSReg;
	unsigned int m_uiFirstCleanIPSReg; // One past last dirty


	uint32 m_uNumIVSConstants;
	uint32 m_uNumIPSConstants;
	
	// Boolean Registers
	BOOL* m_pBVSConstants;
	BOOL* m_pSavedBVSConstants;
	BOOL* m_pBPSConstants;
	BOOL* m_pSavedBPSConstants;

	unsigned int m_uiFirstDirtyBVSReg;
	unsigned int m_uiFirstCleanBVSReg; // One past last dirty
	unsigned int m_uiFirstDirtyBPSReg;
	unsigned int m_uiFirstCleanBPSReg; // One past last dirty


	uint32 m_uNumBVSConstants;
	uint32 m_uNumBPSConstants;

	LPDIRECT3DDEVICE9 m_pD3DDev;
	U2Dx9Renderer*	m_pRenderer;
	
};

#include "U2Dx9ShaderConstantMgr.inl"

typedef U2SmartPtr<U2Dx9ShaderConstantMgr> U2Dx9ShaderConstantMgrPtr;


#endif