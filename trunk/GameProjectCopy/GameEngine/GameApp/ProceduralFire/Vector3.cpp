//Vector Class
//Writen by Hugh Smith summer and fall 2007
// Part of Smoke Framework implementation for procedural Fire system.

// Copyright © 2008-2009 Intel Corporation
// All Rights Reserved
//
// Permission is granted to use, copy, distribute and prepare derivative works of this
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  THIS SOFTWARE IS PROVIDED "AS IS."
// INTEL SPECIFICALLY DISCLAIMS ALL WARRANTIES, EXPRESS OR IMPLIED, AND ALL LIABILITY,
// INCLUDING CONSEQUENTIAL AND OTHER INDIRECT DAMAGES, FOR THE USE OF THIS SOFTWARE,
// INCLUDING LIABILITY FOR INFRINGEMENT OF ANY PROPRIETARY RIGHTS, AND INCLUDING THE
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.

// Part of Tree Grammar Structure for creating procedural trees
#include "Vector3.h"



V3::V3(){};

V3::V3(float a, float b, float c)
{
	x = a;
	y = b;
	z = c;
}

V3::V3( const D3DXVECTOR3 &v )
{
	x = v.x;
	y = v.y;
	z = v.z;
}


float V3::Length( void ) const
{
    float magnitude = ( x * x ) + 
			          ( y * y ) +
			          ( z * z );
	return sqrtf ( magnitude);
}
V3& V3::Normalize( )
{
	float len = Length();
	if ( len > 0.0f ) 
	{
		len = 1.0f / len;
		x *= len;
		y *= len;
		z *= len;
	}
	else
	{
	}
	return *this;
}


V2::V2(){};

V2::V2(float a, float b)
{
	x = a;
	y = b;
}

V2::V2( const D3DXVECTOR2 &v )
{
	x = v.x;
	y = v.y;
}

float	DotProduct ( const V3& v1, const V3& v2 ) 
{
	
	
	return ( v1.x * v2.x ) + ( v1.y * v2.y ) + ( v1.z * v2.z );
}


V3	CrossProduct ( const V3& v1, const V3& v2 ) 
{	
	return V3 (( v1.y * v2.z ) - ( v1.z * v2.y ),
				   ( v1.z * v2.x ) - ( v1.x * v2.z ),
				   ( v1.x * v2.y ) - ( v1.y * v2.x ));
}
void B3::spin()
{
}