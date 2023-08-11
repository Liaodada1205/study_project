
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
	DDX_Control(pDX, IDC_LIST_FILE, m_List);
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
	ON_NOTIFY(NM_CLICK, IDC_TREE_DIR, &CRemoteMfcDlg::OnNMClickTreeDir)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_FILE, &CRemoteMfcDlg::OnNMRClickListFile)
	ON_COMMAND(ID_DOWNLOAD_FILE, &CRemoteMfcDlg::OnDownloadFile)
	ON_COMMAND(ID_DELETE_FILE, &CRemoteMfcDlg::OnDeleteFile)
	ON_COMMAND(ID_RUN_FILE, &CRemoteMfcDlg::OnRunFile)
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
void CRemoteMfcDlg::LoadFileInfo()
{
	CPoint ptMouse;
	GetCursorPos(&ptMouse);//鼠标的全局坐标
	m_Tree.ScreenToClient(&ptMouse);//全局转窗口坐标
	HTREEITEM hTreeSelected = m_Tree.HitTest(ptMouse, 0);
	if (hTreeSelected == NULL) {
		return;
	}
	if (m_Tree.GetChildItem(hTreeSelected) == NULL)return;//如果是文件，没有子文件，不需要做后续处理，退出
	DeleteTreeChildrenItem(hTreeSelected);
	m_List.DeleteAllItems();//清空所以文件列表的数据显示
	CString strPath = GetPath(hTreeSelected);
	int nCmd = SendCommandPacket(2, false, (BYTE*)(LPCTSTR)strPath, strPath.GetLength());
	PFILEINFO pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();//拿到packet包的数据，data是获得包的数据，单独开一个缓冲区的函数
	CClientSocket* pClient = CClientSocket::getInstance();
	while (pInfo->HasNext) {//有没有子文件，去处理  //收到的消息的展示
		//针对目录和不同文件类型，处理
		if (pInfo->IsDirectory) {//目录列表展示在一块  树list只有目录
			TRACE("[%s] isdir %d\r\n", pInfo->szFileName, pInfo->IsDirectory);
			if (CString(pInfo->szFileName) == "." || (CString(pInfo->szFileName) == "..")) {
				int cmd = pClient->DealCommand();//处理下一个命令（处理同级或再下一级的文件info）
				TRACE("ack:%d\r\n", cmd);
				if (cmd < 0)break;
				pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
				continue;
			}
			HTREEITEM hTemp = m_Tree.InsertItem(pInfo->szFileName, hTreeSelected, TVI_LAST);
			m_Tree.InsertItem("", hTemp, TVI_LAST);//如果是目录，插一个空的
			
		}
		else {//如果是文件 插到文件list里
			m_List.InsertItem(0, pInfo->szFileName);
		}

		
		int cmd = pClient->DealCommand();//处理下一个命令（处理同级或再下一级的文件info）
		TRACE("ack:%d\r\n", cmd);
		if (cmd < 0)break;
		pInfo = (PFILEINFO)CClientSocket::getInstance()->GetPacket().strData.c_str();
	}
	pClient->CloseSocket();//用完关
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
	LoadFileInfo();
}


void CRemoteMfcDlg::OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	LoadFileInfo();
}


void CRemoteMfcDlg::OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	CPoint ptMouse,ptList;
	GetCursorPos(&ptMouse);//鼠标的全局坐标
	ptList = ptMouse;
	m_List.ScreenToClient(&ptList);//全局转窗口坐标
	int ListSelected = m_List.HitTest(ptList);//返回一个序号，表面哪个列表被选中了
	if (ListSelected < 0)return;
	//点中，弹出一个菜单，处理任务
	CMenu menu;
	menu.LoadMenu(IDR_MENU_RCLICK);//加载菜单资源
	CMenu* pPupup = menu.GetSubMenu(0);//取子菜单的第一个
	if (pPupup != NULL) {
		pPupup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, this);//弹出子菜单
	}
	
}


void CRemoteMfcDlg::OnDownloadFile()
{
	// TODO: 在此添加命令处理程序代码    
	//用户选择了下载，然后会创建一个对话框，如果选择ok，才真正执行下载处理。
	int nListSelected = m_List.GetSelectionMark();//获得列表选择的标记
	CString strFile = m_List.GetItemText(nListSelected, 0);//获取文件名称  0第一条数据，文件名
	
	CFileDialog dlg(FALSE,"*",m_List.GetItemText(nListSelected, 0),
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,NULL,this);//获得路径()
	if (dlg.DoModal() == IDOK)//模态对话框
	{
		FILE* pFile = fopen(dlg.GetPathName(), "wb+");//路径，  写，二进制创建。
		if (pFile == NULL) {
			AfxMessageBox("本地没有权限保存该文件，或者无法创建！");
			return;
		}
		//选中文件也会选中路径，利用tree被选中的   来获得路径
		HTREEITEM hSelected = m_Tree.GetSelectedItem();
		strFile = GetPath(hSelected) + strFile;
		TRACE("%s\r\n", LPCSTR(strFile));
		int ret = SendCommandPacket(4, false, (BYTE*)(LPCSTR)strFile, strFile.GetLength());
		if (ret < 0) {
			AfxMessageBox("执行下载命令失败！");
			TRACE("执行下载失败： ret = %d\r\n", ret);
			return;
		}
		//成功处理流程 1.先处理长度的包，第一个包  
		CClientSocket* pClient = CClientSocket::getInstance();
		long long nLength = *(long long*)pClient->GetPacket().strData.c_str();
		if (nLength == 0) {
			AfxMessageBox("文件长度为0 或者 无法读取文件！");
			return;
		}

		long long nCount = 0;
		while (nCount < nLength) {//nlenth记录的是文件的长度，所以看够不够，不够继续去读命令拿数据
			ret = pClient->DealCommand();
			if (ret < 0) {
				AfxMessageBox("传输失败！");
				TRACE("传输失败： ret = %d\r\n", ret);
				break;
			}

			fwrite(pClient->GetPacket().strData.c_str(), 1, pClient->GetPacket().strData.size(), pFile);
			nCount += pClient->GetPacket().strData.size();
		}
		fclose(pFile);
		pClient->CloseSocket();
	}

}


void CRemoteMfcDlg::OnDeleteFile()
{
	// TODO: 在此添加命令处理程序代码
}


void CRemoteMfcDlg::OnRunFile()
{
	// TODO: 在此添加命令处理程序代码
}
