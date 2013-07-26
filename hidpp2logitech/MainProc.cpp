#include <Windows.h>
#include "MainProc.h"

int LD_Main(HINSTANCE hInstance)
{
	HANDLE hObject;
	HANDLE hMutex;
	BOOL bStatus;
	MSG Msg;

	if(g_pGbl->ulProcessType && g_pMainShare->dwProcessId == GetCurrentProcessId())
	{
		hMutex = CreateMutexA(0, 0, "LD_KHAL_MainProcessMutex");
		if(WaitForSingleObject(hMutex, 0) != WAIT_TIMEOUT)
		{
			DeviceLoadModelTable();
			if(g_pGbl->dwFlags & GLOBAL_FLAGS_ENCRYPT_DEVICEINI/*0x400*/)
			{
				return MainFreeSystemDll(v2);
			}
			else
			{
				StartIniRepositoryWatcher();
				g_pMain->hMainWindow = (int)WndCreateWindow(hInstance, MainWindowProc, "KHALMainProcessWindow");
				WndQueueWorkItem(&g_pMain->field_13B0, MainDelayedInitialization, 0);
				g_pMain->hGlobalSessionMutex = (int)CreateMutexA(&g_pMain->DummySecurityAttributes, 0, "Global\\LD_KHAL_MainSessionMutex");
				if(WaitForSingleObject((HANDLE)g_pMain->hGlobalSessionMutex, 0x2710u) == 0)
				{
					ReleaseMutex(g_pMain->hGlobalSessionMutex);
				}
				g_pMain->hInstance = hInstance;
				MainLoadUMDDlls();
				MainStartIOThreads();
				MainAPIBroadcastControl(6, 0, 0, 0, 1);
				g_pMainShare->dwState = 2;
				hObject = CreateEventA(0, 1, 0, "LD_KHAL_MainProcStartEvent");
				if ( hObject )
				{
					SetEvent(hObject);
				}
				while ( 1 )
				{
					bStatus = GetMessageA(&Msg, 0, 0, 0);
					if(!bStatus || bStatus == -1)
					{
						break;
					}
					TranslateMessage(&Msg);
					DispatchMessageA(&Msg);
				}
				if ( hObject )
				{
					ResetEvent(hObject);
					CloseHandle(hObject);
				}
				DeviceFreeModelTable();
				MainFreeUMDDlls();
				MainFreeSystemDll(v6);
				bStatus = CloseHandle(g_pMain->hGlobalSessionMutex);
				HeapFree(g_hHeap, 0, g_pClientShare);
				g_pClientShare = 0;
				HeapFree(g_hHeap, 0, g_pMain);
				g_pMain = 0;
				bStatus = ReleaseMutex(hMutex);
				bStatus = CloseHandle(hMutex);
			}
		}
		return true;
	}
	return false;
}

int MainDelayedInitialization()
{
	if(g_pfnWTSRegisterSessionNotification)
	{
		for(size_t i = 0; i < 10; i++)
		{
			g_pMain->bSessionNotificationEnabled = g_pfnWTSRegisterSessionNotification(g_pMain->hMainWindow, 0);
			if ( GetLastError() == RPC_S_INVALID_BINDING )
			{
				HANDLE
					hObject = OpenEventA(0x100000u, 0, "Global\\TermSrvReadyEvent");
				if(hObject && g_pMain->hStopMainProcessThreadEvent)
				{
					WaitForMultipleObjects(2u, &hObject, 0, 0x7530u);
				}
				CloseHandle(hObject);
			}
			else
			{
				if(!g_pMain->bSessionNotificationEnabled)
				{
					WaitForSingleObject(g_pMain->hStopDeviceIoControlThreadEvent, 30000);
				}
			}
			if (g_pMain->bSessionNotificationEnabled)
			{
				break;
			}
		}
	}
	return 0;
}

