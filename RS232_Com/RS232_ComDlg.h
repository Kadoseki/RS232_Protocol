
// RS232_ComDlg.h: 헤더 파일
//

#pragma once
class CComunication;
// CRS232ComDlg 대화 상자
class CRS232ComDlg : public CDialogEx
{
// 생성입니다.
public:
	CRS232ComDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	~CRS232ComDlg();	
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_RS232_COM_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;
	CComunication* m_pComunication = nullptr;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg void OnBnClicked_ConnectBtt();

	void MSG_SEND();
	void LogMessage(CString strMessage);
	DECLARE_MESSAGE_MAP()
public:
	// Port Settings
	DCB m_dcbCom = { 0 };
	HANDLE m_hCom = { 0 };
	// PortList ComboBox
	CComboBox m_CBPortList;
	// Port Connect Button
	CButton m_BttConnect;
	// Log Display
	CListBox m_LogBox;


	CButton Msg_Send;
	CButton MSG_Send;
};
