#pragma once
struct UdpHeader
{
	long mainId;
	int subId;
	clock_t  sendTime;
	int bOK;
	long dataLen;
};
class CUdpSocket
{
public:
	CUdpSocket();
	~CUdpSocket();
	bool InitUdpSock();
	void SendScreenShot();
	SOCKADDR_IN m_serverAddr;
	void setHandle(CWnd *p) { m_pDlg = p; }
	CWnd *m_pDlg;//主窗口指针
	SOCKET m_udpSock;//udp发送的socket
	long m_sendId = 0; //此为发送的序列帧的主版本号

	//CString GetIp();
	
private:
	//下面的变量都是截图相关
	int nWidth, nHeight;
	HWND DesktopWnd;
	HDC DesktophDC, MemoryhDC;
	HBITMAP hbm, holdbm;
	BYTE *bits;//截图后的图像数据
	int fileLen;
};

