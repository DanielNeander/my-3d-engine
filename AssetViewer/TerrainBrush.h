#pragma once 

#include <U2_3D/Src/dx9/U2Mesh.h>

class TerrainBrash 
{
public:
		void SetSizeInner(float size);

		void SetSizeOuter(float size);

		float GetInnterSize() const;

		float GetOuterSize() const;

		

		virtual void SetupBrushOverlay() {}

		// Resize Brush
		// Update Brush

protected:
	virtual bool IsInBrushBound(float fActiveRadiusSqr,
		float fOuterRadiusSqr,						
		D3DXVECTOR3& origin,
		D3DXVECTOR3& point, 
		bool& bInOuterRadius,
		float& fdistSqr
		) = 0;

public:
	static unsigned short ms_usInnerSegments = 32;
	static unsigned short ms_usOuterSegments = 32;

protected:
	D3DXCOLOR m_baseColor;

	float m_fSizeInner;
	float m_fSizeOuter;

	U2MeshPtr m_spBrushOverlay;	



	

};