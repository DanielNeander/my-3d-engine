/************************************************************************
module	:	U2VariableMgr
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_VARIABLEMGR_H
#define U2_VARIABLEMGR_H

//------------------------------------------------------------------------------
/**
@class U2VariableServer
@ingroup NebulaVariableSystem

The variable server implements a registry for named, typed variables,
and offers variable context object. The name registry guarantees a
systemwide consistent mapping between variable names and handles.
Variables are usually accessed by through their handle, as this is
much faster.

See also @ref N2ScriptInterface_U2VariableServer

(C) 2002 RadonLabs GmbH
*/

#include <U2Lib/Src/U2RefObject.h>
#include <U2Lib/Src/U2SmartPtr.h>
#include <U2Lib/Src/U2TVec.h>
#include "U2variable.h"
//#include "U2Variablecontext.h"


//------------------------------------------------------------------------------

class U2_3D U2VariableMgr : public U2RefObject
{
public:
	/// constructor
	U2VariableMgr();
	/// destructor
	virtual ~U2VariableMgr();
	/// return instance pointer
	static U2VariableMgr* Instance();

	static void Terminate();

	/// declare a variable
	U2Variable::Handle DeclareVariable(const TCHAR* varName, U2FourCC fourcc);
	/// get number of variable declarations
	int GetNumVariables() const;
	/// get variable declaration attributes at given index
	void GetVariableAt(int index, const TCHAR*& varName, U2FourCC& varFourCC);
	/// get a variable handle by name, if variable undeclared, create it
	U2Variable::Handle GetVariableHandleByName(const TCHAR* varName);
	/// get a variable handle by fourcc, if variable undeclared, create it
	U2Variable::Handle GetVariableHandleByFourCC(U2FourCC fourcc);
	/// find variable handle by name
	U2Variable::Handle FindVariableHandleByName(const TCHAR* varName) const;
	/// find variable handle by fourcc code
	U2Variable::Handle FindVariableHandleByFourCC(U2FourCC fourcc) const;
	/// get the name of a variable from its handle
	const TCHAR* GetVariableName(U2Variable::Handle varHandle);
	/// get the fourcc code of a variable from its handle
	U2FourCC GetVariableFourCC(U2Variable::Handle varHandle);

	/// get the global variable context
	const U2VariableContext& GetGlobalVariableContext() const;
	/// set a global variable
	void SetGlobalVariable(const U2Variable& var);
	/// get a global variable by handle
	const U2Variable* GetGlobalVariable(U2Variable::Handle varHandle) const;
	/// get global variable by name
	const U2Variable* GetGlobalVariable(const TCHAR* varName) const;
	/// return true if global variable exists by handle
	bool GlobalVariableExists(U2Variable::Handle varHandle) const;
	/// return true if global variable exists by name
	bool GlobalVariableExists(const TCHAR* varName) const;
	/// get the float value of a global variable by handle
	float GetFloatVariable(U2Variable::Handle varHandle) const;
	/// get float value of global variable by name
	float GetFloatVariable(const TCHAR* varName) const;
	/// get the D3DXVECTOR4 value of a global variable by handle
	const D3DXVECTOR4& GetVectorVariable(U2Variable::Handle varHandle) const;
	/// get D3DXVECTOR4 value of a global variable by name
	const D3DXVECTOR4& GetVectorVariable(const TCHAR* varName) const;
	/// get the integer value of a global variable by handle
	int GetIntVariable(U2Variable::Handle varHandle) const;
	/// get the integer value of a global by name
	int GetIntVariable(const TCHAR* varName) const;
	/// get the string value of a gloabl variable by handle
	const TCHAR* GetStringVariable(U2Variable::Handle varHandle) const;
	/// get the string value of a global variable by name
	const TCHAR* GetStringVariable(const TCHAR* varName) const;
	/// set the float value of a global variable by handle
	void SetFloatVariable(U2Variable::Handle varHandle, float v);
	/// set the float value of a global variable by name
	void SetFloatVariable(const TCHAR* varName, float v);
	/// set the D3DXVECTOR4 value of a global variable by handle
	void SetVectorVariable(U2Variable::Handle varHandle, const D3DXVECTOR4& v);
	/// set the D3DXVECTOR4 value of a global variable by name
	void SetVectorVariable(const TCHAR* varName, const D3DXVECTOR4& v);
	/// set the integer value of a global variable by handle
	void SetIntVariable(U2Variable::Handle varHandle, int i);
	/// set the integer value of a global variable by handle
	void SetIntVariable(const TCHAR* varName, int i);
	/// set the string value of a global variable by handle
	void SetStringVariable(U2Variable::Handle varHandle, const TCHAR* s);
	/// set the string value of a global variable by name
	void SetStringVariable(const TCHAR* varName, const TCHAR* s);

