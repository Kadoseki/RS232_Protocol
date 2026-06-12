#pragma once
#define WM_MYRECEIVE (WM_USER + 1) // 데이터 수신 시 Dlg에 보낼 메시지
#define MAXBUF 4096

class CComunication
{
public:
	CComunication();
	~CComunication();
public:
	void PortConnection(int iPortIndex, CString strPort, HANDLE& phComm, CListBox& logBox, HWND hWnd);
	void LogMessage(CString strMessage, CListBox& logBox);
	bool ConfigurePort(HANDLE& hComm);
	void DisConnect();
public:
	int Send(const void* buffer, int length);
	int Recv(void* buffer, int length);

	HANDLE IsOpenCom() const { return m_hComm; }
private:
	static UINT CommThread(LPVOID lpData);

	bool StartCommThread(HWND hWnd); // 스레드 생성 및 시작
	void StopCommThread();           // 스레드 안전 종료
	static UINT CommThreadFunc(LPVOID lpData);
	 
private:
	HANDLE      m_hComm;        // 통신 핸들 내부 저장
	HWND        m_hWnd;         // 메시지를 보낼 창의 핸들
	OVERLAPPED  m_OLR;            // 읽기용 비동기 구조체
	//OVERLAPPED  m_osRead;          // 
	//OVERLAPPED  m_osWrite;          // 
	BYTE        m_sInBuf[MAXBUF * 2]; // 수신 버퍼
	int         m_nLength;      // 버퍼 데이터 길이

	HANDLE      m_hThread;      // 통신 스레드 핸들
	CWinThread* m_pThread;      // 스레드 객체 포인터
	bool        m_bIsOpenned;   // 스레드 while 루프 제어 플래그
public:
	void Closed_Communication();
};


