#include "pch.h"
#include "LIG_Capstone.h"
#include "afxdialogex.h"
#include "TabDlg2.h"
#include "LIG_CapstoneDlg.h"

IMPLEMENT_DYNAMIC(CTabDlg2, CDialog)

CTabDlg2::CTabDlg2(CWnd* pParent)
	: CDialog(IDD_DLG_TAP2, pParent)
	, m_nSelectedCI(0)
	, m_pRULGraphHelper(nullptr)
	, m_bRULDataLoaded(false)
	, m_bPredDataLoaded(false)
	, m_bCacheValid(false)
	, m_bLoaded90(false)
	, m_bLoaded95(false)
	, m_bShowLoadingText(false)
{
}

CTabDlg2::~CTabDlg2()
{
	if (m_pRULGraphHelper != nullptr)
	{
		delete m_pRULGraphHelper;
		m_pRULGraphHelper = nullptr;
	}

	if (!m_cachedPredGraph.IsNull())
		m_cachedPredGraph.Destroy();
}

void CTabDlg2::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTabDlg2, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_RADIO1, &CTabDlg2::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CTabDlg2::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_BTN_RUN, &CTabDlg2::OnBnClickedRun)
END_MESSAGE_MAP()

BOOL CTabDlg2::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_brushBg.CreateSolidBrush(RGB(255, 255, 255));

	m_fontTitle.CreatePointFont(180, _T("맑은 고딕"));
	m_fontGroupTitle.CreatePointFont(140, _T("맑은 고딕"));
	m_fontRadio.CreatePointFont(110, _T("맑은 고딕"));
	m_fontMonth.CreatePointFont(230, _T("맑은 고딕"));
	m_fontLoading.CreatePointFont(240, _T("맑은 고딕"));

	m_pRULGraphHelper = new RULGraphHelper();

	InitializeUI();

	CWnd* pMonth = GetDlgItem(IDC_STATIC_MONTH);
	if (pMonth != NULL)
	{
		pMonth->ModifyStyle(0, SS_CENTER | SS_CENTERIMAGE);
		pMonth->SetFont(&m_fontMonth);
	}

	CWnd* pLoading = GetDlgItem(IDC_STATIC_LOADING);
	if (pLoading != NULL)
	{
		pLoading->SetFont(&m_fontLoading);
		pLoading->ShowWindow(SW_HIDE);
	}

	CRect clientRect;
	GetClientRect(&clientRect);
	ArrangeControls(clientRect.Width(), clientRect.Height());

	CWnd* pRadio1 = GetDlgItem(IDC_RADIO1);
	CWnd* pRadio2 = GetDlgItem(IDC_RADIO2);
	if (pRadio1 != NULL)
	{
		pRadio1->Invalidate();
		pRadio1->UpdateWindow();
	}
	if (pRadio2 != NULL)
	{
		pRadio2->Invalidate();
		pRadio2->UpdateWindow();
	}

	return TRUE;
}

void CTabDlg2::LoadPredictionGraphData(const PredictionGraphData& data)
{
	if (data.ci == 90)
	{
		m_predData90 = data;
		m_bLoaded90 = true;
	}
	else if (data.ci == 95)
	{
		m_predData95 = data;
		m_bLoaded95 = true;
	}

	if (m_nSelectedCI == data.ci)
	{
		m_predGraphData = data;
		m_bPredDataLoaded = true;

		CWnd* pLoading = GetDlgItem(IDC_STATIC_LOADING);
		if (pLoading != NULL)
		{
			pLoading->ShowWindow(SW_HIDE);
		}

		InvalidateCache();
		Invalidate();
	}
}

HBRUSH CTabDlg2::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_DLG)
	{
		return m_brushBg;
	}

	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(0, 0, 0));
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	if (nCtlColor == CTLCOLOR_BTN)
	{
		pDC->SetBkColor(RGB(255, 255, 255));
		pDC->SetTextColor(RGB(0, 0, 0));
		return m_brushBg;
	}

	return hbr;
}

