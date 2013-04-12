#include "stdafx.h"
#include "../Mesh.h"
#include "TrailEffect.h"
#include "GameApp/GameApp.h"

TrailMesh::TrailMesh( )
{
	
}

bool TrailMesh::Init(SceneNode* pkScene, SceneNode* pkBottom, float fInitialTime )
{
	//Establish base values for the swoosh.  We make these protected
	//member variables so we can add methods later to change them 
	//dynamically.
	m_fVelocityMinimum = 2.5f;
	m_fVelocityMaximum = 40.0f;

	unsigned short* pkConnect = new unsigned short[NUM_QUADS * 2 * 3];

	m_iStartingIndex = 0;
	//Seed the system with one quad.
	m_iStartingIndex = 0;
	m_iNumberOfQuads = 1;

	m_kVertices[m_iStartingIndex] = pkBottom->WorldTrans();
	m_kLastPoint = pkBottom->WorldTrans();
	m_kVertices[m_iStartingIndex + 1] = (pkBottom->WorldRotate() * 
		noVec3(0.7f, 0.f, 0.0f)) + pkBottom->WorldTrans();

	uint32 uiTotalVerts = NUM_QUADS * 2;

	FormatDesc vbFmt[] = {
		0, TYPE_VERTEX, FORMAT_FLOAT,  3,
		0, TYPE_NORMAL, FORMAT_FLOAT,   3,
		//0, TYPE_TANGENT, FORMAT_FLOAT, 3,
		//0, TYPE_BINORMAL,FORMAT_FLOAT, 3,
		0, TYPE_TEXCOORD, FORMAT_FLOAT, 2,
		//0, TYPE_COLOR,	FORMAT_FLOAT,	4,
	};

	m_kVerexStride = 0;

	for (int i = 0; i < elementsOf(vbFmt); ++i)
	{
		m_kVerexStride +=vbFmt[i].size * GetRenderer()->getFormatSize(vbFmt[i].format);
	}

	m_rd.shader = GetRenderer()->addShader("Data/Shaders/SimpleColor.hlsl");
	m_rd.vf = GetRenderer()->addVertexFormat(vbFmt, elementsOf(vbFmt), m_rd.shader);
	m_rd.depthMode = GetRenderer()->addDepthState(true, true);
	m_rd.startIndex = 0;
	m_rd.endIndex = NUM_QUADS * 2 * 3;
	m_rd.vertexStart = 0;
	m_rd.vertexEnd = 0;
	m_rd.cull = -1;

	//Esatblish the connectivity array.  This will not change.
	for (int i = 0; i < NUM_QUADS; i++)
	{
		pkConnect[6 * i] = 2 * i;		
		pkConnect[6 * i + 1] = 2 * i + 1;		
		pkConnect[6 * i + 2] = 2 * i + 2;		

		pkConnect[6 * i + 3] = 2 * i + 1;		
		pkConnect[6 * i + 4] = 2 * i + 3;		
		pkConnect[6 * i + 5] = 2 * i + 2;		
	}


	m_rd.ib = GetRenderer()->addIndexBuffer(NUM_QUADS * 2 * 3, sizeof(WORD), STATIC, pkConnect);
	delete pkConnect;

	float fUStep = 1.0f / NUM_QUADS;
	int i = 0;
	for (; i < NUM_QUADS * 2; i++)
	{
		m_kRenderVertices[i].pos = pkBottom->WorldTrans();
		//m_kRenderVertices[i].normal = noVec3(1.0f, 0.0f, 0.0f);

		float fU = fUStep * (i / 2);
		float fV = 1.0f * (i % 2);
		m_kTexCoords[i] = noVec2(fU, fV) ;
		//m_kRenderVertices[i].texcoord = m_kTexCoords[i];
		m_afVelocityScales[i] = 0.0f;
		m_kRenderVertices[i].color = noVec4(1.0f, 1.0f, 1.0f, 1.0f - (i/2) * fUStep);
	}

	m_rd.vb = GetRenderer()->addVertexBuffer(uiTotalVerts * m_kVerexStride, DYNAMIC, m_kRenderVertices);

	m_pkBottomObj = pkBottom;
	m_fLastTime = fInitialTime - 0.001f;
	SamplePoints(fInitialTime);

	return true;
}

