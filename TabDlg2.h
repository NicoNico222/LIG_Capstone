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

	CString m_strRul90; // 90%일 때 표시할 텍스트 저장
	CString m_strRul95; // 95%일 때 표시할 텍스트 저장

	int m_nSelectedCI;

	RULGraphHelper* m_pRULGraphHelper;

	// --- [수정] 현재 화면에 표시할 데이터 ---
	RULGraphData m_rulGraphData;
	bool m_bRULDataLoaded;

	PredictionGraphData m_predGraphData;
	bool m_bPredDataLoaded;

	// --- [추가] 90%와 95% 결과를 각각 저장할 캐시 변수 ---
	PredictionGraphData m_predData90;
	PredictionGraphData m_predData95;
	bool m_bLoaded90; // 90% 데이터가 한 번이라도 로드되었는지 여부
	bool m_bLoaded95; // 95% 데이터가 한 번이라도 로드되었는지 여부

	// RUL 데이터(텍스트 등)도 CI별로 다를 수 있으므로 캐시 필요
	RULGraphData m_rulData90;
	RULGraphData m_rulData95;


	// 비트맵 캐싱
	CImage m_cachedPredGraph;
	bool m_bCacheValid;
	CRect m_lastPredRect;

	void ArrangeControls(int cx, int cy);
	void InitializeUI();
	void UpdateCISelection();
	void InvalidateCache();

	// --- [추가] 뷰 업데이트 헬퍼 함수 ---
	void UpdateViewFromCache();

public:
	void ResetRadioButtons();
	void UpdateRULDisplay(const CString& text);
	void LoadRULGraphData(const RULGraphData& data);
	void LoadPredictionGraphData(const PredictionGraphData& data);
	void SetRULTextCache(int ci, CString text);
	afx_msg void OnBnClickedButton1();
	
};