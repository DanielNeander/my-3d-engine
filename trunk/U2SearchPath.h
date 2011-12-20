/************************************************************************
module	: U2SearchPath
Author	: Yun sangyong
Desc	: 
************************************************************************/
#ifndef U2_SEARCH_PATH
#define U2_SEARCH_PATH


class U2SearchPath
{
public:
	U2SearchPath();
	virtual ~U2SearchPath();

	void SetFilePath(const TCHAR* szPath);
	void SetReferencePath(const TCHAR* szPath);

	void	Reset();
	bool	NextSearchPath(TCHAR* szPath, uint32 len );

	static	void	SetDefaultPath(const TCHAR* path);
	static const TCHAR* GetDefaultPath();	
								

private:
	uint32	m_uNextPath;

	TCHAR	m_filePath[MAX_PATH];
	TCHAR	m_refPath[MAX_PATH];

	static TCHAR ms_defPath[MAX_PATH];	// Default Path

};




#endif
