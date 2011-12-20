/**************************************************************************************************
module	:	U2VertexBufferData
Author	:	Yun sangyong
Desc	:	D3D Vertex Buffer�� ���� ������ ��ŷ Ŭ����
*************************************************************************************************/
#pragma once 
#ifndef U2_DX9_VERTEXBUFFERDATA_H
#define U2_DX9_VERTEXBUFFERDATA_H


class U2_3D U2VertexBufferData : U2Object
{
	
public:
	U2VertexBufferData(U2VertexAttributes* pAttb, uint32 uVertexCnt);
	virtual ~U2VertexBufferData();

	float* Position(int i);
	float*



private:
	U2VertexBufferData();


	U2VertexAttributes* m_pAttb;

	uint32 m_uVertexStride;

	int m_uVertCount;

	int m_iTotalBytesInSize;
	float* m_pfData;	

};


#endif
