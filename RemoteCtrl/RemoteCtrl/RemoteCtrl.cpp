// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include"ServerSocket.h"
#include<direct.h>
#include<atlimage.h>//截图，图形操作

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
    CServerSocket::getInstance()->Send(pack);
    //CServerSocket::getInstance()->Send(pack);//拿到实例，打包发送数据, CPakect 重构，之前里边只有解包
    return 0;
}

#include<io.h>
#include<list>

int MakeDirectoryInfo() {
    std::string strPath;
    //std::list<FILEINFO>lstFileInfos;//文件信息列表
    if (CServerSocket::getInstance()->GetFilePath(strPath) == false) {
        OutputDebugString(_T("当前的命令，不是获取文件列表，命令解析错误！"));
        return -1;
    }
    if (_chdir(strPath.c_str()) != 0) {//访问目录不为0出错
        FILEINFO finfo;
      /*  finfo.IsInvalid = TRUE;
        finfo.IsDirectory = TRUE;*/
        finfo.HasNext = FALSE;
      /*  memcpy(finfo.szFileName, strPath.c_str(), strPath.size());*/
        //lstFileInfos.push_back(finfo);
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        OutputDebugString(_T("没有权限，访问目录！"));
        return -2;
    }
    _finddata_t fdata;//切换成功，遍历文件   文件查找的专用结构体，存路径查到的文件信息
    int hfind = 0;
    if ((hfind = _findfirst("*", &fdata)) == -1) {
        OutputDebugString(_T("没有找到任何文件！"));
        FILEINFO finfo;
        finfo.HasNext = FALSE;
        CPacket pack(2, (BYTE*)&finfo, sizeof(finfo));
        CServerSocket::getInstance()->Send(pack);
        return -3;
    }
    do {//有文件，就开始遍历，文件是树型结构，一层一层查
        FILEINFO finfo;
        finfo.IsDirectory = (fdata.attrib & _A_SUBDIR) != 0;//相与不为0，是文件夹
        memcpy(finfo.szFileName, fdata.name, strlen(fdata.name));//把fdata里找到的name拷给info
        TRACE("%s\r\n", finfo.szFileName);
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

int MouseEvent() {
    MOUSEEV mouse;
    if (CServerSocket::getInstance()->GetMouseEvent(mouse)) {
        DWORD nFlags = 0;//对不同操作设置的标志
        switch (mouse.nButton) {
        case 0://左键
            nFlags = 1;
            break;
        case 1://右键
            nFlags = 2;
            break;
        case 2://中键
            nFlags = 4;
            break;
        case 4://没有按键
            nFlags = 8;
            break;
        }
        if(nFlags!=8)        SetCursorPos(mouse.ptXY.x, mouse.ptXY.y);//设置鼠标位置
        switch (mouse.nAction) {
        case 0://单机
            nFlags |= 0x10;
            break;
        case 1://双击
            nFlags |= 0x20;
            break;
        case 2://按下
            nFlags |= 0x40;
            break;
        case 3://放开
            nFlags |= 0x80;
            break;
        default:
            break;
        }
        switch (nFlags)
        {
        case 0x21://左键双机      没有break再跑一次
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x11://左键单机
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());//GetMessageExtraInfo获取当前线程的额外信息，键盘鼠标，发送到当前鼠标键盘的应用里
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
  
        case 0x41://左键按下
            mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x81://左键放开
            mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x22://右键双机
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x12://右键单机
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x42://右键按下
            mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x82://右键放开
            mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
            break;

        case 0x24://中键双机
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
        case 0x14://中键单机
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x44://中键按下
            mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x84://中键放开
            mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
            break;
        case 0x08://单纯鼠标移动
            mouse_event(MOUSEEVENTF_MOVE, mouse.ptXY.x, mouse.ptXY.y, 0, GetMessageExtraInfo());
            break;
        }
        CPacket pack(5, NULL, 0);//应答读完包，表面收到并执行完毕
        CServerSocket::getInstance()->Send(pack);
    }
    else {
        OutputDebugString(_T("获取鼠标操作参数失败！"));
        return -1;
    }
    return 0;
}

int SendScreen() {
    CImage screen;//GDI全局设备接口（显示器）
    HDC hScreen = ::GetDC(NULL);//获取设备上下文,屏幕的句柄，方便获取要用的参数
    int nBitPerPixel = GetDeviceCaps(hScreen, BITSPIXEL);//获得位图bit数，位宽
    int nWidth = GetDeviceCaps(hScreen,HORZRES);//宽
    int nHeight = GetDeviceCaps(hScreen,VERTRES);//高
    screen.Create(nWidth, nHeight, nBitPerPixel);//按照显示器创建画布
    BitBlt(screen.GetDC(), 0, 0, 1920, 1020, hScreen, 0, 0, SRCCOPY); //把屏幕的图形从hScreen复制到当前屏幕screen.GetDC(),
    ReleaseDC(NULL, hScreen);

    HGLOBAL hMem =  GlobalAlloc(GMEM_MOVEABLE,0);//分配一个堆上的内存大小可调的句柄
    if (hMem == NULL)return -1;
    IStream* pStream = NULL;//创建一个可写的内存流
   HRESULT ret =  CreateStreamOnHGlobal(hMem, TRUE, &pStream);//全局对象上创建流，1全局 2是否在release时释放流 3流 。
   if (ret == S_OK) {
       screen.Save(pStream, Gdiplus::ImageFormatPNG);//pStream保存的是png的数据了
       //想通过pack发走，需要对流处理
       LARGE_INTEGER bg = { 0 };//
       pStream->Seek(bg, STREAM_SEEK_SET, NULL);//把流的指针放置回开头
       PBYTE pData = (PBYTE)GlobalLock(hMem);//把数据和hmem关联起来，能读到数据
       SIZE_T nSize = GlobalSize(hMem);
       CPacket pack(6, pData, nSize);//存到内存中并取得数据发送
       CServerSocket::getInstance()->Send(pack);
       GlobalUnlock(hMem);
   }
    //screen.Save(_T("text.png"), Gdiplus::ImageFormatPNG);
    //screen.Save(_T("text.JPG"), Gdiplus::ImageFormatJPEG);
   pStream->Release();
   GlobalFree(hMem);
    screen.ReleaseDC();
    return 0;
}
#include"LockInfoDialog.h"
CLockInfoDialog dlg;
unsigned threadid = 0;
//锁机情况下，主线程无法执行解锁程序了，所以要起子线程执行锁机
unsigned __stdcall threadLockDlg(void* arg) {
    dlg.Create(IDD_DIALOG_INFO, NULL);//创建窗口

    dlg.ShowWindow(SW_SHOW);

    CRect rect;//遮蔽后台的窗口
    rect.left = 0;
    rect.top = 0;
    rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
    rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN);
    dlg.MoveWindow(rect);//调整窗口的范围

    dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);//设置顶层显示,禁止调大小移动
    ShowCursor(false);//隐藏鼠标
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);//把任务栏隐藏

    rect.left = 0;
    rect.top = 0;
    rect.right = 1;
    rect.bottom = 1;
    ClipCursor(rect);    //限制鼠标的移动范围
    //没有消息循环，窗口立马析构了
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        if (msg.message == WM_KEYDOWN) {
            TRACE("msg:%08x wparam:%08x 1param:%08x\r\n", msg.message, msg.wParam, msg.lParam);
            if (msg.wParam == 0x41) {// 按下a退出
                break;
            }
        }
    }

    ShowCursor(true);
    ::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);//把任务栏显示，鼠标一样
    dlg.DestroyWindow();
    _endthreadex(0);//结束线程
    return 0;
}

