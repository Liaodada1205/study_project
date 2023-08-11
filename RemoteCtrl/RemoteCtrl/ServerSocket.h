#pragma once
#include "pch.h"
#include"framework.h"
void Dump(BYTE* pData, size_t nSize);
#pragma pack(push)//保存当前字节对齐情况 确保数据包中的数据按格式获得
#pragma pack(1)//然后改成1
class CPacket {
public:
	CPacket() :sHead(0),nLength(0),sCmd(0),sSum(0){	}
	CPacket(WORD nCmd,const BYTE* pData,size_t nSize) {//打包重构
		sHead = 0xFEFF;
		nLength = nSize + 4;
		sCmd = nCmd;
		if (nSize > 0) {
			strData.resize(nSize);
			memcpy((void*)strData.c_str(), pData, nSize);
		}
		else {
			strData.clear();
		}

		sSum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sSum += BYTE(strData[j]) & 0xFF;
		}
	}

	CPacket(const CPacket& pack) {//拷贝构造
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket& operator=(const CPacket& pack) {//=运算符重载
		if (this != &pack) {//不等于赋值
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;//等于直接返回自己
	}
	CPacket(const BYTE* pData, size_t& nSize) {//包数据的解析
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;//防止只有两个字节  FEFF，读完就没了，不加判断不出来
				break;
			}
		}
		if (i+4+2+2 > nSize) {//包数据可能不全，或者包头未能全部接收到
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {//包接收不全，数据缺失，解析失败，返回
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);//去掉命令和校验的长度
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size();j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {//解析成功
			nSize = i;// nLength + 2 + 4   数据   head2   length4
			return;
		}
		nSize = 0;

	}
	~CPacket() {

	}
	int Size() {//获得报数据的大小
		return nLength + 6;
	}
	const char* Data() {//获得包的数据，单独开一个缓冲区
		strOut.resize(nLength + 6);
		BYTE* pData = (BYTE*)strOut.c_str();
		*(WORD*)pData = sHead; pData += 2;
		*(DWORD*)pData = nLength; pData += 4;
		*(WORD*)pData = sCmd; pData += 2;
		memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
		*(WORD*)pData = sSum; 
		return strOut.c_str();
	}

public:
	WORD sHead;//固定位FE FF
	DWORD nLength;//包长度（从控制命令开始，到和校验结束）
	WORD sCmd;//控制命令
	std::string strData;//包数据
	WORD sSum;//和校验
	std::string strOut;//整个包的数据
};

#pragma pack(pop)//还原对齐

typedef struct MouseEvent{
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击、双击、移动
	WORD nButton;//左键、右键、中键
	POINT ptXY;//坐标
}MOUSEEV,*PMOUSEEV;

typedef struct file_info {//结构体，用于处理文件信息的存储显示
	file_info() {//结构体也一样可以用构造函数
		IsInvalid = 0;
		IsDirectory = -1;//默认无效
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;//是否有效，有链接等文件存在，默认存在
	BOOL IsDirectory;//是否为目录  0否   1 是
	BOOL HasNext;//是否还有下一个文件，没有0   有1
	char szFileName[256];//文件名

}FILEINFO, * PFILEINFO;

class CServerSocket
{
public:
	static CServerSocket* getInstance(){//类的静态函数不能使用this，无法直接访问成员变量
		if ( m_instance == NULL) {
			m_instance = new CServerSocket();
		}
		return m_instance;
	}
	bool InitSocket() {
		if (m_sock == -1) return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = INADDR_ANY;
		serv_adr.sin_port = htons(9527);
		if (bind(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr)) == -1) {
			return false;
		}
		if (listen(m_sock, 1)== -1) {
			return false;
		}
		return true;
	}

	bool AcceptClient() {
		TRACE("enter AcceptClient\r\n");
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		TRACE("m_client = %d\r\n", m_client);
		if (m_client == -1) {
			return false;
		}
		return true;
	}
#define BUFFER_SIZE 4096
	int DealCommand() {
		if (m_client == -1)return -1;
		char* buffer = new char[BUFFER_SIZE];
		if (buffer == NULL) {
			TRACE("内存不足！\r\n");
			return -2;
		}
		memset(buffer, 0, BUFFER_SIZE);
		size_t index = 0;
		while (true) {
			size_t len = recv(m_client, buffer+index, BUFFER_SIZE-index, 0);
			if (len <= 0) {
				delete[]buffer;
				return -1;
			}
			TRACE("recv :%d\r\n", len);
			index += len;
			len = index;//对整个缓冲区去处理
			m_packet =  CPacket((BYTE*)buffer, len);//对接收的数据进行处理
			if (len > 0) {
				memmove(buffer, buffer + len, BUFFER_SIZE -len);//把后续的数据前移
				index -= len;
				delete[]buffer;
				return m_packet.sCmd;
			}
		}
		delete[]buffer;
		return -1;
	}

	bool Send(const char* pData, int nSize) {
		if (m_client == -1)return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack) {
		if (m_client == -1) return false;
		Dump((BYTE*)pack.Data(), pack.Size());
		return send(m_client, pack.Data(), pack.Size(),0) > 0;
	}
	bool GetFilePath(std::string& strPath) {
		if ((m_packet.sCmd == 2)|| (m_packet.sCmd == 3) || (m_packet.sCmd == 4) || (m_packet.sCmd == 9)){//如果命令是w2获取文件列表时，获取文件路径，就是strdata里的数据
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	bool GetMouseEvent(MOUSEEV& mouse) {//
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;//成功拿到mouseevent
		}
		return false;
	}
	CPacket& GetPacket() {
		return m_packet;
	}
	void CloseClient() {
		closesocket(m_client);
		m_client = INVALID_SOCKET;
	}
private://单例
	SOCKET m_sock,m_client;
	CPacket m_packet;
	CServerSocket& operator=(const CServerSocket& serversock) {//赋值构造函数

	}
	CServerSocket(const CServerSocket& serversock) {//拷贝构造函数
		m_sock = serversock.m_sock;
		m_client = serversock.m_client;
	}
	CServerSocket() {
		m_client = INVALID_SOCKET;//给一个无效值-1  初始化用
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
	}

	~CServerSocket() {
		closesocket(m_sock);
		WSACleanup();
	}

	BOOL InitSockEnv() {
		WSADATA data;
		if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
			return FALSE;
		}
		return TRUE;
	}

	static void releaseInstance() {
		if (m_instance != NULL) {
			CServerSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	static CServerSocket* m_instance;//保证唯一
	class CHelper {
	public:
		CHelper() {
			CServerSocket::getInstance();
		}
		~CHelper() {
			CServerSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};


//extern CServerSocket server;