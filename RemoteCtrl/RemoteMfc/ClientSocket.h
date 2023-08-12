#pragma once

#include "pch.h"
#include"framework.h"
#include <string>
#include<vector>

#pragma pack(push)//保存当前字节对齐情况 确保数据包中的数据按格式获得
#pragma pack(1)//然后改成1
class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {	}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {//打包重构
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
		if (i + 4 + 2 + 2 > nSize) {//包数据可能不全，或者包头未能全部接收到
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
		for (size_t j = 0; j < strData.size(); j++) {
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

typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//点击、双击、移动
	WORD nButton;//左键、右键、中键
	POINT ptXY;//坐标
}MOUSEEV, * PMOUSEEV;

std::string GetErrInfo(int wsaErrCode);

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

class CClientSocket
{
public:
	static CClientSocket* getInstance() {//类的静态函数不能使用this，无法直接访问成员变量
		if (m_instance == NULL) {
			m_instance = new CClientSocket();
		}
		return m_instance;
	}
	bool InitSocket(int nIP,int nPort) {
		if (m_sock != INVALID_SOCKET) CloseSocket();//如果不是无效情况，需要清理套接字
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sock == -1) return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = htonl(nIP);//客户端需要指定ip地址   htonl解决字节序问题
		serv_adr.sin_port = htons(nPort);
		if (serv_adr.sin_addr.s_addr == INADDR_NONE) {
			AfxMessageBox("指定的IP地址不存在！");
			return false;
		}
		int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
		if (ret == -1) {
			AfxMessageBox("连接失败！");
			TRACE("连接失败：%d %s\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()).c_str());
			return false;
		}
		
		return true;
	}


#define BUFFER_SIZE 2048000//图片数据大，缓存装不下recv0，会一直循环在收包这里
	int DealCommand() {
		if (m_sock == -1)return -1;
		char* buffer = m_buffer.data();
		static size_t index = 0;
		while (true) {
			size_t len = recv(m_sock, buffer + index, BUFFER_SIZE - index, 0);
			if ((len <= 0)&&(index<=0)) {
				return -1;
			}
			index += len;
			len = index;//对整个缓冲区去处理
			m_packet = CPacket((BYTE*)buffer, len);//对接收的数据进行处理   len变为用掉的数据长度
			if (len > 0) {
				memmove(buffer, buffer + len, index - len);//把后续的数据前移
				index -= len;
				return m_packet.sCmd;
			}
		}
		return -1;
	}

	bool Send(const char* pData, int nSize) {
		if (m_sock == -1)return false;
		return send(m_sock, pData, nSize, 0) > 0;
	}
	bool Send(CPacket& pack) {
		TRACE("m_sock = %d\r\n", m_sock);
		if (m_sock == -1) return false;
		return send(m_sock, pack.Data(), pack.Size(), 0) > 0;
	}
	bool GetFilePath(std::string& strPath) {
		if ((m_packet.sCmd == 2) || (m_packet.sCmd == 3) || (m_packet.sCmd == 4)) {//如果命令是w2获取文件列表时，获取文件路径，就是strdata里的数据
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
	void CloseSocket() {
		closesocket(m_sock);
		m_sock = INVALID_SOCKET;
	}
private://单例
	std::vector<char> m_buffer;//1.动态分配 堆上  2.内存不用主动管理  3.直接取地址使用
	SOCKET m_sock;
	CPacket m_packet;
	CClientSocket& operator=(const CClientSocket& serversock) {//赋值构造函数

	}
	CClientSocket(const CClientSocket& serversock) {//拷贝构造函数
		m_sock = serversock.m_sock;
	}
	CClientSocket() {
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置！"), _T("初始化错误！"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		//改变套接字的创建方式，不在构造函数中创建。

		m_buffer.resize(BUFFER_SIZE);
		memset(m_buffer.data(), 0, BUFFER_SIZE);

	}

	~CClientSocket() {
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
			CClientSocket* tmp = m_instance;
			m_instance = NULL;
			delete tmp;
		}
	}
	static CClientSocket* m_instance;//保证唯一
	class CHelper {
	public:
		CHelper() {
			CClientSocket::getInstance();
		}
		~CHelper() {
			CClientSocket::releaseInstance();
		}
	};
	static CHelper m_helper;
};


//extern CServerSocket server;