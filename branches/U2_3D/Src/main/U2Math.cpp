#include <U2_3D/Src/U23DLibPCH.h>
#include "U2Math.h"

namespace U2Math {;


//-------------------------------------------------------------------------------------------------
void Orthonormalize (D3DXVECTOR3& v0, D3DXVECTOR3& v1, D3DXVECTOR3& v2)
{
	// If the input vectors are v0, v1, and v2, then the Gram-Schmidt
	// orthonormalization produces vectors u0, u1, and u2 as follows,
	//
	//   u0 = v0/|v0|
	//   u1 = (v1-(u0*v1)u0)/|v1-(u0*v1)u0|
	//   u2 = (v2-(u0*v2)u0-(u1*v2)u1)/|v2-(u0*v2)u0-(u1*v2)u1|
	//
	// where |A| indicates length of vector A and A*B indicates dot
	// product of vectors A and B.

	// compute u0

	D3DXVec3Normalize(&v0, &v0);

	// compute u1
	float fDot0 = D3DXVec3Dot(&v0, &v1);
	v1 -= fDot0*v0;

	D3DXVec3Normalize(&v1, &v1);	

	float fDot1 = D3DXVec3Dot(&v1, &v2);
	// compute u2		
	fDot0 = D3DXVec3Dot(&v0, &v2);
	v2 -= fDot0*v0 + fDot1*v1;
	D3DXVec3Normalize(&v2, &v2);	
}

//-------------------------------------------------------------------------------------------------
D3DXQUATERNION FromRotationMatrix(const D3DXVECTOR3 rotColumn[3])
{
	D3DXMATRIX rot;
	for(uint32 col=0; col < 3; ++col)
	{
		rot.m[0][col] = rotColumn[col][0];
		rot.m[1][col] = rotColumn[col][1];
		rot.m[2][col] = rotColumn[col][2];
	}

	D3DXQUATERNION output;
	D3DXQuaternionRotationMatrix(&output, &rot);
	return output;
}






} // namespace U2Math

//-------------------------------------------------------------------------------------------------

#ifdef _DEBUG


int DPrintf(const char* formatString, ...)
{
	va_list argumentList;
	int     returnValue = 0;
	char    outputString[4096];

	//va_start(argumentList, formatString);
	//returnValue = vsprintf_s(outputString, 4096, formatString, argumentList);
	//va_end(argumentList);

#ifdef UNICODE 
	OutputDebugString(ToUnicode(outputString));
#else 
	//OutputDebugString(outputString);
#endif


	return(returnValue);
}



//-----------------------------------------------------------------------------
// 
// print a matrix to the debugger
//
//-----------------------------------------------------------------------------
void DPrintf(const D3DXMATRIX& matrix)
{
	DPrintf("%8.3f, %8.3f, %8.3f, %8.3f\n", matrix._11, matrix._12, matrix._13, matrix._14);
	DPrintf("%8.3f, %8.3f, %8.3f, %8.3f\n", matrix._21, matrix._22, matrix._23, matrix._24);
	DPrintf("%8.3f, %8.3f, %8.3f, %8.3f\n", matrix._31, matrix._32, matrix._33, matrix._34);
	DPrintf("%8.3f, %8.3f, %8.3f, %8.3f\n", matrix._41, matrix._42, matrix._43, matrix._44);  
}


//-----------------------------------------------------------------------------
// 
// print a matrix to the debugger
//
//-----------------------------------------------------------------------------
void DPrintf(const char* description, const D3DXMATRIX& matrix)
{
	U2ASSERT(description != NULL);

	DPrintf("%s\n", description);
	DPrintf(matrix);
	DPrintf("\n");
}


#endif



//-----------------------------------------------------------------------------
// 
// Render a quad(코드 수정 필요함)
//
//-----------------------------------------------------------------------------
HRESULT DrawQuad(LPDIRECT3DDEVICE9 device, const D3DXVECTOR3& position0, D3DCOLOR color0, const D3DXVECTOR2& texCoord0,
				 const D3DXVECTOR3& position1, D3DCOLOR color1, const D3DXVECTOR2& texCoord1,
				 const D3DXVECTOR3& position2, D3DCOLOR color2, const D3DXVECTOR2& texCoord2,
				 const D3DXVECTOR3& position3, D3DCOLOR color3, const D3DXVECTOR2& texCoord3)
{
	//Vertex  vertices[4];
	//HRESULT hr;
	//void*   buffer;

	//hr = device->SetFVF(Vertex::FVF);
	//assert(hr == D3D_OK);

	//// Load the vertex data
	//vertices[0].position = position0;
	//vertices[0].diffuse = color0;
	//vertices[0].tex0[0] = texCoord0.x;
	//vertices[0].tex0[1] = texCoord0.y;

	//vertices[1].position = position1;
	//vertices[1].diffuse = color1;
	//vertices[1].tex0[0] = texCoord1.x;
	//vertices[1].tex0[1] = texCoord1.y;

	//vertices[2].position = position2;
	//vertices[2].diffuse = color2;
	//vertices[2].tex0[0] = texCoord2.x;
	//vertices[2].tex0[1] = texCoord2.y;

	//vertices[3].position = position3;
	//vertices[3].diffuse = color3;
	//vertices[3].tex0[0] = texCoord3.x;
	//vertices[3].tex0[1] = texCoord3.y;

	// Copy the vertex data into the vertex buffer
//	hr = Water::m_quadVertexBuffer->Lock(0, 0, &buffer, D3DLOCK_DISCARD);
//	CHECK_RETURN_CODE(L"drawQuad  Water::m_quadVertexBuffer->Lock()", hr);

	//memcpy(buffer, vertices, 4 * sizeof(Vertex));

//	Water::m_quadVertexBuffer->Unlock();

	// Render the vertex buffer
	//device->SetStreamSource(0, Water::m_quadVertexBuffer, 0, sizeof(Vertex));

//	hr = device->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
//	CHECK_RETURN_CODE(L"drawQuad  DrawPrimitive()", hr);  

	return S_OK;
}