int __cdecl MainLoadUMD(unsigned int ulUMDtype, int DLL, const char *Prefix, int UMD, int a5)
{
  int v6; // [sp+0h] [bp-23Ch]@0
  int v7; // [sp+10h] [bp-22Ch]@33
  const CHAR ProcName; // [sp+14h] [bp-228h]@9
  const CHAR Dst; // [sp+11Ch] [bp-120h]@6
  unsigned int v10; // [sp+228h] [bp-14h]@1
  HMODULE hModule; // [sp+22Ch] [bp-10h]@6
  int v12; // [sp+230h] [bp-Ch]@47
  int v13; // [sp+234h] [bp-8h]@49
  int v14; // [sp+238h] [bp-4h]@1
  int v15; // [sp+23Ch] [bp+0h]@1

	v14 = 0;
	strcpy_s((char *)&Dst, 0x104u, g_pGbl->szModulePath);
	strcat_s((char *)&Dst, 0x104u, (const char *)DLL);
	hModule = LoadLibraryA(&Dst);
	g_pMain->HwApi[ulUMDtype].hModule = hModule;
	if(hModule)
	{
		char ProcName[MAX_PATH];
		strcpy_s(ProcName, MAX_PATH, Prefix);
		strcat_s(ProcName, MAX_PATH, "MainInit");
		g_pMain->HwApi[ulUMDtype].MainInit = GetProcAddress(hModule, ProcName);
		strcpy_s(ProcName, MAX_PATH, Prefix);
		strcat_s(ProcName, MAX_PATH, "MainExit");
		g_pMain->HwApi[ulUMDtype].MainExit = GetProcAddress(hModule, ProcName);
		strcpy_s(ProcName, MAX_PATH, Prefix);
		strcat_s(ProcName, MAX_PATH, "Control");
		g_pMain->HwApi[ulUMDtype].Control = GetProcAddress(hModule, ProcName);
		strcpy_s(ProcName, MAX_PATH, Prefix);
		strcat_s(ProcName, MAX_PATH, "EvaluateEventCompatibility");
		g_pMain->HwApi[ulUMDtype].EvaluateEventCompatibility = GetProcAddress(hModule, ProcName);
		strcpy_s(ProcName, MAX_PATH, Prefix);
		strcat_s(ProcName, MAX_PATH, "RegisterEvent");
		g_pMain->HwApi[ulUMDtype].RegisterEvent = GetProcAddress(hModule, ProcName);
		strcpy_s(ProcName, MAX_PATH, Prefix);
		strcat_s(ProcName, MAX_PATH, "UnregisterEvent");
		g_pMain->HwApi[ulUMDtype].UnregisterEvent = GetProcAddress(hModule, ProcName);
		strcpy_s(ProcName, MAX_PATH, Prefix);
		strcat_s(ProcName, MAX_PATH, "DeviceControl");
		g_pMain->HwApi[ulUMDtype].DeviceControl = GetProcAddress(hModule, ProcName);
		strcpy_s(ProcName, MAX_PATH, Prefix);
		strcat_s(ProcName, MAX_PATH, "DeviceCleanup");
		g_pMain->HwApi[ulUMDtype].DeviceCleanup = GetProcAddress(hModule, ProcName);
		strcpy_s(ProcName, MAX_PATH, Prefix);
		strcat_s(ProcName, MAX_PATH, "DeviceStop");
		g_pMain->HwApi[ulUMDtype].DeviceStop = GetProcAddress(hModule, ProcName);
		v14 = MainTestUMD(ulUMDtype);
	}
	if ( v14 )
	{
		v14 = g_pMain->HwApi[ulUMDtype].Control(0, &v12, 0, 0);
		if ( v14 )
		{
			if ( v12 != UMD || v13 != a5 )
			{
				v14 = 0;
			}
		}
	}
	if ( v14 != 1 )
		MainClearUMD(ulUMDtype);
	return v14;
}

