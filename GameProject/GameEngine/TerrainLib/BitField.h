#pragma once 

/// \brief
///   Template class for a bit field
template <int iStaticBitCount> class TBitfield
{
public:

	/// \brief
	///   Constructor that takes the initial size of the bit field
	TBitfield(int iBitCount=0)
	{
		m_iBitCount=0;
		m_pField=m_OwnBits;
		AllocateBitfield(iBitCount);
	}

	/// \brief
	///   Destructor, frees the allocated resources
	~TBitfield() {FreeBitfield();}

	/// \brief
	///   Frees the allocated resources
	inline void FreeBitfield()
	{
		if (m_pField!=m_OwnBits)
		{
			//V_SAFE_DELETE_ARRAY(m_pField);  ///<TODO: VBaseDealloc
			if (m_pField) {
				delete [] m_pField; 
				m_pField = 0;
			}
		}
		m_iBitCount = 0;
		m_pField=m_OwnBits;
	}

	/// \brief
	///   (Re-)allocates the passed number of bits. The bit field is initialized with all bits set to
	///   0 or copied from the passed array
	inline void AllocateBitfield(int iBitCount, const unsigned int *pInitArray=NULL)
	{
		FreeBitfield();
		if (iBitCount<1) return;
		m_iBitCount = iBitCount;
		size_t iIntCount = GetIntCount();
		if (iIntCount>(sizeof(m_OwnBits)/sizeof(m_OwnBits[0])))
			m_pField = new unsigned int[iIntCount];
		if (pInitArray)
			memcpy(m_pField,pInitArray,iIntCount*sizeof(int));
		else
			memset(m_pField,0,iIntCount*sizeof(int));
	}

	/// \brief
	///   Sets all bits to 0
	inline void Clear()
	{
		const int iIntCount = GetIntCount();
		for (int i=0;i<iIntCount;i++) m_pField[i]=0;
	}

	/// \brief
	///   Sets all bits to 1
	inline void SetAll()
	{
		const int iIntCount = GetIntCount();
		for (int i=0;i<iIntCount;i++) m_pField[i]=0xffffffff;
	}

	/// \brief
	///   Inverts all bits
	inline void Invert()
	{
		const int iIntCount = GetIntCount();
		for (int i=0;i<iIntCount;i++) m_pField[i]^=0xffffffff;
	}

	/// \brief
	///   Queries the status of the bit with specified 0-based index
	inline bool IsBitSet(int iBitNumber) const
	{
		//VASSERT(iBitNumber>=0 && iBitNumber<m_iBitCount);
		return (m_pField[iBitNumber>>5] & (1<<(iBitNumber&31)))>0;
	}

	/// \brief
	///   Sets the status of the bit with specified 0-based index to 1
	inline void SetBit(int iBitNumber) const
	{
		//VASSERT(iBitNumber>=0 && iBitNumber<m_iBitCount);
		m_pField[iBitNumber>>5] |= (1<<(iBitNumber&31));
	}

	/// \brief
	///   Clears the status of the bit with specified 0-based index
	inline void RemoveBit(int iBitNumber) const
	{
		//VASSERT(iBitNumber>=0 && iBitNumber<m_iBitCount);
		m_pField[iBitNumber>>5] &= (~(1<<(iBitNumber&31)));
	}

	/// \brief
	///   Evaluates whether any bit is set in the field
	inline bool IsAnyBitSet() const
	{
		const int iIntCount = GetIntCount();
		for (int i=0;i<iIntCount;i++)
			if (m_pField[i])
				return true;
		return false;
	}

	/// \brief
	///   Returns the 0-based index of the first bit that is set, or -1 if no bit is set
	inline int GetIndexOfFirstSetBit() const
	{
		int iIndex = 0;
		const int iIntCount = GetIntCount();
		for (int i=0;i<iIntCount;i++,iIndex+=32) if (m_pField[i])
			for (int j=0;j<32 && (iIndex+j)<m_iBitCount;j++) 
				if (IsBitSet(iIndex+j))
					return iIndex+j;
		return -1;
	}

	/// \brief
	///   Returns the 0-based index of the first bit that is not set, or -1 if all bits are set
	inline int GetIndexOfFirstClearedBit() const
	{
		int iIndex = 0;
		const int iIntCount = GetIntCount();
		for (int i=0;i<iIntCount;i++,iIndex+=32) if (m_pField[i]!=0xffffffff)
			for (int j=0;j<32 && (iIndex+j)<m_iBitCount;j++) 
				if (!IsBitSet(iIndex+j))
					return iIndex+j;
		return -1;
	}

	/// \brief
	///   Merges another bitfield into this one using logical OR. The fields don't have to have the
	///   same size
	void Add(const TBitfield &other)
	{
		const int iCount = __min(GetIntCount(),other.GetIntCount());
		for (int i=0;i<iCount;i++)
			m_pField[i] |= other.m_pField[i];
	}

	/// \brief
	///   Assignment operator to copy all bits from another field. Also allocates the same amount of
	///   bits
	TBitfield &operator = (const TBitfield &other)
	{
		AllocateBitfield(other.m_iBitCount);
		const int iIntCount = other.GetIntCount();
		for (int i=0;i<iIntCount;i++)
			m_pField[i] = other.m_pField[i];
		return *this;
	}

	/// \brief
	///   Evaluates the number of 32 bit integers necessary to represent the bitfiled, i.e.
	///   (GetBitCount()+31)/32
	inline int GetIntCount() const 
	{
		return (m_iBitCount+31)>>5;
	}

	/// \brief
	///   Returns a 32 bit integer of specified index, where index must be [0..GetIntCount()-1]
	inline unsigned int GetInt(int idx) const
	{
		return m_pField[idx];
	}

	/// \brief
	///   Returns a pointer to the 32 bit integer array that represents the bitfield
	inline unsigned int *GetIntArray() const 
	{
		return m_pField;
	}

	/// \brief
	///   Returns the number of bits (see AllocateBitfield)
	inline int GetBitCount() const 
	{
		return m_iBitCount;
	}

	/// \brief
	///   Counts the number of bits that are set
	inline int CountSetBits() const
	{
		const int iIntCount = GetIntCount();
		int iCount = 0;
		for (int i=0;i<iIntCount;i++)
		{
			unsigned int iInt32 = m_pField[i];
			while (iInt32)
			{
				iCount += (iInt32&1);
				iInt32>>=1;
			}
		}
		return iCount;
	}

protected:
	int m_iBitCount;
	unsigned int *m_pField;
	unsigned int m_OwnBits[(iStaticBitCount+31)/32]; ///< in case we only need <=iStaticBitCount bits, we do not need to allocate
};
