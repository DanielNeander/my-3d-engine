#pragma once 

class WeakPointerData : public RefCounter
{
public:
	WeakPointerData(void *pWeakPointerObject)
	{
		m_pWeakPointerObject = pWeakPointerObject;
	}

	// should only be called by the object the weak pointer is pointing to!
	void DestroyWeakPtr()
	{
		m_pWeakPointerObject = NULL;
	}

	void* Get()         { return m_pWeakPointerObject; }

protected:
	void			*m_pWeakPointerObject;
};

typedef MSmartPtr<WeakPointerData>	WeakPointerDataPtr;



class WeakPointerBase
{
public:
	WeakPointerBase()
	{
		m_spData = NULL;
	}

	~WeakPointerBase()
	{
		m_spData =  NULL;
	}

	WeakPointerData *GetWeakPointerData()       
	{ 
		return m_spData; 
	}

protected:
	WeakPointerDataPtr				            m_spData;

	void SetNULL()
	{
		m_spData = NULL;
	}
};

class WeakPointerMaster : public WeakPointerBase
{
public:

	~WeakPointerMaster()
	{
		DestroyWeakPointer();
	}

	// ---------------------------------------------------
	// This function must be called by the object that we want
	// weak pointers to be able to point to
	void SetupWeakPointer(void *pMasterObject)
	{
		assert(m_spData==NULL);
		m_spData = new WeakPointerData( pMasterObject );
	}
	// ---------------------------------------------------

	// ---------------------------------------------------
	// When this function is called, all weak pointers that
	// are pointing to the object will return NULL
	void DestroyWeakPointer()
	{
		assert(m_spData);
		m_spData->DestroyWeakPtr();
		m_spData = NULL;
	}
};

template<class TYPE>
class WeakPointer : public WeakPointerBase
{
public:

	TYPE *operator->() const
	{
		if( !m_spData )
			return NULL;

		return static_cast<TYPE*>(m_spData->Get());
	}

	TYPE& operator*() const
	{
		TYPE *pTemp = static_cast<TYPE*>(m_spData->Get());
		return *pTemp;
	}

	bool operator==(TYPE *other)
	{
		if( !m_spData )
			return false;

		if( other==m_spData->Get() )
			return true;

		return false;
	}

	operator TYPE*() const
	{
		if( !m_spData )
			return NULL;

		return static_cast<TYPE*>(m_spData->Get());
	}

	const WeakPointer&operator=(const WeakPointer<TYPE>&pOther)
	{
		m_spData	= pOther.m_spData;

		return *this;
	}

	// only use this operator to set the weak pointer to NULL
	const WeakPointer& operator=(int i)
	{
		NIASSERT(i==0);

		if( i==0 )
			SetNULL();

		return *this;
	}

};

#define noWeakPointer(classname)                                            \
class classname##WPtr : public WeakPointer<classname>                    \
{                                                                       \
public:                                                                 \
	const classname##WPtr &operator=(classname *pEnt)                    \
{                                                                   \
	if( pEnt==NULL )                                                \
{                                                               \
	m_spData = NULL;                                            \
	return *this;                                               \
}                                                               \
	m_spData = pEnt->GetWeakPointerData();                          \
	assert(m_spData);                                             \
	return *this;                                                   \
}                                                                   \
};                                                                      
