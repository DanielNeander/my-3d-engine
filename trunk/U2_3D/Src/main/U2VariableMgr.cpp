//------------------------------------------------------------------------------
//  U2VariableMgr_main.cc
//  (C) 2002 RadonLabs GmbH
//------------------------------------------------------------------------------
#include <U2_3D/Src/U23DLibPCH.h>
#include "U2VariableMgr.h"
#include "u2variablecontext.h"

U2VariableMgr* U2VariableMgr::Singleton = 0;

//------------------------------------------------------------------------------
/**
*/
U2VariableMgr::U2VariableMgr() :
registry(64, 64),
m_spGlobalVarCxt(0)
{
	U2ASSERT(0 == Singleton);
	Singleton = this;
	m_spGlobalVarCxt = U2_NEW U2VariableContext;
}

//------------------------------------------------------------------------------
/**
*/
U2VariableMgr::~U2VariableMgr()
{
	for(unsigned int i=0; i < registry.FilledSize(); ++i)
	{
		U2_DELETE registry[i];
	}

	registry.RemoveAll();

	m_spGlobalVarCxt = 0;

	U2ASSERT(Singleton);
	Singleton = 0;
}

//-------------------------------------------------------------------------------------------------
void U2VariableMgr::Terminate()
{
	U2_DELETE Singleton;
	Singleton = NULL;
}


//------------------------------------------------------------------------------
/**
Declare a new variable name with it's associated fourcc code. Returns
the handle of the variable.

NOTE: variables which are not declared are declared on the fly
by GetVariableHandleByName() or GetVariableHandleByFourCC(), however,
those functions will let either the name or the fourcc code of the
variable in an invalid state. Only variables which are properly
declared before used will get a valid name AND fourcc code.

@param  varName     name of a new variable
@param  varFourCC   associated fourcc code of variable
@return             variable handle of new variable
*/
U2Variable::Handle
U2VariableMgr::DeclareVariable(const TCHAR* varName, U2FourCC varFourCC)
{
	U2ASSERT(varName);
	U2ASSERT(0 != varFourCC);

	// check if the variable already exists, either by name or by fourcc
	U2Variable::Handle nameHandle = this->FindVariableHandleByName(varName);
	U2Variable::Handle fourccHandle = this->FindVariableHandleByFourCC(varFourCC);
	if ((nameHandle != U2Variable::InvalidHandle) || (fourccHandle != U2Variable::InvalidHandle))
	{
		// variable is already declared, make sure that the declaration is consistent
		if (nameHandle != fourccHandle)
		{
			TCHAR buf[5];
			FDebug("Inconsistent variable declaration: '%s'/'%s'!\n"
				"Either the name or the fourcc code is already used",
				varName, this->FourCCToString(varFourCC, buf, sizeof(buf)));
			return U2Variable::InvalidHandle;
		}
		return nameHandle;
	}
	else
	{
		// a new variable declaration
		VariableDeclaration* newDecl = U2_NEW VariableDeclaration(varName, varFourCC);
		this->registry.AddElem(newDecl);
		return this->registry.FilledSize() - 1;
	}
}

//------------------------------------------------------------------------------
/**
Returns number of variable declarations.

@return     number of variable declarations
*/
int
U2VariableMgr::GetNumVariables() const
{
	return this->registry.FilledSize();
}

//------------------------------------------------------------------------------
/**
Returns variable declaration attributes at given index. The index
is NOT the same as the variable handle.

@param  index       [in] variable index
@param  varName     [out] variable name
@param  varFourCC   [out] variable fourcc code
*/
void
U2VariableMgr::GetVariableAt(int index, const TCHAR*& varName, U2FourCC& varFourCC)
{
	varName   = this->registry[index]->GetName();
	varFourCC = this->registry[index]->GetFourCC();
}

