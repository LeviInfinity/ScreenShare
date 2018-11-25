#include "stdafx.h"
#include "UdpSocket.h"

const float n = 0.5;
UdpSocket::UdpSocket()
{
	m_picData = new char[1024 * 1024 * 2];//一开始分配2MB内存
	mdc = new CDC;
	m_rcvBmp = new CBitmap;
	nWidth = GetSystemMetrics(SM_CXSCREEN);        //屏幕宽度
	nHeight = GetSystemMetrics(SM_CYSCREEN);       //屏幕高度
	
}


UdpSocket::~UdpSocket()
{
	delete[]m_picData;
	delete mdc;
	delete m_rcvBmp;
	m_drawDC->DeleteDC();
	
}
void UdpSocket::OutputFormatError(int line)
{
	TCHAR szBuf[128];
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
	sprintf(szBuf, "(行号:%d 出错码=%d): %s", line, dw, lpMsgBuf);
	LocalFree(lpMsgBuf);
	AfxMessageBox(szBuf);

}
bool UdpSocket::initSocket()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
	//SOCKET m_listenSocket;
	SOCKADDR_IN serverAddr;
	int addrLen = sizeof(serverAddr);
	memset(&serverAddr, 0, addrLen);

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	serverAddr.sin_port = htons(8000);
	m_listenSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (m_listenSocket == INVALID_SOCKET)
	{
		OutputFormatError(__LINE__);
		WSACleanup();
		return false;
	}
	bool bOpt = true;
	setsockopt(m_listenSocket, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));
	setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&bOpt, sizeof(bOpt));//设置地址可重用
	int nRecvBuf = 1024 * 1024 * 5;//接收缓存5M
	if (setsockopt(m_listenSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&nRecvBuf, sizeof(int)) == SOCKET_ERROR)
		;// cout << "设置缓冲区失败" << endl;
	if (::bind(m_listenSocket, (sockaddr*)&serverAddr, addrLen) == SOCKET_ERROR)
	{
		OutputFormatError(__LINE__);
		WSACleanup();
		return false;
	}

	sin_from.sin_family = AF_INET;
	sin_from.sin_port = htons(8000);
	sin_from.sin_addr.s_addr = INADDR_BROADCAST;
	//sin_from.sin_addr.S_un.S_addr = INADDR_ANY;
	
	
	//u_long mode = 1;
	//ioctlsocket(m_listenSocket, FIONBIO, &mode);
	long recvLen = 1024 * 32 + sizeof(UdpHeader);
	setsockopt(m_listenSocket, SOL_SOCKET, SO_RCVBUF, (char*)&recvLen, sizeof(long));

