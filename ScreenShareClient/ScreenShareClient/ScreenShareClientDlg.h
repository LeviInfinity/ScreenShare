
// ScreenShareClientDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "UdpSocket.h"
#define WM_CUSTOM_NETWORK_MSG (WM_USER + 100) 

//TODO 文件发送时的文本发送问题
class CScreenShareClientDlg : public CDialogEx
{
// 构造
public:
	CScreenShareClientDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SCREENSHARECLIENT_DIALOG };
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
	afx_msg void OnBnClickedButtonTcp();//断开 、 连接TCP服务器
	afx_msg void OnClose();
	virtual void OnOK();
	afx_msg void OnBnClickedSharedScreen();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
public:
	CListBox m_listChat;
	CProgressCtrl m_progress;

	void OutputFormatError(int);
	void initSock();
	void setStatus(int t);//设置tcp状态，0为未连接，1为已连接
	void setUdpStatus();
	
	bool SendData(const char*, int len,bool isfile=false);
	bool handleHeaderPack(char buf[],int);
	void handleRecvFile(char buf[], int);
	SOCKET m_tcpSock;
	bool m_bIsRecvFile;//此标志判断是否正在接收文件
	CString m_recvFilePath;//接收文件的绝对路径

	long m_totalSize;//当前传输的文件的总大小
	long m_haveGetSize;//当前传输文件已传大小
	long m_currentPackSize;//文件中分包的大小
	long m_currentPackGetSize;//文件分包实际已得到的大小

	CString m_startSendAttackMsg = "###START_HEAD###";//开始发送文件时的第一个包
	CString m_recvPackAttackMsg = "###SUB_PACK###";//文件分包时的的包确认
	CString m_recvCurrentPackEndMsg = "###SUB_PACK_END###";//当前包发送完成确认
	
public:
	UdpSocket m_udpSocket;
	HANDLE m_hThread=0;//不断接收udp发送的截图线程句柄
	static DWORD WINAPI ReceiveScreenShotProc(LPVOID p);//不断接收udp发送的截图线程

	bool m_bSharedScreen = false; //当前是否在分享屏幕
	bool m_bFullScreen = false;//TODO 当前是否全屏
	CRect m_oldClientRect;
	bool m_bConnectTcp = false; //当前有没有连接到TCP服务器
	void SetFullScreen();

	CDC *m_pDC,*m_picDC;
};
