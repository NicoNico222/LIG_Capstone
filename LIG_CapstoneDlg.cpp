#include "pch.h"
#include "framework.h"
#include "LIG_Capstone.h"
#include "LIG_CapstoneDlg.h"
#include "afxdialogex.h"
#include <winhttp.h>
#include <fstream>
#include <sstream>
#pragma comment(lib, "winhttp.lib")

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

	int windowWidth = (int)(screenWidth * 0.90);
	int windowHeight = (int)(screenHeight * 0.90);

	int posX = (screenWidth - windowWidth) / 2;
	int posY = (screenHeight - windowHeight) / 2;

	SetWindowPos(NULL, posX, posY, windowWidth, windowHeight, SWP_NOZORDER);

	InitializeTabControl();

	m_tabDlg1.Create(IDD_DLG_TAP1, &m_tabControl);
	m_tabDlg2.Create(IDD_DLG_TAP2, &m_tabControl);
	m_tabDlg3.Create(IDD_DLG_TAP3, &m_tabControl);

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

	item.pszText = _T("Page1");
	m_tabControl.InsertItem(0, &item);

	item.pszText = _T("Page2");
	m_tabControl.InsertItem(1, &item);

	item.pszText = _T("Page3");
	m_tabControl.InsertItem(2, &item);
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
	else if (nTab == 2)  // 이 부분만 추가
	{
		m_pCurrentTab = &m_tabDlg3;
		m_tabDlg3.MoveWindow(&tabRect);
		m_tabDlg3.ShowWindow(SW_SHOW);
	}
}

void CLIGCapstoneDlg::OnTcnSelchangeTabPage(NMHDR* pNMHDR, LRESULT* pResult)
{
	int nSel = m_tabControl.GetCurSel();
	ShowTab(nSel);

	*pResult = 0;
}

