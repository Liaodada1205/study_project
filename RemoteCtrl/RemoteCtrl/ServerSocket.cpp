#include "pch.h"
#include "ServerSocket.h"

//CServerSocket server;
CServerSocket* CServerSocket::m_instance = NULL;//��ʾ�ĳ�ʼ����h�ļ��ı�����ֻ������
CServerSocket::CHelper CServerSocket::m_helper;//ʵ�֣����ù��캯��

CServerSocket* psever = CServerSocket::getInstance();//ȫ��ָ���ʼ��