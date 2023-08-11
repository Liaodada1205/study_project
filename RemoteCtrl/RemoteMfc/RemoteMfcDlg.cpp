﻿
// RemoteMfcDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "RemoteMfc.h"
#include "RemoteMfcDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteMfcDlg 对话框



CRemoteMfcDlg::CRemoteMfcDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_REMOTEMFC_DIALOG, pParent)
	, m_server_address(0)
	, m_nPort(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteMfcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_IPAddress(pDX, IDC_IPADDRESS_SERV, m_server_address);
	DDX_Text(pDX, IDC_EDIT_PORT, m_nPort);
	DDX_Control(pDX, IDC_TREE_DIR, m_Tree);
}

int CRemoteMfcDlg::SendCommandPacket(int nCmd, bool bAutoClose, BYTE* pData, size_t nLength)
{
	UpdateData();//控件值拿到成员变量

	CClientSocket* pClient = CClientSocket::getInstance();
	bool ret = pClient->InitSocket(m_server_address, atoi((LPCTSTR)m_nPort));//使用控件变量取得的值
	if (!ret) {
		AfxMessageBox("网络初始化失败！");
		return -1;
	}

	CPacket pack(nCmd, pData, nLength);//客户端发送一个包，并接受
	ret = pClient->Send(pack);
	TRACE("send ret %d\r\n", ret);
	int cmd = pClient->DealCommand();
	TRACE("ack:%d\r\n", cmd);
	if(bAutoClose)
		pClient->CloseSocket();//是否自动关闭
	return cmd;
}

BEGIN_MESSAGE_MAP(CRemoteMfcDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_TEST, &CRemoteMfcDlg::OnBnClickedBtnTest)
	ON_BN_CLICKED(IDC_BTN_FILEINFO, &CRemoteMfcDlg::OnBnClickedBtnFileinfo)
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DIR, &CRemoteMfcDlg::OnNMDblclkTreeDir)
END_MESSAGE_MAP()


// CRemoteMfcDlg 消息处理程序

BOOL CRemoteMfcDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	UpdateData();
	m_server_address = 0x7F000001;
	m_nPort = _T("9527");
	UpdateData(false);//默认值mfc

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteMfcDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteMfcDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteMfcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRemoteMfcDlg::OnBnClickedBtnTest()
{
	// TODO: 在此添加控件通知处理程序代码
	SendCommandPacket(1981);
}


void CRemoteMfcDlg::OnBnClickedBtnFileinfo()//查看文件信息，根目录开始拿
{
	// TODO: 在此添加控件通知处理程序代码
	int ret = SendCommandPacket(1);//先查磁盘分区，目录信息
	if (ret == -1) {
		AfxMessageBox(_T("处理命令失败！"));
		return;
	}
	CClientSocket* pClient = CClientSocket::getInstance();
	std::string drivers = pClient->GetPacket().strData;
	std::string dr;
	m_Tree.DeleteAllItems();//每次获取的时候清空之前的树节点
	for (size_t i = 0; i < drivers.size(); i++)
	{
		if (drivers[i] == ',')
		{
			dr += ":";
			HTREEITEM hTemp = m_Tree.InsertItem(dr.c_str(),TVI_ROOT,TVI_LAST);//把dr的数据加入树节点,TVI_ROOT根目录下,TVI_LAST追加方式
			m_Tree.InsertItem(NULL, hTemp, TVI_LAST);//驱动一定是目录，这样有空子，好和文件区分

			dr.clear();
			continue;
		}
		dr += drivers[i];
	}

}
CString CRemoteMfcDlg::GetPath(HTREEITEM hTree) {//获取当前路径,从双击的目录开始往上遍历父母节点，得到路径
	CString strRet, strTmp;
	do {
		strTmp = m_Tree.GetItemText(hTree);
		strRet = strTmp + '\\' + strRet;
		hTree = m_Tree.GetParentItem(hTree);
	} while (hTree != NULL);
	return strRet;
}
void CRemoteMfcDlg::DeleteTreeChildrenItem(HTREEITEM hTree)
{
	HTREEITEM hSub = NULL;
	do {
		hSub = m_Tree.GetChildItem(hTree);
		if (hSub != NULL) m_Tree.DeleteItem(hSub);
	} while (hSub != NULL);
}

void CRemoteMfcDlg::OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult)//树列表的事件处理函数，
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse;
	GetCursorPos(&ptMouse);//鼠标的全局坐标
	m_Tree.ScreenToClient(&ptMouse);//全局转窗口坐标
	HTREEITEM hTreeSelected= m_Tree.HitTest(ptMouse,0);
	if (hTreeSelected == NULL) {
		return;
	}
	if (m_Tree.GetChildItem(hTreeSelected) == NULL)return;//如果是文件，没有子文件，不需要做后续处理，退出
	DeleteTreeChildrenItem(hTreeSelected);
	CString strPath = GetPath(hTreeSelected);
	int nCmd = SendCommandPacket(2,false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();//拿到packet包的数据，data是获得包的数据，单独开一个缓冲区的函数
	CClientSocket* pClient = CClientSocket::getInstance();
	while (pInfo->HasNext) {//有没有子文件，去处理  //收到的消息的展示
		//针对目录和不同文件类型，处理
		if (pInfo->IsDirectory) {
			TRACE("[%s] isdir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
			if (CString(pInfo->szFileName) == "." || (CString(pInfo->szFileName) == "..")) {
				int cmd = pClient->DealCommand();//处理下一个命令（处理同级或再下一级的文件info）
				TRACE("ack:%d\r\n", cmd);
				if (cmd < 0)break;
				pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
		}


		HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
		if (pInfo->IsDirectory) {
			m_Tree.InsertItem("", hTemp, TVI_LAST);//如果是目录，插一个空的
		}
		int cmd = pClient->DealCommand();//处理下一个命令（处理同级或再下一级的文件info）
		TRACE("ack:%d\r\n", cmd);
		if (cmd < 0)break;
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}
	pClient->CloseSocket();//用完关
}