int LockMachine() {
    //避免反复创建锁机线程
    if ((dlg.m_hWnd == NULL) || (dlg.m_hWnd == INVALID_HANDLE_VALUE)) {
        //_beginthread(threadLockDlg, 0, NULL);
        _beginthreadex(NULL, 0, threadLockDlg, NULL, 0, &threadid);//线程函数的结束和返回值和回调都要 和 ex定义的一致
    }
    CPacket pack(7, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;//返回一个应答，表示锁机成功
}

int UnlockMachine() {
    //dlg.SendMessage(WM_KEYDOWN, 0x41, 0x01E0001);
    //: : SendMessage(dlg.m _hWnd, wM_KEYDOw 0x41，Ox01EO001);
    //window消息泵，依赖于线程，只能拿到自己线程的消息！！！
    PostThreadMessage(threadid,WM_KEYDOWN, 0x41, 0);//向指定的线程发送消息
    CPacket pack(7, NULL, 0);
    CServerSocket::getInstance()->Send(pack);
    return 0;
}

int TestConnect() {
    CPacket pack(1891, NULL, 0);//发一个包回去
    bool ret = CServerSocket::getInstance()->Send(pack);
    TRACE("TestConnect send ret :%d\r\n", ret);
    return 0;
}

int ExcuteCommand(int nCmd) {
    int ret = 0;
    switch (nCmd)
    {
    case 1://查看磁盘分区
        ret = MakeDriverInfo();
        break;
    case 2://查看指定目录下的文件
        ret = MakeDirectoryInfo();
        break;
    case 3://打开文件
        ret = RunFile();
        break;
    case 4://文件下载
        ret = DownloadFile();
        break;
    case 5://鼠标操作
        ret = MouseEvent();
        break;
    case 6://发送屏幕内容=>发送屏幕的截图
        ret = SendScreen();
        break;
    case 7://锁机
        ret = LockMachine();
        break;
    case 8://解锁
        ret = UnlockMachine();
        break;
    case 1981://测试收发包
        ret = TestConnect();
        break;
    }
    return ret;
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

            CServerSocket* pserver = CServerSocket::getInstance();//创建服务器单例
            int count = 0;//记录重连次数
            if (pserver->InitSocket() == false) {
                MessageBox(NULL, _T("网络初始化异常，未能初始化，请检查网络设置！"), _T("网络初始化错误！"), MB_OK | MB_ICONERROR);
                exit(0);
            }
            while (CServerSocket::getInstance() != NULL) {//等价于while（true）  getInstance静态的自定义的类的对象，析构才为空
                if (pserver->AcceptClient() == false) {
                    if (count >= 3) {
                        MessageBox(NULL, _T("无法正常接入用户，结束程序！"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
                        exit(0);
                    }
                    MessageBox(NULL, _T("无法正常接入用户，自动重试！"), _T("接入用户失败！"), MB_OK | MB_ICONERROR);
                    count++;
                }
                TRACE("accept client return true \r\n");
                int ret = pserver->DealCommand();
                TRACE("DealCommand ret :%d\r\n", ret);
                if (ret > 0) {
                    ret = ExcuteCommand(ret);//(pserver->GetPacket().sCmd);//根据hCmd执行对应程序
                    if (ret != 0) {
                        TRACE("执行命令失败：%d ret = %d\r\n", pserver->GetPacket().sCmd, ret);
                    }
                    pserver->CloseClient();
                    TRACE("command has done \r\n");
                }
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
