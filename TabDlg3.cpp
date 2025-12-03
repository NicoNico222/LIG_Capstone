#include "pch.h"
#include "LIG_Capstone.h"
#include "afxdialogex.h"
#include "TabDlg3.h"

IMPLEMENT_DYNAMIC(CTabDlg3, CDialogEx)

CTabDlg3::CTabDlg3(CWnd* pParent)
	: CDialogEx(IDD_DLG_TAP3, pParent)
	, m_pRULGraphHelper(nullptr)
	, m_bRULDataLoaded(false)
	, m_bCacheValid(false)
{
}

CTabDlg3::~CTabDlg3()
{
	if (m_pRULGraphHelper != nullptr)
	{
		delete m_pRULGraphHelper;
		m_pRULGraphHelper = nullptr;
	}

	if (!m_cachedRulGraph.IsNull())
		m_cachedRulGraph.Destroy();
}

void CTabDlg3::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTabDlg3, CDialogEx)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CTabDlg3::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_brushBg.CreateSolidBrush(RGB(255, 255, 255));

	m_fontTitle.CreatePointFont(180, _T("맑은 고딕"));
	m_fontGroupTitle.CreatePointFont(140, _T("맑은 고딕"));
	m_fontMonth.CreatePointFont(230, _T("맑은 고딕"));

	m_pRULGraphHelper = new RULGraphHelper();

	InitializeUI();

	CWnd* pMonth = GetDlgItem(IDC_STATIC_MONTH2);
	if (pMonth != NULL)
	{
		pMonth->ModifyStyle(0, SS_CENTER | SS_CENTERIMAGE);
		pMonth->SetFont(&m_fontMonth);
	}

	CWnd* pCIBox = GetDlgItem(IDC_STATIC_RUL_CI_BOX);
	if (pCIBox != NULL)
	{
		pCIBox->ModifyStyle(0, SS_CENTER | SS_CENTERIMAGE);
		pCIBox->SetFont(&m_fontMonth);
	}

	CRect clientRect;
	GetClientRect(&clientRect);
	ArrangeControls(clientRect.Width(), clientRect.Height());

	return TRUE;
}

HBRUSH CTabDlg3::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

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

	return hbr;
}

void CTabDlg3::OnPaint()
{
	CPaintDC dc(this);

	if (m_bRULDataLoaded && m_pRULGraphHelper != nullptr)
	{
		CWnd* pWndRul = GetDlgItem(IDC_PICTURE_RUL);
		if (pWndRul != nullptr)
		{
			CRect rectRul;
			pWndRul->GetWindowRect(&rectRul);
			ScreenToClient(&rectRul);

			if (!m_bCacheValid || rectRul != m_lastRulRect)
			{
				if (!m_cachedRulGraph.IsNull())
					m_cachedRulGraph.Destroy();

				m_cachedRulGraph.Create(rectRul.Width(), rectRul.Height(), 32);

				CDC* pRulDC = CDC::FromHandle(m_cachedRulGraph.GetDC());
				CRect tempRect(0, 0, rectRul.Width(), rectRul.Height());
				m_pRULGraphHelper->DrawRULGraph(pRulDC, tempRect, m_rulGraphData);
				m_cachedRulGraph.ReleaseDC();

				m_bCacheValid = true;
				m_lastRulRect = rectRul;
			}

			m_cachedRulGraph.BitBlt(dc.m_hDC, rectRul.left, rectRul.top);
		}

		CWnd* pWndLegend = GetDlgItem(IDC_STATIC_LEGEND);
		if (pWndLegend != nullptr)
		{
			CRect rectLegend;
			pWndLegend->GetWindowRect(&rectLegend);
			ScreenToClient(&rectLegend);

			m_pRULGraphHelper->DrawLegend(&dc, rectLegend, m_rulGraphData.ci);
		}
	}
}

void CTabDlg3::OnDestroy()
{
	CDialogEx::OnDestroy();
}

void CTabDlg3::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	ArrangeControls(cx, cy);

	InvalidateCache();
	Invalidate();
}

void CTabDlg3::InitializeUI()
{
	CWnd* pRulMargin = GetDlgItem(IDC_STATIC_RUL_MARGIN2);
	CWnd* pRulCI = GetDlgItem(IDC_STATIC_RUL_CI);
	CWnd* pRulText = GetDlgItem(IDC_STATIC_RUL_TEXT);

	if (pRulMargin != NULL)
		pRulMargin->SetFont(&m_fontGroupTitle);
	if (pRulCI != NULL)
		pRulCI->SetFont(&m_fontGroupTitle);
	if (pRulText != NULL)
		pRulText->SetFont(&m_fontGroupTitle);
}