//------------------------------------------------------------------------------
/**
Find a variable handle by variable name. The variable must have been
declared before. Otherwise returns U2Variable::InvalidHandle.

@param  varName     name of a variable
@return             variable handle, or U2Variable::InvalidHandle
*/
U2Variable::Handle
U2VariableMgr::FindVariableHandleByName(const TCHAR* varName) const
{
	U2ASSERT(varName);

	int i;
	int num = this->registry.FilledSize();
	for (i = 0; i < num; i++)
	{
		if (_tcscmp(this->registry[i]->GetName(), varName) == 0)
		{
			return i;
		}
	}
	// fallthrough: not found
	return U2Variable::InvalidHandle;
}

//------------------------------------------------------------------------------
/**
Find a variable handle by the variable's fourcc code. The variable must have been
declared before. Otherwise returns U2Variable::InvalidHandle.

@param  varFourCC   fourcc code of a variable
@return             variable handle, or U2Variable::InvalidHandle
*/
U2Variable::Handle
U2VariableMgr::FindVariableHandleByFourCC(U2FourCC varFourCC) const
{
	U2ASSERT(0 != varFourCC);

	int i;
	int num = this->registry.FilledSize();
	for (i = 0; i < num; i++)
	{
		if (this->registry[i]->GetFourCC() == varFourCC)
		{
			return i;
		}
	}
	// fallthrough: not found
	return U2Variable::InvalidHandle;
}

//------------------------------------------------------------------------------
/**
Get a variable handle by name. If variable doesn't exist yet, declare a
new variable with valid name, but invalid fourcc.
*/
U2Variable::Handle
U2VariableMgr::GetVariableHandleByName(const TCHAR* varName)
{
	U2ASSERT(varName);
	U2Variable::Handle varHandle = this->FindVariableHandleByName(varName);
	if (U2Variable::InvalidHandle != varHandle)
	{
		return varHandle;
	}
	else
	{
		// a new variable declaration
		VariableDeclaration* newDecl = U2_NEW VariableDeclaration(varName);
		this->registry.AddElem(newDecl);
		return this->registry.FilledSize() - 1;
	}
}

//------------------------------------------------------------------------------
/**
Get a variable handle by fourcc. If variable doesn't exist yet, declare a
new variable with valid fourcc, but invalid name.
*/
U2Variable::Handle
U2VariableMgr::GetVariableHandleByFourCC(U2FourCC fourcc)
{
	U2ASSERT(0 != fourcc);
	U2Variable::Handle varHandle = this->FindVariableHandleByFourCC(fourcc);
	if (U2Variable::InvalidHandle != varHandle)
	{
		return varHandle;
	}
	else
	{
		// a new variable declaration
		VariableDeclaration* newDecl = U2_NEW VariableDeclaration(fourcc);
		this->registry.AddElem(newDecl);
		return this->registry.FilledSize() - 1;
	}
}

//------------------------------------------------------------------------------
/**
Return the name for a variable handle.

@param  h       a variable handle
@return         the variable's name
*/
const TCHAR*
U2VariableMgr::GetVariableName(U2Variable::Handle h)
{
	U2ASSERT(U2Variable::InvalidHandle != h);
	return this->registry[h]->GetName();
}

//------------------------------------------------------------------------------
/**
Return the fourcc code for a variable handle.

@param  h       a variable handle
@return         the variable's fourcc code
*/
U2FourCC
U2VariableMgr::GetVariableFourCC(U2Variable::Handle h)
{
	U2ASSERT(U2Variable::InvalidHandle != h);
	return this->registry[h]->GetFourCC();
}


//------------------------------------------------------------------------------
/**
Get float value of a global variable from the global variable
context.
*/
float
U2VariableMgr::GetFloatVariable(U2Variable::Handle varHandle) const
{
	U2Variable* var = this->m_spGlobalVarCxt->GetVariable(varHandle);
	if (var)
	{
		return var->GetFloat();
	}
	else
	{
		const TCHAR* variableName = this->Instance()->GetVariableName(varHandle);
		FDebug("Variable  %s does not exist in global variable context!", variableName);
		return 0;
	}
}

