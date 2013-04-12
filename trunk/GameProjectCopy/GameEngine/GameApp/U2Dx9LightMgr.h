
#pragma once 
#ifndef	U2_DX9_LIGHTMGR_H
#define	U2_DX9_LIGHTMGR_H


#include <U2_3D/src/U2_3DLibType.h>
#include <U2_3D/Src/main/U2TPointerList.h>
#include <u2Lib/src/U2THashTable.h>
#include <U2_3D/Src/Main/U2LightNode.h>

class U2Dx9RenderStateMgr;
class U2Dx9TextureState;

class U2_3D	U2Dx9LightMgr :public U2MemObj
{
public:
	enum LightingType 
	{
		LT_OFF = 0,			// no lighting
		LT_FIXED_FUNC,		// fixed function per-vertex lighting 
		LT_SHADER,			// shader controlled lighting
	};

	U2Dx9LightMgr(U2Dx9RenderStateMgr* pRS, LPDIRECT3DDEVICE9 pD3DDev);
	~U2Dx9LightMgr();
	
	void SetState(U2Dx9TextureState*);
	void RemoveLight(U2Light* light);

	void ReleaseLights();

	bool IsLit() const;


	// D3DXEffect shader Support  
	unsigned int AddLight(U2LightNode& light);
	void SetupD3D9Light(U2LightNode& lightNode);

	void		DisableLightRenderStates();

	void SetLightingType(LightingType type);
	LightingType GetLightingType() const;

	void UpdateLights();

	void InitLightRenderStates();	


	
		
private:
	LPDIRECT3DDEVICE9 m_pD3DDev;	

	U2Dx9RenderStateMgr* m_pRS;
//	U2THashTable<U2Dx9Light*, 
	U2TPointerList<U2Light*> m_lights;		// 실제 삭제는 U2PoolAlloc::Terminate에서 실행.

	LightingType m_eLitType;


};

#endif
