#include "pch.h"
#include "framework.h"
#include "LIG_Capstone.h"
#include "LIG_CapstoneDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


CLIGCapstoneDlg::CLIGCapstoneDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_LIG_CAPSTONE_DIALOG, pParent)
	, m_nCurrentTab(0) // 기본값: 페이지1
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLIGCapstoneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB_PAGE, m_tabControl);
}

BEGIN_MESSAGE_MAP(CLIGCapstoneDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_PAGE, &CLIGCapstoneDlg::OnTcnSelchangeTabPage)
END_MESSAGE_MAP()

BOOL CLIGCapstoneDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	CMenu menu;
	if (menu.LoadMenu(IDR_MENU1))
	{
		SetMenu(&menu);
		menu.Detach();
	}

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_brushBg.CreateSolidBrush(RGB(64, 64, 64));
	m_brushTabBg.CreateSolidBrush(RGB(64, 64, 64)); // 탭 내부도 같은 색

	SetWindowPos(NULL, 0, 0, 1500, 750, SWP_NOMOVE | SWP_NOZORDER);
	CenterWindow();

	// Tab Control 초기화 (UI 초기화보다 먼저)
	InitializeTabControl();

	InitializeUI();
	LoadAndDisplayImages();

	// 페이지1을 기본으로 표시
	ShowTab(0);

	return TRUE;
}

void CLIGCapstoneDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CLIGCapstoneDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();

		// 페이지1일 때만 이미지 표시
		if (m_nCurrentTab == 0)
		{
			if (!m_imageIMU.IsNull())
				DisplayImage(IDC_PICTURE_IMU, m_imageIMU);
			if (!m_imageDrift.IsNull())
				DisplayImage(IDC_PICTURE_DRIFT, m_imageDrift);
		}
	}
}

HCURSOR CLIGCapstoneDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

HBRUSH CLIGCapstoneDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_DLG)
	{
		return m_brushBg;
	}

	if (nCtlColor == CTLCOLOR_STATIC)
	{
		pDC->SetBkMode(TRANSPARENT);
		pDC->SetTextColor(RGB(255, 255, 255));
		return (HBRUSH)GetStockObject(NULL_BRUSH);
	}

	// 탭 컨트롤 내부 배경색 처리
	if (pWnd->GetDlgCtrlID() == IDC_TAB_PAGE)
	{
		return m_brushTabBg;
	}

	return hbr;
}

void CLIGCapstoneDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	if (!m_imageIMU.IsNull())
		m_imageIMU.Destroy();
	if (!m_imageDrift.IsNull())
		m_imageDrift.Destroy();
}

void CLIGCapstoneDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_tabControl.GetSafeHwnd() != NULL)
	{
		ArrangeControls(cx, cy);

		if (m_nCurrentTab == 0)
		{
			if (!m_imageIMU.IsNull())
				DisplayImage(IDC_PICTURE_IMU, m_imageIMU);
			if (!m_imageDrift.IsNull())
				DisplayImage(IDC_PICTURE_DRIFT, m_imageDrift);
		}
	}
}

void CLIGCapstoneDlg::InitializeTabControl()
{
	// 탭 아이템 추가
	TCITEM item;
	item.mask = TCIF_TEXT;

	// 페이지1 탭
	item.pszText = _T("페이지1");
	m_tabControl.InsertItem(0, &item);

	// 페이지2 탭
	item.pszText = _T("페이지2");
	m_tabControl.InsertItem(1, &item);
}

void CLIGCapstoneDlg::InitializeUI()
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

void CLIGCapstoneDlg::ArrangeControls(int cx, int cy)
{
	int margin = 20;
	int tabTop = 0; // 탭을 최상단에 붙임 (메뉴 바로 아래)

	// Tab Control 크기 및 위치 설정
	if (m_tabControl.GetSafeHwnd() != NULL)
	{
		// 탭을 다이얼로그 전체 크기로 설정 (메뉴 바로 아래부터)
		m_tabControl.MoveWindow(0, tabTop, cx, cy - tabTop);
	}

	// Tab Control의 디스플레이 영역 가져오기
	CRect tabRect;
	m_tabControl.GetClientRect(&tabRect);
	m_tabControl.AdjustRect(FALSE, &tabRect); // 탭 헤더를 제외한 내부 영역

	// 내부 컨텐츠 배치
	int contentMargin = 20;
	int titleHeight = 40;
	int titleToImageSpacing = 20;

	int halfWidth = (tabRect.Width() - contentMargin * 3) / 2;
	int availableImageHeight = tabRect.Height() - contentMargin * 2 - titleHeight - titleToImageSpacing;

	int contentLeft = tabRect.left + contentMargin;
	int contentTop = tabTop + tabRect.top + contentMargin;

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

void CLIGCapstoneDlg::LoadAndDisplayImages()
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

void CLIGCapstoneDlg::DisplayImage(UINT controlID, CImage& image)
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

	pDC->FillSolidRect(&rect, RGB(64, 64, 64));

	image.StretchBlt(pDC->m_hDC,
		offsetX, offsetY, drawWidth, drawHeight,
		0, 0, imgWidth, imgHeight,
		SRCCOPY);

	pWnd->ReleaseDC(pDC);
}

void CLIGCapstoneDlg::ShowTab(int nTab)
{
	m_nCurrentTab = nTab;

	CWnd* pTitleLeft = GetDlgItem(IDC_STATIC_TITLE_LEFT);
	CWnd* pTitleRight = GetDlgItem(IDC_STATIC_TITLE_RIGHT);
	CWnd* pPictureIMU = GetDlgItem(IDC_PICTURE_IMU);
	CWnd* pPictureDrift = GetDlgItem(IDC_PICTURE_DRIFT);

	if (nTab == 0) // 페이지1
	{
		// 페이지1 컨트롤 표시
		if (pTitleLeft != NULL) pTitleLeft->ShowWindow(SW_SHOW);
		if (pTitleRight != NULL) pTitleRight->ShowWindow(SW_SHOW);
		if (pPictureIMU != NULL) pPictureIMU->ShowWindow(SW_SHOW);
		if (pPictureDrift != NULL) pPictureDrift->ShowWindow(SW_SHOW);
	}
	else if (nTab == 1) // 페이지2
	{
		// 페이지1 컨트롤 숨김 (페이지2는 빈 화면)
		if (pTitleLeft != NULL) pTitleLeft->ShowWindow(SW_HIDE);
		if (pTitleRight != NULL) pTitleRight->ShowWindow(SW_HIDE);
		if (pPictureIMU != NULL) pPictureIMU->ShowWindow(SW_HIDE);
		if (pPictureDrift != NULL) pPictureDrift->ShowWindow(SW_HIDE);
	}

	Invalidate();
}

void CLIGCapstoneDlg::OnTcnSelchangeTabPage(NMHDR* pNMHDR, LRESULT* pResult)
{
	// 탭 선택 변경 이벤트 처리
	int nSel = m_tabControl.GetCurSel();
	ShowTab(nSel);

	*pResult = 0;
}