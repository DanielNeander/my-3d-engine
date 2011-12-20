/************************************************************************
module	: U2FilePath
Author	: Yun sangyong
Desc	: ���� ��θ� �ٷ�� ��� 	GameBryo 2.6 ������ ����.
		 ���ϰ� �ϴ� �������� ����ؾ� �� ��..
		 ������ ������ API ���������� ����.
		���� ���� ���� : ��� ��θ� �Ѱ� �� ��. 
					������ ǥ�ô� \�� ����
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