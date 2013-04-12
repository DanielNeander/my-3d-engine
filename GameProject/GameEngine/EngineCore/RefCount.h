#ifndef __REFCOUNT_H__
#define __REFCOUNT_H__


class RefCounter //: public VBaseObject
{
public:
	/// \brief
	///   constructor; initialises the counter.
	RefCounter()
	{
		m_iRefCount = 0;
	}

	/// \brief
	///   Copy constructor; restarts new object with a refcount of 0.
	RefCounter( const RefCounter &other )
	{
		m_iRefCount = 0;
	}

	/// \brief
	///   destructor; asserts when the refcounter is not 0.
	virtual ~RefCounter()
	{
//		VASSERT_MSG(m_iRefCount==0 , "Tried to delete a reference counted object with refcount!=0")
	}

	/// \brief
	///   Special version of assignment operator that leaves the refcount unmodified.
	RefCounter& operator=( const RefCounter& other)
	{
		return *this;
	}


	/// \brief
	///   Overridable function that is called when the refcounter reaches zero. The default
	///   implementation calls "delete this".
	virtual void DeleteThis()
	{
		delete this;
	}

	/// \brief
	///   Increases the reference counter, should be called when any object receives a pointer to a
	///   reference counted object.
	inline void AddRef() const
	{
		m_iRefCount++;
	}

	/// \brief
	///   Decreases the reference counter, should be called when objects remove their reference to
	///   this object.
	/// 
	/// Checks that the counter isn't already already zero. If this was the last reference, the
	/// object is deleted.
	inline void Release() const
	{
		//VASSERT(m_iRefCount>0);
		m_iRefCount--;
		if (m_iRefCount==0)
			((RefCounter *)this)->DeleteThis(); ///< DeleteThis isn't const...
	}

	/// \brief
	///   Gets the current number of references. Only for internal/debugging purposes.
	inline int GetRefCount() const
	{ 
		return m_iRefCount;
	}

protected:
	mutable int   m_iRefCount;  ///<reference count
};

template<class C> class MSmartPtr
{
public:

	/// \brief
	///   Empty constructor. Object pointer is set to NULL.
	inline MSmartPtr()
	{
		m_pPtr = NULL;
	}

	/// \brief
	///   Constructor that takes an object reference. For pPtr!=NULL, the reference count of the object is incremented.
	///
	/// \param pPtr
	///   Object reference that should be set. Can be NULL
	inline MSmartPtr(C* pPtr)
	{
		m_pPtr = NULL;
		Set(pPtr);
	}

	/// \brief
	///   Copy constructor. Initializes this smart pointer with the same reference as the passed smart pointer. Reference is increased.
	///
	/// \param other
	///   Object reference that should be set.
	inline MSmartPtr(const MSmartPtr<C>& other)
	{
		m_pPtr = NULL;
		Set(other.GetPtr());
	}

	/// \brief
	///   Destructor. Decreases the reference counter so the object can be deleted eventually (if nothing else holds a reference to the object).
	inline ~MSmartPtr()
	{		
		if (m_pPtr) m_pPtr->Release();
	}


	/// \brief
	///   Assignment operator for the smart pointer.
	///
	/// \param other
	///   New Object reference that should be set
	///
	/// \returns
	///   This instance
	///
	/// This assignment gracefully increases the reference of the new object and decreases the reference of the old instance
	inline MSmartPtr<C>& operator=(const MSmartPtr<C> &other)
	{
		Set(other.GetPtr());
		return *this;
	}

	/// \brief
	///   Assignment operator for the smart pointer.
	///
	/// \param pPtr
	///   New Object reference that should be set
	///
	/// \returns
	///   This instance
	///
	/// This assignment gracefully increases the reference of the new object and decreases the reference of the old instance
	inline MSmartPtr<C>& operator=(C *pPtr)
	{
		Set(pPtr);
		return *this;
	}

	/// \brief
	///   Assigns an object to the smart pointer.
	///
	/// \param pPtr
	///   New Object reference that should be set
	///
	/// This assignment gracefully increases the reference of the new object and decreases the reference of the old instance
	inline void Set(C *pPtr)
	{
		if (pPtr)
			pPtr->AddRef();
		if (m_pPtr)
			m_pPtr->Release();
		m_pPtr = pPtr;
	}

	/// \brief
	///   Returns the pointer of the object that the smart pointer points to.
	inline operator C*() const
	{
		return m_pPtr;
	}

	/// \brief
	///   Returns the pointer of the object that the smart pointer points to.
	inline C* operator->() const
	{
		return m_pPtr;
	}

	/// \brief
	///   Returns the pointer of the object that the smart pointer points to.
	inline C* GetPtr() const
	{
		return m_pPtr;
	}


	C* m_pPtr;  ///< pointer to the object
};

#define MSmartPointer(classname) \
	class classname; \
	typedef MSmartPtr<classname> classname##Ptr
	


#endif