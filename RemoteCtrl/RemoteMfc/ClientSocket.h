#pragma once

#include "pch.h"
#include"framework.h"
#include <string>
#include<vector>

#pragma pack(push)//���浱ǰ�ֽڶ������ ȷ�����ݰ��е����ݰ���ʽ���
#pragma pack(1)//Ȼ��ĳ�1
class CPacket {
public:
	CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {	}
	CPacket(WORD nCmd, const BYTE* pData, size_t nSize) {//����ع�
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

	CPacket(const CPacket& pack) {//��������
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	CPacket& operator=(const CPacket& pack) {//=���������
		if (this != &pack) {//�����ڸ�ֵ
			sHead = pack.sHead;
			nLength = pack.nLength;
			sCmd = pack.sCmd;
			strData = pack.strData;
			sSum = pack.sSum;
		}
		return *this;//����ֱ�ӷ����Լ�
	}
	CPacket(const BYTE* pData, size_t& nSize) {//�����ݵĽ���
		size_t i = 0;
		for (; i < nSize; i++) {
			if (*(WORD*)(pData + i) == 0xFEFF) {
				sHead = *(WORD*)(pData + i);
				i += 2;//��ֹֻ�������ֽ�  FEFF�������û�ˣ������жϲ�����
				break;
			}
		}
		if (i + 4 + 2 + 2 > nSize) {//�����ݿ��ܲ�ȫ�����߰�ͷδ��ȫ�����յ�
			nSize = 0;
			return;
		}
		nLength = *(DWORD*)(pData + i); i += 4;
		if (nLength + i > nSize) {//�����ղ�ȫ������ȱʧ������ʧ�ܣ�����
			nSize = 0;
			return;
		}
		sCmd = *(WORD*)(pData + i); i += 2;
		if (nLength > 4) {
			strData.resize(nLength - 2 - 2);//ȥ�������У��ĳ���
			memcpy((void*)strData.c_str(), pData + i, nLength - 4);
			i += nLength - 4;
		}
		sSum = *(WORD*)(pData + i); i += 2;
		WORD sum = 0;
		for (size_t j = 0; j < strData.size(); j++) {
			sum += BYTE(strData[j]) & 0xFF;
		}
		if (sum == sSum) {//�����ɹ�
			nSize = i;// nLength + 2 + 4   ����   head2   length4
			return;
		}
		nSize = 0;

	}
	~CPacket() {

	}
	int Size() {//��ñ����ݵĴ�С
		return nLength + 6;
	}
	const char* Data() {//��ð������ݣ�������һ��������
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
	WORD sHead;//�̶�λFE FF
	DWORD nLength;//�����ȣ��ӿ������ʼ������У�������
	WORD sCmd;//��������
	std::string strData;//������
	WORD sSum;//��У��
	std::string strOut;//������������
};

#pragma pack(pop)//��ԭ����

typedef struct MouseEvent {
	MouseEvent() {
		nAction = 0;
		nButton = -1;
		ptXY.x = 0;
		ptXY.y = 0;
	}
	WORD nAction;//�����˫�����ƶ�
	WORD nButton;//������Ҽ����м�
	POINT ptXY;//����
}MOUSEEV, * PMOUSEEV;

std::string GetErrInfo(int wsaErrCode);

typedef struct file_info {//�ṹ�壬���ڴ����ļ���Ϣ�Ĵ洢��ʾ
	file_info() {//�ṹ��Ҳһ�������ù��캯��
		IsInvalid = 0;
		IsDirectory = -1;//Ĭ����Ч
		HasNext = TRUE;
		memset(szFileName, 0, sizeof(szFileName));
	}
	BOOL IsInvalid;//�Ƿ���Ч�������ӵ��ļ����ڣ�Ĭ�ϴ���
	BOOL IsDirectory;//�Ƿ�ΪĿ¼  0��   1 ��
	BOOL HasNext;//�Ƿ�����һ���ļ���û��0   ��1
	char szFileName[256];//�ļ���

}FILEINFO, * PFILEINFO;

class CClientSocket
{
public:
	static CClientSocket* getInstance() {//��ľ�̬��������ʹ��this���޷�ֱ�ӷ��ʳ�Ա����
		if (m_instance == NULL) {
			m_instance = new CClientSocket();
		}
		return m_instance;
	}
	bool InitSocket(int nIP,int nPort) {
		if (m_sock != INVALID_SOCKET) CloseSocket();//���������Ч�������Ҫ�����׽���
		m_sock = socket(PF_INET, SOCK_STREAM, 0);
		if (m_sock == -1) return false;
		sockaddr_in serv_adr;
		memset(&serv_adr, 0, sizeof(serv_adr));
		serv_adr.sin_family = AF_INET;
		serv_adr.sin_addr.s_addr = htonl(nIP);//�ͻ�����Ҫָ��ip��ַ   htonl����ֽ�������
		serv_adr.sin_port = htons(nPort);
		if (serv_adr.sin_addr.s_addr == INADDR_NONE) {
			AfxMessageBox("ָ����IP��ַ�����ڣ�");
			return false;
		}
		int ret = connect(m_sock, (sockaddr*)&serv_adr, sizeof(serv_adr));
		if (ret == -1) {
			AfxMessageBox("����ʧ�ܣ�");
			TRACE("����ʧ�ܣ�%d %s\r\n", WSAGetLastError(), GetErrInfo(WSAGetLastError()).c_str());
			return false;
		}
		
		return true;
	}


#define BUFFER_SIZE 2048000//ͼƬ���ݴ󣬻���װ����recv0����һֱѭ�����հ�����
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
			len = index;//������������ȥ����
			m_packet = CPacket((BYTE*)buffer, len);//�Խ��յ����ݽ��д���   len��Ϊ�õ������ݳ���
			if (len > 0) {
				memmove(buffer, buffer + len, index - len);//�Ѻ���������ǰ��
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
		if ((m_packet.sCmd == 2) || (m_packet.sCmd == 3) || (m_packet.sCmd == 4)) {//���������w2��ȡ�ļ��б�ʱ����ȡ�ļ�·��������strdata�������
			strPath = m_packet.strData;
			return true;
		}
		return false;
	}
	bool GetMouseEvent(MOUSEEV& mouse) {//
		if (m_packet.sCmd == 5) {
			memcpy(&mouse, m_packet.strData.c_str(), sizeof(MOUSEEV));
			return true;//�ɹ��õ�mouseevent
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
private://����
	std::vector<char> m_buffer;//1.��̬���� ����  2.�ڴ治����������  3.ֱ��ȡ��ַʹ��
	SOCKET m_sock;
	CPacket m_packet;
	CClientSocket& operator=(const CClientSocket& serversock) {//��ֵ���캯��

	}
	CClientSocket(const CClientSocket& serversock) {//�������캯��
		m_sock = serversock.m_sock;
	}
	CClientSocket() {
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ����������������ã�"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
			exit(0);
		}
		//�ı��׽��ֵĴ�����ʽ�����ڹ��캯���д�����

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
	static CClientSocket* m_instance;//��֤Ψһ
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