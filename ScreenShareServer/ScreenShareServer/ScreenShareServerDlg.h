// ScreenShareServerDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "ListCtrlEx.h"
#include "Client.h"
#include "UdpSocket.h"
#define WM_CUSTOM_NETWORK_MSG (WM_USER + 100) 
// CScreenShareServerDlg 对话框
class CScreenShareServerDlg : public CDialogEx
{
// 构造
public:
	CScreenShareServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SCREENSHARESERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonSendMsg();
	virtual void OnOK();

public:
	void initUI();
	void OutPutErrorMsg(int line, DWORD dw = 0);
	bool InitSock();//初始化SOCKET
	//bool InitUdpSock();//初始化udpSocket
	void AcceptConnect();//接收客户端连接事件
	void SendAllMsg(const char *);//向所有已经连接的客户端发送文本消息
	void AddLog(const CString &);
	void AddChat(const CString &);
	int GetListCtrlRow(SOCKET fd);//根据给定的socket得到在列表中的行的位置
	void SetBottomRightString();//修改右下方已连接的信息
	void ChangeTranferPercent(SOCKET, int);//修改listCtrl中的传输文件进度

	HANDLE m_hThreadHandle;//发送截图的句柄
	bool m_bIsShareScreen;//当前是否在分享名目

public:
	CListCtrlEx m_listCtrl;
	CListBox m_listChat;
	CListBox m_listLog;

	SOCKET m_listenSocket;//TCP的监听socket
	
	CUdpSocket m_udpSocket;
	int m_port = 8000;
	vector<CClient*> m_allClientSocket; //当前tcp连接的socket句柄


	afx_msg void OnBnClickedButtonSendFile();
	afx_msg void OnBnClickedButtonShareScreen();
};
