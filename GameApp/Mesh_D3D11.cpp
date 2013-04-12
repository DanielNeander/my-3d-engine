#include "stdafx.h"
#include "GameApp.h"
#include "Common.h"
#include "LWOFile.h"
#include "SceneNode.h"
#include "Mesh.h"
#include "Mesh_D3D11.h"

Mesh *CreateNewMesh(void)
{
  return new Mesh_D3D11();
}

Mesh_D3D11::Mesh_D3D11()
{
  m_pVertexBuffer = NULL;
  m_pIndexBuffer = NULL;
  m_pVertexLayout = NULL;
}

Mesh_D3D11::~Mesh_D3D11()
{
  if(m_pVertexBuffer != NULL)
  {
    m_pVertexBuffer->Release();
    m_pVertexBuffer = NULL;
  }

  if(m_pIndexBuffer != NULL)
  {
    m_pIndexBuffer->Release();
    m_pIndexBuffer = NULL;
  }

  if(m_pVertexLayout != NULL)
  {
    m_pVertexLayout->Release();
    m_pVertexLayout = NULL;
  }
}

extern ID3DX11EffectTechnique *g_pTechniqueShadows;
extern bool CreateShaders(void);

bool Mesh_D3D11::CreateBuffers(void)
{
  // Define the input layout
  D3D11_INPUT_ELEMENT_DESC layout[] =
  {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },  
      { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
      { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }, 
  };
  unsigned int iElements = sizeof(layout)/sizeof(layout[0]);

  // create shaders if not created yet
  if(g_pTechniqueShadows == NULL)
  {
    if(!CreateShaders()) return false;
  }

  // Create the input layout
  HRESULT hr;
  D3DX11_PASS_DESC PassDesc;
  hr = g_pTechniqueShadows->GetPassByIndex(0)->GetDesc(&PassDesc);
  if(FAILED(hr))
  {
    MessageBox(NULL, TEXT("Getting technique pass description failed!"), TEXT("Error!"), MB_OK);
    return false;
  }

  hr = GetApp()->GetDevice()->CreateInputLayout(layout, iElements, PassDesc.pIAInputSignature, PassDesc.IAInputSignatureSize, &m_pVertexLayout);
  if(FAILED(hr))
  {
    MessageBox(NULL, TEXT("Creating input layout failed!"), TEXT("Error!"), MB_OK);
    return false;
  }

  // Create vertex buffer
  D3D11_BUFFER_DESC bd;
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = m_iVertexSize * m_iNumVertices;
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;
  D3D11_SUBRESOURCE_DATA InitData;
  InitData.pSysMem = m_pVertices;
  hr = GetApp()->GetDevice()->CreateBuffer( &bd, &InitData, &m_pVertexBuffer );
  if(FAILED(hr))
  {
    MessageBox(NULL, TEXT("Creating vertex buffer failed!"), TEXT("Error!"), MB_OK);
    return false;
  }

  // Create index buffer
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(unsigned short) * m_iNumTris * 3;
  bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;
  InitData.pSysMem = m_pIndices;
  hr = GetApp()->GetDevice()->CreateBuffer( &bd, &InitData, &m_pIndexBuffer );
  if(FAILED(hr))
  {
    MessageBox(NULL, TEXT("Creating index buffer failed!"), TEXT("Error!"), MB_OK);
    return false;
  }

  return true;
}

extern int g_iTrisPerFrame;

void Mesh_D3D11::Draw(void)
{
#if 0
  // Set vertex buffer
  unsigned int iOffset = 0;
  GetApp()->GetContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_iVertexSize, &iOffset);

  // Set index buffer
  GetApp()->GetContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

  // Set primitive topology
  GetApp()->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  // Set the input layout
  GetApp()->GetContext()->IASetInputLayout(m_pVertexLayout);

  // Draw
  GetApp()->GetContext()->DrawIndexed(m_iNumTris * 3, 0, 0);
#endif

  noMat4 tm(worldTM_.rotate_ * worldTM_.scale_, worldTM_.trans_);
  tm.TransposeSelf();	

  GetApp()->drawModel(m_rd, ToMat4(tm), ViewMat(), ProjMat());

  g_iTrisPerFrame += m_iNumTris;
}

void Mesh_D3D11::DrawInstanced(int iNumInstances)
{
  // Set vertex buffer
  unsigned int iOffset = 0;
  GetApp()->GetContext()->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_iVertexSize, &iOffset);

  // Set index buffer
  GetApp()->GetContext()->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

  // Set primitive topology
  GetApp()->GetContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  // Set the input layout
  GetApp()->GetContext()->IASetInputLayout(m_pVertexLayout);

  // Draw
  GetApp()->GetContext()->DrawIndexedInstanced(m_iNumTris * 3, iNumInstances, 0, 0, 0);

  g_iTrisPerFrame += m_iNumTris;
}

