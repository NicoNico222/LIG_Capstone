#include "pch.h"
#include "LIG_Capstone.h"
#include "afxdialogex.h"
#include "TabDlg2.h"

IMPLEMENT_DYNAMIC(CTabDlg2, CDialog)

CTabDlg2::CTabDlg2(CWnd* pParent /*=nullptr*/)
	: CDialog(IDD_DLG_TAP2, pParent)
	, m_nSelectedCI(0)  // 초기값 0 (선택 안됨)
{
}

CTabDlg2::~CTabDlg2()
{
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
	ON_BN_CLICKED(IDC_RADIO1, &CTabDlg2::OnBnClickedRadio1)  // 추가
	ON_BN_CLICKED(IDC_RADIO2, &CTabDlg2::OnBnClickedRadio2)  // 추가
END_MESSAGE_MAP()

BOOL CTabDlg2::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_brushBg.CreateSolidBrush(RGB(255, 255, 255));

	// 폰트 생성
	m_fontTitle.CreatePointFont(180, _T("맑은 고딕"));
	m_fontGroupTitle.CreatePointFont(140, _T("맑은 고딕"));

	// 처음에는 아무것도 선택 안함
	CheckRadioButton(IDC_RADIO1, IDC_RADIO2, 0);  // 0 = 선택 안함

	InitializeUI();
	LoadAndDisplayImages();

	CRect clientRect;
	GetClientRect(&clientRect);
	ArrangeControls(clientRect.Width(), clientRect.Height());

	return TRUE;
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
		pDC->SetBkMode(TRANSPARENT);
		return m_brushBg;
	}

	return hbr;
}

void CTabDlg2::OnPaint()
{
	CPaintDC dc(this);

	if (!m_imageBayesian.IsNull())
		DisplayImage(IDC_PICTURE_DRIFT, m_imageBayesian);
	if (!m_imageRUL.IsNull())
		DisplayImage(IDC_PICTURE_RUL, m_imageRUL);
}

void CTabDlg2::OnDestroy()
{
	CDialog::OnDestroy();

	if (!m_imageBayesian.IsNull())
		m_imageBayesian.Destroy();
	if (!m_imageRUL.IsNull())
		m_imageRUL.Destroy();
}

void CTabDlg2::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	ArrangeControls(cx, cy);

	if (!m_imageBayesian.IsNull())
		DisplayImage(IDC_PICTURE_DRIFT, m_imageBayesian);
	if (!m_imageRUL.IsNull())
		DisplayImage(IDC_PICTURE_RUL, m_imageRUL);
}

// 90% 라디오 버튼 클릭
void CTabDlg2::OnBnClickedRadio1()
{
	m_nSelectedCI = 90;
}

// 95% 라디오 버튼 클릭
void CTabDlg2::OnBnClickedRadio2()
{
	m_nSelectedCI = 95;
}

void CTabDlg2::UpdateCISelection()
{
	// 현재 선택된 라디오 버튼 확인
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
		m_nSelectedCI = 0;  // 선택 안됨
	}
}

