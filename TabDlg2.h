#pragma once
#include "afxdialogex.h"
#include "resource.h"

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
	afx_msg void OnBnClickedRadio1();  // 추가: 90% 버튼 클릭
	afx_msg void OnBnClickedRadio2();  // 추가: 95% 버튼 클릭
	DECLARE_MESSAGE_MAP()

private:
	CBrush m_brushBg;
	CFont m_fontTitle;
	CFont m_fontGroupTitle;

	CImage m_imageBayesian;
	CImage m_imageRUL;

	int m_nSelectedCI;  // 추가: 선택된 CI 값 (0=선택안됨, 90, 95)

	void ArrangeControls(int cx, int cy);
	void InitializeUI();
	void LoadAndDisplayImages();
	void DisplayImage(UINT controlID, CImage& image);
	void UpdateCISelection();  // 추가: CI 선택 업데이트
public:
	afx_msg void OnBnClickedButton1();
};