void CTabDlg3::ArrangeControls(int cx, int cy)
{
	if (cx <= 0 || cy <= 0) return;

	CWnd* pLegend = GetDlgItem(IDC_STATIC_LEGEND);

	CWnd* pRulCI = GetDlgItem(IDC_STATIC_RUL_CI);
	CWnd* pCIBox = GetDlgItem(IDC_STATIC_RUL_CI_BOX);

	CWnd* pRulMargin = GetDlgItem(IDC_STATIC_RUL_MARGIN2);
	CWnd* pMonth = GetDlgItem(IDC_STATIC_MONTH2);

	CWnd* pRulText = GetDlgItem(IDC_STATIC_RUL_TEXT);
	CWnd* pPictureRul = GetDlgItem(IDC_PICTURE_RUL);

	int margin = 20;
	int topMargin = 10;
	int titleHeight = 30;
	int groupHeight = 70;
	int spacingSmall = 5;

	int infoBoxWidth = 300;

	int legendLeftMargin = 40;
	int headerHeight = 110;

	int legendWidth = 360;
	if (pLegend != NULL)
	{
		pLegend->MoveWindow(legendLeftMargin, topMargin, legendWidth, headerHeight);
	}

	int halfWidth = (cx - margin * 3) / 2;
	int leftX = margin;
	int boxWidthCI = 250;

	int ciGroupX = leftX + (halfWidth - boxWidthCI) / 2 + 100;

	int currentY = topMargin;

	if (pRulCI != NULL)
		pRulCI->MoveWindow(ciGroupX, currentY, boxWidthCI, titleHeight);

	currentY += titleHeight + spacingSmall;
	if (pCIBox != NULL)
		pCIBox->MoveWindow(ciGroupX, currentY, boxWidthCI, groupHeight);

	int rightX = leftX + halfWidth + margin;
	currentY = topMargin;

	int rulGroupX = rightX + (halfWidth - infoBoxWidth) / 2;

	if (pRulMargin != NULL)
		pRulMargin->MoveWindow(rulGroupX, currentY, infoBoxWidth, titleHeight);

	currentY += titleHeight + spacingSmall;
	if (pMonth != NULL)
		pMonth->MoveWindow(rulGroupX, currentY, infoBoxWidth, groupHeight);

	int bottomY = topMargin + titleHeight + spacingSmall + groupHeight + spacingSmall;
	int graphStartY = bottomY;

	int graphTitleHeight = 30;

	if (pRulText != NULL)
	{
		pRulText->MoveWindow(margin, graphStartY, cx - margin * 2, graphTitleHeight);
	}

	int pictureY = graphStartY + graphTitleHeight + 10;
	int pictureHeight = cy - pictureY - margin;

	if (pPictureRul != NULL)
	{
		pPictureRul->MoveWindow(margin, pictureY, cx - margin * 2, pictureHeight);
	}
}

void CTabDlg3::InvalidateCache()
{
	m_bCacheValid = false;
}

void CTabDlg3::UpdateRULDisplay(const CString& text)
{
	CWnd* pMonth = GetDlgItem(IDC_STATIC_MONTH2);
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

void CTabDlg3::UpdateCIDisplay(int ci)
{
	CWnd* pCIBox = GetDlgItem(IDC_STATIC_RUL_CI_BOX);
	if (pCIBox != NULL)
	{
		CString ciText;
		ciText.Format(_T("RUL    %d%% CI"), ci);
		pCIBox->SetWindowText(ciText);

		CDC* pDC = pCIBox->GetDC();
		if (pDC != NULL)
		{
			pDC->SelectObject(&m_fontMonth);
			CRect textRect;
			pDC->DrawText(ciText, &textRect, DT_CALCRECT);
			pCIBox->ReleaseDC(pDC);
		}

		pCIBox->Invalidate();
		pCIBox->UpdateWindow();
	}
}

void CTabDlg3::LoadRULGraphData(const RULGraphData& data)
{
	m_rulGraphData = data;
	m_bRULDataLoaded = true;

	InvalidateCache();
	Invalidate();
}

void CTabDlg3::ResetUI()
{
	m_bRULDataLoaded = false;

	UpdateRULDisplay(_T(""));

	CWnd* pCIBox = GetDlgItem(IDC_STATIC_RUL_CI_BOX);
	if (pCIBox != NULL)
	{
		pCIBox->SetWindowText(_T(""));
	}

	InvalidateCache();
	Invalidate();
}