#include "stdafx.h"
#include "Client.h"
#include "ScreenShareServer.h"
#include "ScreenShareServerDlg.h"
CClient::CClient()
{
//	m_pDlg = AfxGetMainWnd();

}
CClient::CClient(SOCKET s,CWnd * p)
{
	m_pDlg = p;//初始化时p是有主窗口类传的this
	m_socket = s;
	m_commandList.push_back("ls");
	m_commandList.push_back("get");
	m_commandList.push_back("cd");
	m_commandList.push_back("pwd");
	m_currentDir = "";
}
//CClient CClient::operator=(const CClient& client)
//{
//	this->m_socket = client.m_socket;
//	this->m_hEvent = client.m_hEvent;
//	this->m_pDlg = client.m_pDlg;
//	return *this;
//}

CClient::~CClient()
{
}

void CClient::CloseSocket(int n)
{
	CScreenShareServerDlg* pMainWnd = (CScreenShareServerDlg*)m_pDlg;
	CString strLog;

	strLog.Format("%d 已关闭，当前连接数: %d", m_socket, n);

	pMainWnd->AddLog(strLog);
	::closesocket(m_socket);
}
Header CClient::MakeHead(const CString& msg, const CString & fileName, int len, MSGTYPE msgType)
{
	Header h;
	strcpy(h.msg, msg);
	strcpy(h.fileName, fileName);
	h.totalLen = len;
	h.msg_type = msgType;
	return h;
}

void CClient::SplitCommandAndParam(const CString &str, CString &command, CString &param)
{
	int pos = str.Find(' ');
	if (pos != -1)
	{
		command = str.Left(pos);
		param = str.Mid(pos + 1);
		//AfxMessageBox(command);
		//AfxMessageBox(param);
	}
	else
	{
		//判断是否是不带参数的命令
		for (auto p : m_commandList)
			if (p == str)
			{
				command = p;
				param = "";
				return;
			}
		command = "";
		param = "";
	}
}

void CClient::SendData(const char *data, int len, bool isFile)
{
	int totalLen = len;
	int sendLen = 0;
	int haveSendLen = 0;
	while (haveSendLen != len)
	{
		//send返回值大于0，则为发送的长度，=0则为关闭了sokcet， 小于0则为异常
		sendLen = send(m_socket, data + haveSendLen, totalLen, 0);
		if (sendLen>0)
		{
			haveSendLen += sendLen;
			//此处用于客户端接收不全的情况，比如发送10k，但只接收了3k，则sendLen返回3k
			totalLen -= sendLen;
		}
		else if (sendLen == 0)
		{
			
			//关闭了Socket
			::closesocket(m_socket);
		}
		else
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				sendLen = 0;
			}
			else
				break;
		}

	}
}