void CTabDlg2::OnPaint()
{
	CPaintDC dc(this);

	if (m_pRULGraphHelper != nullptr && m_bPredDataLoaded)
	{
		CWnd* pWndDrift = GetDlgItem(IDC_PICTURE_DRIFT);
		if (pWndDrift != nullptr)
		{
			CRect rectDrift;
			pWndDrift->GetWindowRect(&rectDrift);
			ScreenToClient(&rectDrift);

			if (!m_bCacheValid || rectDrift != m_lastPredRect)
			{
				if (!m_cachedPredGraph.IsNull())
					m_cachedPredGraph.Destroy();

				m_cachedPredGraph.Create(rectDrift.Width(), rectDrift.Height(), 32);

				CDC* pPredDC = CDC::FromHandle(m_cachedPredGraph.GetDC());
				CRect tempRect(0, 0, rectDrift.Width(), rectDrift.Height());
				m_pRULGraphHelper->DrawPredictionGraph(pPredDC, tempRect, m_predGraphData);
				m_cachedPredGraph.ReleaseDC();

				m_bCacheValid = true;
				m_lastPredRect = rectDrift;
			}

			m_cachedPredGraph.BitBlt(dc.m_hDC, rectDrift.left, rectDrift.top);
		}
	}
}

void CTabDlg2::OnDestroy()
{
	CDialog::OnDestroy();
}

void CTabDlg2::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	ArrangeControls(cx, cy);

	InvalidateCache();
	Invalidate();
}

void CTabDlg2::OnBnClickedRadio1()
{
	m_nSelectedCI = 90;
	UpdateViewFromCache();

	CLIGCapstoneDlg* pParent = (CLIGCapstoneDlg*)GetParent()->GetParent();
	if (pParent != nullptr)
	{
		pParent->UpdateTab3Data(90);
	}
}

void CTabDlg2::OnBnClickedRadio2()
{
	m_nSelectedCI = 95;
	UpdateViewFromCache();

	CLIGCapstoneDlg* pParent = (CLIGCapstoneDlg*)GetParent()->GetParent();
	if (pParent != nullptr)
	{
		pParent->UpdateTab3Data(95);
	}
}

void CTabDlg2::UpdateViewFromCache()
{
	bool bFound = false;

	if (m_nSelectedCI == 90 && m_bLoaded90)
	{
		m_predGraphData = m_predData90;
		m_rulGraphData = m_rulData90;
		bFound = true;

		UpdateRULDisplay(m_strRul90);
	}
	else if (m_nSelectedCI == 95 && m_bLoaded95)
	{
		m_predGraphData = m_predData95;
		m_rulGraphData = m_rulData95;
		bFound = true;

		UpdateRULDisplay(m_strRul95);
	}

	if (bFound)
	{
		m_bPredDataLoaded = true;
		m_bRULDataLoaded = true;
	}
	else
	{
		m_bPredDataLoaded = false;
		m_bRULDataLoaded = false;
		UpdateRULDisplay(_T(""));
	}

	InvalidateCache();
	Invalidate();
}

void CTabDlg2::UpdateCISelection()
{
	if (IsDlgButtonChecked(IDC_RADIO1) == BST_CHECKED)
	{
		m_nSelectedCI = 90;
	}
	else if (IsDlgButtonChecked(IDC_RADIO2) == BST_CHECKED)
	{
		m_nSelectedCI = 95;
	}
	else
	{
		m_nSelectedCI = 0;
	}
}

void CTabDlg2::InvalidateCache()
{
	m_bCacheValid = false;
}

