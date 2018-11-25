
// ScreenShareServerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ScreenShareServer.h"
#include "ScreenShareServerDlg.h"
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


// CScreenShareServerDlg 对话框



CScreenShareServerDlg::CScreenShareServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SCREENSHARESERVER_DIALOG, pParent),
	m_bIsShareScreen(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CScreenShareServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST2, m_listCtrl);
	DDX_Control(pDX, IDC_LIST4, m_listChat);
	DDX_Control(pDX, IDC_LIST5, m_listLog);
}

BEGIN_MESSAGE_MAP(CScreenShareServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON3, &CScreenShareServerDlg::OnBnClickedButtonSendMsg)
	ON_BN_CLICKED(IDC_BUTTON2, &CScreenShareServerDlg::OnBnClickedButtonSendFile)
	ON_BN_CLICKED(IDC_BTN_SHARE_SCREEN, &CScreenShareServerDlg::OnBnClickedButtonShareScreen)
END_MESSAGE_MAP()


// CScreenShareServerDlg 消息处理程序

BOOL CScreenShareServerDlg::OnInitDialog()
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

	
	initUI();
	Log::CreateLog();
	if(!InitSock())
		AddLog("初始化tcp失败");
	if (!m_udpSocket.InitUdpSock())
		AddLog("初始化udp失败");
	m_udpSocket.setHandle(this);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CScreenShareServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CScreenShareServerDlg::OnPaint()
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
HCURSOR CScreenShareServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
void CScreenShareServerDlg::OnOK()
{
	//CDialogEx::OnOK();
}

void CScreenShareServerDlg::initUI()
{
	m_listCtrl.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);//设置拓展风格
	m_listCtrl.InsertColumn(0, _T("socketId"), LVCFMT_LEFT, 100);//设置第一列
	m_listCtrl.InsertColumn(1, _T("IP"), LVCFMT_LEFT, 100);//设置第二列
	m_listCtrl.InsertColumn(2, _T("文件传输进度"), LVCFMT_LEFT, 200);//...

}


void CScreenShareServerDlg::OutPutErrorMsg(int line, DWORD dw)
{
	char szBuf[128];
	LPVOID lpMsgBuf;
	if (!dw)
		dw = GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
		dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	sprintf(szBuf, "行:%d (出错码=%d): %s", line, dw, lpMsgBuf);
	LocalFree(lpMsgBuf);
	//MessageBox(szBuf);
	AddLog(szBuf);
}
bool CScreenShareServerDlg::InitSock()
{
	SOCKADDR_IN serverAddr;
	int addrLen = sizeof(serverAddr);
	memset(&serverAddr, 0, addrLen);
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;//inet_addr("127.0.0.1");
	serverAddr.sin_port = htons(m_port);
	m_listenSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_listenSocket == INVALID_SOCKET)
	{
		OutPutErrorMsg(__LINE__);
		WSACleanup();
		return false;
	}
	if (::bind(m_listenSocket, (sockaddr*)&serverAddr, addrLen) == SOCKET_ERROR)
	{
		OutPutErrorMsg(__LINE__);
		WSACleanup();
		return false;
	}
	listen(m_listenSocket, 5);
	//CString log;
	//log.Format("<Listening..> ip: %s port:%d", inet_ntoa(serverAddr.sin_addr), m_port);
	//AddLog(log);

	/* WSAAsyncSelect会自动将套接字设置为非阻塞模式*/

	WSAAsyncSelect(m_listenSocket, m_hWnd, WM_CUSTOM_NETWORK_MSG, FD_ACCEPT);
	return true;
}