void TrailMesh::SamplePoints( float fTime )
{
	//Establish iWriteIndex so we can insert into a circular queue.  Thus,
	//as points age out of the swoosh they are overwritten.
	int iWriteIndex = 0;
	if (m_iNumberOfQuads == NUM_QUADS)
	{
		m_iStartingIndex -= 2;
		if (m_iStartingIndex < 0)
			m_iStartingIndex += NUM_QUADS * 2;
		iWriteIndex = m_iStartingIndex;
	}
	else
	{
		iWriteIndex = m_iStartingIndex + (2 * m_iNumberOfQuads);
		m_iNumberOfQuads++;
	}

	//The swoosh points are the staff and the point -70 on the staff's
	//local y axis which we then rotate into world space.  All swoosh 
	//geometry is in world space.
	m_kVertices[iWriteIndex] = m_pkBottomObj->WorldTrans();
	m_kVertices[iWriteIndex + 1] = (m_pkBottomObj->WorldRotate() * 
		noVec3(0.7f, 0.0f, 0.0f)) + m_pkBottomObj->WorldTrans();

	noVec3 kPoint = m_pkBottomObj->WorldTrans();
	kPoint -= m_kLastPoint;
	float fDist = noMath::Sqrt(kPoint.x * kPoint.x + kPoint.y * kPoint.y + kPoint.z * kPoint.z);

	float fDeltaTime = fTime - m_fLastTime;
	if (fDeltaTime < 0.001f)
	{
		fDist = 0.0f;
		fDeltaTime = 1.0f;
	}

	fDist /= fDeltaTime;

	if (fDist > m_fVelocityMaximum)
	{
		fDist = 1.0f;
	}
	else if (fDist < m_fVelocityMinimum)
	{
		fDist = 0.0f;
	}
	else
	{
		fDist = (fDist - m_fVelocityMinimum) / (m_fVelocityMaximum - 
			m_fVelocityMinimum);
	}
	//Calculate the velocity and lerp between 0 and 1 for the velocity
	//scale.  This means the swoosh only appears when the staff moves
	//quickly.
	m_afVelocityScales[iWriteIndex] = fDist;
	m_afVelocityScales[iWriteIndex + 1] = fDist;

	m_fLastTime = fTime;
	m_kLastPoint = m_pkBottomObj->WorldTrans();

	//Rebuild the data for the renderable object from the values we have
	//stored in the class members.
	float fColorStep = 1.0f / NUM_QUADS;
	for (int i = 0; i < m_iNumberOfQuads * 2; i++)
	{
		m_kRenderVertices[i].pos = m_kVertices[(i + m_iStartingIndex) % (NUM_QUADS * 2)];
		//m_kRenderVertices[i].texcoord = m_kTexCoords[(i + m_iStartingIndex) % (NUM_QUADS * 2)];
		m_kRenderVertices[i].color = noVec4(1.0f, 1.0f, 1.0f, (1.0f - (i/2) * fColorStep))
				* m_afVelocityScales[(i + m_iStartingIndex) % (NUM_QUADS * 2)];
		if ((i + m_iStartingIndex) == NUM_QUADS * 2 ||
			(i + m_iStartingIndex) == NUM_QUADS * 2 + 1)
		{
			m_kRenderVertices[i].pos = m_kVertices[i + m_iStartingIndex - 2];
		}
	}

	Direct3D11Renderer* DX11R = (Direct3D11Renderer*)GetRenderer();
	DX11R->CopyToVB(m_rd.vb, m_kRenderVertices,  m_kVerexStride *  m_iNumberOfQuads * 2);
	m_rd.vertexEnd = m_iNumberOfQuads * 2;
	m_rd.endIndex = m_iNumberOfQuads * 2 - 2;
}

bool TrailMesh::CreateBuffers( void )
{
	return true;
}

void TrailMesh::Draw( void )
{
	GetRenderer()->reset();	
	GetApp()->drawModel(m_rd, WorldMat(), ViewMat(), ProjMat());
}

void TrailMesh::UpdateData()
{
	SamplePoints(GetApp()->GetSecTime());
}
