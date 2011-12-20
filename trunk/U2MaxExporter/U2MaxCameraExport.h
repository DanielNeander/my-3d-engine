#pragma once 
#ifndef U2_MAXCAMERAEPXORT_H
#define U2_MAXCAMERAEPXORT_H


#include "U2MaxLib.h"

class U2MaxCameraExport : public U2MemObj
{
public:
	U2MaxCameraExport(TimeValue animStart, TimeValue animEnd);
	//virtual ~U2MaxCameraExport();

	void Export(Interface* pIf, INode* pMaxNode, Object *obj);

	void ExtractFromViewport(Interface* pIf);

	const D3DXVECTOR3& GetEyePos() const { return m_eyePos; }
	const D3DXVECTOR3& GetEyeDir() const { return m_eyeDir; }
	const D3DXVECTOR3& GetEyeUp() const { return m_eyeUp; }

protected:
	D3DXVECTOR3 m_eyePos;
	D3DXVECTOR3 m_eyeDir;
	D3DXVECTOR3 m_eyeUp;

	TimeValue m_animStart, m_animEnd;
};




#endif 