void CTabDlg2::InitializeUI()
{
	CWnd* pRulPredict = GetDlgItem(IDC_STATIC_RUL_PREDICT);
	CWnd* pRulMargin = GetDlgItem(IDC_STATIC_RUL_MARGIN);
	CWnd* pDriftText = GetDlgItem(IDC_STATIC_DRIFT_TEXT);
	CWnd* pRulText = GetDlgItem(IDC_STATIC_RUL_TEXT);

	if (pRulPredict != NULL)
		pRulPredict->SetFont(&m_fontGroupTitle);
	if (pRulMargin != NULL)
		pRulMargin->SetFont(&m_fontGroupTitle);
	if (pDriftText != NULL)
		pDriftText->SetFont(&m_fontTitle);
	if (pRulText != NULL)
		pRulText->SetFont(&m_fontTitle);
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
	CWnd* pDriftText = GetDlgItem(IDC_STATIC_DRIFT_TEXT);
	CWnd* pRulText = GetDlgItem(IDC_STATIC_RUL_TEXT);
	CWnd* pPictureDrift = GetDlgItem(IDC_PICTURE_DRIFT);
	CWnd* pPictureRul = GetDlgItem(IDC_PICTURE_RUL);

	int margin = 20;
	int topMargin = 20;
	int titleHeight = 30;
	int groupHeight = 100;
	int spacing = 15;

	// 왼쪽 영역과 오른쪽 영역
	int halfWidth = (cx - margin * 3) / 2;

	// === 왼쪽 상단: RUL 예측 영역 ===
	int leftX = margin;
	int currentY = topMargin;

	// RUL 예측 타이틀
	if (pRulPredict != NULL)
	{
		pRulPredict->MoveWindow(leftX, currentY, 150, titleHeight);
		currentY += titleHeight + 10;
	}

	// RUL 예측 박스 (라디오 버튼 포함)
	if (pRulBox != NULL)
	{
		pRulBox->MoveWindow(leftX, currentY, 200, groupHeight);

		// 라디오 버튼 배치
		if (pRadio1 != NULL && pRadio2 != NULL)
		{
			int radioY = currentY + 25;
			pRadio1->MoveWindow(leftX + 20, radioY, 150, 25);
			pRadio2->MoveWindow(leftX + 20, radioY + 35, 150, 25);
		}
	}

	// === 오른쪽 상단: RUL 오차범위 영역 ===
	int rightX = leftX + halfWidth + margin;
	currentY = topMargin;

	// RUL 오차범위 타이틀
	if (pRulMargin != NULL)
	{
		pRulMargin->MoveWindow(rightX, currentY, 150, titleHeight);
		currentY += titleHeight + 10;
	}

	// +- month 텍스트
	if (pMonth != NULL)
	{
		pMonth->MoveWindow(rightX, currentY, 150, 30);
	}

	// === 하단 영역 ===
	int bottomY = topMargin + groupHeight + spacing + 40;
	int imageHeight = cy - bottomY - margin * 2 - titleHeight - spacing;

	// 왼쪽 하단: P3 Drift 예측 결과
	currentY = bottomY;
	if (pDriftText != NULL)
	{
		pDriftText->MoveWindow(leftX, currentY, halfWidth, titleHeight);
		currentY += titleHeight + spacing;
	}

	if (pPictureDrift != NULL)
	{
		pPictureDrift->MoveWindow(leftX, currentY, halfWidth, imageHeight);
	}

	// 오른쪽 하단: RUL 예측 결과
	currentY = bottomY;
	if (pRulText != NULL)
	{
		pRulText->MoveWindow(rightX, currentY, halfWidth, titleHeight);
		currentY += titleHeight + spacing;
	}

	if (pPictureRul != NULL)
	{
		pPictureRul->MoveWindow(rightX, currentY, halfWidth, imageHeight);
	}
}

void CTabDlg2::LoadAndDisplayImages()
{
	TCHAR szPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, szPath);
	CString strPath = szPath;

	CString strBayesianPath = strPath + _T("\\image\\Bayesian.png");
	CString strRULPath = strPath + _T("\\image\\RUL.png");

	m_imageBayesian.Load(strBayesianPath);
	m_imageRUL.Load(strRULPath);

	if (!m_imageBayesian.IsNull())
		DisplayImage(IDC_PICTURE_DRIFT, m_imageBayesian);

	if (!m_imageRUL.IsNull())
		DisplayImage(IDC_PICTURE_RUL, m_imageRUL);
}

void CTabDlg2::DisplayImage(UINT controlID, CImage& image)
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

	pDC->FillSolidRect(&rect, RGB(255, 255, 255));

	image.StretchBlt(pDC->m_hDC,
		offsetX, offsetY, drawWidth, drawHeight,
		0, 0, imgWidth, imgHeight,
		SRCCOPY);

	pWnd->ReleaseDC(pDC);
}