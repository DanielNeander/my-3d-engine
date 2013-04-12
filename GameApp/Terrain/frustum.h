   //  
  // Cached Procedural Textures (see ShaderX4 for details)
 // (w)(c)2005 Carsten Dachsbacher
//

#pragma once
#ifndef _FRUSTUM__H
#define _FRUSTUM__H

static float frustum[ 6 ][ 4 ];

void buildFrustum( D3DXMATRIX &clip )
{
	frustum[0][0] = clip._14 + clip._11;
	frustum[0][1] = clip._24 + clip._21;
	frustum[0][2] = clip._34 + clip._31;
	frustum[0][3] = clip._44 + clip._41;

	frustum[1][0] = clip._14 - clip._11;
	frustum[1][1] = clip._24 - clip._21;
	frustum[1][2] = clip._34 - clip._31;
	frustum[1][3] = clip._44 - clip._41;

	frustum[2][0] = clip._14 + clip._12;
	frustum[2][1] = clip._24 + clip._22;
	frustum[2][2] = clip._34 + clip._32;
	frustum[2][3] = clip._44 + clip._42;

	frustum[3][0] = clip._14 - clip._12;
	frustum[3][1] = clip._24 - clip._22;
	frustum[3][2] = clip._34 - clip._32;
	frustum[3][3] = clip._44 - clip._42;

	frustum[4][0] = clip._13;
	frustum[4][1] = clip._23;
	frustum[4][2] = clip._33;
	frustum[4][3] = clip._43;

	frustum[5][0] = clip._14 - clip._13;
	frustum[5][1] = clip._24 - clip._23;
	frustum[5][2] = clip._34 - clip._33;
	frustum[5][3] = clip._44 - clip._43;
}


int calcClipCode( D3DXVECTOR3 *v )
{
	int clipCode = 0;

	for ( register int i = 0, j = 1; i < 6; i++, j <<= 1 )
		if ( frustum[ i ][ 0 ] * v->x + frustum[ i ][ 1 ] * v->y + frustum[ i ][ 2 ] * v->z + frustum[ i ][ 3 ] <= 0.0f )
			clipCode |= j;
	
	return clipCode;
}

bool boxIntersectsFrustum( D3DXVECTOR3 *aabbMin, D3DXVECTOR3 *aabbMax )
{
	int clipAnd	= 255;
	int clipOr	= 0;

	for ( int i = 0; i < 8; i++ )
	{
		D3DXVECTOR3	boxVertex;
		boxVertex.x = ((i>>0)&1) ? aabbMin->x : aabbMax->x;
		boxVertex.y = ((i>>1)&1) ? aabbMin->y : aabbMax->y;
		boxVertex.z = ((i>>2)&1) ? aabbMin->z : aabbMax->z;

		int clipCode = calcClipCode( (D3DXVECTOR3*)&boxVertex );

		clipAnd &= clipCode;
		clipOr	|= clipCode;
	}

	// trivial reject
	if ( clipAnd ) 
		return false;

	return true;
}

#endif