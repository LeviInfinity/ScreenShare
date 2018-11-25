#pragma once
class Log
{
public:
	Log();
	~Log();

	static void CreateLog();
	static void WrtiteLog(const CString &);
	static CString m_curLogPath;
};
