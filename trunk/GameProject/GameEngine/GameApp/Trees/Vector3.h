//Vector Class
// Part of Tree Grammar Structure for creating procedural trees

// Copyright ?2008-2009 Intel Corporation
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

#ifndef _Vector3
#define _Vector3
class	V3	: public noVec3
{
public:

	V3();

	V3(float a, float b, float c);
	V3( const noVec3 &v );

	float Length( void ) const;

   	V3& Normalize( );
    V3 operator+( const V3& Vector ) const
    {
        V3 v(x + Vector.x, 
             y + Vector.y, 
             z + Vector.z );
        return v;
    }

    V3 operator-( const V3& Vector ) const
    {
        V3 v(x - Vector.x, 
             y - Vector.y, 
             z - Vector.z);
        return v;
    }

};

class	V2	: public noVec2
{
public:

	V2();

	V2(float a, float b);
	V2( const noVec2 &v );

};

class B3
{
public:
    B3(){};
    B3(V3 i,V3 j, V3 k)
    { 
        iAxis = i;
        jAxis = j;
        kAxis = k;
    };
    void normalize()
    {
        iAxis.Normalize();
        jAxis.Normalize();
        kAxis.Normalize();
    };
    void spin();
    V3 iAxis;
    V3 jAxis;
    V3 kAxis;

};


//float	DotProduct ( const V3& v1, const V3& v2 );
V3	CrossProduct ( const V3& v1, const V3& v2 );

#endif
