/************************************************************************
module	:	U2VariableContext
Author	:	Yun sangyong
Desc	:
************************************************************************/
#pragma once
#ifndef U2_VARIABLECONTEXT_H
#define U2_VARIABLECONTEXT_H

#include <U2Lib/Src/U2TVec.h>
#include "U2Variable.h"

//------------------------------------------------------------------------------
class U2_3D U2VariableContext : public U2RefObject
{
public:
	/// constructor
	U2VariableContext();
	/// destructor
	~U2VariableContext();
	/// clear the variable context
	void Clear();
	/// add a variable to the context
	void AddVariable(const U2Variable& var);
	/// update existing variable or append new var
	void SetVariable(const U2Variable& var);
	/// get a variable object by its handle
	U2Variable* GetVariable(U2Variable::Handle handle) const;
	/// get number of variables in context
	int GetNumVariables() const;
	/// get variable at index (this is NOT the variable handle!)
	U2Variable& GetVariableAt(int index) const;

private:
	U2ObjVec<U2VariablePtr> varArray;
};

typedef U2SmartPtr<U2VariableContext> U2VariableContextPtr;

//------------------------------------------------------------------------------
/**
*/
inline
U2VariableContext::U2VariableContext() :
varArray(4, 4)
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
inline
U2VariableContext::~U2VariableContext()
{
	// empty
}

//------------------------------------------------------------------------------
/**
*/
inline
void
U2VariableContext::Clear()
{
	this->varArray.RemoveAll();
}

//------------------------------------------------------------------------------
/**
@param  h       the variable handle
@return         pointer to variable object, or 0 if variable doesn't exist
in this context
*/
inline
U2Variable*
U2VariableContext::GetVariable(U2Variable::Handle h) const
{
	int size = this->varArray.FilledSize();
	int i;
	for (i = 0; i < size; i++)
	{
		if (this->varArray[i]->GetHandle() == h)
		{
			return SmartPtrCast(U2Variable, this->varArray[i]);
		}
	}
	// fallthrough: variable not found
	return 0;
}

//------------------------------------------------------------------------------
/**
Add a new variable to the context. The variable MAY NOT YET EXIST
in the context. Check this with GetVariable() first! No checks are
made if a duplicate variable is added to the context!
*/
inline
void
U2VariableContext::AddVariable(const U2Variable& var)
{
	this->varArray.AddElem((U2Variable*)&var);
}

//------------------------------------------------------------------------------
/**
Update value of existing variable, or create new if it doesn't exist.
*/
inline
void
U2VariableContext::SetVariable(const U2Variable& var)
{
	U2Variable* varPtr = this->GetVariable(var.GetHandle());
	if (varPtr)
	{
		*varPtr = var;
	}
	else
	{
		this->AddVariable(var);
	}
}

//------------------------------------------------------------------------------
/**
*/
inline
int
U2VariableContext::GetNumVariables() const
{
	return this->varArray.FilledSize();
}

//------------------------------------------------------------------------------
/**
*/
inline
U2Variable&
U2VariableContext::GetVariableAt(int index) const
{
	return *this->varArray[index];
}

//------------------------------------------------------------------------------
#endif



