#include "pch.h"
#include "ClientSocket.h"
//CServerSocket server;
CClientSocket* CClientSocket::m_instance = NULL;//��ʾ�ĳ�ʼ����h�ļ��ı�����ֻ������
CClientSocket::CHelper CClientSocket::m_helper;//ʵ�֣����ù��캯��

CClientSocket* pclient = CClientSocket::getInstance();//ȫ��ָ���ʼ��