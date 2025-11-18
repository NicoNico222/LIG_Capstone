#pragma once
#include "afxdialogex.h"

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
	CImage m_imageIMU;
	CImage m_imageDrift;

	void InitializeUI();
	void LoadAndDisplayImages();
	void DisplayImage(UINT controlID, CImage& image);
	void ArrangeControls(int cx, int cy);
};