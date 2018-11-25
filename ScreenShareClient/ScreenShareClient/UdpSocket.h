#pragma once
#include<vector>
using namespace std;
struct UdpHeader
{
	long mainId;
	int subId;
	clock_t sendTime;
	int bOK;
	long dataLen;
};
const int headLen = sizeof(UdpHeader);
class UdpSocket
{
public:
	UdpSocket();
	~UdpSocket();
	bool initSocket();
	void OutputFormatError(int);
	UdpHeader paraseHeader(char *data, int len);
	void ReceiveSreenShot();
	SOCKET m_listenSocket;
	SOCKADDR_IN sin_from;

	float m_losePercent;//丢包率
	int m_ping=0;//延迟ms
	clock_t m_lastPackTime = 0;//上一次收到包的时间

	long m_totalPackNum = 0;//总共收到的包的个数
	long m_recvPackNum = 0;//可用包的个数
	
	void setDrawDC(CDC *dc) { m_drawDC = dc; }

	void DrawScreenShot();//绘制截图
	void DeleteCache(vector<UdpHeader>&,int);//当收到一个最后一个包后删除缓存
	void CalcQos(long);//计算传输质量
	void ConcatData(const vector<UdpHeader>&recvHeader, const UdpHeader &curHead);//将图片数据拼接
	//vector<int> DetectLoseItem(const vector<UdpHeader>&recvHeader, const UdpHeader &curHead);

private:
	CDC *m_drawDC; //绘图的DC
	char *m_picData; //拼接的图片数据，为了不让它重复申请释放内存，所以当做成员变量，一次性申请
	CDC *mdc;//内存DC
	CBitmap *m_rcvBmp;//收到的数据转成CBitmap
	vector<char*>recvData;//接收到的分包数据存在这里，用于拼凑数据
	int nWidth, nHeight;
};