signed int __cdecl MainInit()
{
  signed int result; // eax@12
  int v1; // eax@47
  int bStatus; // [sp+0h] [bp-Ch]@0
  DWORD ThreadId; // [sp+4h] [bp-8h]@11
  int v4; // [sp+8h] [bp-4h]@11
  int v5; // [sp+Ch] [bp+0h]@11

	g_pMain = (CMain *)HeapAlloc(g_hHeap, 8u, 0x1D54u);
	if ( g_pMain )
	{
		memset(g_pMain, 0, 0x1D54u);
		g_pMain->dwSize = 0x1D54u;
		g_pMain->field_4 = 0;
		g_pMain->field_8 = 0;
		g_pMain->bSessionNotificationEnabled = 0;
		g_pMain->hMainWindow = 0;
		g_pMain->field_34 = 0;
		g_pMain->hIniRepositoryWatcherThread = 0;
		MainLoadSystemDll();
		if ( g_pMain->pConvertStringSecurityDescriptorToSecurityDescriptorA )
		{
		  bStatus = ((int (__stdcall *)(_DWORD, _DWORD, _DWORD, _DWORD))g_pMain->pConvertStringSecurityDescriptorToSecurityDescriptorA)(
					  "D:(A;OICI;GA;;;WD)", 1, &g_pMain->DummySecurityAttributes.lpSecurityDescriptor, 0);
		}
		g_pMain->field_1D44 = CreateEventA(0, 1, 0, 0);
		g_pMain->hSupressBeepEvent = (int)CreateEventA( &g_pMain->DummySecurityAttributes, 1, 1, "Global\\LogitechSetPointBeepEnabled");
		g_pMain->hMouseCPLOpened = (int)CreateEventA(&g_pMain->DummySecurityAttributes, 1, 0, "Global\\LogitechSetPointMouseCPLOpened");
		g_pMain->hMouseCPLClosed = (int)CreateEventA(&g_pMain->DummySecurityAttributes, 1, 0, "Global\\LogitechSetPointMouseCPLClosed");
		g_pMain->hLogihtechMouseInUseEvent = (int)CreateEventA( &g_pMain->DummySecurityAttributes, 1, 0, "Global\\LogitechSetPointLogiMouseInUse");
		g_pMain->dwSessionState = 0;
		WndInitWorkItemList(g_pMain, &g_pMain->field_1178);
		WndInitWorkItemList(g_pMain, &g_pMain->field_13B0);
		memset(&g_pMain->field_1908, 0, 0x100u);
		memset(&g_pMain->gap_1910[248], 0, 0x100u);
		g_pMain->field_40 = (int)CreateMutexA(0, 0, 0);
		memset(g_pMain->gap_44, 0, 0xFA0u);
		memset(&g_pMain->gap_44[4000], 0, 0x190u);
		*(_DWORD *)&g_pMain->gap_44[4400] = 1;
		g_pMain->ulModelCount = 0;
		g_pMain->ModelTable = 0;
		*(_DWORD *)&g_pMain->gap_1B14[0] = 0;
		memset(&g_pMain->gap_1B14[4], 0, 0x200u);
		g_pMain->field_1D18 = 0;
		g_pMain->field_1D1C = 0;
		memset(g_pMain->HwApi, 0, 0x208u);
		g_pMain->HwApi[0].hModule = 0;
		g_pMain->HwApi[0].MainInit = (int)DeviceMgrMainInit;
		g_pMain->HwApi[0].MainExit = (int)DeviceMgrMainExit;
		g_pMain->HwApi[0].Control = (int)DeviceMgrControl;
		g_pMain->HwApi[0].EvaluateEventCompatibility = (int)DeviceMgrEvaluateEventCompatibility;
		g_pMain->HwApi[0].RegisterEvent = (int)DeviceMgrRegisterEvent;
		g_pMain->HwApi[0].UnregisterEvent = (int)DeviceMgrUnregisterEvent;
		g_pMain->HwApi[0].DeviceControl = (int)DeviceMgrDeviceControl;
		g_pMain->pDeviceCreate = (int)DeviceCreate;
		g_pMain->pDeviceStart = (int)DeviceStart;
		g_pMain->pDeviceSetModel = (int)DeviceSetModel;
		g_pMain->pDeviceSetRelationship = (int)DeviceSetRelationship;
		g_pMain->pDeviceRemove = (int)DeviceRemove;
		g_pMain->pDeviceRegisterEvent = (int)DeviceRegisterEvent;
		g_pMain->pDeviceUnregisterEvent = (int)DeviceUnregisterEvent;
		g_pMain->pDeviceDispatchEvent = (int)DeviceDispatchEvent;
		g_pMain->pDeviceGetAndLock = (int)DeviceGetAndLock;
		g_pMain->pGetAndLockHardwareEventFromHandle = (int)GetAndLockHardwareEventFromHandle;
		g_pMain->pDeviceGetCapability = (int)DeviceGetCapability;
		g_pMain->pDeviceParseListQuick = (int)DeviceParseListQuick;
		g_pMain->pDeviceParseList = (int)DeviceParseList;
		g_pMain->pDeviceParseListRemoveUmd = (int)DeviceParseListRemoveUmd;
		g_pMain->pLD_DeviceControl = (int)LD_DeviceControl;
		g_pMain->pDeviceFindModelInfo = (int)DeviceFindModelInfo;
		g_pMain->pDeviceReportUnknown = (int)DeviceReportUnknown;
		g_pMain->pDevMgrSignalRefreshComplete = (int)DevMgrSignalRefreshComplete;
		g_pMain->field_1884 = (int)sub_1000FEB0;
		g_pMain->field_1888 = (int)sub_10011C70;
		g_pMain->pDeviceBuildCapabilityString = (int)DeviceBuildCapabilityString;
		g_pMain->field_1890 = (int)sub_1001FBD0;
		g_pMain->field_1894 = (int)sub_1001FDC0;
		g_pMain->pReloadIniFiles = (int)ReloadIniFiles;
		g_pMain->pGbl = (int)g_pGbl;
		g_pMain->pMain = (int)g_pMain;
		g_pMain->pMainShare = (int)g_pMainShare;
		g_pMain->hStopMainProcessThreadEvent = (int)CreateEventA(0, 1, 0, 0);
		v4 = GetLastError();
		if ( g_pMain->hStopMainProcessThreadEvent )
		{
			g_pMain->hBroadcastMutex = (int)CreateMutexA(0, 0, 0);
			g_pMain->hEnumMutex = (int)CreateMutexA(0, 0, 0);
			g_pMain->hHiResSettingListMutex = (int)CreateMutexA(0, 0, 0);
			g_pMain->hMainProcessThread = (int)CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainClientManagerThread, 0, 0, &ThreadId);
			if ( g_pMain->hMainProcessThread )
			{
				g_pMain->hKeyboardFLockThread = (int)CreateThread(0, 0, (LPTHREAD_START_ROUTINE)KeyboardFLockThread, 0, 0, &ThreadId);
				return 1;
			}
		}
	}
	return 0;
}