void CClient::RecvMsg()
{
	CScreenShareServerDlg* pMainWnd = (CScreenShareServerDlg*)m_pDlg;

	char buf[1024] = { 0 };
	int recvLen;
	CString strLog;
	recvLen = recv(m_socket, buf, 1024, 0);
	if (recvLen > 0)
	{
		buf[recvLen] = '\0';
		//发送第一个开始发送文件的数据包<文件大小，名称>后得到返回，则发送正式发送文件前的第一个包的头
		if (HandleAttackMsg(CString(buf), recvLen))
			return;
		/*if (HandleCommmandMsg(CString(buf)))
			return;*/

		strLog.Format("%d==>%s", m_socket, buf);
		pMainWnd->AddChat(strLog);
		pMainWnd->SendAllMsg(buf);
	}
}
void CClient::StartSendFile()
{
	m_curHaveSendLen = 0;
	m_curTotalLen = FileOperate::getFileLen(m_fileName);
	//构造包头
	CString fileName = FileOperate::getFileNameFromPath(m_fileName);
	Header startHead = MakeHead("firstPack", fileName, m_curTotalLen, _FILE);
	SendData((char*)&startHead, sizeof(startHead));
}
void CClient::ContinueSendFile()
{
	FILE * fp = fopen(m_fileName, "rb");
	if (!fp)return;
	char buffer[32767] = { 0 };
	long sendLen = 32767;
	if (m_curTotalLen - m_curHaveSendLen < 32767)
		sendLen = m_curTotalLen - m_curHaveSendLen;

	fseek(fp, m_curHaveSendLen, SEEK_SET);//调整开始读取的位置
	fread(buffer, 1, sendLen, fp);
	fclose(fp);

	SendData(buffer, sendLen);
	m_curHaveSendLen += sendLen;
	

	if (m_curTotalLen)
	{
		CScreenShareServerDlg* pMainWnd = (CScreenShareServerDlg*)m_pDlg;
		pMainWnd->ChangeTranferPercent(m_socket, m_curHaveSendLen /(float) m_curTotalLen *100);
	}
	if (m_curHaveSendLen == m_curTotalLen)
	{
		//AfxMessageBox("发送成功");
		m_curTotalLen = 0;
		m_curHaveSendLen = 0;
	}
}
void CClient::SendPackHeader()
{
	long sendLen = 32767;
	if (m_curTotalLen - m_curHaveSendLen < 32767)
		sendLen = m_curTotalLen - m_curHaveSendLen;
	Header packHeader = MakeHead("SubPack", "", sendLen, _FILE);
	SendData((char*)&packHeader, sizeof(packHeader));
}
bool CClient::HandleAttackMsg(const CString &msg, int recvLen)
{
	if (recvLen == m_startSendAttackMsg.GetLength() && msg == m_startSendAttackMsg)
	{
		SendPackHeader();
		return true;
	}
	//收到SendPackHeader发送的数据包后继续发送文件
	else if (recvLen == m_recvPackAttackMsg.GetLength() && msg == m_recvPackAttackMsg)
	{
		
		ContinueSendFile();
		return true;
	}
	//当前包发送完成确认
	else  if (recvLen == m_recvCurrentPackEndMsg.GetLength() && msg == m_recvCurrentPackEndMsg)
	{
		SendPackHeader();
		return true;
	}
	return false;
}
/*bool CClient::HandleCommmandMsg(const CString &msg)
{

	CString command, param;
	SplitCommandAndParam(msg, command, param);
	if (command != "")
	{
		if (command.MakeLower() == CString("get"))//当要下载某个文件
		{
			m_fileName = m_currentDir + param;
			if (FileOperate::isFileExist(m_fileName))
				StartSendFile();
			else
				send(m_socket, "file not exist!!", 17, 0);
		}
		if (command.MakeLower() == CString("ls"))
		{
			if (m_currentDir == "")
			{
				CString drivers = FileOperate::getLogisticDrivers();
				send(m_socket, drivers, drivers.GetLength(), 0);
			}
			else
			{
				CString files = FileOperate::listDir(m_currentDir);
				send(m_socket, files, files.GetLength(), 0);

			}

		}
		if (command.MakeLower() == CString("cd"))//切换目录
		{
			////输入的是全路径
			if (param.GetLength() >= 3 && param.GetAt(1) == ':'&&param.GetAt(2) == '\\')
			{
				if (FileOperate::isDirectoryExit(param))
					m_currentDir = param;
				else
					send(m_socket, "directory not exist!!", 22, 0);
			}
			else
			{
				if (FileOperate::isDirectoryExit(m_currentDir + param))
					m_currentDir += param;
				else
					send(m_socket, "directory not exist!!", 22, 0);
			}

			if (m_currentDir.Right(1) != "\\")
				m_currentDir += "\\";

		}
		if (command.MakeLower() == CString("pwd"))
		{
			send(m_socket, m_currentDir, m_currentDir.GetLength(), 0);
		}
		return true;
	}
	return false;
}
*/