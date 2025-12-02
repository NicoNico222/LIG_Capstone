#pragma once
#include "TabDlg1.h"
#include "TabDlg2.h"
#include "TabDlg3.h"

class CLIGCapstoneDlg : public CDialogEx
{
public:
	CLIGCapstoneDlg(CWnd* pParent = nullptr);
	CString m_loadedCsvPath;
	void RunInference(const CString& csvPath, int ci);
	void UpdateTab3Data(int ci);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LIG_CAPSTONE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;
	CBrush m_brushBg;
	CFont m_fontTab;

	CTabCtrl m_tabControl;

	CTabDlg1 m_tabDlg1;
	CTabDlg2 m_tabDlg2;
	CTabDlg3 m_tabDlg3;
	CDialog* m_pCurrentTab;

	bool m_bHasResult90;
	bool m_bHasResult95;
	RULGraphData m_rulData90;
	RULGraphData m_rulData95;
	PredictionGraphData m_predData90;
	PredictionGraphData m_predData95;
	CString m_rulText90;
	CString m_rulText95;

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTcnSelchangeTabPage(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnFileLoadCsv();
	DECLARE_MESSAGE_MAP()

private:
	void InitializeTabControl();
	void ShowTab(int nTab);
	void ClearResults();
};