int MainExit()
{
	if ( g_pMain && g_pMainShare && g_pGbl )
	{
		if ( g_pGbl->ulProcessType )
		{
			SetEvent(g_pMain->field_1D44);
			StopNiceThread(g_pMain->hKeyboardFLockThread, g_pMain->hStopMainProcessThreadEvent);
			CleanUpHandle(&g_pMain->hKeyboardFLockThread);
			CleanUpHandle(&g_pMain->hSupressBeepEvent);
			StopIniRepositoryWatcher();
			CCursorCohab::GetSingleton()->StopMouseSwitchingMonitor(1);
			CleanUpHandle(&g_pMain->hMouseCPLOpened);
			CleanUpHandle(&g_pMain->hMouseCPLClosed);
			CleanUpHandle(&g_pMain->hLogihtechMouseInUseEvent);
			MainStopIOThreads();
			WndCleanupWorkItemList((int)&g_pMain->field_1178);
			WndCleanupWorkItemList((int)&g_pMain->field_13B0);
			for ( i = 0; i < 13; ++i )
			{
				g_pMain->HwApi[i].MainExit(0, 0);
			}
			if ( g_pMain->hMainProcessThread )
			{
				StopNiceThread(g_pMain->hMainProcessThread, g_pMain->hStopMainProcessThreadEvent);
				CleanUpHandle(&g_pMain->hBroadcastMutex);
				CleanUpHandle(&g_pMain->hEnumMutex);
				CleanUpHandle(&g_pMain->hHiResSettingListMutex);
				CleanUpHandle(&g_pMain->hMainProcessThread);
			}
			if ( g_pMainShare->DeviceManagerListPosition != -1 )
			{
				v9 = (LD_DEVICE *)ListGetAt(g_pMainShare, &g_pMainShare->DeviceList, g_pMainShare->DeviceManagerListPosition);
				if ( v9 && AcquireRemoveLock(0, 0xFu, v9) )
					DeviceRemove(15, 0, v9, 0);
				g_pMainShare->DeviceManagerListPosition = -1;
			}
			CleanUpHandle(&g_pMain->hBroadcastMutex);
			CleanUpHandle(&g_pMain->hStopMainProcessThreadEvent);
			StopNiceThread(g_pClientShare->hNotificationThread, g_pClientShare->hNotificationThreadStopEvent);
			CleanUpHandle(&g_pClientShare->hNotificationThread);
			CleanUpHandle(&g_pClientShare->hNotificationThreadStopEvent);
			CleanUpHandle(&g_pMain->field_40);
			CleanUpHandle(&g_pMain->field_1D44);
		}
	}
	return result;
}

