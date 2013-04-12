#include "stdafx.h"
#include "DebugDraw.h"
#include <GameApp/GameApp.h>
#include <GameApp/Util/EffectUtil.h>
#include "NavDebugDraw.h"

NavDebugDraw::~NavDebugDraw()
{

}

void NavDebugDraw::depthMask( bool state )
{
	
}

void NavDebugDraw::begin( duDebugDrawPrimitives prim, float size /*= 1.0f*/ )
{	
	switch(prim)
	{
	case DU_DRAW_TRIS:
		m_Prim = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		break;
	case DU_DRAW_LINES:
		m_Prim = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		break;
	case DU_DRAW_LINES_STRIP:
		m_Prim = D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		break;
	case DU_DRAW_POINTS:
		m_Prim = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		break;
	}		

	if (D3D11Context()->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &m_MappedResource ) == S_OK)
	{
		m_CurrVertex = 0;
		m_bLock = true;		
	}
}

void NavDebugDraw::vertex( const float* pos, unsigned int color )
{
	if (!m_bLock) return;
	DebugVertex* pVerts = reinterpret_cast<DebugVertex*>(m_MappedResource.pData);
	pVerts[m_CurrVertex].pos = noVec3(pos[0], pos[1] + mYOffset, pos[2]);
	float col[3];
	duIntToCol(color, col);
	pVerts[m_CurrVertex].color = noVec4(col[0], col[1], col[2], 0.0f);	
	++m_CurrVertex;
}

void NavDebugDraw::vertex( const float x, const float y, const float z, unsigned int color )
{
	if (!m_bLock) return;
	DebugVertex* pVerts = reinterpret_cast<DebugVertex*>(m_MappedResource.pData);
	pVerts[m_CurrVertex].pos = noVec3(x, y + mYOffset, z);
	float col[3];
	duIntToCol(color, col);
	pVerts[m_CurrVertex].color = noVec4(col[0], col[1], col[2], 0.0f);	
	++m_CurrVertex;
}

void NavDebugDraw::end()
{
	if (!m_bLock) return;

	D3D11Context()->Unmap(m_pVertexBuffer, 0);
	m_bLock = false;

	Draw();
}

void NavDebugDraw::Draw() {
	if (m_CurrVertex <= 0 ) return;
	
	D3DXMATRIX mWorldViewProjection;		
	D3DXMATRIX mView;
	D3DXMATRIX mProj;		
	const unsigned int size16 = sizeof(float) * 16;


	memcpy(&mView, &ViewMat(), size16);	 
	memcpy(&mProj, &ProjMat(), size16);

	mWorldViewProjection = mView * mProj;
	mpWorldViewProjection->SetMatrix( ( float* )&mWorldViewProjection );
	D3D11Context()->IASetInputLayout( m_pVertexLayout );
	UINT Strides[1];
	UINT Offsets[1];
	ID3D11Buffer* pVB[1];
	pVB[0] = m_pVertexBuffer;
	Strides[0] = sizeof( DebugVertex );
	Offsets[0] = 0;
	D3D11Context()->IASetVertexBuffers( 0, 1, pVB, Strides, Offsets );
	if (m_pIndexBuffer)
		D3D11Context()->IASetIndexBuffer( m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0 );
	D3D11Context()->IASetPrimitiveTopology( m_Prim );

	D3DX11_TECHNIQUE_DESC techDesc;
	gpDiffuse->GetDesc( &techDesc );

	for( UINT p = 0; p < techDesc.Passes; ++p )
	{
		gpDiffuse->GetPassByIndex( p )->Apply( 0, D3D11Context() );
		if (m_pIndexBuffer)
			D3D11Context()->DrawIndexed( m_CurrVertex, m_BaseIndex, m_BaseVertex);
		else 
			D3D11Context()->Draw( m_CurrVertex, m_BaseVertex);
	}	
}


NavDebugDraw::NavDebugDraw()
	:m_BaseIndex(0),
	m_BaseVertex(0),
	m_CurrVertex(0),
	m_bLock(false), 
	m_pVertexBuffer(NULL),
	m_pIndexBuffer(NULL) 
{
	
}

