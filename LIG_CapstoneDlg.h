#pragma once

class CLIGCapstoneDlg : public CDialogEx
{
public:
	CLIGCapstoneDlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LIG_CAPSTONE_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	HICON m_hIcon;
	CBrush m_brushBg;
	CBrush m_brushTabBg; // 탭 내부 배경색
	CFont m_fontTitle;
	CImage m_imageIMU;
	CImage m_imageDrift;

	// Tab Control
	CTabCtrl m_tabControl;
	int m_nCurrentTab; // 현재 선택된 탭 (0: 페이지1, 1: 페이지2)

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTcnSelchangeTabPage(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()

private:
	void InitializeUI();
	void InitializeTabControl();
	void LoadAndDisplayImages();
	void DisplayImage(UINT controlID, CImage& image);
	void ArrangeControls(int cx, int cy);
	void ShowTab(int nTab);
};