	/// convert a string to a fourcc code
	static U2FourCC StringToFourCC(const TCHAR* str);
	/// convert a fourcc code to a string
	static const TCHAR* FourCCToString(U2FourCC, TCHAR* buf, int bufSize);

private:
	static U2VariableMgr* Singleton;

	class VariableDeclaration : public U2MemObj
	{
	public:
		/// default constructor
		VariableDeclaration();
		/// constructor
		VariableDeclaration(const TCHAR* name, U2FourCC fourcc);
		/// constructor with name only (fourcc will be invalid)
		VariableDeclaration(const TCHAR* name);
		/// constructor with fourcc only (name will be invalid)
		VariableDeclaration(U2FourCC fourcc);
		/// get name
		const TCHAR* GetName() const;
		/// get fourcc code
		U2FourCC GetFourCC() const;
		/// check if variable name is valid
		bool IsNameValid() const;
		/// check if variable fourcc is valid
		bool IsFourCCValid() const;

		U2DynString name;
		U2FourCC fourcc;
	};
//
	U2VariableContextPtr m_spGlobalVarCxt;
	U2PrimitiveVec<VariableDeclaration*> registry;
};

typedef U2SmartPtr<U2VariableMgr> U2VariableMgrPtr;

//------------------------------------------------------------------------------
/*
*/
inline
U2VariableMgr*
U2VariableMgr::Instance()
{
	U2ASSERT(Singleton);
	return Singleton;
}

//------------------------------------------------------------------------------
/*
*/
inline
U2VariableMgr::VariableDeclaration::VariableDeclaration() :
fourcc(0)
{
	// empty
}

//------------------------------------------------------------------------------
/*
*/
inline
U2VariableMgr::VariableDeclaration::VariableDeclaration(const TCHAR* n) :
name(n),
fourcc(0)
{
	// empty
}

//------------------------------------------------------------------------------
/*
*/
inline
U2VariableMgr::VariableDeclaration::VariableDeclaration(U2FourCC fcc) :
fourcc(fcc)
{
	// empty
}

//------------------------------------------------------------------------------
/*
*/
inline
U2VariableMgr::VariableDeclaration::VariableDeclaration(const TCHAR* n, U2FourCC fcc) :
name(n),
fourcc(fcc)
{
	// empty
}

//------------------------------------------------------------------------------
/*
*/
inline
const TCHAR*
U2VariableMgr::VariableDeclaration::GetName() const
{
	return this->name.IsEmpty() ? 0 : this->name.Str();
}

//------------------------------------------------------------------------------
/*
*/
inline
U2FourCC
U2VariableMgr::VariableDeclaration::GetFourCC() const
{
	return this->fourcc;
}

//------------------------------------------------------------------------------
/*
*/
inline
bool
U2VariableMgr::VariableDeclaration::IsNameValid() const
{
	return !(this->name.IsEmpty());
}

//------------------------------------------------------------------------------
/*
*/
inline
bool
U2VariableMgr::VariableDeclaration::IsFourCCValid() const
{
	return (0 != this->fourcc);
}

//------------------------------------------------------------------------------
/*
Converts a string to a fourcc code.
*/
inline
U2FourCC
U2VariableMgr::StringToFourCC(const TCHAR* str)
{
	U2ASSERT(str);

	// create a valid FourCC even if the string is not,
	// fill invalid stuff with spaces
	TCHAR c[4] = {' ',' ',' ',' '};
	int i;
	for (i = 0; i < 4; i++)
	{
		if (0 == str[i])
		{
			break;
		}
		else if (isalnum(str[i]))
		{
			c[i] = str[i];
		}
	}
	return MAKE_FOURCC(c[0], c[1], c[2], c[3]);
}

