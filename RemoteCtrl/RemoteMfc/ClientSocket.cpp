#include "pch.h"
#include "ClientSocket.h"
//CServerSocket server;
CClientSocket* CClientSocket::m_instance = NULL;//显示的初始化，h文件的变量都只是声明
CClientSocket::CHelper CClientSocket::m_helper;//实现，调用构造函数

CClientSocket* pclient = CClientSocket::getInstance();//全局指针初始化

std::string GetErrInfo(int wsaErrCode) {//报错函数
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