#include "pch.h"
#include "ServerSocket.h"

//CServerSocket server;
CServerSocket* CServerSocket::m_instance = NULL;//显示的初始化，h文件的变量都只是声明
CServerSocket::CHelper CServerSocket::m_helper;//实现，调用构造函数

CServerSocket* psever = CServerSocket::getInstance();//全局指针初始化