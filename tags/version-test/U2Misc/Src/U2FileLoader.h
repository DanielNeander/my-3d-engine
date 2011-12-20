/**************************************************************************************************
module	: U2FileLoader
Author	: Yun sangyong
Desc	: 
***************************************************************************************************/

#ifndef U2_FILELOADER_H
#define U2_FILELOADER_H

class U2FileLoader : public U2Object
{
public:
	U2FileLoader();
	virtual ~U2FileLoader() {}

	static U2FileLoader* Instance();

		
private:
	
	static U2FileLoader* ms_pSingleton;

	int m_iBytesRead;
	int m_iBytesWritten;
	int m_iNumSeeks;
	
};



#endif