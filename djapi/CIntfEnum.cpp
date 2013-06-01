#include "CIntfEnum.h"
#include "CEnumerator.h"

CIntfEnum::CIntfEnum()
{
	m_hEnumNotifWnd = 0;
	m_hSetupApiLibrary = 0;
	m_Events[0] = CreateEventW(0, 0, 0, 0);
	m_Events[1] = CreateEventW(0, 0, 0, 0);
	SIZE_T ThreadId = 0;
	m_EnumeratorThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CIntfEnum::InterfaceEnumeratorThreadStatic, (LPVOID)this, 0, &ThreadId);
}

int CALLBACK CIntfEnum::InterfaceEnumeratorThreadStatic(LPVOID Param)
{
	if(CIntfEnum* pIntfEnum = (CIntfEnum*)Param)
	{
		pIntfEnum->InterfaceEnumeratorThread();
	}
	return 0;
}

void CIntfEnum::InterfaceEnumeratorThread()
{
	while(1)
	{
		if(WaitForMultipleObjects(2, (const HANDLE*)&m_Events, 0, INFINITE) != 1)
		{
			break;
		}
		m_pfnCMP_WaitNoPendingInstallEvents(30000);
		//(*(void (__thiscall **)(CEnumerator *))(pThis->vtbl + 12))(pThis);
	}
}

BOOL CIntfEnum::CloseHandle(HANDLE hObject)
{
	return CloseHandle(hObject);
}

CIntfEnum::~CIntfEnum()
{
	if(m_EnumeratorThread )
	{
		SetEvent(m_Events[1]);
		if(WaitForSingleObject(m_EnumeratorThread, 5000))
		{
			TerminateThread(m_EnumeratorThread, 0);
		}
		CloseHandle(m_EnumeratorThread);
		CloseHandle(m_Events[0]);
		CloseHandle(m_Events[1]);
	}
	if(m_hSetupApiLibrary)
	{
		FreeLibrary(m_hSetupApiLibrary);
	}
}

CIntfEnum *g_pEnumerator;

int CIntfEnum::CreateEnumNotifWnd(HINSTANCE hInstance)
{
	if(hInstance)
	{
		WNDCLASSW WndClass;
		WndClass.style = 0;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 0;
		WndClass.hIcon = 0;
		WndClass.hCursor = 0;
		WndClass.hbrBackground = 0;
		WndClass.lpszMenuName = 0;
		WndClass.lpfnWndProc = CIntfEnum::EnumNotifWndProcStatic;
		WndClass.hInstance = hInstance;
		WndClass.lpszClassName = L"CIntfEnumNotifWnd";
		g_pEnumerator = this;
		RegisterClassW(&WndClass);
		m_hEnumNotifWnd = CreateWindowExW(0x80, L"CIntfEnumNotifWnd", L"CIntfEnumNotifWnd", 0x80000000u, 0, 0, 0, 0, 0, 0, hInstance, (LPVOID)this);
		if(m_hEnumNotifWnd)
		{
			ShowWindow(m_hEnumNotifWnd, 0);
		}
	}
	m_hSetupApiLibrary = LoadLibraryW(L"SetupApi.dll");
	if(m_hSetupApiLibrary)
	{
#define RESOLVE_SETUPAPI_FUNC(def) do{if(!(m_pfn##def = (Def_##def*)GetProcAddress(m_hSetupApiLibrary, #def)))return 0;}while(0)
		RESOLVE_SETUPAPI_FUNC(CM_Get_Parent);
		RESOLVE_SETUPAPI_FUNC(CM_Get_Depth);
		RESOLVE_SETUPAPI_FUNC(CM_Get_Device_IDA);
		RESOLVE_SETUPAPI_FUNC(CM_Locate_DevNodeA);
		RESOLVE_SETUPAPI_FUNC(CMP_WaitNoPendingInstallEvents);
		RESOLVE_SETUPAPI_FUNC(SetupDiGetClassDevsW);
		RESOLVE_SETUPAPI_FUNC(SetupDiGetDeviceInterfaceDetailW);
		RESOLVE_SETUPAPI_FUNC(SetupDiGetDeviceRegistryPropertyW);
		RESOLVE_SETUPAPI_FUNC(SetupDiEnumDeviceInterfaces);
		RESOLVE_SETUPAPI_FUNC(SetupDiDestroyDeviceInfoList);
		RESOLVE_SETUPAPI_FUNC(SetupDiEnumDeviceInfo);
		RESOLVE_SETUPAPI_FUNC(SetupDiOpenDeviceInterfaceRegKey);
		return 1;
	#undef RESOLVE_SETUPAPI_FUNC
	}
	return 0;
}

LRESULT CALLBACK CIntfEnum::EnumNotifWndProcStatic(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if(g_pEnumerator)
	{
		return g_pEnumerator->EnumNotifWndProc(hWnd, Msg, wParam, lParam);
	}
	else
	{
		return DefWindowProcW(hWnd, Msg, wParam, lParam);
	}
}

LRESULT CALLBACK CIntfEnum::EnumNotifWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	switch(Msg)
	{
		case WM_DEVICECHANGE:
		{
			PDEV_BROADCAST_HDR lpdb = (PDEV_BROADCAST_HDR)lParam;
			switch(wParam)
			{
				case DBT_DEVICEQUERYREMOVE:
				case DBT_DEVICEREMOVEPENDING:
				case DBT_DEVICEREMOVECOMPLETE:
				{
					if(lpdb->dbch_devicetype == DBT_DEVTYP_HANDLE)
					{
						PDEV_BROADCAST_HANDLE lphnd = (PDEV_BROADCAST_HANDLE)lParam;
						this->CloseHandle(lphnd->dbch_handle);
					}
				}
				break;
				case DBT_DEVICEARRIVAL:
				case DBT_DEVICEQUERYREMOVEFAILED:
				{
					if(lpdb->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE)
					{
						SetEvent(m_Events[0]);
					}
				}
				default:
					return DefWindowProcW(hWnd, Msg, wParam, lParam);
			}
		}
		break;
		case 0x808F:
		{
			this->CloseHandle((HANDLE)wParam);
		}
		break;
		case WM_TIMER:
		{
			if(wParam)
			{
				v7 = CEnumerator::GetSingleton()->GetDeviceHandle(a2);
				if(v7)
				{
					RetryReceiverScan((int)v7);
					CEnumerator::GetSingleton();
					CWirelessDevice::Unknown25(v7);
				}
			}
		}
		break;
	}
}