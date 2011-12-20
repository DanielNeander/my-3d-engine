#pragma once 
#ifndef TERRAIN_BRUSH_CIRCLE_H
#define TERRAIN_BRUSH_CIRCLE_H

#include "TerrainBrush.h"

class TerrainBrushCircle : public TerrainBrash
{
public:

	virtual void SetupBrushOverlay();


	virtual bool IsInBrushBound(float fActiveRadiusSqr,
		float fOuterRadiusSqr,						
		D3DXVECTOR3& origin,
		D3DXVECTOR3& point, 
		bool& bInOuterRadius,
		float& fdistSqr
		);

private:
	static unsigned short ms_usVerts = ms_usInnerSegments + ms_usOuterSegments;

};

#endif