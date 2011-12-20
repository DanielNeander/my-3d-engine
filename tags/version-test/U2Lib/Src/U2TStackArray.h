/**************************************************************************************************
module	:	U2TStackArray
Author	:	Yun sangyong
Desc	:	
***************************************************************************************************/
#pragma once
#ifndef U2_TSTACKARRAY_H
#define U2_TSTACKARRAY_H



template<typename T, size_t COUNT>
class U2TStackArray
{
private:
	/// A fixed-size array of COUNT elements.
	T m_kArray[COUNT];
public:
	/// Get the number of elements.
	inline size_t Count() const;

	/// Indexing operator.
	inline T& operator[](size_t stIndex);

	/// Indexing operator (const-safe).
	inline const T& operator[](size_t stIndex) const;
};

//---------------------------------------------------------------------------
template<typename T, size_t COUNT>
inline size_t U2TStackArray<T, COUNT>::Count() const
{
	return COUNT;
}
//---------------------------------------------------------------------------
template<typename T, size_t COUNT>
inline T& U2TStackArray<T, COUNT>::operator[](size_t stIndex)
{
	return m_kArray[stIndex];
}
//---------------------------------------------------------------------------
template<typename T, size_t COUNT>
inline const T& U2TStackArray<T, COUNT>::operator[](size_t stIndex) const
{
	return m_kArray[stIndex];
}
//---------------------------------------------------------------------------

#endif