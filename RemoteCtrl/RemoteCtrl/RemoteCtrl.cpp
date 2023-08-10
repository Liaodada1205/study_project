// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include"ServerSocket.h"
#include<direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;

using namespace std;
void Dump(BYTE* pData,size_t nSize) {
    std::string strOut;
    for (size_t i = 0; i < nSize;i++) {
        char buf[8] = "";
        if (i > 0 &&( i % 16 == 0))strOut += "\n";
        snprintf(buf, sizeof(buf), "%02X", pData[i] & 0xFF);
        strOut += buf;
    }
    strOut += "\n";
    OutputDebugStringA(strOut.c_str());
}

int MakeDriverInfo() {//1a盘 2b盘。。26z盘
    std::string result;
    for (int i = 1; i <= 26; i++) {
        if (_chdrive(i) == 0) {//切换磁盘，返回0切换成功
            if (result.size() > 0)
                result += ',';//分割符
            result += 'A' + i - 1;
        }
    }
    CPacket pack(1, (BYTE*)result.c_str(), result.size());
    Dump((BYTE*)pack.Data(), pack.Size());

    //CServerSocket::getInstance()->Send(pack);//拿到实例，打包发送数据, CPakect 重构，之前里边只有解包
    return 0;
}

#include<io.h>
#include<list>
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

}FILEINFO,*PFILEINFO;
int MakeDirectoryInfo() {
    std::string strPath;
    //std::list<FILEINFO>lstFileInfos;//文件信息列表
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("当前的命令，不是获取文件列表，命令解析错误！"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0) {//访问目录不为0出错
        FILEINFO finfo;
        finfo.IsInvalid = TRUE;
        finfo.IsDirectory = TRUE;
        finfo.HasNext = FALSE;
        memcpy(finfo.szFileName, strPath.c_str(), strPath.size());
        //lstFileInfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有权限，访问目录！"));
        return -2;
    }
    _finddata_t fdata;//切换成功，遍历文件   文件查找的专用结构体，存路径查到的文件信息
    int hfind = 0;
    if (hfind = _findfirst("*", &fdata) == -1) {
        OutputDebugString(_T("没有找到任何文件！"));
        return -3;
    }
    do {//有文件，就开始遍历，文件是树型结构，一层一层查
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;//相与不为0，是文件夹
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));//把fdata里找到的name拷给info
        //lstFileInfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
    } while (!_findnext(hfind, &fdata));
    //发送文件，切片发送。防止文件路径太多，读完太久,所以舍弃list
    FILEINFO finfo;//退出do while循环后，发送一条结束的消息给控制端
    finfo.HasNext = FALSE;
    CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int RunFile() {
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    ShellExecuteA(NULL,NULL,strPath.c_str(),NULL,NULL,SW_SHOWNORMAL);//窗口句柄，操作，path，。。。  返回一个实例
     //根据对应的文件使用对应的程序打开文件
    CPacket pack(3, NULL,0);//应答消息执行完毕
    CServerSocket::getInstance()->Send(pack);
    return 0;          
}
int DownloadFile() {
    std::string strPath;
    CServerSocket::getInstance()->GetFilePath(strPath);
    long long data = 0;//专用的8位，用于传递消息传递和读不了的情况
    FILE* pFile = NULL;
    errno_t err = fopen_s(&pFile,strPath.c_str(), "rb");//fopen 报错，独占情况，读不到数据，返回值为空。用_s解决，调用完验证，读不到去处理
    if (err != 0) {
        CPacket pack(4, (BYTE*)&data, 8);//应答错误包
        CServerSocket::getInstance()->Send(pack);
        return -1;
    }
    if (pFile != NULL) {
        fseek(pFile, 0, SEEK_END);
        data = _ftelli64(pFile);//计算文件长度
        CPacket head(4, (BYTE*)&data, 8);
        //CServerSocket::getInstance()->Send(pack);
        fseek(pFile, 0, SEEK_SET);//q设置回来，文件指针！！

        char buffer[1024] = "";//tcp传输有上限
        size_t rlen = 0;
        do {
            rlen = fread(buffer, 1, 1024, pFile);
            CPacket pack(4, (BYTE*)buffer, rlen);
            CServerSocket::getInstance()->Send(pack);
        } while (rlen >= 1024);

        fclose(pFile);
    }
    CPacket pack(4, NULL, 0);//应答读完包
    CServerSocket::getInstance()->Send(pack);
    return 0;
}
int main()
{
    int nRetCode = 0;

    HMODULE hModule = ::GetModuleHandle(nullptr);

    if (hModule != nullptr)
    {
        // 初始化 MFC 并在失败时显示错误  
        if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
        {
            // TODO: 在此处为应用程序的行为编写代码。
            wprintf(L"错误: MFC 初始化失败\n");
            nRetCode = 1;
        }
        else
        {
            // TODO: 在此处为应用程序的行为编写代码。
            //1进度可控制 2对接方便 3可行性评估   服务器套接字
            //windows环境下  套接字初始化  整理到socket类h里了

            //CServerSocket* pserver = CServerSocket::getInstance();//创建服务器单例
            //int count = 0;//记录重连次数
            //if (pserver->InitSocket() == false) {
            //    MessageBox(NULL, _T("网络初始化异常，未能初始化，请检查网络设置！"), _T("网络初始化错误！"), MB_OK | MB_ICONERROR);
            //    exit(0);
            //}
            //while (CServerSocket::getInstance() != NULL) {
            //    if (pserver->AcceptClient() == false) {
            //        if (count >= 3) {
            //            MessageBox(NULL, _T("无法正常接入用户，结束程序！"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
            //            exit(0);
            //        }
            //        MessageBox(NULL, _T("无法正常接入用户，自动重试！"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
            //        count++;
            //    }
            //    int ret = pserver->DealCommand();
            //    //TODO:
            //}
            int nCmd = 1;
            switch (nCmd)
            {
            case 1://查看磁盘分区
                MakeDriverInfo();
                break;
            case 2://查看指定目录下的文件
                MakeDirectoryInfo();
                break;
            case 3://打开文件
                RunFile();
                break;
            case 4://文件下载
                DownloadFile();
                break;
            default:
                break;
            }
       

        }
    }
    else
    {
        // TODO: 更改错误代码以符合需要
        wprintf(L"错误: GetModuleHandle 失败\n");
        nRetCode = 1;
    }

    return nRetCode;
}
