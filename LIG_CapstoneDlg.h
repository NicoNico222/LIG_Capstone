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
	CFont m_fontTitle;
	CImage m_imageIMU;
	CImage m_imageDrift;

	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

private:
	void InitializeUI();
	void LoadAndDisplayImages();
	void DisplayImage(UINT controlID, CImage& image);
	void ArrangeControls(int cx, int cy);
};