BOOL CScreenShareServerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_CUSTOM_NETWORK_MSG)
	{
		SOCKET socket = (SOCKET)pMsg->wParam;//有网络消息的socket
		CClient *currentSocket;
		for (auto &p : m_allClientSocket)
		{
			if (p->m_socket == socket)
				currentSocket = p;
		}
		long event = WSAGETSELECTEVENT(pMsg->lParam);
		int error = WSAGETSELECTERROR(pMsg->lParam); //对于WSAAsyncSelect只能用这个来获得错误，因为GetLastError不能根据网络事件消息来检查对错
		/*if (error)
		{
			OutPutErrorMsg(__LINE__, error);
		}*/
		CString strLog;
		int row;
		vector<CClient*>::iterator pos;//用于指向删除m_allClientSocket某一项后的位置
		switch (event)
		{
		case FD_ACCEPT:
			AcceptConnect();
			break;
		case FD_READ:
			//注：每次未读完缓冲区的recv()调用，都会重新触发一个FD_READ消息,所以如果需要循环读取的话则需要先关闭对FD_READ的监听
			//WSAAsyncSelect(socket, m_hWnd, WM_CUSTOM_NETWORK_MSG, FD_WRITE | FD_CLOSE);
			currentSocket->RecvMsg();
			//WSAAsyncSelect(socket, m_hWnd, WM_CUSTOM_NETWORK_MSG, FD_READ | FD_WRITE | FD_CLOSE);
			break;
		case FD_WRITE:

			break;
		case FD_CLOSE:
			currentSocket->CloseSocket(m_allClientSocket.size() - 1);
			pos = remove_if(m_allClientSocket.begin(), m_allClientSocket.end(), [=](CClient* c) {return c->m_socket == socket; });	
		
			row = GetListCtrlRow(currentSocket->m_socket);
			if (row != -1)
				m_listCtrl.DeleteItem(row);
			else
				AddLog("未在找到关闭的socket索引");
			m_allClientSocket.erase(pos, m_allClientSocket.end());
			SetBottomRightString();
			break;
		default:
			break;
		}
	}
	else if (pMsg->message == WM_KEYDOWN&&pMsg->hwnd == GetDlgItem(IDC_EDIT1)->m_hWnd && pMsg->wParam == VK_RETURN)
	{
		OnBnClickedButtonSendMsg();
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}
void CScreenShareServerDlg::AcceptConnect()//收到新连接
{
	SOCKADDR_IN sockAddr;
	int len = sizeof(SOCKADDR_IN);
	ZeroMemory(&sockAddr, len);
	SOCKET client = accept(m_listenSocket, (sockaddr*)&sockAddr, &len);
	if (client == INVALID_SOCKET)
	{
		OutPutErrorMsg(__LINE__);
		return;
	}
	//设置发送缓冲区大小（默认8k，最大设置64k）
	int sendBufSize = 1024 * 64 - 1;
	setsockopt(client, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufSize, sizeof(sendBufSize));

	m_allClientSocket.push_back(new CClient(client, this));
	WSAAsyncSelect(client, m_hWnd, WM_CUSTOM_NETWORK_MSG, FD_READ | FD_WRITE | FD_CLOSE);

	CString strLog;
	strLog.Format("%d 已连接，地址:%s ,当前连接数: %d", client, inet_ntoa(sockAddr.sin_addr), m_allClientSocket.size());
	AddLog(strLog);
	SetBottomRightString();

	int num = m_listCtrl.GetItemCount();
	strLog.Format("%d", client);
	m_listCtrl.InsertItem(num, strLog);
	m_listCtrl.SetItemText(num, 1, inet_ntoa(sockAddr.sin_addr));
	m_listCtrl.SetItemText(num, 2, "");

}
void CScreenShareServerDlg::SendAllMsg(const char *msg)
{
	for (auto client : m_allClientSocket)
	{
		client->SendData(msg, strlen(msg), false);
	}
}
void CScreenShareServerDlg::AddLog(const CString &str)
{
	m_listLog.AddString(str);
	m_listLog.SetTopIndex(m_listLog.GetCount() - 1);//让ListBox永远保持在最下方
}
void CScreenShareServerDlg::AddChat(const CString &str)
{
	m_listChat.AddString(str);
	m_listChat.SetTopIndex(m_listChat.GetCount() - 1);//让ListBox永远保持在最下方
}
void CScreenShareServerDlg::OnBnClickedButtonSendMsg()
{
	CString strMsg;
	GetDlgItemText(IDC_EDIT1, strMsg);
	if (strMsg.IsEmpty())
		return;
	else
	{
		SendAllMsg("Server: " + strMsg);
		AddChat(CString("我:")+ strMsg);
		SetDlgItemText(IDC_EDIT1, "");
	}
}

void CScreenShareServerDlg::OnBnClickedButtonSendFile()
{
	CString strPath;
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		_T("All Files (*.*)|*.*||"), NULL);
	if (dlg.DoModal() == IDOK)
	{
		strPath = dlg.GetPathName(); //文件路径名保存在了FilePathName里
		AddLog("发送文件:" + strPath);
		for (auto client : m_allClientSocket)
		{
			//TODO 如果在此发送过程中，有客户端断开连接会是什么情况？

			client->setFileName(strPath);
			client->StartSendFile();
		}
	}
	else
	{
		return;
	}
}
int CScreenShareServerDlg::GetListCtrlRow(SOCKET fd)
{
	for (int i = 0; i < m_listCtrl.GetItemCount(); i++)
	{
		if (atoi(m_listCtrl.GetItemText(i, 0)) == fd)
		{
			return i;
		}
	}
	return -1;
}
void CScreenShareServerDlg::SetBottomRightString()
{
	CString str;
	str.Format("当前连接数:%d", m_allClientSocket.size());
	SetDlgItemText(IDC_Info, str);
}
void CScreenShareServerDlg::ChangeTranferPercent(SOCKET s, int percent)
{
	int row = GetListCtrlRow(s);
	if (row != -1)
	{
		CString strPercent;
		strPercent.Format("%d%%", percent);
		m_listCtrl.SetItemText(row, 2, strPercent);
	}
}

DWORD WINAPI shareScreenThread(LPVOID p)
{
	CScreenShareServerDlg * pDlg = (CScreenShareServerDlg*)p;
	pDlg->AddLog("开始共享");
	while (1)
	{
		if (!pDlg->m_bIsShareScreen)
			break;
		pDlg->m_udpSocket.SendScreenShot();
		
	}
	pDlg->AddLog("关闭共享");
	return 0;
}

void CScreenShareServerDlg::OnBnClickedButtonShareScreen()
{
	// TODO: 在此添加控件通知处理程序代码
	if (!m_bIsShareScreen)
	{
		m_bIsShareScreen = true;
		m_hThreadHandle = CreateThread(NULL, 0, shareScreenThread, this, 0, 0);
		GetDlgItem(IDC_BTN_SHARE_SCREEN)->SetWindowText("关闭共享");
	}
	else
	{
		m_bIsShareScreen = false;
		CloseHandle(m_hThreadHandle);
		GetDlgItem(IDC_BTN_SHARE_SCREEN)->SetWindowText("打开共享");
	}
}
