#pragma once
#include "afxdialogex.h"
#include "RULGraphHelper.h"

class CTabDlg3 : public CDialogEx
{
	DECLARE_DYNAMIC(CTabDlg3)

public:
	CTabDlg3(CWnd* pParent = nullptr);
	virtual ~CTabDlg3();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_TAP3 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

private:
	CBrush m_brushBg;
	CFont m_fontTitle;
	CFont m_fontGroupTitle;
	CFont m_fontMonth;

	RULGraphHelper* m_pRULGraphHelper;
	RULGraphData m_rulGraphData;
	bool m_bRULDataLoaded;

	CImage m_cachedRulGraph;
	bool m_bCacheValid;
	CRect m_lastRulRect;

	void InitializeUI();
	void ArrangeControls(int cx, int cy);
	void InvalidateCache();

public:
	void UpdateRULDisplay(const CString& text);
	void UpdateCIDisplay(int ci);
	void LoadRULGraphData(const RULGraphData& data);

	void ResetUI();
};