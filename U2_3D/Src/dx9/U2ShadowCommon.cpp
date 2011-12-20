#include <U2_3D/src/U23DLibPCH.h>
#include "U2ShadowCommon.h"

//  Transform an axis-aligned bounding box by the specified matrix, and compute a new axis-aligned bounding box
void XFormBoundingBox( U2Aabb* result, const U2Aabb* src, const D3DXMATRIX* matrix )
{
	D3DXVECTOR3  pts[8];
	for ( int i=0; i<8; i++ )
		pts[i] = src->CornerPoint(i);

	result->m_vMin = D3DXVECTOR3(3.3e33f, 3.3e33f, 3.3e33f);
	result->m_vMax = D3DXVECTOR3(-3.3e33f, -3.3e33f, -3.3e33f);

	for (int i=0; i<8; i++)
	{
		D3DXVECTOR3 tmp;
		D3DXVec3TransformCoord(&tmp, &pts[i], matrix);
		result->Extend(tmp);
	}
}

void GetTerrainBoundingBox( std::vector<U2Aabb>* shadowReceivers, const D3DXMATRIX* modelView, const U2Culler* sceneFrustum )
{
	D3DXVECTOR3 smq_start(-SMQUAD_SIZE, -10.f, -SMQUAD_SIZE);
	D3DXVECTOR3 smq_width(2.f*SMQUAD_SIZE, 0.f, 0.f);
	D3DXVECTOR3 smq_height(0.f, 0.f, 2.f*SMQUAD_SIZE);

	for (int k=0; k<4*4; k++)
	{
		float kx = float(k&0x3);
		float ky = float((k>>2)&0x3);
		U2Aabb hugeBox;
		hugeBox.m_vMin = smq_start + (kx/4.f)*smq_width + (ky/4.f)*smq_height;
		hugeBox.m_vMax = smq_start + ((kx+1.f)/4.f)*smq_width + ((ky+1.f)/4.f)*smq_height;

		int hugeResult = sceneFrustum->TestAabb2(hugeBox);
		if ( hugeResult !=2 )  //  2 requires more testing...  0 is fast reject, 1 is fast accept
		{
			if ( hugeResult == 1 )
			{
				XFormBoundingBox(&hugeBox, &hugeBox, modelView);
				shadowReceivers->push_back(hugeBox);
			}
			continue;
		}


		for (int j=0; j<4*4; j++)
		{
			float jx = kx*4.f + float(j&0x3);
			float jy = ky*4.f + float((j>>2)&0x3);
			U2Aabb bigBox;
			bigBox.m_vMin = smq_start + (jx/16.f)*smq_width + (jy/16.f)*smq_height;
			bigBox.m_vMax = smq_start + ((jx+1.f)/16.f)*smq_width + ((jy+1.f)/16.f)*smq_height;

			

			int bigResult = sceneFrustum->TestAabb2(bigBox);
			if ( bigResult != 2 )
			{
				if ( bigResult == 1 )
				{
					XFormBoundingBox(&bigBox, &bigBox, modelView);
					shadowReceivers->push_back(bigBox);
				}
				continue;
			}

			int stack = 0;

			for (int q=0; q<4; q++)
			{
				float iy = jy*4.f + float(q);
				int stack = 0;

				for (int r=0; r<4; r++)
				{
					float ix = jx*4.f + float(r);
					U2Aabb smallBox;
					smallBox.m_vMin = smq_start + (ix/64.f)*smq_width + (iy/64.f)*smq_height;
					smallBox.m_vMax = smq_start + ((ix+1.f)/64.f)*smq_width + ((iy+1.f)/64.f)*smq_height;

					
					if (sceneFrustum->TestAabb2(smallBox))
					{
						stack |= (1 << r);
					}
				}

				if (stack)
				{
					float firstX, lastX;
					int i;
					for (i=0; i<4; i++)
					{
						if ( (stack&(1<<i)) != 0)
						{
							firstX = float(i);
							break;
						}
					}
					for (i=3; i>=0; i--)
					{
						if ( (stack&(1<<i)) !=0)
						{
							lastX = float(i);
							break;
						}
					}
					firstX += jx*4.f;
					lastX  += jx*4.f;

					U2Aabb coalescedBox;
					coalescedBox.m_vMin = smq_start + (firstX/64.f)*smq_width + (iy/64.f)*smq_height;
					coalescedBox.m_vMax = smq_start + ((lastX+1.f)/64.f)*smq_width + ((iy+1.f)/64.f)*smq_height;

					XFormBoundingBox(&coalescedBox, &coalescedBox, modelView);
					shadowReceivers->push_back(coalescedBox);
				}
			}
		}
	}
}