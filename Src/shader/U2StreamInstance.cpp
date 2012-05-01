#include <U2_3D/src/U23DLibPCH.h>
#include "U2StreamInstance.h"
#include <U2_3D/src/Dx9/U2Dx9Renderer.h>

U2StreamInstance::U2StreamInstance()
	:m_pStreamDecl(0),
	m_iLockFlags(0),
	m_iStreamStride(0),
	m_fData(0),
	m_streamArray(0, 128)
{


}


U2StreamInstance::~U2StreamInstance()
{
	UnloadResource();
}


bool U2StreamInstance::LoadResource()
{
	U2ASSERT(this->m_pStreamDecl->Size() > 0);
	U2ASSERT(!this->IsLocked());
	U2ASSERT(0 == this->m_fData);
	U2ASSERT(0 == this->m_streamArray.Size());

	int i;
	this->m_iStreamStride = 0;
	for(i=0; i < this->m_pStreamDecl->Size(); ++i)
	{
		this->m_pStreamDecl->GetElem(i)->m_usOffset = this->m_iStreamStride;
		switch(m_pStreamDecl->GetElem(i)->m_eType)
		{
		case U2ShaderArg::AT_FLOAT:			this->m_iStreamStride++; break;
		case U2ShaderArg::AT_FLOAT4:		this->m_iStreamStride += 4; break;
		case U2ShaderArg::AT_MATRIX44:		this->m_iStreamStride += 16; break;
		default:
			U2ASSERT(_T("U2StreamInstatnce: Invalid data type in stream declaration!"));
			break;			
		}
	}

	return true;
}

void U2StreamInstance::UnloadResource()
{
	U2ASSERT(!this->IsLocked());
	U2ASSERT(0 == this->m_fData);

	this->m_streamArray.RemoveAll();
	this->m_iStreamStride = 0;
}


int U2StreamInstance::GetByteSize()
{
	return this->m_iStreamStride * this->m_streamArray.FilledSize() * sizeof(float);
}

float* U2StreamInstance::Lock(int flags)
{
	U2ASSERT(!this->IsLocked());
	U2ASSERT(0 != flags);
	U2ASSERT(0 == this->m_fData);
	
	if(flags & LF_WRITE)
	{
		this->m_streamArray.RemoveAll();
	}
	else if(flags & LF_READ)
	{
		U2ASSERT(0 == (flags & LF_APPEND));
		this->m_fData = this->m_streamArray.GetBase();
	}
	this->m_iLockFlags = flags;
	return this->m_fData;
}


void 
U2StreamInstance::Unlock()
{
	U2ASSERT(this->IsLocked());
	if(LF_READ & this->m_iLockFlags)
	{
		U2ASSERT(this->m_fData <= this->m_fData + this->m_streamArray.FilledSize());
	}
	m_fData = 0;
	m_iLockFlags = 0;
}

