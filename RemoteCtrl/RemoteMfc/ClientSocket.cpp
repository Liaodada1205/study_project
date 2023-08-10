#include "pch.h"
#include "ClientSocket.h"
//CServerSocket server;
CClientSocket* CClientSocket::m_instance = NULL;//显示的初始化，h文件的变量都只是声明
CClientSocket::CHelper CClientSocket::m_helper;//实现，调用构造函数

CClientSocket* pclient = CClientSocket::getInstance();//全局指针初始化