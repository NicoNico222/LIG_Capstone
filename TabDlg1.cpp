#include "pch.h"
#include "LIG_Capstone.h"
#include "afxdialogex.h"
#include "TabDlg1.h"

IMPLEMENT_DYNAMIC(CTabDlg1, CDialog)

CTabDlg1::CTabDlg1(CWnd* pParent)
	: CDialog(IDD_DLG_TAP1, pParent)
	, m_pGraphHelper(nullptr)
	, m_bDataLoaded(false)
	, m_bCacheValid(false)
{
}

CTabDlg1::~CTabDlg1()
{
	if (m_pGraphHelper != nullptr)
	{
		delete m_pGraphHelper;
		m_pGraphHelper = nullptr;
	}

	if (!m_cachedLeftGraph.IsNull())
		m_cachedLeftGraph.Destroy();
	if (!m_cachedRightGraph.IsNull())
		m_cachedRightGraph.Destroy();
}

void CTabDlg1::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTabDlg1, CDialog)
	ON_WM_CTLCOLOR()
	ON_WM_PAINT()
	ON_WM_DESTROY()
	ON_WM_SIZE()
END_MESSAGE_MAP()

BOOL CTabDlg1::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_brushBg.CreateSolidBrush(RGB(255, 255, 255));

	m_pGraphHelper = new CGraphHelper();

	InitializeUI();

	return TRUE;
}

HBRUSH CTabDlg1::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
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

	return hbr;
}

void CTabDlg1::OnPaint()
{
	CPaintDC dc(this);

	if (m_bDataLoaded && m_pGraphHelper != nullptr)
	{
		CWnd* pWndLeft = GetDlgItem(IDC_PICTURE_IMU);
		CWnd* pWndRight = GetDlgItem(IDC_PICTURE_DRIFT);

		if (pWndLeft != nullptr && pWndRight != nullptr)
		{
			CRect rectLeft, rectRight;
			pWndLeft->GetWindowRect(&rectLeft);
			ScreenToClient(&rectLeft);
			pWndRight->GetWindowRect(&rectRight);
			ScreenToClient(&rectRight);

			if (!m_bCacheValid ||
				rectLeft != m_lastLeftRect ||
				rectRight != m_lastRightRect)
			{
				if (!m_cachedLeftGraph.IsNull())
					m_cachedLeftGraph.Destroy();
				if (!m_cachedRightGraph.IsNull())
					m_cachedRightGraph.Destroy();

				m_cachedLeftGraph.Create(rectLeft.Width(), rectLeft.Height(), 32);
				m_cachedRightGraph.Create(rectRight.Width(), rectRight.Height(), 32);

				CDC* pLeftDC = CDC::FromHandle(m_cachedLeftGraph.GetDC());
				CRect tempRect(0, 0, rectLeft.Width(), rectLeft.Height());
				m_pGraphHelper->DrawGraph(pLeftDC, tempRect, m_imuData, true);
				m_cachedLeftGraph.ReleaseDC();

				CDC* pRightDC = CDC::FromHandle(m_cachedRightGraph.GetDC());
				tempRect = CRect(0, 0, rectRight.Width(), rectRight.Height());
				m_pGraphHelper->DrawGraph(pRightDC, tempRect, m_imuData, false);
				m_cachedRightGraph.ReleaseDC();

				m_bCacheValid = true;
				m_lastLeftRect = rectLeft;
				m_lastRightRect = rectRight;
			}

			m_cachedLeftGraph.BitBlt(dc.m_hDC, rectLeft.left, rectLeft.top);
			m_cachedRightGraph.BitBlt(dc.m_hDC, rectRight.left, rectRight.top);
		}
	}
}

void CTabDlg1::OnDestroy()
{
	CDialog::OnDestroy();
}

void CTabDlg1::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	ArrangeControls(cx, cy);

	InvalidateCache();
	Invalidate();
}

void CTabDlg1::InitializeUI()
{
	m_fontTitle.CreatePointFont(180, _T("맑은 고딕"));

	CWnd* pTitleLeft = GetDlgItem(IDC_STATIC_TITLE_LEFT);
	CWnd* pTitleRight = GetDlgItem(IDC_STATIC_TITLE_RIGHT);

	if (pTitleLeft != NULL)
		pTitleLeft->SetFont(&m_fontTitle);
	if (pTitleRight != NULL)
		pTitleRight->SetFont(&m_fontTitle);

	CRect clientRect;
	GetClientRect(&clientRect);
	ArrangeControls(clientRect.Width(), clientRect.Height());
}

void CTabDlg1::ArrangeControls(int cx, int cy)
{
	int contentMargin = 20;
	int titleHeight = 40;
	int titleToImageSpacing = 20;

	int halfWidth = (cx - contentMargin * 3) / 2;
	int availableImageHeight = cy - contentMargin * 2 - titleHeight - titleToImageSpacing;

	int contentLeft = contentMargin;
	int contentTop = contentMargin;

	CWnd* pTitleLeft = GetDlgItem(IDC_STATIC_TITLE_LEFT);
	CWnd* pTitleRight = GetDlgItem(IDC_STATIC_TITLE_RIGHT);
	CWnd* pPictureIMU = GetDlgItem(IDC_PICTURE_IMU);
	CWnd* pPictureDrift = GetDlgItem(IDC_PICTURE_DRIFT);

	if (pTitleLeft != NULL)
	{
		int titleY = contentTop + (titleHeight / 4);
		pTitleLeft->MoveWindow(contentLeft, titleY, halfWidth, titleHeight);
	}

	if (pTitleRight != NULL)
	{
		int titleY = contentTop + (titleHeight / 4);
		pTitleRight->MoveWindow(contentLeft + halfWidth + contentMargin, titleY, halfWidth, titleHeight);
	}

	int imageTop = contentTop + titleHeight + titleToImageSpacing;

	if (pPictureIMU != NULL)
	{
		pPictureIMU->MoveWindow(contentLeft, imageTop, halfWidth, availableImageHeight);
	}

	if (pPictureDrift != NULL)
	{
		pPictureDrift->MoveWindow(contentLeft + halfWidth + contentMargin, imageTop, halfWidth, availableImageHeight);
	}
}

void CTabDlg1::InvalidateCache()
{
	m_bCacheValid = false;
}

void CTabDlg1::LoadCSVFile(const CString& filePath)
{
	if (m_pGraphHelper != nullptr)
	{
		if (m_pGraphHelper->LoadCSV(filePath, m_imuData))
		{
			m_bDataLoaded = true;

			InvalidateCache();

			Invalidate();
		}
		else
		{
			AfxMessageBox(_T("CSV 파일 로드 실패!"), MB_ICONERROR);
		}
	}
}