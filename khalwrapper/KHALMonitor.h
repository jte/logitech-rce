/**
 */
#pragma once

#include <Windows.h>

class cKHALMonitor
{
public:
	static void StartMonitoring();
	static void StopMonitoring();
	cKHALMonitor();
	void Init();
	static int CALLBACK KhalMonitorThread(int);
	static bool IsKHALRunning();
	void Exit();
	~cKHALMonitor();
protected:
private:
	HANDLE m_hEvent;
	HANDLE m_hMonitorThread;
	DWORD m_dwMonitorThreadId;
	static HANDLE s_KHALMonitorhandles;
	static HANDLE s_EventInitKhalMonitorThread;
	static bool s_KHALRunning;
	static cKHALMonitor *s_pKHALMonitor;
};