void CTabDlg2::InitializeUI()
{
	CWnd* pRulPredict = GetDlgItem(IDC_STATIC_RUL_PREDICT);
	CWnd* pRulMargin = GetDlgItem(IDC_STATIC_RUL_MARGIN);
	CWnd* pDriftText = GetDlgItem(IDC_STATIC_DRIFT_TEXT);
	CWnd* pRulText = GetDlgItem(IDC_STATIC_RUL_TEXT);
	CWnd* pRadio1 = GetDlgItem(IDC_RADIO1);
	CWnd* pRadio2 = GetDlgItem(IDC_RADIO2);

	if (pRulPredict != NULL)
		pRulPredict->SetFont(&m_fontGroupTitle);
	if (pRulMargin != NULL)
		pRulMargin->SetFont(&m_fontGroupTitle);
	if (pDriftText != NULL)
		pDriftText->SetFont(&m_fontGroupTitle);
	if (pRulText != NULL)
		pRulText->SetFont(&m_fontGroupTitle);

	if (pRadio1 != NULL)
		pRadio1->SetFont(&m_fontRadio);
	if (pRadio2 != NULL)
		pRadio2->SetFont(&m_fontRadio);
}

void CTabDlg2::ArrangeControls(int cx, int cy)
{
	if (cx <= 0 || cy <= 0) return;

	CWnd* pRulPredict = GetDlgItem(IDC_STATIC_RUL_PREDICT);
	CWnd* pRulBox = GetDlgItem(IDC_STATIC_RUL_BOX);
	CWnd* pRadio1 = GetDlgItem(IDC_RADIO1);
	CWnd* pRadio2 = GetDlgItem(IDC_RADIO2);
	CWnd* pRulMargin = GetDlgItem(IDC_STATIC_RUL_MARGIN);
	CWnd* pMonth = GetDlgItem(IDC_STATIC_MONTH);
	CWnd* pBtnRun = GetDlgItem(IDC_BTN_RUN);
	CWnd* pDriftText = GetDlgItem(IDC_STATIC_DRIFT_TEXT);
	CWnd* pPictureDrift = GetDlgItem(IDC_PICTURE_DRIFT);
	CWnd* pLoading = GetDlgItem(IDC_STATIC_LOADING);

	int margin = 20;
	int topMargin = 10;
	int titleHeight = 30;
	int groupHeight = 70;
	int spacingSmall = 5;

	int halfWidth = (cx - margin * 3) / 2;

	int leftX = margin;
	int currentY = topMargin;

	int boxWidth = 250;
	int btnWidth = 100;
	int boxCenterX = (cx - boxWidth) / 2;

	if (pRulPredict != NULL)
	{
		pRulPredict->MoveWindow(boxCenterX, currentY, boxWidth, titleHeight);
		currentY += titleHeight + spacingSmall;
	}

	if (pRulBox != NULL)
	{
		pRulBox->MoveWindow(boxCenterX, currentY, boxWidth, groupHeight);

		if (pRadio1 != NULL && pRadio2 != NULL)
		{
			int radioWidth = 80;
			int radioHeight = 22;
			int radioCenterX = boxCenterX + (boxWidth - radioWidth) / 2;

			int availableSpace = groupHeight;
			int totalRadioHeight = radioHeight * 2;
			int spacing = (availableSpace - totalRadioHeight) / 3;

			int radio1Y = currentY + spacing;
			int radio2Y = currentY + spacing + radioHeight + spacing;

			pRadio1->MoveWindow(radioCenterX, radio1Y, radioWidth, radioHeight);
			pRadio2->MoveWindow(radioCenterX, radio2Y, radioWidth, radioHeight);
		}
	}

	if (pBtnRun != NULL)
	{
		int btnX = boxCenterX + boxWidth + margin;
		pBtnRun->MoveWindow(btnX, currentY, btnWidth, groupHeight);
	}

	int rightX = leftX + halfWidth + margin;
	currentY = topMargin;

	int marginBoxWidth = 300;
	int marginCenterX = rightX + (halfWidth - marginBoxWidth) / 2;

	if (pRulMargin != NULL)
	{
		pRulMargin->MoveWindow(marginCenterX, currentY, marginBoxWidth, titleHeight);
		currentY += titleHeight + spacingSmall;
	}

	if (pMonth != NULL)
	{
		pMonth->MoveWindow(marginCenterX, currentY, marginBoxWidth, groupHeight);
	}

	int bottomY = topMargin + titleHeight + spacingSmall + groupHeight + spacingSmall;
	int imageHeight = cy - bottomY - margin - titleHeight - spacingSmall;

	currentY = bottomY;
	if (pDriftText != NULL)
	{
		pDriftText->MoveWindow(leftX, currentY, cx - margin * 2, titleHeight);
	}

	currentY += titleHeight + spacingSmall;
	if (pPictureDrift != NULL)
	{
		pPictureDrift->MoveWindow(leftX, currentY, cx - margin * 2, imageHeight);
	}

	if (pLoading != NULL && pPictureDrift != NULL)
	{
		CRect rectDrift;
		pPictureDrift->GetWindowRect(&rectDrift);
		ScreenToClient(&rectDrift);

		int loadingWidth = 400;
		int loadingHeight = 50;
		int loadingX = rectDrift.left + (rectDrift.Width() - loadingWidth) / 2;
		int loadingY = rectDrift.top + (rectDrift.Height() - loadingHeight) / 2;

		pLoading->MoveWindow(loadingX, loadingY, loadingWidth, loadingHeight);
	}
}

