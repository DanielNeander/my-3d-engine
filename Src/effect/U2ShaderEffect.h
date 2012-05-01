/************************************************************************
module	:	U2ShaderEffect
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_SHADEREFFECT_H
#define U2_SHADEREFFECT_H

#include "U2Effect.h"
#include <U2_3D/src/Shader/U2VertexShader.h>
#include <U2_3D/src/shader/U2PixelShader.h>
#include <U2_3D/src/dx9/U2Dx9BaseTexture.h>
#include <U2_3D/src/dx9/U2AlphaState.h>

class U2ShaderUserConstant;
class U2Dx9Renderer;
class U2Mesh;
class U2Dx9RenderStateMgr;

class U2_3D U2ShaderEffect : public U2Effect 
{

public:
	U2ShaderEffect(int passQuantity);
	virtual ~U2ShaderEffect();

	void SetPassCnt(int iPassCnt);
	uint32 GetPassCnt() const;


	U2AlphaState* GetBlending(int iPass);

	virtual void SetRenderState(int iPass, U2Dx9RenderStateMgr* pRenderer, 
		bool bPrimaryEffect);
	virtual void RestoreRenderState(int iPAss, U2Dx9RenderStateMgr* pRenderer,
		bool bPrimaryEffect);

	void ConnVSConstant(int iPass, const U2DynString& name, float* pfSrc);
	void ConnPSConstant(int iPass, const U2DynString& name, float* pfSrc);

	void ConnVSConstants(int iPass);
	void ConnPSConstants(int iPass);

	bool SetVShader(int iPass, U2VertexShader* pVS);
	U2VertexShaderPtr GetVShader(int iPass);
	U2VertexProgramPtr GetVProgram(int iPass);
	const U2DynString& GetVSName(int iPass) const;
	uint32 GetVConstantQuantity (int iPass) const;
	U2ShaderUserConstant* GetVConstant (int iPass, int i);
	U2ShaderUserConstant* GetVConstant (int iPass, const U2DynString& rkName);
	uint32 GetVTextureQuantity (int iPass) const;
	U2Dx9BaseTexturePtr GetVTexture (int iPass, int i) const;
	U2Dx9BaseTexturePtr GetVTexture (int iPass, const U2DynString& rkName) const;
	bool SetVTexture (int iPass, int i, U2Dx9BaseTexturePtr spkTexture);
	bool SetVTexture (int iPass, int i, const U2DynString& rkName);

	bool SetPShader(int iPass, U2PixelShader* pPS);
	U2PixelShaderPtr GetPShader(int iPass);
	U2PixelProgramPtr GetPProgram(int iPass);
	const U2DynString& GetPSName(int iPass) const;
	uint32 GetPConstantQuantity (int iPass) const;
	U2ShaderUserConstant* GetPConstant (int iPass, int i);
	U2ShaderUserConstant* GetPConstant (int iPass, const U2DynString& rkName);
	uint32 GetPTextureQuantity (int iPass) const;
	U2Dx9BaseTexturePtr GetPTexture (int iPass, int i) const;
	U2Dx9BaseTexturePtr GetPTexture (int iPass, const U2DynString& rkName) const;
	bool SetPTexture (int iPass, int i, U2Dx9BaseTexturePtr pkTexture);
	bool SetPTexture (int iPass, int i, const U2DynString& rkName);

	virtual void LoadResoures(U2Dx9Renderer* pRenderer, U2Mesh* pMesh);
	virtual void ReleaseResources(U2Dx9Renderer* pRenderer, U2Mesh* pMesh);
	

	virtual int  Begin();
	virtual void End();
	virtual void BeginEffect(int pass, U2Dx9Renderer* pRenerer, bool bPrimary);
	virtual void EndEffect(int pass, U2Dx9Renderer* pRenerer, bool bPrimary);
	virtual void SetupStage(int pass, U2Dx9Renderer* pRenerer);
	virtual void Commit(int pass, U2Dx9Renderer* pRenerer);


protected:
	U2ShaderEffect();


	void SetDefaultAlphaState();

	bool AreProgramsCompatable(const U2VertexProgram* pVProgram, const U2PixelProgram*
		pPProgram);

	uint32 m_uPassCnt;
	U2ObjVec<U2VertexShaderPtr> m_vsArray;
	U2ObjVec<U2PixelShaderPtr> m_psArray;
	U2ObjVec<U2Dx9AlphaStatePtr> m_alphaStateArray;

	class Connection : public U2RefObject
	{
	public:
		~Connection();
		U2ShaderUserConstant *m_pUC;
		float *m_pfSrc;
	};

	typedef U2SmartPtr<Connection> ConnectionPtr;

	class ConnContainer : public U2RefObject
	{
	public:
		~ConnContainer();
		U2ObjVec<ConnectionPtr> m_connArray;
	};

	typedef U2SmartPtr<ConnContainer> ConnContainerPtr;

	U2ObjVec<ConnContainerPtr>  m_vsConnect;
	U2ObjVec<ConnContainerPtr>  m_psConnect;
	

};

#include "U2ShaderEffect.inl"

typedef U2SmartPtr<U2ShaderEffect> U2ShaderEffectPtr;


#endif