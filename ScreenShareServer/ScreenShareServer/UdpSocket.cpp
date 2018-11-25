#include "stdafx.h"
#include "UdpSocket.h"
#include "ScreenShareServer.h"
#include "ScreenShareServerDlg.h"
const float n = 0.5;
CUdpSocket::CUdpSocket()
{
	nWidth = GetSystemMetrics(SM_CXSCREEN);        //屏幕宽度
	nHeight = GetSystemMetrics(SM_CYSCREEN);       //屏幕高度
	DesktopWnd = ::GetDesktopWindow();             // 获取桌面句柄
	DesktophDC = ::GetDC(DesktopWnd);              // 为屏幕创建设备描述表
	MemoryhDC = CreateCompatibleDC(DesktophDC);    // 为屏幕设备描述表创建兼容的内存设备描述表
	hbm = CreateCompatibleBitmap(DesktophDC, nWidth * n, nHeight * n);// 创建一个与屏幕设备描述表兼容的位图

	bits = new BYTE[nWidth * nHeight * n * n * 4];
	fileLen = nWidth * nHeight * n * n * 4;
}


CUdpSocket::~CUdpSocket()
{
	delete[]bits;
	::ReleaseDC(DesktopWnd, DesktophDC);
	::DeleteDC(MemoryhDC);
	::DeleteObject(hbm);
}
/*CString CUdpSocket::GetIp()
{
	CString m_broadCastIp;
	char name[128] = { 0 };
	gethostname(name, 128);
	CString str;
	str.Format(_T("%s"), name);
	//AfxMessageBox(str);
	struct hostent *ph = gethostbyname(name);
	if (ph == NULL)
		AfxMessageBox("获取本机IP失败");
	else
	{
		in_addr addr;
		memcpy(&addr, ph->h_addr_list[0], sizeof(in_addr));
		m_broadCastIp = inet_ntoa(addr);
		int pos = m_broadCastIp.ReverseFind('.');
		m_broadCastIp = m_broadCastIp.Left(pos) + ".255"; //同一网段，最后一位为255
		//m_ip.SetWindowText(str);
	//	AfxMessageBox(m_broadCastIp);
	}
	return m_broadCastIp;
}*/

bool CUdpSocket::InitUdpSock()
{
//	CString m_broadCastIp = GetIp();
//	AfxMessageBox(m_broadCastIp);
	int addrLen = sizeof(m_serverAddr);
	memset(&m_serverAddr, 0, addrLen);
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_BROADCAST);//inet_addr(m_broadCastIp);//
	m_serverAddr.sin_port = htons(8000);
	m_udpSock = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_udpSock == INVALID_SOCKET)
	{
		WSACleanup();
		return false;
	}
	bool bOpt = true;
	//设置该套接字为广播类型  
	int res = setsockopt(m_udpSock, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));

	//设置200k缓冲区
	long sendBufLen=1024 * 32;
	int len = sizeof(long);
	setsockopt(m_udpSock, SOL_SOCKET, SO_SNDBUF, (char*)&sendBufLen, len);
	
	//getsockopt(m_udpSock, SOL_SOCKET, SO_SNDBUF, (char*)&sendLen, &len);
	//cout << sendLen << endl;

	if (res != 0)
		return false;
	return true;
}
/*vector<CString> SplitCString(CString strSource, CString ch)
{

	vector <CString> vecString;
	int iPos = 0;
	CString strTmp;
	strTmp = strSource.Tokenize(ch, iPos);
	while (strTmp.Trim() != _T(""))
	{
		vecString.push_back(strTmp);
		strTmp = strSource.Tokenize(ch, iPos);
	}
	return vecString;
}*/

