#pragma once
#include "afxdialogex.h"
#include "resource.h"

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
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()

private:
	CBrush m_brushBg;
	CFont m_fontTitle;
	CFont m_fontGroupTitle;

	CImage m_imageBayesian;
	CImage m_imageRUL;

	void ArrangeControls(int cx, int cy);
	void InitializeUI();
	void LoadAndDisplayImages();
	void DisplayImage(UINT controlID, CImage& image);
};