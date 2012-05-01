/************************************************************************
module	:	U2Variable
Author	:	Yun sangyong
Desc	:	
************************************************************************/
#pragma once 
#ifndef U2_VARIABLE_H
#define U2_VARIABLE_H

//------------------------------------------------------------------------------
/**
@class nVariable
@ingroup NebulaVariableSystem

A variable contains typed data and is identified by a variable handle.

(C) 2002 RadonLabs GmbH
*/
#include <d3dx9math.h>
#include <U2lIB/Src/U2DataType.h>
#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>

//------------------------------------------------------------------------------

class U2_3D U2Variable : public U2RefObject
{
public:
	/// variable types
	enum Type
	{
		Void,
		Int,
		Float,
		Float4,
		String,
		Object,
		Matrix,
		HandleVal,
		Vector4
	};

	typedef U2FourCC Handle;
	enum
	{
		InvalidHandle = 0xffffffff
	};

	/// default constructor
	U2Variable();
	/// int constructor
	U2Variable(Handle h, int val);
	/// float constructor
	U2Variable(Handle h, float val);
	/// float4 constructor
	U2Variable(Handle h, const U2Float4& val);
	/// string constructor
	U2Variable(Handle h, const TCHAR* str);
	/// object constructor
	U2Variable(Handle h, void* ptr);
	/// matrix constructor
	U2Variable(Handle h, const D3DXMATRIX& val);
	/// D3DXVECTOR4 constructor
	U2Variable(Handle h, const D3DXVECTOR4& val);
	/// handle constructor
	U2Variable(Handle h, Handle val);
	/// constructor
	U2Variable(Type t, Handle h);
	/// copy constructor
	U2Variable(const U2Variable& rhs);
	/// destructor
	~U2Variable();
	/// assignment operator
	U2Variable& operator=(const U2Variable& rhs);
	/// clear content
	void Clear();
	/// set variable type
	void SetType(Type t);
	/// get variable type
	Type GetType() const;
	/// set variable handle
	void SetHandle(Handle h);
	/// get variable handle
	Handle GetHandle() const;
	/// set int content
	void SetInt(int val);
	/// get int content
	int GetInt() const;
	/// set float content
	void SetFloat(float val);
	/// get float content
	float GetFloat() const;
	/// set float4 content
	void SetFloat4(const U2Float4& v);
	/// get float4 content
	const U2Float4& GetFloat4() const;
	/// set string content (will be copied internally)
	void SetString(const TCHAR* str);
	/// get string content
	const TCHAR* GetString() const;
	/// set object content
	void SetObj(void* ptr);
	/// get object context
	void* GetObj() const;
	/// set matrix content
	void SetMatrix(const D3DXMATRIX& val);
	/// get matrix content
	const D3DXMATRIX& GetMatrix() const;
	/// set D3DXVECTOR4 content
	void SetVector4(const D3DXVECTOR4& val);
	/// get D3DXVECTOR4 content
	const D3DXVECTOR4& GetVector4() const;
	/// set handle content
	void SetHandleVal(Handle h);
	/// get handle content
	Handle GetHandleVal() const;

private:
	/// delete content
	void Delete();
	/// copy content
	void Copy(const U2Variable& from);

	Handle handle;
	Type type;
	union
	{
		int intVal;
		float floatVal;
		U2Float4 float4Val;
		const TCHAR* stringVal;
		void* objectVal;
		D3DXMATRIX* matrixVal;
		Handle handleVal;
	};
};

