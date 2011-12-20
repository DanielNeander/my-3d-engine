/************************************************************************
module	: U2StreamBase
Author	: Yun sangyong
Desc	: 
************************************************************************/
#ifndef U2_STREAMBASE_H
#define U2_STREAMBASE_H

#include <U2Lib/src/Memory/U2MemObj.h>

class U2LIB U2StreamBase : public U2MemObj
{
public:
	U2StreamBase();
	virtual ~U2StreamBase();
	virtual void Seek(int numBytes) = 0;
	virtual uint32 GetPos() const;

	virtual uint32 ReadEndianDatas(void * pvBuffer, uint32 uTotalBytes, 
		uint32* puComponentSizes, uint32 uNumComponents = 1) = 0;
	virtual uint32 WriteEndianDatas(const void * pvBuffer, uint32 uTotalBytes, 
		uint32* puComponentSizes, uint32 uNumComponents = 1) = 0;

	virtual uint32 Read(void* pvBuffer, uint32 uBytes);
	virtual uint32 Write(const void* pvBuffer, uint32 uBytes);

	uint32 GetLine(TCHAR* pvBuffer, uint32 uMaxBytes);
	uint32 PutString(const TCHAR* pvBuffer);

	static void U2StreamBase::DoByteSwap(void* pvData, uint32 uBytes, 
		uint32* puComponentSizes, UINT uNumComponents);

protected:

	uint32 m_uCurrStreamPos;
};



#endif 