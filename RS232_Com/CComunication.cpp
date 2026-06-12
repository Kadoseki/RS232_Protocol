#include "pch.h"
#include "CComunication.h"
#include <afxwin.h>

CComunication::CComunication()
	: m_hComm(nullptr)
{
}

CComunication::~CComunication()
{
}

void CComunication::Closed_Communication()
{
	if(nullptr != this)
		delete this;
}


void CComunication::PortConnection(int iPortIndex, CString strPort,	HANDLE &phComm, CListBox &logBox, HWND hWnd)
{
	//포트 열기 (Windows API)

	CString strPath;
	strPath.Format(_T("\\\\.\\%s"), strPort);// 두 자리 수를 위한 만약의 \\

	HANDLE hComm = CreateFile(strPath,
		GENERIC_READ | GENERIC_WRITE, // 읽기/쓰기 권한
		0,                            // 공유 불가
		NULL,                         // 보안 속성 없음
		OPEN_EXISTING,                // 반드시 기존 포트 열기
		0,                            // 동기식 통신 (간단한 구현용)
		NULL);

	if (INVALID_HANDLE_VALUE == hComm)
	{
		CloseHandle(hComm);
		LogMessage(_T("Connect Error"), logBox);
		return;
	}

	//그릇 크기 정하기 (SetupComm)
	SetupComm(hComm, 4096, 4096);
	SetCommMask(hComm, 0);

	SetCommMask(hComm, EV_RXCHAR); // (중요) 수신 이벤트를 켜줌

	if (ConfigurePort(hComm))
	{
		this->m_hComm = hComm; // 클래스 멤버에 핸들 저장

		// ★ 스레드 생성 및 가동 시작!
		if (StartCommThread(hWnd)) {
			phComm = hComm;
			LogMessage(_T("Connect Success & Thread Started"), logBox);
		}
		else {
			CloseHandle(hComm);
			LogMessage(_T("Thread Create Error"), logBox);
		}
	}

	COMMTIMEOUTS cto;
	cto.ReadIntervalTimeout = 0;
	cto.ReadTotalTimeoutMultiplier = 1;
	cto.ReadTotalTimeoutConstant = 100;
	cto.WriteTotalTimeoutMultiplier = 1;
	cto.WriteTotalTimeoutConstant = 100;
	SetCommTimeouts(hComm, &cto);

	// Read / Write buffer를 모두 비운다.
	PurgeComm(hComm, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	// 3. 통신 속도 및 타임아웃 설정 (중요!)
	if (ConfigurePort(hComm))
	{
		phComm = hComm;
		LogMessage(_T("Connect Success"), logBox);
	}
	else
	{
		CloseHandle(hComm);
		LogMessage(_T("Connect Error"), logBox);
	}
}

void CComunication::LogMessage(CString strMessage, CListBox &logBox)
{
	// 1. 현재 시간 가져오기
	CTime time = CTime::GetCurrentTime();
	CString strTime = time.Format(_T("[%H:%M:%S] "));

	// 2. 시간 + 메시지
	CString strFullLog = strTime + strMessage;

	// 3. 리스트 박스 맨 아래에 추가
	int nIndex = logBox.AddString(strFullLog);

	// 4. 자동으로 마지막 줄로 스크롤 이동 (매우 중요!)
	logBox.SetCurSel(nIndex);
}

bool CComunication::ConfigurePort(HANDLE& hComm)
{
	DCB dcb = { 0 };
	dcb.DCBlength = sizeof(DCB);

	if (!GetCommState(hComm, &dcb))
		return false;

	dcb.BaudRate = CBR_9600;      // 속도 9600
	dcb.ByteSize = 8;             // 데이터 8비트
	dcb.StopBits = ONESTOPBIT;    // 정지 비트 1
	dcb.Parity = NOPARITY;      // 패리티 없음

	if (!SetCommState(hComm, &dcb))
		return false;

	// 타임아웃 설정 (데이터가 안 올 때 무한 대기 방지)
	COMMTIMEOUTS timeouts = { 0 };
	timeouts.ReadIntervalTimeout = MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;

	return SetCommTimeouts(hComm, &timeouts) != FALSE;
}

void CComunication::DisConnect()
{
	// 이미 닫혀있으면 실행 안 함
	if (!m_bIsOpenned || m_hComm == INVALID_HANDLE_VALUE)
		return;

	StopCommThread();

	// 2. 스레드가 완전히 죽은 것을 확인한 후 포트 닫기
	if (m_hComm != INVALID_HANDLE_VALUE && m_hComm != nullptr)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}

	// 1. 스레드의 while 루프를 빠져나오도록 플래그 해제
	m_bIsOpenned = false;

	// 2. WaitCommEvent 블로킹 강제 해제 (핵심!)
	SetCommMask(m_hComm, 0);

	// 3. 스레드가 완전히 종료될 때까지 대기
	// (안 기다리고 바로 포트를 닫으면 스레드 안에서 에러가 날 수 있음)
	if (m_hThread != NULL) {
		WaitForSingleObject(m_hThread, 2000); // 스레드가 죽을 때까지 최대 2초 대기
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	// 4. 안전하게 시리얼 포트 닫기
	CloseHandle(m_hComm);
	m_hComm = INVALID_HANDLE_VALUE;
}

int CComunication::Send(const void* buffer, int length)
{
	DWORD bytesWritten = 0;

	if (!WriteFile(m_hComm, buffer, length, &bytesWritten, NULL)) {
		if (GetLastError() == ERROR_IO_PENDING) {
			// 전송 대기 처리...
		}
	}
	return (int)bytesWritten;
}

int CComunication::Recv(void* buffer, int length)
{
	DWORD bytesRead = 0;

	if (!ReadFile(m_hComm, buffer, length, &bytesRead, NULL)) {
		if (GetLastError() == ERROR_IO_PENDING) {
			// 전송 대기 처리...
		}
	}
	return (int)bytesRead;
}

UINT CComunication::CommThread(LPVOID lpData)
{
	CComunication* pComm = (CComunication*)lpData; // 클래스 접근을 위한 포인터 변환
	DWORD dwEvtMask = 0;
	BYTE buf[MAXBUF];

	while (pComm->m_bIsOpenned) {
		// 이벤트 대기 (데이터 수신 신호 대기)
		WaitCommEvent(pComm->m_hComm, &dwEvtMask, NULL);

		if ((dwEvtMask & EV_RXCHAR)) {
			COMSTAT ComStat;
			DWORD dwError;
			ClearCommError(pComm->m_hComm, &dwError, &ComStat);

			if (ComStat.cbInQue > 0) {
				DWORD dwRead = 0;
				// 데이터 읽기 시도
				if (!ReadFile(pComm->m_hComm, buf, ComStat.cbInQue, &dwRead, &pComm->m_OLR)) {
					if (GetLastError() == ERROR_IO_PENDING) {
						WaitForSingleObject(pComm->m_OLR.hEvent, 1000);
						GetOverlappedResult(pComm->m_hComm, &pComm->m_OLR, &dwRead, FALSE);
					}
				}

				if (dwRead > 0) {
					// 버퍼에 데이터 저장 (임시 구현)
					memcpy(pComm->m_sInBuf + pComm->m_nLength, buf, dwRead);
					pComm->m_nLength += dwRead;

					// Dlg에 통보: "데이터가 들어왔으니 처리하세요"
					::SendMessage(pComm->m_hWnd, WM_MYRECEIVE, pComm->m_nLength, (LPARAM)pComm);
				}
			}
		}
	}
	return 0;
}

bool CComunication::StartCommThread(HWND hWnd)
{
	m_hWnd = hWnd;             // 데이터를 전달할 다이얼로그 화면 주소 저장
	m_bIsOpenned = true;       // 스레드 루프 작동 ON
	m_nLength = 0;             // 수신 버퍼 길이 초기화

	// 스레드를 일단 '일시정지' 상태로 생성
	m_pThread = AfxBeginThread(CComunication::CommThreadFunc, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED);

	if (m_pThread == nullptr)
		return false;

	// MFC의 자동 삭제 기능을 끄고 우리가 수동으로 관리 (메모리 에러 방지)
	m_pThread->m_bAutoDelete = FALSE;

	// 스레드 가동 시작!
	m_pThread->ResumeThread();
	return true;
}

void CComunication::StopCommThread()
{
	if (!m_bIsOpenned || m_pThread == nullptr)
		return;

	// 스레드의 while 루프를 빠져나오도록 플래그 OFF
	m_bIsOpenned = false;

	// 자고 있는 WaitCommEvent를 강제로 깨워서 루프를 탈출하게 만듦
	if (m_hComm != INVALID_HANDLE_VALUE) {
		SetCommMask(m_hComm, 0);
	}

	// 스레드가 짐을 싸고 완전히 종료될 때까지 최대 3초 기다려줌
	if (::WaitForSingleObject(m_pThread->m_hThread, 3000) == WAIT_TIMEOUT) {
		// (옵션) 3초가 지나도 안 죽으면 강제 종료할 수도 있으나, 보통 자연 종료됨
	}

	// 내가 직접 메모리 해제
	delete m_pThread;
	m_pThread = nullptr;
}

UINT CComunication::CommThreadFunc(LPVOID lpData)
{
	CComunication* pComm = (CComunication*)lpData;
	DWORD dwEvtMask = 0;
	BYTE buf[1024];

	while (pComm->m_bIsOpenned)
	{
		// 데이터가 들어올 때까지 여기서 대기 (CPU 점유율 0%)
		if (WaitCommEvent(pComm->m_hComm, &dwEvtMask, NULL))
		{
			if (dwEvtMask & EV_RXCHAR)
			{
				DWORD dwError;
				COMSTAT ComStat;
				ClearCommError(pComm->m_hComm, &dwError, &ComStat);

				if (ComStat.cbInQue > 0)
				{
					DWORD dwRead = 0;
					// 데이터 읽어오기 (동기식)
					if (ReadFile(pComm->m_hComm, buf, ComStat.cbInQue, &dwRead, NULL))
					{
						if (dwRead > 0)
						{
							// 버퍼 오버플로우 방지
							if (pComm->m_nLength + dwRead >= 4096) {
								pComm->m_nLength = 0;
							}

							memcpy(pComm->m_sInBuf + pComm->m_nLength, buf, dwRead);
							pComm->m_nLength += dwRead;

							// UI 쪽으로 "데이터 왔음!" 신호 보내기
							if (pComm->m_hWnd != NULL) {
								::SendMessage(pComm->m_hWnd, WM_MYRECEIVE, pComm->m_nLength, (LPARAM)pComm);
							}
						}
					}
				}
			}
		}
	}
	return 0; // 스레드 자연 종료
}