typedef U2SmartPtr<U2Variable> U2VariablePtr;

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::U2Variable() :
handle(InvalidHandle),
type(Void),
stringVal(0)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::U2Variable(Type t, Handle h) :
handle(h),
type(t),
stringVal(0)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2Variable::Delete()
{
	if ((String == this->type) && (this->stringVal))
	{
		U2_FREE((void*) this->stringVal);
		this->stringVal = 0;
	}
	else if ((Matrix == this->type) && (this->matrixVal))
	{
		U2_FREE(this->matrixVal);
		this->matrixVal = 0;
	}
	this->handle = U2Variable::InvalidHandle;
	this->type = Void;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2Variable::Copy(const U2Variable& from)
{
	this->handle   = from.handle;
	this->type     = from.type;
	switch (from.type)
	{
	case Int:
		this->intVal = from.intVal;
		break;

	case Float:
		this->floatVal = from.floatVal;
		break;

	case Vector4:
	case Float4:
		this->float4Val = from.float4Val;
		break;

	case String:
		U2ASSERT(0 == this->stringVal);
		if (from.stringVal)
		{
			this->stringVal = U2Strdup(from.stringVal);
		}
		break;

	case Object:
		this->objectVal = from.objectVal;
		break;

	case Matrix:
		U2ASSERT(0 == this->matrixVal);
		if (from.matrixVal)
		{
			this->matrixVal = U2_ALLOC(D3DXMATRIX, 1);
			*(this->matrixVal) = *(from.matrixVal);
		}
		break;

	case HandleVal:
		this->handleVal = from.handleVal;
		break;

	default:
		U2ASSERT(false);
		break;
	}
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::U2Variable(const U2Variable& rhs) :
stringVal(0)
{
	this->Copy(rhs);
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::~U2Variable()
{
	this->Delete();
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable&
U2Variable::operator=(const U2Variable& rhs)
{
	this->Delete();
	this->Copy(rhs);
	return (*this);
}

//------------------------------------------------------------------------------
/**
Clear content, this resets the variable type, handle and content.
*/
inline
void
U2Variable::Clear()
{
	this->Delete();
}

//------------------------------------------------------------------------------
/**
Set the variable's data type. This can only be invoked on a
new or cleared variable object.
*/
inline
void
U2Variable::SetType(Type t)
{
	U2ASSERT(Void == this->type);
	U2ASSERT(Void != t);
	this->type = t;
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::Type
U2Variable::GetType() const
{
	return this->type;
}

//------------------------------------------------------------------------------
/**
Set the variable's handle, this can only be invoked on a new or cleared
variable object.
*/
inline
void
U2Variable::SetHandle(Handle h)
{
	U2ASSERT(InvalidHandle != h);
	U2ASSERT(InvalidHandle == this->handle);
	this->handle = h;
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::Handle
U2Variable::GetHandle() const
{
	return this->handle;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2Variable::SetInt(int val)
{
	U2ASSERT(Int == this->type);
	this->intVal = val;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
U2Variable::GetInt() const
{
	U2ASSERT(Int == this->type);
	return this->intVal;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2Variable::SetFloat(float val)
{
	U2ASSERT(Float == this->type);
	this->floatVal = val;
}

//------------------------------------------------------------------------------
/**
*/
inline
float
U2Variable::GetFloat() const
{
	U2ASSERT(Float == this->type);
	return this->floatVal;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2Variable::SetFloat4(const U2Float4& val)
{
	U2ASSERT(Float4 == this->type);
	this->float4Val = val;
}

//------------------------------------------------------------------------------
/**
*/
inline
const U2Float4&
U2Variable::GetFloat4() const
{
	U2ASSERT(Float4 == this->type);
	return this->float4Val;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2Variable::SetVector4(const D3DXVECTOR4& val)
{
	U2ASSERT(Vector4 == this->type);
	*(D3DXVECTOR4*)&this->float4Val  = val;
}

//------------------------------------------------------------------------------
/**
*/
inline
const D3DXVECTOR4&
U2Variable::GetVector4() const
{
	return *(D3DXVECTOR4*)&this->float4Val;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2Variable::SetString(const TCHAR* str)
{
	U2ASSERT(String == this->type);
	U2ASSERT(str);
	if (this->stringVal)
	{
		U2_FREE((void*) this->stringVal);
		this->stringVal = 0;
	}
	this->stringVal = U2Strdup(str);
}

//------------------------------------------------------------------------------
/**
*/
inline
const TCHAR*
U2Variable::GetString() const
{
	U2ASSERT(String == this->type);
	return this->stringVal;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2Variable::SetObj(void* ptr)
{
	U2ASSERT(Object == this->type);
	this->objectVal = ptr;
}

//------------------------------------------------------------------------------
/**
*/
inline
void*
U2Variable::GetObj() const
{
	U2ASSERT(Object == this->type);
	return this->objectVal;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2Variable::SetMatrix(const D3DXMATRIX& val)
{
	U2ASSERT(Matrix == this->type);
	if (0 == this->matrixVal)
	{
		this->matrixVal = U2_ALLOC(D3DXMATRIX, 1);
	}
	*(this->matrixVal) = val;
}

//------------------------------------------------------------------------------
/**
*/
inline
const D3DXMATRIX&
U2Variable::GetMatrix() const
{
	U2ASSERT(Matrix == this->type);
	U2ASSERT(this->matrixVal);
	return *(this->matrixVal);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2Variable::SetHandleVal(Handle val)
{
	U2ASSERT(HandleVal == this->type);
	this->handleVal = val;
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::Handle
U2Variable::GetHandleVal() const
{
	U2ASSERT(HandleVal == this->type);
	return this->handleVal;
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::U2Variable(Handle h, int val) :
handle(h),
type(Int),
intVal(val)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::U2Variable(Handle h, float val) :
handle(h),
type(Float),
floatVal(val)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::U2Variable(Handle h, const U2Float4& val) :
handle(h),
type(Float4)
{
	this->SetFloat4(val);
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::U2Variable(Handle h, const D3DXVECTOR4& val) :
handle(h),
type(Vector4)
{
	this->SetVector4(val);
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::U2Variable(Handle h, const TCHAR* str) :
handle(h),
type(String),
stringVal(0)
{
	this->SetString(str);
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::U2Variable(Handle h, void* ptr) :
handle(h),
type(Object),
objectVal(ptr)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::U2Variable(Handle h, const D3DXMATRIX& val) :
handle(h),
type(Matrix),
matrixVal(0)
{
	this->SetMatrix(val);
}
//------------------------------------------------------------------------------
/**
*/
inline
U2Variable::U2Variable(Handle h, Handle val) :
handle(h),
type(HandleVal),
handleVal(val)
{
	// empty
}

//------------------------------------------------------------------------------
#endif