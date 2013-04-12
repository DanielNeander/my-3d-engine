#pragma once

#include "AIEngine.h"

namespace Intel
{

	///////////////////////////////////////////////////////////////////////////////
	// The Bugs class definition.
	///////////////////////////////////////////////////////////////////////////////

	class Bugs
	{
		unsigned int m_uNumBugs;
		ID3DXMesh* m_pStartPositionsMesh;

		unsigned int m_uNumWayPoints;
		ID3DXMesh* m_pWayPointsMesh;

		MeshFile m_BugXFile;
		MeshFile m_StartPositionsXFile;
		MeshFile m_WayPointsXFile;

		MeshFile::Instance* m_pInstances;

		D3DXVECTOR4 m_v4TweenFactor;

		D3DXHANDLE m_hHullEffect;
		D3DXHANDLE m_hBugEffect;
		D3DXHANDLE m_hBugShadowEffect;

		float m_fTickTime;

		AI::PATH m_Path;
		AI::Engine *m_engine;

		static bool StartPositionsMeshEnumCallBack( Mesh* pMesh, void* pContext );
		void ResetBugPositions();

		static bool WayPointsMeshEnumCallBack( Mesh* pMesh, void* pContext );
		void ResetWayPoints();

	public:

		Bugs();
		~Bugs();

		void Create();
		void Destroy();
		void Tick();
		void Update( Dynamics* pDynamics );

		void Render( D3DXMATRIXA16* pm4x4World );
		void RenderShadows( D3DXMATRIXA16* pm4x4World );

		HRESULT OnCreateDevice( IDirect3DDevice9* pd3dDevice );
		HRESULT OnDestroyDevice();

		unsigned int GetInstances( MeshFile::Instance** ppInstances );

		float GetTickTime() { return m_fTickTime; }
	};

};