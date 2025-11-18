#pragma once
#include "afxdialogex.h"

class CTabDlg2 : public CDialog
{
	DECLARE_DYNAMIC(CTabDlg2)

public:
	CTabDlg2(CWnd* pParent = nullptr);
	virtual ~CTabDlg2();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_TAP2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

private:
	CBrush m_brushBg;
};