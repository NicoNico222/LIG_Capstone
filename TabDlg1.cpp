#include "pch.h"
#include "LIG_Capstone.h"
#include "afxdialogex.h"
#include "TabDlg1.h"

IMPLEMENT_DYNAMIC(CTabDlg1, CDialog)

CTabDlg1::CTabDlg1(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_TAP1, pParent)
{
}

CTabDlg1::~CTabDlg1()
{
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

	m_brushBg.CreateSolidBrush(RGB(255, 255, 255)); // 흰색 배경

	InitializeUI();
	LoadAndDisplayImages();

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
		pDC->SetTextColor(RGB(0, 0, 0)); // 검은색 텍스트
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	return hbr;
}

void CTabDlg1::OnPaint()
{
	CPaintDC dc(this);

	if (!m_imageIMU.IsNull())
		DisplayImage(IDC_PICTURE_IMU, m_imageIMU);
	if (!m_imageDrift.IsNull())
		DisplayImage(IDC_PICTURE_DRIFT, m_imageDrift);
}

void CTabDlg1::OnDestroy()
{
	CDialog::OnDestroy();

	if (!m_imageIMU.IsNull())
		m_imageIMU.Destroy();
	if (!m_imageDrift.IsNull())
		m_imageDrift.Destroy();
}

void CTabDlg1::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	ArrangeControls(cx, cy);

	if (!m_imageIMU.IsNull())
		DisplayImage(IDC_PICTURE_IMU, m_imageIMU);
	if (!m_imageDrift.IsNull())
		DisplayImage(IDC_PICTURE_DRIFT, m_imageDrift);
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

void CTabDlg1::LoadAndDisplayImages()
{
	TCHAR szPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szPath);
	CString strPath = szPath;

	CString strIMUPath = strPath + _T("\\image\\IMU_senser.png");
	CString strDriftPath = strPath + _T("\\image\\drift.png");

	m_imageIMU.Load(strIMUPath);
	m_imageDrift.Load(strDriftPath);

	if (!m_imageIMU.IsNull())
		DisplayImage(IDC_PICTURE_IMU, m_imageIMU);

	if (!m_imageDrift.IsNull())
		DisplayImage(IDC_PICTURE_DRIFT, m_imageDrift);
}

void CTabDlg1::DisplayImage(UINT controlID, CImage& image)
{
	CWnd* pWnd = GetDlgItem(controlID);
	if (pWnd == NULL || image.IsNull()) return;

	CDC* pDC = pWnd->GetDC();
	if (pDC == NULL) return;

	CRect rect;
	pWnd->GetClientRect(&rect);

	int imgWidth = image.GetWidth();
	int imgHeight = image.GetHeight();

	float aspectRatio = (float)imgWidth / (float)imgHeight;
	float controlRatio = (float)rect.Width() / (float)rect.Height();

	int drawWidth, drawHeight;
	int offsetX = 0, offsetY = 0;

	if (aspectRatio > controlRatio)
	{
		drawWidth = rect.Width();
		drawHeight = (int)(rect.Width() / aspectRatio);
		offsetY = (rect.Height() - drawHeight) / 2;
	}
	else
	{
		drawHeight = rect.Height();
		drawWidth = (int)(rect.Height() * aspectRatio);
		offsetX = (rect.Width() - drawWidth) / 2;
	}

	pDC->FillSolidRect(&rect, RGB(255, 255, 255)); // 흰색 배경

	image.StretchBlt(pDC->m_hDC,
		offsetX, offsetY, drawWidth, drawHeight,
		0, 0, imgWidth, imgHeight,
		SRCCOPY);

	pWnd->ReleaseDC(pDC);
}