
// ScreenShareClientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScreenShareClient.h"
#include "ScreenShareClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CScreenShareClientDlg 对话框



CScreenShareClientDlg::CScreenShareClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SCREENSHARECLIENT_DIALOG, pParent),
	m_bIsRecvFile(false),
	m_totalSize(0),
	m_haveGetSize(0),
	m_currentPackSize(0),
	m_currentPackGetSize(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
}

void CScreenShareClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, m_listChat);
	DDX_Control(pDX, IDC_PROGRESS1, m_progress);
}

BEGIN_MESSAGE_MAP(CScreenShareClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CScreenShareClientDlg::OnBnClickedButtonTcp)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON2, &CScreenShareClientDlg::OnBnClickedSharedScreen)
//	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()


// CScreenShareClientDlg 消息处理程序

BOOL CScreenShareClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	Log::CreateLog();

	initSock();
	m_pDC = GetDC();
	m_picDC = GetDlgItem(IDC_PIC)->GetDC();
	m_udpSocket.setDrawDC(m_picDC); //此处必须要在initSocket之前，因为在initSocket里需要设置绘图的DC
	if (!m_udpSocket.initSocket())
		AfxMessageBox("初始化upd失败");
	
	m_progress.SetRange(0, 100);
	
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CScreenShareClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CScreenShareClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CScreenShareClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CScreenShareClientDlg::OutputFormatError(int line)
{
	TCHAR szBuf[128];
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	sprintf(szBuf, "(行号:%d 出错码=%d): %s", line, dw, lpMsgBuf);
	LocalFree(lpMsgBuf);
	MessageBox(szBuf);

}
void CScreenShareClientDlg::setStatus(int s)
{
	if (s)
		SetDlgItemText(IDC_Status, "tcp连接状态:已连接");
	else
		SetDlgItemText(IDC_Status, "tcp连接状态:未连接");
}
void CScreenShareClientDlg::setUdpStatus()
{
	CString str;
	str.Format("udp延迟:%dms", m_udpSocket.m_ping);
	SetDlgItemText(IDC_TEXT_Ping, str);
	str.Format("udp丢包率:%.1f%%", m_udpSocket.m_losePercent);
	SetDlgItemText(IDC_TEXT_LOSE, str);
}
void CScreenShareClientDlg::initSock()
{
	WSAData wsa;
	ZeroMemory(&wsa, sizeof(WSAData));
	WSAStartup(MAKEWORD(2, 2), &wsa);
	SOCKADDR_IN sockAddr;
	int sockAddrLen = sizeof(sockAddr);
	ZeroMemory(&sockAddr, sockAddrLen);
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");//inet_addr("172.20.10.5");//inet_addr("192.168.137.1");
	sockAddr.sin_port = htons(8000);

	m_tcpSock = socket(AF_INET, SOCK_STREAM, 0);


	if (m_tcpSock == INVALID_SOCKET)
	{
		OutputFormatError(__LINE__);
		return;
	}
	if (connect(m_tcpSock, (sockaddr*)&sockAddr, sockAddrLen) == SOCKET_ERROR)
	{
		m_bConnectTcp = false;
		OutputFormatError(__LINE__);
		return;
	}
	else
	{
		m_bConnectTcp = true;
		GetDlgItem(IDC_BUTTON1)->SetWindowTextA("断开连接");
	}
	int recvBufSize = 1024 * 64 - 1;
	setsockopt(m_tcpSock, SOL_SOCKET, SO_RCVBUF, (char*)&recvBufSize, sizeof(recvBufSize));
	//AfxMessageBox("")
	setStatus(1);
	WSAAsyncSelect(m_tcpSock, m_hWnd, WM_CUSTOM_NETWORK_MSG, FD_READ | FD_WRITE | FD_CLOSE);
	m_bIsRecvFile = false;
}

