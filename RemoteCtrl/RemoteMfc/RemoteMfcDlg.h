
// RemoteMfcDlg.h: 头文件
//

#pragma once
#include"ClientSocket.h"

// CRemoteMfcDlg 对话框
class CRemoteMfcDlg : public CDialogEx
{
// 构造
public:
	CRemoteMfcDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTEMFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

private:
	void LoadFileCurrent();
	void LoadFileInfo();//获取列表文件信息
	CString GetPath(HTREEITEM hTree);
	void DeleteTreeChildrenItem(HTREEITEM hTree);
	//1查看磁盘分区
	//2查看指定目录下的文件
	//3打开文件
	//4文件下载
	//5鼠标操作
	//6发送屏幕内容=>发送屏幕的截图
	//7锁机
	//8解锁
	//9删除文件
	//1981测试收发包
	//返回值是命令行，-1错误
	int SendCommandPacket(int nCmd,bool bAutoClose = true, BYTE* pData = NULL, size_t nLength = 0);//2参设置默认自动关闭
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnTest();
	DWORD m_server_address;
	CString m_nPort;
	afx_msg void OnBnClickedBtnFileinfo();
	CTreeCtrl m_Tree;
	afx_msg void OnNMDblclkTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClickTreeDir(NMHDR* pNMHDR, LRESULT* pResult);
	// 显示文件
	CListCtrl m_List;
	afx_msg void OnNMRClickListFile(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDownloadFile();
	afx_msg void OnDeleteFile();
	afx_msg void OnRunFile();
};