void CUdpSocket::SendScreenShot()
{
	holdbm = (HBITMAP)SelectObject(MemoryhDC, hbm); // 加载新的位图hbm,返回MemoryhDC原先旧的位图给holdbm
	StretchBlt(MemoryhDC, 0, 0, nWidth * n, nHeight * n, DesktophDC, 0, 0, nWidth, nHeight, SRCCOPY);  // 绘制桌面到MemoryhDC中
	hbm = (HBITMAP)SelectObject(MemoryhDC, holdbm); // 还原MemoryhDC原先的位图,返回新的位图给hbm

	CBitmap *pbm;
	pbm = CBitmap::FromHandle(hbm);
	pbm->GetBitmapBits(nWidth * nHeight * n * n * 4 * sizeof(BYTE), bits);	
	/*本机绘制测试
	CScreenShareServerDlg *p = (CScreenShareServerDlg *)m_pDlg;
	CDC *pDC = p->GetDC();
	CDC *mdc = new CDC;
	CBitmap *bmp = new CBitmap;
	mdc->CreateCompatibleDC(pDC);
	bmp->CreateCompatibleBitmap(pDC, nWidth * n, nHeight * n);
	bmp->SetBitmapBits(nWidth * nHeight * n * n * 4, bits);
	mdc->SelectObject(bmp);
	pDC->StretchBlt(0, 0, nWidth * n, nHeight * n, mdc, 0, 0, nWidth * n, nHeight * n, SRCCOPY);
	pDC->DeleteDC();
	delete mdc;
	delete bmp;
*/
	
	int haveSendLen = 0;
	long sendBufLen = 1024 * 32;

	int subId = 0;
	while (haveSendLen < fileLen)
	{
		int curSendLen;//本次要发送的长度
		UdpHeader head;
		int headLen = sizeof(UdpHeader);
		if (fileLen - haveSendLen > sendBufLen)//如果剩余长度大于32k，则发送32k
		{
			curSendLen = sendBufLen;
			head.bOK = 0;
			head.mainId = m_sendId;
			head.subId = subId++;
			head.dataLen = sendBufLen;
			head.sendTime = clock();
		}
		else//否则发送剩余部分
		{
			curSendLen = fileLen - haveSendLen;
			head.bOK = 1;
			head.mainId = m_sendId;
			head.subId = subId++;
			head.dataLen = fileLen - haveSendLen;
			head.sendTime = clock();
		}
		byte *buf = new byte[curSendLen + headLen];
		memset(buf, 0, curSendLen + headLen);
		memcpy(buf, &head, headLen);//拷贝头部
		memcpy(buf + headLen, bits + haveSendLen, curSendLen);//拷贝当前数据

		int sendlen = sendto(m_udpSock, (char*)buf, curSendLen + headLen, 0, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
		haveSendLen += curSendLen;
		Sleep(6);

		/*
		TODO ：原本计划是在一张图片接收完成后，查找丢失项发送给服务端，让它重新发送丢失项，但忽然又想到udp的广播
				如果各个客户端接收到的不一致，也就是说丢失的不一致，这样都给服务端发送丢失项那会乱套了，所以现在改成
				了每次发送一个包后延迟几毫秒，竟然大大降低了丢包的几率。
		另外一个想法，暂未手动实现，就是每个包发送两遍，这样就算丢一个也不要紧
		if (head.bOK == 1)
		{
			struct timeval tv = { 1,0 };
			setsockopt(m_udpSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));//设置接收超时
			int addrLen = sizeof(m_serverAddr);
			char * recvBuf = new char[128];
			ZeroMemory(recvBuf, 128);
			int recvLen = recvfrom(m_udpSock, (char*)recvBuf, 128, 0, (sockaddr*)&m_serverAddr, &addrLen);
			if (recvLen > 0)
			{
				vector<CString> res = SplitCString(recvBuf, "|");
				
				for (int i = 0; i < res.size() - 1; i++)
				{
					head.bOK = 0;
					head.mainId = m_sendId;
					head.subId = atoi(res[i]);
					head.dataLen = sendBufLen;
					head.sendTime = clock();
					memset(buf, 0, sendBufLen + headLen);
					memcpy(buf, &head, headLen);//拷贝头部
					memcpy(buf + headLen, bits + atoi(res[i]) * sendBufLen, sendBufLen);//拷贝当前数据
					sendto(m_udpSock, (char*)buf, sendBufLen + headLen, 0, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
				}
			}
			delete[] recvBuf;
		}
		*/

		delete[]buf;

	}

	m_sendId++;
}