void CScreenShareClientDlg::SetFullScreen()
{
	int cx, cy;
	cx = GetSystemMetrics(SM_CXSCREEN);
	cy = GetSystemMetrics(SM_CYSCREEN);
	if (!m_bFullScreen)
	{
		GetClientRect(m_oldClientRect);
		CRect rcTemp;
		rcTemp.BottomRight() = CPoint(cx, cy);
		rcTemp.TopLeft() = CPoint(0, 0);
		MoveWindow(&rcTemp);
		LONG Style = ::GetWindowLong(this->m_hWnd, GWL_STYLE);
		::SetWindowLong(this->m_hWnd, GWL_STYLE, Style&~WS_CAPTION);
		::SetWindowPos(this->m_hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		m_bFullScreen = true;
		
		m_udpSocket.setDrawDC(m_pDC);
	//	GetDlgItem(IDC_PIC)->MoveWindow(&rcTemp);
	}
	else
	{
		CRect rcTemp;
		rcTemp.TopLeft() = CPoint(150,100);
		rcTemp.BottomRight() = CPoint(m_oldClientRect.Width() + 170 /*+ (m_oldClientRect.Width() - cx) >> 2*/,
			m_oldClientRect.Height() + 140 /*+ (m_oldClientRect.Height() - cy) >> 2*/);
		MoveWindow(&rcTemp);
		LONG Style = ::GetWindowLong(this->m_hWnd, GWL_STYLE);
		::SetWindowLong(this->m_hWnd, GWL_STYLE, Style | WS_CAPTION);
		::SetWindowPos(this->m_hWnd, NULL,0,0, 0,0,SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		m_bFullScreen = false;
		m_udpSocket.setDrawDC(m_picDC);
		Invalidate();
	}
}

BOOL CScreenShareClientDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (pMsg->message == WM_CUSTOM_NETWORK_MSG)
	{
		long event = WSAGETSELECTEVENT(pMsg->lParam);
		if (event == FD_READ)
		{
			char buf[32767] = { 0 };
			int len = recv(m_tcpSock, buf, 32767, 0);
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				AfxMessageBox("阻塞");
				return false;
			}
			else if (len <= 0)
			{
				AfxMessageBox("与服务端断开连接");
				::closesocket(m_tcpSock);
				m_bConnectTcp = false;
				GetDlgItem(IDC_BUTTON1)->SetWindowTextA("连接TCP服务");
				setStatus(0);
			}
			else
			{
				//如果刚好文件发送的长度是包头的长度，那不能被误导解析成包头了
				if (len == HEADLEN && handleHeaderPack(buf, len))
					return true;
				if (m_bIsRecvFile)
					handleRecvFile(buf, len);
				else
				{
					buf[len] = '\0';
					m_listChat.AddString(buf);
				}

			}
		}
		if (event == FD_WRITE)
		{

		}
		if (event == FD_CLOSE)
		{
			AfxMessageBox("与服务端断开连接");
			::closesocket(m_tcpSock);
			m_bConnectTcp = false;
			GetDlgItem(IDC_BUTTON1)->SetWindowTextA("连接TCP服务");
			setStatus(0);
		}
	}
	else if (pMsg->message == WM_KEYDOWN&&pMsg->hwnd == GetDlgItem(IDC_EDIT1)->m_hWnd && pMsg->wParam==VK_RETURN)
	{
		//OnBnClickedButtonTcp();
		CString strMsg;
		GetDlgItemText(IDC_EDIT1, strMsg);
		if (!strMsg.IsEmpty())
		{
			SendData(strMsg, strMsg.GetLength());
			SetDlgItemText(IDC_EDIT1, "");
		}
	}
	else if (pMsg->message == WM_LBUTTONDBLCLK /*&& pMsg->hwnd == GetDlgItem(IDC_PIC)->m_hWnd*/)
	{
		SetFullScreen();
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
bool CScreenShareClientDlg::handleHeaderPack(char buf[],int len)
{
	Header * p = reinterpret_cast<Header*>(buf);
	if (strcmp(p->msg, "firstPack") == 0)//此处是发送文件的第一个包，里面包含了文件名与文件的总大小
	{
		m_bIsRecvFile = true;
		m_progress.ShowWindow(SW_SHOW);

		m_recvFilePath.Format("E:\\%s", p->fileName);
		m_totalSize = p->totalLen;
		m_haveGetSize = 0;
		FILE * fp = fopen(m_recvFilePath, "w");//新建文件，然后后面以追加方式写入文件
		fclose(fp);
		SendData(m_startSendAttackMsg, m_startSendAttackMsg.GetLength());
		return true;
	}
	else if (strcmp(p->msg, "SubPack") == 0)//收到发送文件的包
	{
		m_currentPackGetSize = 0;
		m_currentPackSize = p->totalLen;
		//writeQueue.push_back("###SUB_PACK###");
		SendData(m_recvPackAttackMsg, m_recvPackAttackMsg.GetLength());
		return true;
	}
	return false;
}
void CScreenShareClientDlg::handleRecvFile(char buf[], int len)
{
	FILE * fp = fopen(m_recvFilePath, "ab+");
	if (fp)
	{
		fwrite(buf, 1, len, fp);
		fclose(fp);
	}
	else
	{
		OutputFormatError(__LINE__);
	}
	m_haveGetSize += len;
	m_currentPackGetSize += len;
	int pos = m_haveGetSize / (float)m_totalSize *100;
	m_progress.SetPos(pos);
	if (m_currentPackSize == m_currentPackGetSize)
		SendData(m_recvCurrentPackEndMsg, m_recvCurrentPackEndMsg.GetLength());
	//printf("Have download : %.2f%%\n", haveGetSize / (float)totalSize *100.0);

	if (m_haveGetSize == m_totalSize)
	{
		m_haveGetSize = 0;
		m_totalSize = 0;
		m_currentPackSize = 0;
		m_currentPackGetSize = 0;
		//writeQueue.push_back("接收成功");
		m_bIsRecvFile = false;
		//m_progress.ShowWindow(SW_HIDE);
	}
}
bool CScreenShareClientDlg::SendData(const char * data, int len,bool isfile)
{
	int totalLen = len;
	int sendLen = 0;
	int haveSendLen = 0;
	while (haveSendLen != len)
	{
		//send返回值大于0，则为发送的长度，=0则为关闭了sokcet， 小于0则为异常
		sendLen = send(m_tcpSock, data + haveSendLen, totalLen, 0);
		if (sendLen>0)
		{
			haveSendLen += sendLen;
			//此处用于对方接收不全的情况，比如发送10k，但只接收了3k，则sendLen返回3k
			totalLen -= sendLen;
		}
		else if (sendLen == 0)
		{
			//关闭了Socket
			::closesocket(m_tcpSock);
		}
		else
		{
			if (WSAGetLastError() == WSAEWOULDBLOCK)
			{
				sendLen = 0;
			}
			else
			{
				return false;
			//	break;
			}
		}

	}
	return true;
}

void CScreenShareClientDlg::OnBnClickedButtonTcp()
{
	if (!m_bConnectTcp)
	{
		initSock();
	}
	else
	{
		::closesocket(m_tcpSock);
		WSACleanup();
		m_bConnectTcp = false;
		GetDlgItem(IDC_BUTTON1)->SetWindowTextA("连接TCP服务");
	}

/*	CString strMsg;
	GetDlgItemText(IDC_EDIT1, strMsg);
	if (strMsg.IsEmpty())
		return;
	else
	{
		SendData(strMsg, strMsg.GetLength());
		SetDlgItemText(IDC_EDIT1, "");
	}
	*/
}


void CScreenShareClientDlg::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialogEx::OnOK();
}


void CScreenShareClientDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (m_hThread)
	{
		CloseHandle(m_hThread);
		m_hThread = 0;
	}
	if (m_bSharedScreen)
		OnBnClickedSharedScreen();
	CDialogEx::OnClose();
	ReleaseDC(m_pDC);
	ReleaseDC(m_picDC);
}

