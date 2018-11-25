#include "stdafx.h"
#include "Log.h"

CString Log::m_curLogPath = "";

Log::Log()
{
}


Log::~Log()
{
}
void Log::CreateLog()
{
	CTime tm;
	tm = CTime::GetCurrentTime();
	m_curLogPath = tm.Format("./%Y-%m-%d_%H-%M-%S.log");	//年月日：%Y-%m-%d
	FILE *fp;
	fp = fopen(m_curLogPath, "w");
	if (fp)
		fclose(fp);
	else
		AfxMessageBox("创建日志文件失败");
}

void Log::WrtiteLog(const CString & str)
{
	FILE * fp;
	fp = fopen(m_curLogPath, "a+");
	fwrite(str, 1, str.GetLength(), fp);
	fclose(fp);
}