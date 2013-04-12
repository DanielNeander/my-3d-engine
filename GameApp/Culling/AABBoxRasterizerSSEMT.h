//--------------------------------------------------------------------------------------
// Copyright 2011 Intel Corporation
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
//
//--------------------------------------------------------------------------------------

#ifndef AABBOXRASTERIZERSSEMT_H
#define AABBOXRASTERIZERSSEMT_H

#include "AABBoxRasterizerSSE.h"

class AABBoxRasterizerSSEMT : public AABBoxRasterizerSSE
{
	public:
		AABBoxRasterizerSSEMT();
		~AABBoxRasterizerSSEMT();

		void IsInsideViewFrustum(ShadowMapCamera *pCamera);
		void TransformAABBoxAndDepthTest();

	private:
		static void IsInsideViewFrustum(VOID* taskData, INT context, UINT taskId, UINT taskCount);
		void IsInsideViewFrustum(UINT taskId, UINT taskCount);

		static void TransformAABBoxAndDepthTest(VOID* pTaskData, INT context, UINT taskId, UINT taskCount);
		void TransformAABBoxAndDepthTest(UINT taskId);
};

#endif //AABBOXRASTERIZERSSEMT_H