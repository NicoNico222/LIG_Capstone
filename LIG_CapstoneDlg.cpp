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
	, m_pCurrentTab(nullptr)
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
	ON_COMMAND(ID_FILE_LOAD_CSV, &CLIGCapstoneDlg::OnFileLoadCsv)
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

	m_brushBg.CreateSolidBrush(RGB(255, 255, 255));

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	int windowWidth = (int)(screenWidth * 0.95);
	int windowHeight = (int)(screenHeight * 0.95);

	int posX = (screenWidth - windowWidth) / 2;
	int posY = (screenHeight - windowHeight) / 2;

	SetWindowPos(NULL, posX, posY, windowWidth, windowHeight, SWP_NOZORDER);

	InitializeTabControl();

	m_tabDlg1.Create(IDD_DLG_TAP1, &m_tabControl);
	m_tabDlg2.Create(IDD_DLG_TAP2, &m_tabControl);

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

	return hbr;
}

void CLIGCapstoneDlg::OnDestroy()
{
	CDialogEx::OnDestroy();
}

void CLIGCapstoneDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (m_tabControl.GetSafeHwnd() != NULL)
	{
		m_tabControl.MoveWindow(0, 0, cx, cy);

		CRect tabRect;
		m_tabControl.GetClientRect(&tabRect);
		m_tabControl.AdjustRect(FALSE, &tabRect);

		if (m_pCurrentTab != nullptr && m_pCurrentTab->GetSafeHwnd() != NULL)
		{
			m_pCurrentTab->MoveWindow(&tabRect);
		}
	}
}

void CLIGCapstoneDlg::InitializeTabControl()
{
	m_fontTab.CreatePointFont(120, _T("맑은 고딕"));
	m_tabControl.SetFont(&m_fontTab);

	m_tabControl.SetItemSize(CSize(150, 40)); 

	m_tabControl.SetPadding(CSize(30, 5)); 

	TCITEM item;
	item.mask = TCIF_TEXT;

	item.pszText = _T("페이지1");
	m_tabControl.InsertItem(0, &item);

	item.pszText = _T("페이지2");
	m_tabControl.InsertItem(1, &item);
}

void CLIGCapstoneDlg::ShowTab(int nTab)
{
	if (m_pCurrentTab != nullptr && m_pCurrentTab->GetSafeHwnd() != NULL)
	{
		m_pCurrentTab->ShowWindow(SW_HIDE);
	}

	CRect tabRect;
	m_tabControl.GetClientRect(&tabRect);
	m_tabControl.AdjustRect(FALSE, &tabRect);

	if (nTab == 0)
	{
		m_pCurrentTab = &m_tabDlg1;
		m_tabDlg1.MoveWindow(&tabRect);
		m_tabDlg1.ShowWindow(SW_SHOW);
	}
	else if (nTab == 1)
	{
		m_pCurrentTab = &m_tabDlg2;
		m_tabDlg2.MoveWindow(&tabRect);
		m_tabDlg2.ShowWindow(SW_SHOW);
	}
}

void CLIGCapstoneDlg::OnTcnSelchangeTabPage(NMHDR* pNMHDR, LRESULT* pResult)
{
	int nSel = m_tabControl.GetCurSel();
	ShowTab(nSel);

	*pResult = 0;
}


void CLIGCapstoneDlg::OnFileLoadCsv()
{
	// 파일 다이얼로그
	CFileDialog dlg(TRUE, _T("csv"), NULL,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		_T("CSV Files (*.csv)|*.csv|All Files (*.*)|*.*||"));

	if (dlg.DoModal() == IDOK)
	{
		CString filePath = dlg.GetPathName();

		// 현재 활성화된 탭이 Tab1인 경우에만 로드
		int nSel = m_tabControl.GetCurSel();
		if (nSel == 0)  // Tab1
		{
			m_tabDlg1.LoadCSVFile(filePath);
		}
		else
		{
			AfxMessageBox(_T("CSV 파일은 페이지1에서만 로드 가능합니다."), MB_ICONINFORMATION);
		}
	}
}