int CALLBACK MainWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM hRawInput)
{
	switch(Msg)
	{
		case WM_INPUT:
			v6 = CCursorCohab::GetSingleton()->ProcessRawInput((HRAWINPUT)hRawInput);
			return DefWindowProcA(hWnd, Msg, wParam, hRawInput);
		break;
		case WM_POWERBROADCAST:
			MainOnPowerBroadcast(wParam, v5, wParam);
			return 1;
		break;
		case WM_WTSSESSION_CHANGE: 
			MainOnSessionChange(wParam, hRawInput);
			return 0;
		break;
		case WM_ENDSESSION:
			if ( wParam == 1 )
			{
				if ( hEvent )
				{
					v4 = -(WaitForSingleObject(hEvent, 0x7D0u) != 0);
					v12 = v4 + 1;
					CloseHandle(hEvent);
					hEvent = 0;
				}
				DeviceMgrDispatchEvent(5, 5u, 0, 0);
			}
			else
			{
				if ( hEvent )
				{
					CloseHandle(hEvent);
					hEvent = 0;
				}
				WndQueueWorkItem(&g_pMain->field_1178, MainOnConsoleConnectThread, 0);
			}
			return 0;
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
		break;
		case WM_CLOSE:
		if ( g_pMain->field_34 )
		{
			DestroyWindow(hWnd);
		}
		else
		{
			g_pMain->field_34 = 1;
			if ( g_pMain->bSessionNotificationEnabled == 1 && g_pfnWTSUnRegisterSessionNotification )
			{
				v12 = g_pfnWTSUnRegisterSessionNotification(g_pMain->hMainWindow);
			}
			hObject = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainKillThread, 0, 0, &ThreadId);
			CloseHandle(hObject);
		}
		return 0;
		break;
		case WM_QUERYENDSESSION:
			g_pMainShare->lMainProcessState = 4;
			hEvent = CreateEventA(0, 0, 0, 0);
			WndQueueWorkItem(&g_pMain->field_1178, MainOnConsoleDisconnectThread, hEvent);
			return 1;
		break;
		default:
			return DefWindowProcA(hWnd, Msg, wParam, hRawInput);
		break;
	}
}

