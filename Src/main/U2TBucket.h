/**************************************************************************************************
module	:	U2TBucket
Author	:	Yun sangyong
Desc	:	Array Bucket Sort
**************************************************************************************************/
#pragma once
#ifndef	U2_TBUCKET_H
#define	U2_TBUCKET_H

#include <U2Lib/Src/U2TVec.h>

template<class DataType, class T, unsigned int NUMBUCKETS>
class U2TBuket : public U2MemObj
{
public:
	U2TBuket(unsigned int initSize, unsigned int growBySize);
	virtual ~U2TBuket();

	DataType& operator[](UINT bucketIdx);

	void Clear();
	
	void Destory();

	//void Reset();

	unsigned int Size() const;

public:
	U2TBuket();
	U2TBuket& operator=(const U2TBuket& rhs);

	DataType m_arrays[NUMBUCKETS];
};

//-------------------------------------------------------------------------------------------------
template<class DataType, class T, unsigned int NUMBUCKETS>
U2TBuket<DataType, T, NUMBUCKETS >::U2TBuket(unsigned int initSize, 
										  unsigned int growBySize)
{
	unsigned int i;
	for(i = 0; i < NUMBUCKETS; ++i)
	{
		m_arrays[i].SetIncreSize(growBySize);
		m_arrays[i].Resize(initSize);		
	}
}

template<class DataType, class T, unsigned int NUMBUCKETS>
U2TBuket<DataType, T, NUMBUCKETS >::U2TBuket()
{
	
}


//-------------------------------------------------------------------------------------------------
template<class DataType, class T, unsigned int NUMBUCKETS>
U2TBuket<DataType, T, NUMBUCKETS >::~U2TBuket()
{

}

//-------------------------------------------------------------------------------------------------
template<class DataType, class T, unsigned int NUMBUCKETS>
DataType& U2TBuket<DataType, T, NUMBUCKETS >::operator[](UINT bucketIdx)
{
	return m_arrays[bucketIdx];
}

//-------------------------------------------------------------------------------------------------
template<class DataType, class T, unsigned int NUMBUCKETS>
void U2TBuket<DataType, T, NUMBUCKETS >::Clear()
{
	unsigned int i;
	for(i=0; i < NUMBUCKETS; ++i)
	{
		m_arrays[i].RemoveAll();
	}
}

//-------------------------------------------------------------------------------------------------
template<class DataType, class T, unsigned int NUMBUCKETS>
void U2TBuket<DataType, T, NUMBUCKETS >::Destory()
{
	unsigned int i;
	for(i=0; i < NUMBUCKETS; ++i)
	{
		for(unsigned int j=0; j < m_arrays[i].FilledSize(); ++j)
		{
			U2_DELETE m_arrays[i]->GetElem(j);
		}
	}

	Clear();
}

////-------------------------------------------------------------------------------------------------
//template<class DataType, class T, unsigned int NUMBUCKETS>
//void U2TBuket<DataType, T, NUMBUCKETS >::Reset()
//{
//	
//}
 
//-------------------------------------------------------------------------------------------------
template<class DataType, class T, unsigned int NUMBUCKETS>
unsigned int U2TBuket<DataType, T, NUMBUCKETS >::Size() const
{
	return NUMBUCKETS;
}

//-------------------------------------------------------------------------------------------------


#endif 