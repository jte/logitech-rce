#include "KHALMonitor.h"

HANDLE g_hKHALMainProcessWindow;
HANDLE cKHALMonitor::s_KHALMonitorhandles;
HANDLE cKHALMonitor::s_EventInitKhalMonitorThread;
bool cKHALMonitor::s_KHALRunning;
cKHALMonitor *cKHALMonitor::s_pKHALMonitor;

cKHALMonitor::cKHALMonitor()
{
	m_hEvent = CreateEventW(NULL,FALSE,FALSE,NULL);
	cKHALMonitor::s_KHALMonitorhandles = m_hEvent;
	cKHALMonitor::s_EventInitKhalMonitorThread = CreateEventW(NULL,FALSE,FALSE,NULL);
}

void cKHALMonitor::Init()
{
	HWND hWnd = FindWindowW(L"KHALMainProcessWindow", NULL);
	if(hWnd)
	{
		DWORD dwProcessId;
		GetWindowThreadProcessId(hWnd, &dwProcessId);
		HANDLE hProc = OpenProcess(0x100000 | 0x400, FALSE, dwProcessId);
		if(hProc)
		{
			g_hKHALMainProcessWindow = hProc;
			m_hMonitorThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)KhalMonitorThread, NULL, 0, &m_dwMonitorThreadId);
			WaitForSingleObject(cKHALMonitor::s_EventInitKhalMonitorThread, 2000);
		}
	}
}

void cKHALMonitor::Exit()
{
	SetEvent(m_hEvent);
	DWORD dwStatus = WaitForSingleObject(m_hMonitorThread, 1000);
	dwStatus = !dwStatus || dwStatus == ERROR_WAIT_NO_CHILDREN;
	if(!dwStatus)
	{
		TerminateThread(m_hMonitorThread, 0);
		Sleep(100);
	}
}

int CALLBACK cKHALMonitor::KhalMonitorThread(int)
{
	SetEvent(cKHALMonitor::s_EventInitKhalMonitorThread);
	if(WaitForMultipleObjects(2, &cKHALMonitor::s_KHALMonitorhandles, FALSE, INFINITE) == 1)
	{
		cKHALMonitor::s_KHALRunning = false;
	}
	return 0;
}

bool cKHALMonitor::IsKHALRunning()
{
	return cKHALMonitor::s_KHALRunning;
}

cKHALMonitor::~cKHALMonitor()
{
	CloseHandle(m_hEvent);
	CloseHandle(cKHALMonitor::s_EventInitKhalMonitorThread);
}

void cKHALMonitor::StartMonitoring()
{
	if(!cKHALMonitor::s_pKHALMonitor)
	{
		cKHALMonitor::s_pKHALMonitor = new cKHALMonitor;
		cKHALMonitor::s_pKHALMonitor->Init();
	}
}

void cKHALMonitor::StopMonitoring()
{
	if(cKHALMonitor::s_pKHALMonitor)
	{
		cKHALMonitor::s_pKHALMonitor->Exit();
		delete cKHALMonitor::s_pKHALMonitor;
	}
}