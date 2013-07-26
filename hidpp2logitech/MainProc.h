/**
 */
#pragma once

#include <Windows.h>

#define UMD_MAX_INDEX 0xD

enum
{
	LD_MAINPROC_STOPPED,
	LD_MAINPROC_STARTING,
	LD_MAINPROC_RUNNING
};


struct DeviceEventControl_t
{
	void *pInit;
	void *pStart;
	void *pStop;
	void *pCompareEvent;
	void *pPreDeviceControl;
	void *pPostDeviceControl;
	void *pIsEventValid;
	void *pRestoreAllSettings;
};


struct CGlobal
{
	DWORD ulProcessType;
	LPSECURITY_ATTRIBUTES lpDummySecurityAttributes;
	DWORD dwDefaultPriorityClass;
	DWORD pad1;
	DWORD dwFlags;
	char szModulePath[MAX_PATH];
	HANDLE hMapFileObject;
	HMODULE hUser32Module;
	BOOL bDelayedInitDone;
	HANDLE hDelayedInitThread;
	HANDLE hFLockChangeEvent;
	HANDLE hKeyboardConfigChangeEvent;
	HANDLE hDeviceListMutex;
	HANDLE hClientListMutex;
	HANDLE hIoFctMainFifoMutex;
	DeviceEventControl_t DeviceEventControl;
};