/*	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(m_listenSocket, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));//设置发送超时
*/
	//需要先setDC
	mdc->CreateCompatibleDC(m_drawDC);
	m_rcvBmp->CreateCompatibleBitmap(m_drawDC, nWidth * n, nHeight * n);
	return true;
}
UdpHeader UdpSocket::paraseHeader(char *data, int len)
{
	UdpHeader tmp;
	if (len < headLen)
		;//cout << "error";
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
void UdpSocket::ReceiveSreenShot()
{
	long recvLen = 1024 * 32 + sizeof(UdpHeader);
	int addrLen = sizeof(sin_from);
	while (1)
	{
		char *buf = new char[recvLen];
		//此处使用的是阻塞socket
		int len = recvfrom(m_listenSocket, buf, recvLen, 0, (SOCKADDR*)&sin_from, &addrLen);
		if (len > 0)
		{
			recvData.push_back(buf);
			auto curHead = paraseHeader(buf, len);
			
			CString log;
			log.Format("ver: %d , %d\n", curHead.mainId, curHead.subId);
			Log::WrtiteLog(log);

			if (curHead.bOK)
			{
				vector<UdpHeader>recvHeader;
				recvHeader.reserve(recvData.size());
				m_totalPackNum += recvData.size();//总共接收的包
				for (auto p : recvData)//解析所有的header用于重组
				{
					recvHeader.push_back(paraseHeader(p, headLen + 1));
				}
				
		/* TODO ：原本计划是在一张图片接收完成后，查找丢失项发送给服务端，让它重新发送丢失项，但忽然又想到udp的广播
				如果各个客户端接收到的不一致，也就是说丢失的不一致，这样都给服务端发送丢失项那会乱套了，所以现在改成
				了每次发送一个包后延迟几毫秒，竟然大大降低了丢包的几率。
		另外一个想法，暂未手动实现，就是每个包发送两遍，这样就算丢一个也不要紧

			vector<int> loseItem = DetectLoseItem(recvHeader, curHead);//查找丢失图片的次版本号
				if (!loseItem.empty())
				{
					CString strLose="";
					for (auto &ele : loseItem)
					{
						CString tmp;
						tmp.Format("%d|", ele);
						strLose += tmp;
					}
					//此处先设置接收超时为1s，接收完丢失的包后再取消超时
					sendto(m_listenSocket, strLose, strLose.GetLength(), 0, (SOCKADDR*)&sin_from, addrLen);
					struct timeval tv = { 1,0 };
					setsockopt(m_listenSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));//设置接收超时

					int loseNum = loseItem.size();
					while (loseNum--)
					{
						int recvLoseItemLen = recvfrom(m_listenSocket, buf, recvLen, 0, (SOCKADDR*)&sin_from, &addrLen);
						if (recvLoseItemLen > 0)
						{
							m_totalPackNum++;
							recvData.push_back(buf);
							recvHeader.push_back(paraseHeader(buf, headLen + 1));
						}
					}
					tv.tv_sec = 0;
					setsockopt(m_listenSocket, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));//取消接收超时	
				}
				*/

				ConcatData(recvHeader,curHead);//拼接图片
				DrawScreenShot();//绘制截图
				DeleteCache(recvHeader, curHead.mainId);//删除缓存
				CalcQos(curHead.sendTime);//计算传输质量
				break;
			}
		}
		else
			delete[]buf;
	}//while(true)
}
/*vector<int> UdpSocket::DetectLoseItem(const vector<UdpHeader>&recvHeader, const UdpHeader &curHead)
{
	vector<int> loseItem;
	int mainId = curHead.mainId;
	for (int i = 0; i < curHead.subId; i++)
	{
		bool bFind = false;
		for (auto p : recvHeader)
		{
			if (p.mainId == mainId && p.subId == i)
			{
				bFind = true;
			}
		}
		if (!bFind)
		{
			loseItem.push_back(i);
		}

	}
	return loseItem;
}*/
void UdpSocket::ConcatData(const vector<UdpHeader>&recvHeader, const UdpHeader &curHead)
{
	long totalLen = 0;
	int mainId = curHead.mainId;
	int subId = curHead.subId;//此处的subId也是最大id，所以往下找
	for (int j = 0; j <= subId; j++)
	{
		for (int i = 0; i < recvHeader.size(); i++)
		{//
			if (recvHeader[i].subId == j&& recvHeader[i].mainId == mainId)
			{
				m_recvPackNum++;//有用的包
				memcpy(m_picData + totalLen, recvData[i] + headLen, recvHeader[i].dataLen);
				totalLen += recvHeader[i].dataLen;
			}
		}
	}
}
void UdpSocket::DrawScreenShot()
{
	//将重组后的图片绘制出来
	CRect rc;
	m_drawDC->GetWindow()->GetWindowRect(&rc);
	m_rcvBmp->SetBitmapBits(rc.Width()*rc.Height() * 4, m_picData);
	mdc->SelectObject(m_rcvBmp);
	m_drawDC->SetStretchBltMode(HALFTONE);
	m_drawDC->StretchBlt(0, 0,rc.Width(),rc.Height(), mdc, 0, 0, nWidth * n, nHeight * n, SRCCOPY);
}
void UdpSocket::DeleteCache(vector<UdpHeader>&recvHeader,int mainId)
{
	//为防止第一张图与后一张图的发送乱序，即第二张图的包在第一张最后一个包发送之前发送，所以清空不能直接全部清了
	vector<char*>tempRecvData;
	for (int i = 0; i < recvHeader.size(); i++)
	{
		if (recvHeader[i].mainId > mainId)
		{
			tempRecvData.push_back(recvData[i]);//保存下版本号高的数据
		}
		else
			delete[]recvData[i];//其他数据删除
	}
	recvData.swap(tempRecvData);
	tempRecvData.swap(vector<char*>());
	recvHeader.swap(vector<UdpHeader>());
//	totalLen = 0;
}
void UdpSocket::CalcQos(long curTime )
{
	m_losePercent = (1 - m_recvPackNum / (float)m_totalPackNum) * 100;//计算丢包率
	m_ping = curTime - m_lastPackTime;//计算与上一个包的延迟
	m_lastPackTime = curTime;
}