//------------------------------------------------------------------------------
/**
Get vector4 value of a global variable from the global variable
context.
*/
const D3DXVECTOR4&
U2VariableMgr::GetVectorVariable(U2Variable::Handle varHandle) const
{
	U2Variable* var = this->m_spGlobalVarCxt->GetVariable(varHandle);
	if (var)
	{
		return var->GetVector4();
	}
	else
	{
		const TCHAR* variableName = this->Instance()->GetVariableName(varHandle);
		FDebug("Variable '%s' does not exist in global variable context!", variableName);
		static D3DXVECTOR4 zero;
		return zero;
	}
}

//------------------------------------------------------------------------------
/**
Get integer value of a global variable from the global variable
context.
*/
int
U2VariableMgr::GetIntVariable(U2Variable::Handle varHandle) const
{
	U2Variable* var = this->m_spGlobalVarCxt->GetVariable(varHandle);
	if (var)
	{
		return var->GetInt();
	}
	else
	{
		const TCHAR* variableName = this->Instance()->GetVariableName(varHandle);
		FDebug("Variable '%s' does not exist in global variable context!", variableName);
		return 0;
	}
}

//------------------------------------------------------------------------------
/**
Get vector4 value of a global variable from the global variable
context.
*/
const TCHAR*
U2VariableMgr::GetStringVariable(U2Variable::Handle varHandle) const
{
	U2Variable* var = this->m_spGlobalVarCxt->GetVariable(varHandle);
	if (var)
	{
		return var->GetString();
	}
	else
	{
		const TCHAR* variableName = this->Instance()->GetVariableName(varHandle);
		FDebug("Variable '%s' does not exist in global variable context!", variableName);
		return 0;
	}
}

//------------------------------------------------------------------------------
/**
Sets float value of a global variable in the global variable
context.
*/
void
U2VariableMgr::SetFloatVariable(U2Variable::Handle varHandle, float f)
{
	U2Variable* var = this->m_spGlobalVarCxt->GetVariable(varHandle);
	if (var)
	{
		var->SetFloat(f);
	}
	else
	{
		U2Variable* newVar = U2_NEW U2Variable(varHandle, f);
		this->m_spGlobalVarCxt->AddVariable(*newVar);
	}
}

//------------------------------------------------------------------------------
/**
Sets integer value of a global variable in the global variable
context.
*/
void
U2VariableMgr::SetIntVariable(U2Variable::Handle varHandle, int i)
{
	U2Variable* var = this->m_spGlobalVarCxt->GetVariable(varHandle);
	if (var)
	{
		var->SetInt(i);
	}
	else
	{
		U2Variable* newVar = U2_NEW U2Variable(varHandle, i);
		this->m_spGlobalVarCxt->AddVariable(*newVar);
	}
}

//------------------------------------------------------------------------------
/**
Sets vector4 value of a global variable in the global variable
context.
*/
void
U2VariableMgr::SetVectorVariable(U2Variable::Handle varHandle, const D3DXVECTOR4& v)
{
	U2Variable* var = this->m_spGlobalVarCxt->GetVariable(varHandle);
	if (var)
	{
		var->SetVector4(v);
	}
	else
	{
		U2Variable* newVar = U2_NEW U2Variable(varHandle, v);
		this->m_spGlobalVarCxt->AddVariable(*newVar);
	}
}

//------------------------------------------------------------------------------
/**
Sets string value of a global variable in the global variable
context.
*/
void
U2VariableMgr::SetStringVariable(U2Variable::Handle varHandle, const TCHAR* s)
{
	U2Variable* var = this->m_spGlobalVarCxt->GetVariable(varHandle);
	if (var)
	{
		var->SetString(s);
	}
	else
	{
		U2Variable* newVar = U2_NEW U2Variable(varHandle, s);
		this->m_spGlobalVarCxt->AddVariable(*newVar);
	}
}

