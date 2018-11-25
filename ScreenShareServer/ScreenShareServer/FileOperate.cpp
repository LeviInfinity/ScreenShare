#include "stdafx.h"
#include "FileOperate.h"


FileOperate::FileOperate()
{
}


FileOperate::~FileOperate()
{
}

long FileOperate::getFileLen(const CString& szFileName)
{
	long len = 0;
	FILE *fp = fopen(szFileName,"rb");
	if (fp)
	{
		fseek(fp, 0, SEEK_END);
		len = ftell(fp);
		fclose(fp);
		return len;
	}
	/*HANDLE hFile = CreateFile(szFileName, FILE_READ_ONLY, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (!hFile)
		return len;
	len = GetFileSize(hFile, NULL);
	CloseHandle(hFile);*/
	return len;
}
CString FileOperate::getFileNameFromPath(const CString& path)
{
	int pos = path.ReverseFind('\\');
	return path.Right(path.GetLength() - pos - 1);
}

CString FileOperate::getLogisticDrivers()
{
	CString m_allLogisticDrivers = "";
	//获取系统分区驱动器字符串信息
	size_t szAllDriveStrings = GetLogicalDriveStrings(0, NULL);           //驱动器总长度
	int num = (szAllDriveStrings - 1) / 4;
	char m_pDriveStrings[128] = { 0 };
	GetLogicalDriveStrings(szAllDriveStrings, m_pDriveStrings);
	char dst[128] = { 0 };
	int len = 0;
	char *p = m_pDriveStrings;
	while (len<num)
	{
		//MessageBox(p);
		m_allLogisticDrivers += p;
		m_allLogisticDrivers += '\n';
		p += 4;
		len++;

	}
	m_allLogisticDrivers = m_allLogisticDrivers.Left(m_allLogisticDrivers.GetLength() - 1);

	return m_allLogisticDrivers;
}
CString FileOperate::listDir( CString  path)
{
	CFileFind find;
	//CString strFull;
	if (path.Right(1) == "\\")
		path += "*.*";
	else
		path += "\\*.*";
	BOOL IsFind = find.FindFile(path);
	CString res = "";
	while (IsFind)
	{
		IsFind = find.FindNextFile();
		if (find.IsDots()||find.IsSystem()||find.IsHidden())
		{
			continue;
		}
		else
		{
			CString filename = find.GetFileName();
			res += filename;
			res += '\n';
		}
	}
	return res;
				
}
bool FileOperate::isFileExist(const CString& filePath)
{
	DWORD dwAttrib = GetFileAttributes(filePath);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}
bool FileOperate::isDirectoryExit(const CString& directoryPath)
{
	DWORD dwAttrib = GetFileAttributes(directoryPath);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}