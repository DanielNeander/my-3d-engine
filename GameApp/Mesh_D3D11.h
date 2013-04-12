#pragma once


class Mesh_D3D11 : public Mesh
{
public:
  Mesh_D3D11();
  virtual ~Mesh_D3D11();

  virtual bool CreateBuffers(void);
  virtual void Draw(void);
  virtual void DrawInstanced(int iNumInstances);  

public:
  ID3D11Buffer *m_pVertexBuffer;
  ID3D11Buffer *m_pIndexBuffer;
  ID3D11InputLayout *m_pVertexLayout;
};

// Creates a new mesh and returns it
extern Mesh *CreateNewMesh(void);