//------------------------------------------------------------------------------
/*
Convertes a fourcc code to a string.
*/
inline
const TCHAR*
U2VariableMgr::FourCCToString(U2FourCC fourcc, TCHAR* buf, int bufSize)
{
	U2ASSERT(bufSize >= 5);
	buf[0] = (fourcc)     & 0xff;
	buf[1] = (fourcc>>8)  & 0xff;
	buf[2] = (fourcc>>16) & 0xff;
	buf[3] = (fourcc>>24) & 0xff;
	buf[4] = 0;
	return buf;
}

//------------------------------------------------------------------------------
/**
Get reference to global variable context.
*/
inline
const U2VariableContext&
U2VariableMgr::GetGlobalVariableContext() const
{
	return *m_spGlobalVarCxt;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
U2VariableMgr::GlobalVariableExists(U2Variable::Handle varHandle) const
{
	return (0 != this->m_spGlobalVarCxt->GetVariable(varHandle));
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
U2VariableMgr::GlobalVariableExists(const TCHAR* varName) const
{
	return this->GlobalVariableExists(this->FindVariableHandleByName(varName));
}

//------------------------------------------------------------------------------
/*
*/
inline
void
U2VariableMgr::SetGlobalVariable(const U2Variable& var)
{
	this->m_spGlobalVarCxt->SetVariable(var);
}

//------------------------------------------------------------------------------
/*
*/
inline
const U2Variable*
U2VariableMgr::GetGlobalVariable(U2Variable::Handle varHandle) const
{
	return this->m_spGlobalVarCxt->GetVariable(varHandle);
}

//------------------------------------------------------------------------------
/*
*/
inline
const U2Variable*
U2VariableMgr::GetGlobalVariable(const TCHAR* varName) const
{
	return this->GetGlobalVariable(this->FindVariableHandleByName(varName));
}

//------------------------------------------------------------------------------
/*
*/
inline
float
U2VariableMgr::GetFloatVariable(const TCHAR* varName) const
{
	return this->GetFloatVariable(this->FindVariableHandleByName(varName));
}

//------------------------------------------------------------------------------
/*
*/
inline
const D3DXVECTOR4&
U2VariableMgr::GetVectorVariable(const TCHAR* varName) const
{
	return this->GetVectorVariable(this->FindVariableHandleByName(varName));
}

//------------------------------------------------------------------------------
/*
*/
inline
int
U2VariableMgr::GetIntVariable(const TCHAR* varName) const
{
	return this->GetIntVariable(this->FindVariableHandleByName(varName));
}

//------------------------------------------------------------------------------
/*
*/
inline
const TCHAR*
U2VariableMgr::GetStringVariable(const TCHAR* varName) const
{
	return this->GetStringVariable(this->FindVariableHandleByName(varName));
}

//------------------------------------------------------------------------------
/*
*/
inline
void
U2VariableMgr::SetFloatVariable(const TCHAR* varName, float v)
{
	this->SetFloatVariable(this->GetVariableHandleByName(varName), v);
}

//------------------------------------------------------------------------------
/*
*/
inline
void
U2VariableMgr::SetVectorVariable(const TCHAR* varName, const D3DXVECTOR4& v)
{
	this->SetVectorVariable(this->GetVariableHandleByName(varName), v);
}

//------------------------------------------------------------------------------
/*
*/
inline
void
U2VariableMgr::SetIntVariable(const TCHAR* varName, int i)
{
	this->SetIntVariable(this->GetVariableHandleByName(varName), i);
}

//------------------------------------------------------------------------------
/*
*/
inline
void
U2VariableMgr::SetStringVariable(const TCHAR* varName, const TCHAR* s)
{
	this->SetStringVariable(this->GetVariableHandleByName(varName), s);
}

//------------------------------------------------------------------------------
#endif