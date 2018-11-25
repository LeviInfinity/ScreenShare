#pragma once
#include<iostream>
using namespace std;

class FileOperate
{
public:
	FileOperate();
	~FileOperate();
	static long getFileLen(const CString& szFileName);
	static CString getFileNameFromPath(const CString&path);
	static CString getLogisticDrivers();
	static CString listDir( CString path);
	static bool isFileExist(const CString& filePath);
	static bool isDirectoryExit(const CString& directoryPath);
};

