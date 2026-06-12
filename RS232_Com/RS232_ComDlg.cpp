
// RS232_ComDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "RS232_Com.h"
#include "RS232_ComDlg.h"
#include "afxdialogex.h"
#include "CComunication.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRS232ComDlg 대화 상자



CRS232ComDlg::CRS232ComDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_RS232_COM_DIALOG, pParent)
{
	m_pComunication = new CComunication;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CRS232ComDlg::~CRS232ComDlg()
{
	if(nullptr != m_pComunication)
	{
		m_pComunication->Closed_Communication();
		m_pComunication = nullptr;
	}
}

void CRS232ComDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, m_CBPortList);
	DDX_Control(pDX, IDC_BUTTON1, m_BttConnect);
	DDX_Control(pDX, IDC_LIST1, m_LogBox);
	DDX_Control(pDX, IDC_BUTTON4, MSG_Send);
}

BEGIN_MESSAGE_MAP(CRS232ComDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CRS232ComDlg::OnBnClicked_ConnectBtt)
	ON_BN_CLICKED(IDC_BUTTON4, &CRS232ComDlg::MSG_SEND)
END_MESSAGE_MAP()


// CRS232ComDlg 메시지 처리기

BOOL CRS232ComDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	//포트 번호 나열
	CString strPortNum = _T("");
	for (int i = 0; i < 10; i++)
	{
		strPortNum.Format(_T("COM%d"), i);
		m_CBPortList.AddString(strPortNum);
	}
	m_CBPortList.SetCurSel(0);


	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CRS232ComDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CRS232ComDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CRS232ComDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CRS232ComDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CRS232ComDlg::OnBnClicked_ConnectBtt()
{
	int iPortIndex = m_CBPortList.GetCurSel();

	if (iPortIndex == CB_ERR)
	{
		AfxMessageBox(_T("포트를 먼저 선택해주세요."));
		return;
	}
	else
	{
		CString strPort;
		m_CBPortList.GetLBText(iPortIndex, strPort);

		if (nullptr == m_hCom)
			m_pComunication->PortConnection(iPortIndex, strPort, m_hCom, m_LogBox);
		else if (nullptr != m_hCom)
		{
			CloseHandle(m_hCom);
			m_hCom = nullptr;
			m_pComunication->PortConnection(iPortIndex, strPort, m_hCom, m_LogBox);
		}
	}
}


void CRS232ComDlg::MSG_SEND()
{
	if (nullptr == m_pComunication->IsOpenCom())
	{
		LogMessage(L"아직 연결되지 않음");
		return;
	}

	CString strInput;
	GetDlgItemText(IDC_EDIT1, strInput);

	if (strInput.IsEmpty()) return;

	// 1. 입력받은 문자열을 아스키로 변환
	CStringA strAscii(strInput);

	// 2. NT20 장비 규격에 맞게 STX(\x02)와 CR EOT(\x0D\x04)를 앞뒤로 결합
	CStringA strPacket;
	strPacket.Format("\x02%s\x0D\x04", strAscii.GetString());

	// 입력된 명령어가 'CP' (프린트 시작), 'CM' (이미지 만들기) 등 <0d>가 필요 없는 경우
	if (strAscii == "CP" || strAscii == "CM" || strAscii == "CS" || strAscii == "CN")
	{
		// 끝에 \x04 (EOT)만 붙임
		strPacket.Format("\x02%s\x04", strAscii.GetString());
	}
	else
	{
		// 그 외의 일반적인 명령어들은 끝에 \x0D\x04 (CR + EOT)를 붙임
		strPacket.Format("\x02%s\x0D\x04", strAscii.GetString());
	}

	// 3. 조립된 완전한 패킷을 전송
	if ((nullptr != m_pComunication) && m_pComunication->IsOpenCom())
	{
		m_pComunication->Send(strPacket.GetString(), strPacket.GetLength());
		LogMessage((CString(strPacket) + _T("전송")));
	}

}

void CRS232ComDlg::LogMessage(CString strMessage)
{// 1. 현재 시간 가져오기
	CTime time = CTime::GetCurrentTime();
	CString strTime = time.Format(_T("[%H:%M:%S] "));

	// 2. 시간 + 메시지
	CString strFullLog = strTime + strMessage;

	// 3. 리스트 박스 맨 아래에 추가
	int nIndex = m_LogBox.AddString(strFullLog);

	// 4. 자동으로 마지막 줄로 스크롤 이동 (매우 중요!)
	m_LogBox.SetCurSel(nIndex);
}
