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

}

CWatchDialog::~CWatchDialog()
{
}

void CWatchDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
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
			//缓存是满的，显示在控件处理

		}
	}
	CDialog::OnTimer(nIDEvent);
}