UdpHeader paraseHeader(char *data, int len)
{
	UdpHeader tmp;
	if (len < headLen)
		;// cout << "error";
	else
	{
		char * buf = new char[headLen];
		memcpy(buf, data, headLen);
		UdpHeader *header = reinterpret_cast<UdpHeader*>(buf);
		tmp = *header;
		delete[]buf;
	}
	return tmp;
}
DWORD WINAPI CScreenShareClientDlg::ReceiveScreenShotProc(LPVOID p)
{
	CScreenShareClientDlg * pDlg = (CScreenShareClientDlg*)p;
	
	while (1)
	{
		if (!pDlg->m_bSharedScreen)
			break;
		pDlg->m_udpSocket.ReceiveSreenShot();
		pDlg->setUdpStatus();
		Sleep(10);
			
	}

	return 0;
}

void CScreenShareClientDlg::OnBnClickedSharedScreen()
{
	if (!m_bSharedScreen)
	{
		m_bSharedScreen = true;
		m_hThread = CreateThread(NULL, 0, ReceiveScreenShotProc, this, 0, 0);
		SetDlgItemText(IDC_BUTTON2, "关闭共享屏幕");
		//m_udpSocket.setDrawDC(GetDlgItem(IDC_PIC)->GetDC());
	}
	else
	{
		m_bSharedScreen = false;
		if (m_hThread)
		{
			CloseHandle(m_hThread);
			m_hThread = 0;
		}
		SetDlgItemText(IDC_BUTTON2, "打开共享屏幕");
	}
}
