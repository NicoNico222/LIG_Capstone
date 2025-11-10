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
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLIGCapstoneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLIGCapstoneDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	ON_WM_SIZE()
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

	SetWindowPos(NULL, 0, 0, 1500, 750, SWP_NOMOVE | SWP_NOZORDER);
	CenterWindow();

	InitializeUI();
	LoadAndDisplayImages();

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

		if (!m_imageIMU.IsNull())
			DisplayImage(IDC_PICTURE_IMU, m_imageIMU);
		if (!m_imageDrift.IsNull())
			DisplayImage(IDC_PICTURE_DRIFT, m_imageDrift);
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

	if (GetDlgItem(IDC_STATIC_TITLE_LEFT) != NULL)
	{
		ArrangeControls(cx, cy);

		if (!m_imageIMU.IsNull())
			DisplayImage(IDC_PICTURE_IMU, m_imageIMU);
		if (!m_imageDrift.IsNull())
			DisplayImage(IDC_PICTURE_DRIFT, m_imageDrift);
	}
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
	int titleHeight = 40;
	int titleToImageSpacing = 20;

	int halfWidth = (cx - margin * 3) / 2;
	int availableImageHeight = cy - margin * 2 - titleHeight - titleToImageSpacing;

	int imageTop = margin + titleHeight + titleToImageSpacing;

	CWnd* pTitleLeft = GetDlgItem(IDC_STATIC_TITLE_LEFT);
	CWnd* pTitleRight = GetDlgItem(IDC_STATIC_TITLE_RIGHT);
	CWnd* pPictureIMU = GetDlgItem(IDC_PICTURE_IMU);
	CWnd* pPictureDrift = GetDlgItem(IDC_PICTURE_DRIFT);

	if (pTitleLeft != NULL)
	{
		int titleY = margin + (titleHeight / 4);
		pTitleLeft->MoveWindow(margin, titleY, halfWidth, titleHeight);
	}

	if (pTitleRight != NULL)
	{
		int titleY = margin + (titleHeight / 4);
		pTitleRight->MoveWindow(margin * 2 + halfWidth, titleY, halfWidth, titleHeight);
	}

	if (pPictureIMU != NULL)
	{
		pPictureIMU->MoveWindow(margin, imageTop, halfWidth, availableImageHeight);
	}

	if (pPictureDrift != NULL)
	{
		pPictureDrift->MoveWindow(margin * 2 + halfWidth, imageTop, halfWidth, availableImageHeight);
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