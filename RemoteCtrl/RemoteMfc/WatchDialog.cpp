﻿// WatchDialog.cpp: 实现文件
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
END_MESSAGE_MAP()


// CWatchDialog 消息处理程序


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