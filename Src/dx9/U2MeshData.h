/************************************************************************
module	:	U2MeshData
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef	U2_MESHDATA_H
#define	U2_MESHDATA_H

#include <U2_3D/src/U2_3DLibType.h>
#include <U2_3d/src/Main/U2Object.h>
#include <U2Lib/Src/U2SmartPtr.h>

class U2VertexAttributes;

class U2_3D U2MeshData : public U2Object
{		
	DECLARE_RTTI;
public:	


	virtual ~U2MeshData();

	virtual void SetActiveVertexCount(unsigned short usActive);
	unsigned short GetActiveVertexCount() const;

	enum MeshState
	{
		STATIC_MESH = 0x0001, 
		DYNAMIC_MESH = 0x0002,	// 매 프레임 변한다.
		MESH_PROP_MASK = 0xffff
	};

		
	void SetSoftwareVP(bool bSoftwareVP);
	bool GetSoftwareVP() const;

	unsigned int GetFVF() const;
	LPDIRECT3DVERTEXDECLARATION9 GetVertexDeclaration() const;
	unsigned int GetStreamCount() const;

	unsigned int GetVertexStride(unsigned int uiIndex) const;


	// IndexBuffer data
	unsigned int GetIndexCount() const;
	unsigned int GetIBSize() const;
	LPDIRECT3DINDEXBUFFER9 GetIB() const;
	unsigned int GetBaseVertexIndex() const;

	void RemoveIB();


	D3DPRIMITIVETYPE GetPrimitiveType() const;
	unsigned int GetTriCount() const;	
	unsigned int GetNumArrays() const;
	const unsigned short* GetArrayLengths() const;
	const unsigned short* GetIndexArray() const;
	void SetIndexArray(unsigned short* pIdxArray);
	void SetArrayLengths(unsigned short* pIdxLengths);


	void SetMeshState(MeshState eState);
	MeshState GetMeshState() const;

	void CreateVertices();
	unsigned short GetVertexCount() const;
	D3DXVECTOR3* GetVertices();
	const D3DXVECTOR3* GetVertices() const;

	void CreateNormals(bool bInit);
	D3DXVECTOR3* GetNormals();
	const D3DXVECTOR3* GetNormals() const;
	
	void CreateColors();
	D3DXCOLOR* GetColors();
	const D3DXCOLOR* GetColors() const;

	void CreateTexCoords(unsigned short texcoordIdx);
	D3DXVECTOR2* GetTexCoordSet(
		unsigned short texcoordIdx);
	const D3DXVECTOR2* GetTexCoordSet(
		unsigned short texcoordIdx) const;

	D3DXVECTOR2* GetTexCoords();
	const D3DXVECTOR2* GetTexCoords() const;
	void AppendTexCoords(D3DXVECTOR2* pAddedTexCoords);
	unsigned short GetTexCoordSetCnt() const;


	void SetStreamCount(unsigned int uiStreamCount);
	void SetFVF(unsigned int uiFVF);
	void SetVertexDeclaration(LPDIRECT3DVERTEXDECLARATION9 hDecl);

	void SetVertexCount(unsigned int uiVertCount, unsigned int uiMaxVertCount);

	inline void SetIB(LPDIRECT3DINDEXBUFFER9 pkIB, 
		unsigned int uiIndexCount, unsigned int uiIBSize);

	inline void SetType(D3DPRIMITIVETYPE ePrimType);

	inline void SetIndices(unsigned int uiTriCount, 
		unsigned int uiMaxTriCount, unsigned short* pusIndexArray, 
		unsigned short* pusArrayLengths, unsigned int uiNumArrays);

	
	inline void SetVertexStride(unsigned int uiStream, 
		unsigned int uiStride);

	inline void SetBaseVertexIndex(
		unsigned int uiBaseVertexIndex);

	void SetVertexAttbs(U2VertexAttributes* pAttb);
	U2VertexAttributes* GetVertexAttbs() const;	

protected:
	U2MeshData(unsigned int uiNumVerts, D3DXVECTOR3* pVert, 
		D3DXVECTOR3 *pNormal, D3DXCOLOR* pColor, D3DXVECTOR2* pTexCoords,
		unsigned short numTexCoordSets);

	U2MeshData();

			
	unsigned short m_usDirtyFlags;

	unsigned int m_uiActiveVerts;

	unsigned int m_uiNumVerts;
	unsigned short m_usNumTextureSets;
	D3DXVECTOR3* m_pVerts;
	D3DXVECTOR3* m_pNorms;
	D3DXCOLOR* m_pColors;
	D3DXVECTOR2* m_pTexCoords;

	unsigned int m_uStreamCnt;
	unsigned int* m_puVertexStride;

	unsigned int m_uFVF;
	LPDIRECT3DVERTEXDECLARATION9 m_pVertDecl;
	bool m_bSoftwareVP;

	U2VertexAttributes* m_pVertsAttb;

	unsigned int m_uIndexCnt;
	unsigned int m_uIBSize;
	LPDIRECT3DINDEXBUFFER9 m_pIB;
	unsigned int m_uBaseVertIdx;

	D3DPRIMITIVETYPE m_eType;

	unsigned int m_uTriCnt;
	unsigned int m_uMaxTriCnt;

	unsigned int m_uiNumArrays;
	unsigned short* m_pusArrayLengths;
	unsigned short* m_pusIndexArray;

};

#include "U2MeshData.inl"

typedef U2SmartPtr<U2MeshData> U2MeshDataPtr;

#endif