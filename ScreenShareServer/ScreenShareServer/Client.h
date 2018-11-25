#ifndef CLIENT_H
#define CLIENT_H
#include"stdafx.h"
#include"FileOperate.h"
#include<vector>
using namespace std;
#define BUF_SIZE 32767
class CClient
{
public:
	CClient();
	CClient(SOCKET,CWnd *);
	~CClient();
	//CClient operator=(const CClient&);
	void CloseSocket(int);
	void RecvMsg();//接收消息，在收到FD_READ时调用
	void StartSendFile();//调用get命令后调用，此时发送一个包含文件大小，文件名称的包头
	void ContinueSendFile();//当收到客户端的确认时文件
	void SendPackHeader();//每次发送文件包前的发送的包，包括下一个包的大小

	//对输入的字符串命令解析出命令与参数，对于不带参数的，在m_commandList中匹配
	void SplitCommandAndParam(const CString &, CString &, CString &);
	//对send的封装，用于防止发送的数据包客户端一次只接收了一部分的情况，就要根据send返回值发送剩余部分
	void SendData(const char *data, int len,bool isFile=true);
	Header MakeHead(const CString& msg, const CString & fileName, int len, MSGTYPE msgType);//构造包头，返回包的结构体
	bool HandleAttackMsg(const CString &,int len);//处理收到客户端应答消息
	//处理客户端发送的是命令（get cd ls pwd）,此为上一个版本客户端是控制台时的做法，此版暂未使用
	//bool HandleCommmandMsg(const CString &);
	void setFileName(const CString &file) { m_fileName = file; }
	

	SOCKET m_socket; //当前客户端的socket
private:	
	CWnd *m_pDlg;//主窗口指针
	
	CString m_fileName;//当前发送的文件名
	CString m_currentDir;//客户端通过 cd 得到的当前路径
	vector<CString>m_commandList;
	CString m_startSendAttackMsg = "###START_HEAD###";//开始发送文件时的第一个包
	CString m_recvPackAttackMsg = "###SUB_PACK###";//文件分包时的的包确认
	CString m_recvCurrentPackEndMsg = "###SUB_PACK_END###";//当前包发送完成确认
	long m_curTotalLen;//当前发送的文件长度
	long m_curHaveSendLen;//当前已发送的文件长度

};
#endif
