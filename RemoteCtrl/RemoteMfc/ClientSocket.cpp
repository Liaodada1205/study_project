#include "pch.h"
#include "ClientSocket.h"
//CServerSocket server;
CClientSocket* CClientSocket::m_instance = NULL;//��ʾ�ĳ�ʼ����h�ļ��ı�����ֻ������
CClientSocket::CHelper CClientSocket::m_helper;//ʵ�֣����ù��캯��

CClientSocket* pclient = CClientSocket::getInstance();//ȫ��ָ���ʼ��

std::string GetErrInfo(int wsaErrCode) {//������
	std::string ret;
	LPVOID lpMsgBuf = NULL;
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
		NULL,
		wsaErrCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	ret = (char*)lpMsgBuf;
	LocalFree(lpMsgBuf);
	return ret;
}