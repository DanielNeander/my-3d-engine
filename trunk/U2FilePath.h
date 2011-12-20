/************************************************************************
module	: U2FilePath
Author	: Yun sangyong
Desc	: 파일 경로를 다루는 모듈 	GameBryo 2.6 버전을 참조.
		 편하게 하는 디자인을 고려해야 할 듯..
		 디자인 방향은 API 독립적으로 정함.
		현재 지원 사항 : 상대 경로를 넘겨 줄 것. 
					슬레시 표시는 \만 지원
************************************************************************/
#ifndef U2_FILEPATH_H
#define U2_FILEPATH_H

class U2FilePath
{
public:
	static void RemoveSlashDotSlash(TCHAR* szPath);
	static void RemoveDotDot(TCHAR* szPath);

	bool	IsRelPath(const TCHAR *pcPath);

	
	size_t ConvertToAbs(TCHAR* szPath, size_t pathBytes);
	
	size_t ConvertToAbs(TCHAR* szPath, size_t pathBytes, 
		const TCHAR* szRelHere);

	size_t ConvertToAbs(TCHAR *szPath, size_t absBytes, 
		const TCHAR* szRelPath,const TCHAR* szRelHere);
	
	bool GetCurrentWorkingDirectory(TCHAR* pcPath, size_t stDestSize);
};


#endif