void CLIGCapstoneDlg::RunInference(const CString& csvPath, int ci)
{
	CW2A utf8(csvPath, CP_UTF8);
	std::string path = std::string(utf8);
	std::string escaped;
	for (char c : path)
	{
		if (c == '\\')
			escaped += "\\\\";
		else
			escaped += c;
	}
	std::string jsonBody = "{ \"csv_path\": \"" + escaped + "\", \"ci\": " + std::to_string(ci) + " }";
	LPCWSTR server = L"127.0.0.1";
	INTERNET_PORT port = 8000;
	HINTERNET hSession = WinHttpOpen(L"MFC Client",
		WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
		WINHTTP_NO_PROXY_NAME,
		WINHTTP_NO_PROXY_BYPASS, 0);
	if (!hSession) return;
	HINTERNET hConnect = WinHttpConnect(hSession, server, port, 0);
	if (!hConnect) {
		WinHttpCloseHandle(hSession);
		return;
	}
	HINTERNET hRequest = WinHttpOpenRequest(
		hConnect,
		L"POST",
		L"/run_inference",
		NULL,
		WINHTTP_NO_REFERER,
		WINHTTP_DEFAULT_ACCEPT_TYPES,
		WINHTTP_FLAG_REFRESH);
	if (!hRequest) {
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}
	LPCWSTR header = L"Content-Type: application/json; charset=utf-8\r\n";
	BOOL bResult = WinHttpSendRequest(
		hRequest,
		header,
		-1,
		(LPVOID)jsonBody.c_str(),
		(DWORD)jsonBody.length(),
		(DWORD)jsonBody.length(),
		0);
	if (!bResult) {
		WinHttpCloseHandle(hRequest);
		WinHttpCloseHandle(hConnect);
		WinHttpCloseHandle(hSession);
		return;
	}
	WinHttpReceiveResponse(hRequest, NULL);
	DWORD totalSize = 0;
	std::string fullResponse;
	while (true)
	{
		DWORD size = 0;
		WinHttpQueryDataAvailable(hRequest, &size);
		if (size == 0) break;
		std::string buffer(size, 0);
		DWORD downloaded = 0;
		WinHttpReadData(hRequest, &buffer[0], size, &downloaded);
		fullResponse += buffer;
		totalSize += downloaded;
	}

	WinHttpCloseHandle(hRequest);
	WinHttpCloseHandle(hConnect);
	WinHttpCloseHandle(hSession);

	if (totalSize > 0)
	{
		TCHAR szPath[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, szPath);
		CString debugFilePath = CString(szPath) + _T("\\received_data.txt");

		std::ofstream outFile(debugFilePath);
		outFile << fullResponse;
		outFile.close();

		bool success = false;
		RULGraphData rulData;

		size_t data_pos = fullResponse.find("\"data\"");
		if (data_pos == std::string::npos)
		{
			AfxMessageBox(_T("실패했습니다"), MB_ICONERROR);
			return;
		}

		double min_rul = 0.0;
		double range_val = 0.0;
		size_t rul_result_pos = fullResponse.find("\"rul_result\"", data_pos);
		if (rul_result_pos != std::string::npos)
		{
			size_t min_rul_pos = fullResponse.find("\"min_rul_mean\"", rul_result_pos);
			size_t range_pos = fullResponse.find("\"range\"", rul_result_pos);

			if (min_rul_pos != std::string::npos)
			{
				size_t colon = fullResponse.find(":", min_rul_pos);
				size_t start = colon + 1;
				size_t end = fullResponse.find_first_of(",}", start);
				std::string val = fullResponse.substr(start, end - start);
				val.erase(0, val.find_first_not_of(" \t\n\r"));
				val.erase(val.find_last_not_of(" \t\n\r") + 1);
				try { min_rul = std::stod(val); }
				catch (...) {}
			}

			if (range_pos != std::string::npos)
			{
				size_t colon = fullResponse.find(":", range_pos);
				size_t start = colon + 1;
				size_t end = fullResponse.find_first_of(",}", start);
				std::string val = fullResponse.substr(start, end - start);
				val.erase(0, val.find_first_not_of(" \t\n\r"));
				val.erase(val.find_last_not_of(" \t\n\r") + 1);
				try { range_val = std::stod(val); }
				catch (...) {}
			}
		}

		size_t vis_rul_pos = fullResponse.find("\"vis_rul_graph\"", data_pos);
		if (vis_rul_pos != std::string::npos)
		{
			rulData.ci = ci;

			auto parseDoubleArray = [](const std::string& json, const std::string& key, size_t start) -> std::vector<double> {
				std::vector<double> result;
				size_t keyPos = json.find("\"" + key + "\"", start);
				if (keyPos == std::string::npos) return result;
				size_t bracketStart = json.find("[", keyPos);
				if (bracketStart == std::string::npos) return result;
				size_t bracketEnd = json.find("]", bracketStart);
				if (bracketEnd == std::string::npos) return result;
				std::string content = json.substr(bracketStart + 1, bracketEnd - bracketStart - 1);
				std::stringstream ss(content);
				std::string token;
				while (std::getline(ss, token, ','))
				{
					token.erase(0, token.find_first_not_of(" \t\n\r"));
					token.erase(token.find_last_not_of(" \t\n\r") + 1);
					if (!token.empty())
					{
						try { result.push_back(std::stod(token)); }
						catch (...) {}
					}
				}
				return result;
				};

			auto parseThresholdDict = [](const std::string& json, size_t start) -> std::vector<double> {
				std::vector<double> result(6, 0.0);

				size_t thresholdPos = json.find("\"threshold\"", start);
				if (thresholdPos == std::string::npos) return result;

				size_t braceStart = json.find("{", thresholdPos);
				if (braceStart == std::string::npos) return result;

				size_t braceEnd = json.find("}", braceStart);
				if (braceEnd == std::string::npos) return result;

				std::string content = json.substr(braceStart, braceEnd - braceStart + 1);

				std::vector<std::string> keys = {
					"Roll Drift", "Pitch Drift", "Yaw Drift",
					"V_North Drift", "V_East Drift", "V_Down Drift"
				};

				for (int i = 0; i < 6; i++)
				{
					size_t keyPos = content.find("\"" + keys[i] + "\"");
					if (keyPos != std::string::npos)
					{
						size_t colonPos = content.find(":", keyPos);
						if (colonPos != std::string::npos)
						{
							size_t valStart = colonPos + 1;
							size_t valEnd = content.find_first_of(",}", valStart);
							if (valEnd != std::string::npos)
							{
								std::string valStr = content.substr(valStart, valEnd - valStart);
								valStr.erase(0, valStr.find_first_not_of(" \t\n\r"));
								valStr.erase(valStr.find_last_not_of(" \t\n\r") + 1);
								try {
									result[i] = std::stod(valStr);
								}
								catch (...) {}
							}
						}
					}
				}

				return result;
				};

			auto parseDouble = [](const std::string& json, const std::string& key, size_t start) -> double {
				size_t keyPos = json.find("\"" + key + "\"", start);
				if (keyPos == std::string::npos) return 0.0;
				size_t colon = json.find(":", keyPos);
				size_t valStart = colon + 1;
				size_t valEnd = json.find_first_of(",}", valStart);
				std::string val = json.substr(valStart, valEnd - valStart);
				val.erase(0, val.find_first_not_of(" \t\n\r"));
				val.erase(val.find_last_not_of(" \t\n\r") + 1);
				try { return std::stod(val); }
				catch (...) { return 0.0; }
				};

			auto parse2DArray = [](const std::string& json, const std::string& key, size_t start) -> std::vector<std::vector<double>> {
				std::vector<std::vector<double>> result;
				size_t keyPos = json.find("\"" + key + "\"", start);
				if (keyPos == std::string::npos) return result;
				size_t outerStart = json.find("[", keyPos);
				if (outerStart == std::string::npos) return result;

				size_t pos = outerStart + 1;
				while (pos < json.length())
				{
					size_t innerStart = json.find("[", pos);
					if (innerStart == std::string::npos) break;
					size_t innerEnd = json.find("]", innerStart);
					if (innerEnd == std::string::npos) break;

					std::string content = json.substr(innerStart + 1, innerEnd - innerStart - 1);
					std::vector<double> inner;
					std::stringstream ss(content);
					std::string token;
					while (std::getline(ss, token, ','))
					{
						token.erase(0, token.find_first_not_of(" \t\n\r"));
						token.erase(token.find_last_not_of(" \t\n\r") + 1);
						if (!token.empty())
						{
							try { inner.push_back(std::stod(token)); }
							catch (...) {}
						}
					}
					result.push_back(inner);
					pos = innerEnd + 1;

					if (json.find("[", pos) == std::string::npos || json.find("]", pos) < json.find("[", pos))
						break;
				}
				return result;
				};

			rulData.threshold = parseThresholdDict(fullResponse, vis_rul_pos);
			rulData.slope_mean = parseDoubleArray(fullResponse, "slope_mean", vis_rul_pos);
			rulData.input_y = parseDoubleArray(fullResponse, "input_y", vis_rul_pos);
			rulData.target_y = parseDoubleArray(fullResponse, "target_y", vis_rul_pos);
			rulData.gap_mean_y_mean = parseDoubleArray(fullResponse, "gap_mean_y_mean", vis_rul_pos);
			rulData.rul_mean_list = parseDoubleArray(fullResponse, "rul_mean_list", vis_rul_pos);

			rulData.input_x = parseDouble(fullResponse, "input_x", vis_rul_pos);
			rulData.target_x = parseDouble(fullResponse, "target_x", vis_rul_pos);
			rulData.gap_mean = parseDouble(fullResponse, "gap_mean", vis_rul_pos);
			rulData.p2_x_mean = parseDouble(fullResponse, "p2_x_mean", vis_rul_pos);

			rulData.y_line_mean_list = parse2DArray(fullResponse, "y_line_mean_list", vis_rul_pos);
			rulData.y_line_0_list = parse2DArray(fullResponse, "y_line_0_list", vis_rul_pos);
			rulData.y_line_1_list = parse2DArray(fullResponse, "y_line_1_list", vis_rul_pos);
			rulData.x_line_list = parse2DArray(fullResponse, "x_line_list", vis_rul_pos);

			if (rulData.threshold.size() == 6 && rulData.slope_mean.size() == 6)
			{
				m_tabDlg2.LoadRULGraphData(rulData);
				success = true;
			}
		}

		if (success)
		{
			CString resultText;
			resultText.Format(_T("%.1f ± %.1f Month"), min_rul, range_val);
			m_tabDlg2.UpdateRULDisplay(resultText);
			m_tabDlg3.UpdateRULDisplay(resultText);
			m_tabDlg3.UpdateCIDisplay(ci);
			m_tabDlg3.LoadRULGraphData(rulData);
			AfxMessageBox(_T("성공했습니다"), MB_ICONINFORMATION);
		}
		else
		{
			AfxMessageBox(_T("실패했습니다"), MB_ICONERROR);
		}
	}
	else
	{
		AfxMessageBox(_T("실패했습니다"), MB_ICONERROR);
	}
}

void CLIGCapstoneDlg::OnFileLoadCsv()
{
	CFileDialog dlg(TRUE, _T("csv"), NULL,
		OFN_HIDEREADONLY | OFN_FILEMUSTEXIST,
		_T("CSV Files (*.csv)|*.csv|All Files (*.*)|*.*||"));

	if (dlg.DoModal() == IDOK)
	{
		CString filePath = dlg.GetPathName();

		int nSel = m_tabControl.GetCurSel();
		if (nSel == 0)
		{
			m_tabDlg1.LoadCSVFile(filePath);

			m_loadedCsvPath = filePath;

			m_tabDlg2.ResetRadioButtons();

			AfxMessageBox(_T("CSV 파일이 로드되었습니다.\nTab2에서 CI를 선택하고 '실행'을 눌러주세요."));
		}
		else
		{
			AfxMessageBox(_T("CSV 파일은 페이지1에서만 로드 가능합니다."), MB_ICONINFORMATION);
		}
	}
}