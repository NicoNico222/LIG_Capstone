#pragma once
#include "afxdialogex.h"
#include "resource.h"
#include "RULGraphHelper.h"

class CTabDlg2 : public CDialog
{
	DECLARE_DYNAMIC(CTabDlg2)

public:
	CTabDlg2(CWnd* pParent = nullptr);
	int GetSelectedCI();
	virtual ~CTabDlg2();
	afx_msg void OnBnClickedRun();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_TAP2 };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedRadio1();
	afx_msg void OnBnClickedRadio2();
	DECLARE_MESSAGE_MAP()

private:
	CBrush m_brushBg;
	CFont m_fontTitle;
	CFont m_fontGroupTitle;
	CFont m_fontRadio;
	CFont m_fontMonth;

	int m_nSelectedCI;

	RULGraphHelper* m_pRULGraphHelper;
	RULGraphData m_rulGraphData;
	bool m_bRULDataLoaded;

	void ArrangeControls(int cx, int cy);
	void InitializeUI();
	void UpdateCISelection();

public:
	void ResetRadioButtons();
	void UpdateRULDisplay(const CString& text);
	void LoadRULGraphData(const RULGraphData& data);
	afx_msg void OnBnClickedButton1();
};