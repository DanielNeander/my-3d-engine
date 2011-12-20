/**************************************************************************************************
module	:	U2StreamInstatnce
Author	:	Yun sangyong
Desc	:
*************************************************************************************************/
#pragma once
#ifndef U2_STREAMINSTANCE_H
#define U2_STREAMINSTANCE_H

#include <U2Lib/src/U2RefObject.h>
#include "U2FxShaderParam.h"
#include "U2ShaderArg.h"

class U2_3D U2StreamInstance : public U2RefObject
{
public:
	class Component : public U2MemObj
	{
	public:
		Component();

		Component(U2ShaderArg::ArgType eType, 
			U2FxShaderState::Param eParam);

		uint16 GetOffect() const;

		U2ShaderArg::ArgType GetType() const;
		U2FxShaderState::Param GetSCM() const;

	private:
		void SetOffset(uint16 offset);

		friend class U2StreamInstance;

		U2ShaderArg::ArgType m_eType;
		U2FxShaderState::Param m_eParam;
		uint16 m_usOffset;
	};

	typedef U2PrimitiveVec<Component*> Declaration;

	enum LockFlags 
	{
		LF_READ = ( 1 << 0), 
		LF_WRITE = ( 1 << 1),
		LF_APPEND = ( 1 << 2)
	};

	U2StreamInstance(); 
	virtual ~U2StreamInstance();

	void SetDeclaration(Declaration* compArray);
	const Declaration& GetDeclaration() const;

	int GetStride() const;

	int GetCurrSize() const;

	float* Lock(int flags);
	void Unlock();
	bool IsLocked() const;
	void WriteMatrix44(const D3DXMATRIX& val);
	void WriteFloat4(const U2Float4& val);
	void WriteVector3(const D3DXVECTOR3& val);
	void WriteFloat(float val);
	const D3DXMATRIX& ReadMatrix44();
	const U2Float4& ReadFloat4();
	float ReadFloat();

	virtual int GetByteSize();

	bool LoadResource();
	void UnloadResource();

protected:
	Declaration* m_pStreamDecl;
	int m_iLockFlags;
	int m_iStreamStride;
	float* m_fData;
	U2PrimitiveVec<float> m_streamArray;
};

#include "U2StreamInstance.inl"

#endif 