int CTabDlg2::GetSelectedCI()
{
	if (IsDlgButtonChecked(IDC_RADIO1) == BST_CHECKED)
		return 90;
	else if (IsDlgButtonChecked(IDC_RADIO2) == BST_CHECKED)
		return 95;
	else
		return 0;
}

void CTabDlg2::OnBnClickedRun()
{
	int ci = GetSelectedCI();

	if (ci == 0) {
		AfxMessageBox(_T("CI 값을 선택해주세요."));
		return;
	}

	CLIGCapstoneDlg* pParent = (CLIGCapstoneDlg*)GetParent()->GetParent();

	if (pParent->m_loadedCsvPath.IsEmpty()) {
		AfxMessageBox(_T("CSV 파일을 먼저 페이지1에서 로드해주세요."));
		return;
	}

	CWnd* pLoading = GetDlgItem(IDC_STATIC_LOADING);
	if (pLoading != NULL)
	{
		pLoading->SetWindowText(_T("베이지안 모델 학습중..."));
		pLoading->ShowWindow(SW_SHOW);
		pLoading->UpdateWindow();
	}

	pParent->RunInference(pParent->m_loadedCsvPath, ci);
}

void CTabDlg2::ResetRadioButtons()
{
	CheckDlgButton(IDC_RADIO1, BST_UNCHECKED);
	CheckDlgButton(IDC_RADIO2, BST_UNCHECKED);

	m_nSelectedCI = 0;

	m_bPredDataLoaded = false;
	m_bRULDataLoaded = false;

	m_bLoaded90 = false;
	m_bLoaded95 = false;
	m_strRul90.Empty();
	m_strRul95.Empty();

	CWnd* pLoading = GetDlgItem(IDC_STATIC_LOADING);
	if (pLoading != NULL)
	{
		pLoading->ShowWindow(SW_HIDE);
	}

	UpdateRULDisplay(_T(""));

	InvalidateCache();
	Invalidate();
}

void CTabDlg2::UpdateRULDisplay(const CString& text)
{
	CWnd* pMonth = GetDlgItem(IDC_STATIC_MONTH);
	if (pMonth != NULL)
	{
		pMonth->SetWindowText(text);

		CDC* pDC = pMonth->GetDC();
		if (pDC != NULL)
		{
			pDC->SelectObject(&m_fontMonth);
			CRect textRect;
			pDC->DrawText(text, &textRect, DT_CALCRECT);
			pMonth->ReleaseDC(pDC);
		}

		pMonth->Invalidate();
		pMonth->UpdateWindow();
	}
}

void CTabDlg2::LoadRULGraphData(const RULGraphData& data)
{
	if (data.ci == 90)
	{
		m_rulData90 = data;
	}
	else if (data.ci == 95)
	{
		m_rulData95 = data;
	}

	if (m_nSelectedCI == data.ci)
	{
		m_rulGraphData = data;
		m_bRULDataLoaded = true;
		Invalidate();
	}
}

void CTabDlg2::SetRULTextCache(int ci, CString text)
{
	if (ci == 90) m_strRul90 = text;
	else if (ci == 95) m_strRul95 = text;
}