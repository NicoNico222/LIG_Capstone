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

	m_brushBg.CreateSolidBrush(RGB(255, 255, 255)); // 흰색 배경

	SetWindowPos(NULL, 0, 0, 1500, 750, SWP_NOMOVE | SWP_NOZORDER);
	CenterWindow();

	// Tab Control 초기화
	InitializeTabControl();

	// Tab Dialog 생성
	m_tabDlg1.Create(IDD_DLG_TAP1, &m_tabControl);
	m_tabDlg2.Create(IDD_DLG_TAP2, &m_tabControl);

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
		// Tab Control 크기 조정
		m_tabControl.MoveWindow(0, 0, cx, cy);

		// 현재 탭 다이얼로그 크기 조정
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

void CLIGCapstoneDlg::ShowTab(int nTab)
{
	// 이전 탭 숨기기
	if (m_pCurrentTab != nullptr && m_pCurrentTab->GetSafeHwnd() != NULL)
	{
		m_pCurrentTab->ShowWindow(SW_HIDE);
	}

	// Tab Control의 디스플레이 영역 계산
	CRect tabRect;
	m_tabControl.GetClientRect(&tabRect);
	m_tabControl.AdjustRect(FALSE, &tabRect);

	// 새 탭 표시
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
	// 탭 선택 변경 이벤트 처리
	int nSel = m_tabControl.GetCurSel();
	ShowTab(nSel);

	*pResult = 0;
}