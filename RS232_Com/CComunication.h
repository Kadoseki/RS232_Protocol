#pragma once
#define WM_MYRECEIVE (WM_USER + 1) // 데이터 수신 시 Dlg에 보낼 메시지
#define MAXBUF 4096

class CComunication
{
public:
	CComunication();
	~CComunication();
public:
	void PortConnection(int iPortIndex, CString strPort, HANDLE& phComm, CListBox& logBox);
	void LogMessage(CString strMessage, CListBox& logBox);
	bool ConfigurePort(HANDLE& hComm);
	void DisConnect();
public:
	int Send(const void* buffer, int length);
	int Recv(void* buffer, int length);

	HANDLE IsOpenCom() const { return m_hComm; }
private:
	UINT CommThread(LPVOID lpData);


private:
	HANDLE      m_hComm;        // 통신 핸들 내부 저장
	HANDLE      m_hThread;      // 통신 스레드 핸들
	HWND        m_hWnd;         // 메시지를 보낼 창의 핸들
	BOOL        m_bIsOpenned;   // 스레드 루프 제어
	OVERLAPPED  m_OLR;            // 읽기용 비동기 구조체
	//OVERLAPPED  m_osRead;          // 
	//OVERLAPPED  m_osWrite;          // 
	BYTE        m_sInBuf[MAXBUF * 2]; // 수신 버퍼
	int         m_nLength;      // 버퍼 데이터 길이

public:
	void Closed_Communication();
};