bool NavDebugDraw::CreateVertexBuffer(const void* pVertexData, unsigned int uiNumVertices, unsigned int uiVertexSize)
{
	bool bSuccess = false;

	if (uiNumVertices > 0 && uiVertexSize > 0)
	{
		assert(GetApp()->GetDevice());

		D3D11_BUFFER_DESC sBufferDesc;
		sBufferDesc.ByteWidth = uiNumVertices * uiVertexSize;
		sBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		sBufferDesc.MiscFlags = 0;
		//if (m_bDynamicVertexBuffer)
		{
			sBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
			sBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		}
		/*else
		{
			sBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			sBufferDesc.CPUAccessFlags = 0;
		}*/

		D3D11_SUBRESOURCE_DATA sInitData;
		if (pVertexData)
		{
			sInitData.pSysMem = pVertexData;

			if (GetApp()->GetDevice()->CreateBuffer(&sBufferDesc, &sInitData, &m_pVertexBuffer) == S_OK)
			{
				m_uiCurrentVertexBufferSize = uiNumVertices * uiVertexSize;
				bSuccess = true;
			}
		}
		else 
			if (GetApp()->GetDevice()->CreateBuffer(&sBufferDesc, NULL, &m_pVertexBuffer) == S_OK)
			{
				m_uiCurrentVertexBufferSize = uiNumVertices * uiVertexSize;
				bSuccess = true;
			}

	}

	return bSuccess;
}

bool NavDebugDraw::CreateIndexBuffer(const void* pIndexData, unsigned int uiNumIndices)
{
	bool bSuccess = false;

	//assert(pIndexData);
	assert(uiNumIndices > 0);
	assert(D3D11Dev());

	D3D11_BUFFER_DESC sBufferDesc;
	sBufferDesc.ByteWidth = uiNumIndices * 2;	// 32Bit
	sBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	sBufferDesc.MiscFlags = 0;
	if (pIndexData && uiNumIndices > 0)
	{	
		sBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		sBufferDesc.CPUAccessFlags = 0;
	}
	else if (uiNumIndices > 0) {
		sBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		sBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	}

	if (pIndexData) {
		D3D11_SUBRESOURCE_DATA sInitData;
		sInitData.pSysMem = pIndexData;

		if (D3D11Dev( )->CreateBuffer(&sBufferDesc, &sInitData, &m_pIndexBuffer) == S_OK)
		{
			FindMaxIndex(pIndexData, uiNumIndices);
			m_uiCurrentIndexBufferSize = uiNumIndices;
			bSuccess = true;
		}
	}
	else {
		if (D3D11Dev( )->CreateBuffer(&sBufferDesc, NULL, &m_pIndexBuffer) == S_OK) 		{			
			m_uiCurrentIndexBufferSize = uiNumIndices;
			bSuccess = true;
		}
	}
	
	return bSuccess;
}

void NavDebugDraw::FindMaxIndex(const void* pIndexData, unsigned int uiNumIndices)
{
	m_uiMaxIndex = 0;
	//if (m_eIndexFormat == INDEX_FORMAT_UNSIGNED_16BIT)
	//{
	//	unsigned short* pIndices = (unsigned short*) pIndexData;
	//	for (unsigned int i = 0; i < uiNumIndices; ++i)
	//		m_uiMaxIndex = __max(pIndices[i], m_uiMaxIndex);
	//}
	//else // assume 32-bit
	{
		unsigned int* pIndices = (unsigned int*) pIndexData;
		for (unsigned int i = 0; i < uiNumIndices; ++i)
			m_uiMaxIndex = __max(pIndices[i], m_uiMaxIndex);
	}
}

void NavDebugDraw::SetupShader()
{
	LoadEffectFromFile(D3D11Dev(), "Data/Shaders/Diffuse.fx", &gpEffect);

	gpDiffuse = gpEffect->GetTechniqueByName("TShader");

	mpWorldViewProjection = gpEffect->GetVariableByName( "matWorldViewProj" )->AsMatrix();	

	const D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "SV_POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },				
		{ "COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,  0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	ID3D11Device* device = GetApp()->GetDevice();
	D3DX11_PASS_DESC PassDesc;
	gpDiffuse->GetPassByIndex( 0 )->GetDesc( &PassDesc ) ;
	device->CreateInputLayout( layout, 2, PassDesc.pIAInputSignature,
		PassDesc.IAInputSignatureSize, &m_pVertexLayout ) ;

}

DebugVertex* NavDebugDraw::LockVB( int offset )
{

	D3D11_MAPPED_SUBRESOURCE ms;	
	D3D11_MAP flag = D3D11_MAP_WRITE_DISCARD;
	D3D11Context()->Map(m_pVertexBuffer, 0, flag, 0, &ms);	

	return (DebugVertex*)(ms.pData) + offset;
}

void NavDebugDraw::UnlockVB( )
{
	D3D11Context()->Unmap(m_pVertexBuffer, 0);
}

WORD* NavDebugDraw::LockIB16(  )
{

	D3D11_MAPPED_SUBRESOURCE ms;	
	D3D11_MAP flag = D3D11_MAP_WRITE_DISCARD;
	D3D11Context()->Map(m_pIndexBuffer, 0, flag, 0, &ms);	

	return (WORD*)ms.pData;
}

void NavDebugDraw::UnlockIB16( )
{
	D3D11Context()->Unmap(m_pIndexBuffer, 0);
}