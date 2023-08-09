#pragma once
#include "pch.h"
#include"framework.h"

class CServerSocket
{
public:
	static CServerSocket* getInstance(){//��ľ�̬��������ʹ��this���޷�ֱ�ӷ��ʳ�Ա����
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
		sockaddr_in client_adr;
		int cli_sz = sizeof(client_adr);
		m_client = accept(m_sock, (sockaddr*)&client_adr, &cli_sz);
		if (m_client == -1) {
			return false;
		}
		return true;
	}

	int DealCommand() {
		if (m_client == -1)return false;
		char buffer[1024];
		while (true) {
			int len = recv(m_client, buffer, sizeof(buffer), 0);
			if (len <= 0)return - 1;
			//TODO:��������
		}
	}

	bool Send(const char* pData, int nSize) {
		if (m_client == -1)return false;
		return send(m_client, pData, nSize, 0) > 0;
	}
private://����
	SOCKET m_sock,m_client;
	CServerSocket& operator=(const CServerSocket& serversock) {//��ֵ���캯��

	}
	CServerSocket(const CServerSocket& serversock) {//�������캯��
		m_sock = serversock.m_sock;
		m_client = serversock.m_client;
	}
	CServerSocket() {
		m_client = INVALID_SOCKET;//��һ����Чֵ-1  ��ʼ����
		if (InitSockEnv() == FALSE) {
			MessageBox(NULL, _T("�޷���ʼ���׽��ֻ����������������ã�"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
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
	static CServerSocket* m_instance;//��֤Ψһ
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