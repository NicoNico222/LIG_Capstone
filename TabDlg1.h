#pragma once
#include "afxdialogex.h"
#include "GraphHelper.h"

class CTabDlg1 : public CDialog
{
	DECLARE_DYNAMIC(CTabDlg1)

public:
	CTabDlg1(CWnd* pParent = nullptr);
	virtual ~CTabDlg1();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_TAP1 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

private:
	CBrush m_brushBg;
	CFont m_fontTitle;


	CGraphHelper* m_pGraphHelper;
	IMUData m_imuData;
	bool m_bDataLoaded;


	CImage m_cachedLeftGraph;
	CImage m_cachedRightGraph;
	bool m_bCacheValid;
	CRect m_lastLeftRect;
	CRect m_lastRightRect;

	void InitializeUI();
	void ArrangeControls(int cx, int cy);
	void InvalidateCache(); 

public:

	void LoadCSVFile(const CString& filePath);
};