/**************************************************************************************************
module	: U2File
Author	: Yun sangyong
Desc	: 데이터 시리얼라이즈를 담당하는 클래스
***************************************************************************************************/
#ifndef U2_FILE_H
#define U2_FILE_H

#include "U2StreamBase.h"

class U2LIB	U2File : public U2StreamBase
{

public:
	enum FileMode 
	{
		READ_ONLY,
		WRITE_ONLY,
		APPEND_ONLY,
	};
	U2File(const TCHAR* szFilename, FileMode eMode, unsigned int uBuffSize = 32768);
	virtual ~U2File();

	static uint32 ReadNoSwapBytes(U2StreamBase *pThis,void* pvBuffer, uint32 uBytes, 
		uint32* puComponentSizes, uint32 uNumComponents);
	static uint32 WriteNoSwapBytes(U2StreamBase *pThis, const void* pvBuffer, uint32 uBytes, 
		uint32* puComponentSizes, uint32 uNumComponents);

	virtual uint32 ReadEndianDatas(void * pvBuffer, uint32 uTotalBytes, 
		uint32* puComponentSizes, uint32 uNumComponents = 1);
	virtual uint32 WriteEndianDatas(const void * pvBuffer, uint32 uTotalBytes, 
		uint32* puComponentSizes, uint32 uNumComponents = 1);

	virtual operator bool() const;	


	static U2File* GetFile(const TCHAR* szName, FileMode eMode, 
		uint32 uBuffSize = 32768);

	static bool Access(const TCHAR* szName, FileMode eMode);
	
	static bool CreateDir(const TCHAR* szDirname);
	static bool IsDirExists(const TCHAR* szDirname);
	
	static const int ms_iSeekSet;
	static const int ms_iSeekCur;
	static const int ms_iSeekEnd;

	virtual void Seek(int numBytes);
	void Seek(int iOffset, int iSeek);

	uint32 GetfileSize() const;

	

		
	
private:
	U2File();
	bool Flush();
	uint32 DiskWrite(const void* pvBuff, uint32 uBytes);
	uint32 DiskRead(void* pvBuff, uint32 uBytes);


	uint32 m_uBuffAllocSize;
	uint32 m_uBuffReadSize;
	uint32 m_uPos;

	TCHAR* m_pBuff;
	FILE* m_pFile;
	FileMode m_eMode;
	bool	m_bGood;

	unsigned int FileRead(void* pvBuffer, unsigned int uiBytes);
	unsigned int FileWrite(const void* pvBuffer, unsigned int uiBytes);


};


#endif 