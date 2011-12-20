#pragma once 

#include <stdexcept>
#include <vector>

template<class TArrayElement, int iNumElements> class CArrayPool
{
public:
	CArrayPool()
	{
		DeleteAll();
	}

	virtual ~CArrayPool() {}

	inline TArrayElement* New() 
	{
		if (GetNumFreeElements() < 1) throw(std::out_of_range("CRecyclingArray::New: too many objects!"));

		int i = m_iLastNew;
		for (int q=0; q < iNumElements; q++) {
			if (!m_bAlive[i]) {
				// we've found our free spot!  use it!
				break;
			}
			else {
				i++; if (i >= iNumElements) i = 0;
			}
		}

		if (m_bAlive[i]) {
			// huh? if we got here, there are no free elements in the list... yet 
			// GetNumFreeElements didn't tell us that in the beginning.  Logic error.
			throw(std::logic_error("TArrayElement::New(): internal logic error."));
		}

		// clear it
		m_aElems[i] = m_NewTemplate;

		// increment used count
		m_iUsedElements++;
		m_bAlive[i] = true;
		m_iLastNew = i;

		// return it
		return(&m_aElems[i]);


	}

	inline bool Delete(int index)
	{
		if (index < 0 || index >= iNumElements || !m_bAlive[index]) return false;
		// don't actually delete element at index;
		// just mark it free.
		m_bAlive[index] = false;
		m_iUsedElements--;
		return true;
	}

	inline bool Delete(TArrayElement* pItem) 
	{
		if (m_iUsedElements == 0) return(false);
		int iElem = reinterpret_cast<int>(elem);
		iElem -= reinterpret_cast<int>(&m_aElements[0]);
		int index = iElem / sizeof(TArrayElement);
		if (index < 0 || index >= iNumElements || !m_bAlive[index]) return(false);
		m_bAlive[index] = false;
		m_iUsedElements--;
		return(true);
	}

	inline int GetNumFreeElements(void) { return iNumElements - GetNumUsedElements(); };
	inline int GetNumUsedElements(void) { return m_iUsedElements; };

	inline int GetTotalElements() { return iNumElements; };

	inline TArrayElement& GetAt(int index) { return m_aElems[index]; };
	inline bool IsAlive(int index) { return m_bAlive[index]; };

	inline void DeleteAll(void)
	{
		for (int q=0; q < iNumElements; q++) {
			m_bAlive[q] = false;
		}
		m_iUsedElements = 0;
		m_iLastNew = 0;
	}
	

protected:
	int m_iUsedElements;
	TArrayElement m_aElems[iNumElements];
	bool m_bAlive[iNumElements];
	int m_iLastNew;
	TArrayElement m_NewTemplate;
	

};