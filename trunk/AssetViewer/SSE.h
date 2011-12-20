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
#ifndef __SSE_H__
#define __SSE_H__

#include "emmintrin.h"

// This function simply gathers 4 floats and places them on a __m128 variable.
static __forceinline SSE_GatherFourFloats( const float* pf0, 
										  const float* pf1,
										  const float* pf2,
										  const float* pf3, 
										  __m128* pm128Result)
{
	__m128 xmm0 = _mm_load_ss( pf0 );	// 0 0 0 pf0
	__m128 xmm1 = _mm_load_ss( pf1 );	// 0 0 0 pf1
	__m128 xmm2 = _mm_load_ss( pf2 );	// 0 0 0 pf2
	__m128 xmm3 = _mm_load_ss( pf3 );	// 0 0 0 pf3

	xmm0 = _mm_movelh_ps( xmm0, xmm2 ); // 0 pf2 0 pf0	
	xmm1 = _mm_shuffle_ps( xmm1, xmm3, _MM_SHUFFLE( 0, 1, 0, 1 ) ); // pf3 0 pf1 0

	*pm128Result = _mm_or_ps( xmm0, xmm1 ); // pf3 pf2 pf1 pf0
}

// This function simply scatters the contents of a __m128 variable on to 4 floats.
static __forceinline void SSE_ScatterFourFloats( float* pf0,
												float* pf1, 
												float* pf2,
												float* pf3,
												__m128 m128Value)
{
	__m128 xmm1 = _mm_shuffle_ps( m128Value, m128Value, _MM_SHUFFLE(3,2, 1, 1)); // 3 2 1 1
	__m128 xmm2 = _mm_movehl_ps( _mm_setzero_ps(), m128Value); // 0 0 3 2
	__m128 xmm3 = _mm_shuffle_ps( m128Value, m128Value, _MM_SHUFFLE(3, 2, 1, 1)); // 3 2 1 3

	_mm_store_ss( pf0, m128Value ); // 3 2 1 0
	_mm_store_ss( pf1, xmm1 );	// 3 2 1 1
	_mm_store_ss( pf2, xmm2 ); // 0 0 3 2
	_mm_store_ss( pf3, xmm3 ); // 3 2 1 3
}

#endif