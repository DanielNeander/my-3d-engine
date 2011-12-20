/*
* Copyright (c) 2006, Intel Corp.
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Intel Corp. nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE Intel Corp. AND CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE Intel Corp. AND CONTRIBUTORS BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef __CURVES_H__
#define __CURVES_H__

#include "SSE.h"

struct CurveFFDCtxt
{
	__m128 ffdc;	// coefficients
	__m128 ffdstep;
	__forceinline float step()
	{
		float res;
		// initially add
		// 3 2 1 0
		// c0 c1 c2 c3 and
		// d1 d2 d3 0
		// --> we can construct new d1 d2 d3 0 from
		// new c1, c2, d3, 0
		ffdc = _mm_add_ps( ffdc, ffdstep );
		ffdstep = _mm_shuffle_ps( ffdc, ffdstep, _MM_SHUFFLE( 3, 2, 2, 1));
		_mm_store_ss( &res, ffdc );
		return res;
	}
};

// Bernstein basis polynomials 
// http://en.wikipedia.org/wiki/Bernstein_polynomial
struct CubicPolynomial
{
	__forceinline void init( const float p1, const float p2, const float p3, const float p4,
		const float d1, const float d2, const float d3, const float d4)
	{

	}

	float coeffs[4];
	float dzcoeffs[4];
	float dxcoeffs[4];
};

struct CubicCurve : public CubicPolynomial
{
	// construct from 4 height values
	CubicCurve( const float h_1, const float h0, const float h1, const float h2 )
	{
		float p1 = h0;
		float p2 = h0 + ( 1.0f/ 3.0f ) * ( 0.5f * ( h1 - h_1 ) );
		float p3 = h1 - ( 1.0f/ 3.0f ) * ( 0.5f * ( h2 - h0  ) );
		float p4 = h1;
		
		init(p1, p2, p3, p4);
	}
	
	void init( const float p1, const float p2, const float p3, const float p4)
	{
		CubicPolynomial::init( p1, p2, p3, p4, 0, 0, 0, 0 );

		heights[0] = p1;
		heights[1] = p2;
		heights[2] = p3;
		heights[3] = p4;
	}

	float heights[4];

};