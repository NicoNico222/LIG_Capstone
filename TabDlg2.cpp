#include "pch.h"
#include "LIG_Capstone.h"
#include "afxdialogex.h"
#include "TabDlg2.h"
#include "LIG_CapstoneDlg.h"

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

	InitializeUI();
	LoadAndDisplayImages();

	CWnd* pMonth = GetDlgItem(IDC_STATIC_MONTH);
	if (pMonth != NULL)
	{
		pMonth->ModifyStyle(0, SS_CENTER | SS_CENTERIMAGE);
		pMonth->SetFont(&m_fontMonth);
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
	CWnd* pRulText = GetDlgItem(IDC_STATIC_RUL_TEXT);
	CWnd* pPictureDrift = GetDlgItem(IDC_PICTURE_DRIFT);
	CWnd* pPictureRul = GetDlgItem(IDC_PICTURE_RUL);

	int margin = 20;
	int topMargin = 20;
	int titleHeight = 30;
	int groupHeight = 70;
	int spacing = 10;

	int halfWidth = (cx - margin * 3) / 2;

	int leftX = margin;
	int currentY = topMargin;

	int boxWidth = 250; // RUL 예측
	int btnWidth = 100;
	int boxCenterX = leftX + (halfWidth - boxWidth) / 2;

	if (pRulPredict != NULL)
	{
		pRulPredict->MoveWindow(boxCenterX, currentY, boxWidth, titleHeight);
		currentY += titleHeight + 10;
	}

	if (pRulBox != NULL)
	{
		pRulBox->MoveWindow(boxCenterX, currentY, boxWidth, groupHeight);

		if (pRadio1 != NULL && pRadio2 != NULL)
		{
			int radioWidth = 160;
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

	int marginBoxWidth = 330; // RUL 오차범위 가로 크기
	int marginCenterX = rightX + (halfWidth - marginBoxWidth) / 2;

	if (pRulMargin != NULL)
	{
		pRulMargin->MoveWindow(marginCenterX, currentY, marginBoxWidth, titleHeight);
		currentY += titleHeight + 10;
	}

	if (pMonth != NULL)
	{
		pMonth->MoveWindow(marginCenterX, currentY, marginBoxWidth, groupHeight);
	}

	int bottomY = topMargin + titleHeight + 10 + groupHeight + spacing + 20;
	int imageHeight = cy - bottomY - margin * 2 - titleHeight - spacing;

	currentY = bottomY;
	if (pDriftText != NULL)
	{
		pDriftText->MoveWindow(leftX, currentY, halfWidth, titleHeight);
	}

	currentY += titleHeight + spacing;
	if (pPictureDrift != NULL)
	{
		pPictureDrift->MoveWindow(leftX, currentY, halfWidth, imageHeight);
	}

	currentY = bottomY;
	if (pRulText != NULL)
	{
		pRulText->MoveWindow(rightX, currentY, halfWidth, titleHeight);
	}

	currentY += titleHeight + spacing;
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

int CTabDlg2::GetSelectedCI()
{
	if (IsDlgButtonChecked(IDC_RADIO1) == BST_CHECKED)
		return 90;
	else if (IsDlgButtonChecked(IDC_RADIO2) == BST_CHECKED)
		return 95;
	else
		return 0;   // 선택 안함 → 에러 처리해야 함
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

	pParent->RunInference(pParent->m_loadedCsvPath, ci);
}

void CTabDlg2::ResetRadioButtons()
{
	// 모든 라디오 버튼 선택 해제
	CheckDlgButton(IDC_RADIO1, BST_UNCHECKED);
	CheckDlgButton(IDC_RADIO2, BST_UNCHECKED);

	// 내부 변수도 초기화
	m_nSelectedCI = 0;
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
			pDC->SelectObject(&m_fontGroupTitle);
			CRect textRect;
			pDC->DrawText(text, &textRect, DT_CALCRECT);
			pMonth->ReleaseDC(pDC);
		}

		pMonth->Invalidate();
		pMonth->UpdateWindow();
	}
}