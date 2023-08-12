// WatchDialog.cpp: 实现文件
//

#include "pch.h"
#include "RemoteMfc.h"
#include "WatchDialog.h"
#include "afxdialogex.h"
#include "RemoteMfcDlg.h"

// CWatchDialog 对话框

IMPLEMENT_DYNAMIC(CWatchDialog, CDialog)

CWatchDialog::CWatchDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_WATCH, pParent)
{
	m_nObjWidth = -1;
	m_nObjHeight = -1;
}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_WATCH, m_picture);
}


BEGIN_MESSAGE_MAP(CWatchDialog, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_STN_CLICKED(IDC_WATCH, &CWatchDialog::OnStnClickedWatch)
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


CPoint CWatchDialog::UserPoint2RemoteScreenPoint( CPoint& point,bool isScreen)
{
	CRect clientRect;
	if(isScreen)ScreenToClient(&point);//把本地的屏幕坐标转成监视窗口区域坐标，自带函数 ;默认直接获得的就是监视窗口的坐标

	m_picture.GetWindowRect(clientRect);//获得 监视窗口  的屏幕信息
	int width0 = clientRect.Width();
	int height0 = clientRect.Height();
	int x = point.x * m_nObjWidth / width0;
	int y = point.y * m_nObjHeight / height0;
	return CPoint(x,y);
}

BOOL CWatchDialog::OnInitDialog()//watchdlg 的初始化
{
	CDialog::OnInitDialog();

	// TODO:  在此添加额外的初始化
	SetTimer(0, 50, NULL);//50ms

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}


void CWatchDialog::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//取数据
	if (nIDEvent == 0) {
		CRemoteMfcDlg* pParent = (CRemoteMfcDlg*) GetParent();//获得父窗口
		if (pParent->isFull()) {
			CRect rect;
			m_picture.GetWindowRect(rect);//获得屏幕的信息
			if (m_nObjWidth == -1) {//获得宽高
				m_nObjWidth = pParent->GetImage().GetWidth();
			}
			if (m_nObjHeight == -1) {
				m_nObjHeight = pParent->GetImage().GetHeight();
			}
			//缓存是满的，显示在控件处理
			//DC  picture控件来显示
			//无缩放  pParent->GetImage().BitBlt(m_picture.GetDC()->GetSafeHdc(),0,0,SRCCOPY );//1HDC 2绘制位置  3绘制方式(直接拷贝）
			pParent->GetImage().StretchBlt(
				m_picture.GetDC()->GetSafeHdc(), 0, 0,rect.Width(),rect.Height(), SRCCOPY);
			m_picture.InvalidateRect(NULL);//通知重绘
			pParent->GetImage().Destroy();
			pParent->SetImageStatus();//设置isfull变为false
		}
	}
	CDialog::OnTimer(nIDEvent);
}


void CWatchDialog::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {//获取完宽高再执行
		// TODO: 在此添加消息处理程序代码和/或调用默认值
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 2;
		CRemoteMfcDlg* pParent = (CRemoteMfcDlg*)GetParent();//获得父窗口
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);//用父窗口的消息机制
	}
	CDialog::OnLButtonDblClk(nFlags, point);
}


void CWatchDialog::OnLButtonDown(UINT nFlags, CPoint point)
{
	//TODO: 在此添加消息处理程序代码和/或调用默认值
	//	坐标转换
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {//获取完宽高再执行
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 2;//摁下
		//CClientSocket* pClient = CClientSocket::getInstance();
		//CPacket pack(5, (BYTE*)&event, sizeof(event));
		//pClient->Send(pack);
		CRemoteMfcDlg* pParent = (CRemoteMfcDlg*)GetParent();//获得父窗口
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);//用父窗口的消息机制
	}
	CDialog::OnLButtonDown(nFlags, point);
}


void CWatchDialog::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//	坐标转换
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {//获取完宽高再执行
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;
		event.nAction = 3;//弹起
		CRemoteMfcDlg* pParent = (CRemoteMfcDlg*)GetParent();//获得父窗口
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);//用父窗口的消息机制
	}
	CDialog::OnLButtonUp(nFlags, point);
}


void CWatchDialog::OnRButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
		//坐标转换
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {//获取完宽高再执行
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;//右键
		event.nAction = 1;//双击
		CRemoteMfcDlg* pParent = (CRemoteMfcDlg*)GetParent();//获得父窗口
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);//用父窗口的消息机制
	}
	CDialog::OnRButtonDblClk(nFlags, point);
}


void CWatchDialog::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
		//坐标转换
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {//获取完宽高再执行
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 2;//摁下
		CRemoteMfcDlg* pParent = (CRemoteMfcDlg*)GetParent();//获得父窗口
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);//用父窗口的消息机制
	}
	CDialog::OnRButtonDown(nFlags, point);
}


void CWatchDialog::OnRButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
		//坐标转换
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {//获取完宽高再执行
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 1;
		event.nAction = 3;//弹起
		CRemoteMfcDlg* pParent = (CRemoteMfcDlg*)GetParent();//获得父窗口
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);//用父窗口的消息机制
	}
	CDialog::OnRButtonUp(nFlags, point);
}


void CWatchDialog::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
		//坐标转换
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {//获取完宽高再执行
		CPoint remote = UserPoint2RemoteScreenPoint(point);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 8;//没有按键
		event.nAction = 0;//移动
		/*CClientSocket* pClient = CClientSocket::getInstance();
		CPacket pack(5, (BYTE*)&event, sizeof(event));
		pClient->Send(pack);*/
		CRemoteMfcDlg* pParent = (CRemoteMfcDlg*)GetParent();//获得父窗口
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);//用父窗口的消息机制
	}
	CDialog::OnMouseMove(nFlags, point);
}


void CWatchDialog::OnStnClickedWatch()//窗口的点击事件
{
	// TODO: 在此添加控件通知处理程序代码
	if ((m_nObjWidth != -1) && (m_nObjHeight != -1)) {//获取完宽高再执行
		CPoint point;
		GetCursorPos(&point);//这里获得的是全局坐标，整个本地屏幕
		//坐标转换
		CPoint remote = UserPoint2RemoteScreenPoint(point, true);
		//封装
		MOUSEEV event;
		event.ptXY = remote;
		event.nButton = 0;//左键
		event.nAction = 0;//单机
		CRemoteMfcDlg* pParent = (CRemoteMfcDlg*)GetParent();//获得父窗口
		pParent->SendMessage(WM_SEND_PACKET, 5 << 1 | 1, (WPARAM) & event);//用父窗口的消息机制
	}
}


void CWatchDialog::OnOK()
{
	// TODO: 在此添加专用代码和/或调用基类

	//CDialog::OnOK();     是回车键失去作用
}
