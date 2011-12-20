//-------------------------------------------------------------------------------------------------
inline 
const U2Rtti& U2Object::GetType() const 
{
	return ms_rtti;
}

//-------------------------------------------------------------------------------------------------
inline 
bool U2Object::IsExactly(const U2Rtti& type) const
{
	return GetType().IsExactly(type);
}

//-------------------------------------------------------------------------------------------------
inline 
bool U2Object::IsDerived(const U2Rtti& type) const 
{
	return GetType().IsDerived(type);
}

//-------------------------------------------------------------------------------------------------
inline 
bool U2Object::IsExactlyTypeOf(const U2Object* pObj) const 
{
	return pObj && GetType().IsExactly(pObj->GetType());
}

//-------------------------------------------------------------------------------------------------
inline 
bool U2Object::IsDerivedTypeOf(const U2Object *pObj) const
{
	return pObj && GetType().IsDerived(pObj->GetType());
}

//-------------------------------------------------------------------------------------------------
template <class T>
T* StaticCast (U2Object* pkObj)
{
	return (T*)pkObj;
}

//----------------------------------------------------------------------------
template <class T>
const T* StaticCast (const U2Object* pkObj)
{
	return (const T*)pkObj;
}

//----------------------------------------------------------------------------
template <class T>
T* DynamicCast (U2Object* pkObj)
{
	return pkObj && pkObj->IsDerived(T::ms_rtti) ? (T*)pkObj : 0;
}

//----------------------------------------------------------------------------
template <class T>
const T* DynamicCast (const U2Object* pkObj)
{
	return pkObj && pkObj->IsDerived(T::ms_rtti) ? (const T*)pkObj : 0;
}

//-------------------------------------------------------------------------------------------------
inline void U2Object::SetName(const U2DynString& szName)
{
	m_szName = szName;
}

//-------------------------------------------------------------------------------------------------
inline const U2DynString& U2Object::GetName() const
{
	return m_szName;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2Object::GetUniqueID () const
{
	return m_uiId;
}

//-------------------------------------------------------------------------------------------------
inline unsigned int U2Object::GetNextID ()
{
	return ms_uiNextId;
}

//-------------------------------------------------------------------------------------------------
inline uint32 U2Object::GetNumControllers() const
{
	return m_controllers.FilledSize();
}

//-------------------------------------------------------------------------------------------------
inline U2Controller* U2Object::GetController(uint32 idx) const
{
	return m_controllers[idx];
}

//-------------------------------------------------------------------------------------------------
inline void U2Object::AttachController(U2Controller* pCtrl)
{
	m_controllers.AddElem(pCtrl);
}

