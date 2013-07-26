#include <Windows.h>
#include "KHALMonitor.h"
#include "CDrvApi.h"
#include <queue>

typedef struct
{
	DWORD *lpBuffer;
	DWORD dwFunction;
	BOOL bIsEvent;
	DWORD dwDeviceDUID;
} RegQItem;
std::queue<RegQItem> g_Queue;
HANDLE g_hQueueMutex;
HANDLE g_hThreadCreatedEvent;
HANDLE g_hItemAddedToQueue;
HANDLE g_hKHALRegQThread;

BOOL CALLBACK DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return 1;
}

bool WaitQueueMutex(HANDLE hHandle)
{
	DWORD dwStatus = WaitForSingleObject(hHandle, 4000u);
	return !dwStatus || dwStatus == ERROR_WAIT_NO_CHILDREN;
}

BOOL ReleaseQueueMutex(HANDLE hMutex)
{
	BOOL bStatus = ReleaseMutex(hMutex);
	GetLastError();
	return bStatus;
}

bool AddEventToQueue(LD_EVENT_REGISTRATION *pEvent)
{
	LD_EVENT_REGISTRATION* pLocalEvent = new LD_EVENT_REGISTRATION;
	*pLocalEvent = *pEvent;
	BOOL Status = WaitQueueMutex(g_hQueueMutex);
	if(Status)
	{
		RegQItem item;
		item.bIsEvent = TRUE;
		item.dwFunction = 0x6000000;
		item.lpBuffer = (DWORD*)pLocalEvent;
		item.dwDeviceDUID = pLocalEvent->DUID;
		g_Queue.push(item);
		ReleaseQueueMutex(g_hQueueMutex);
		Status = SetEvent(g_hItemAddedToQueue);
	}
	return Status;
}

bool AddDevCtrlToQueue(int dwDeviceDUID, int dwFunction, BYTE* lpBuffer)
{
	BOOL bStatus = WaitQueueMutex(g_hQueueMutex);
	if(bStatus)
	{
		RegQItem item;
		item.bIsEvent = FALSE;
		item.dwDeviceDUID = dwDeviceDUID;
		item.dwFunction = dwFunction;
		item.lpBuffer = (DWORD*)lpBuffer;
		g_Queue.push(item);
		ReleaseQueueMutex(g_hQueueMutex);
		bStatus = SetEvent(g_hItemAddedToQueue);
	}
	return bStatus;
}


int PLD_DeviceControl(int dwDeviceDUID, int Function, DWORD* lpOutput)
{
	if(!cKHALMonitor::IsKHALRunning() || !CDrvApi::m_LD_DeviceControl)
	{
		return 0x8000001Au;
	}
	if(lpOutput == NULL)
	{
		return 0x80000001;
	}
	switch(Function)
	{
		case 0x1000028:
		{
			BYTE *lpBuffer = new BYTE[0xC];
			*lpBuffer = 0xC;
			lpBuffer[1] = lpOutput[1];
			lpBuffer[2] = lpOutput[2];
			AddDevCtrlToQueue(dwDeviceDUID, Function, lpBuffer);
		}
		break;
		case 0x1000026:
		{
			BYTE *lpBuffer = new BYTE[0x14];
			*lpBuffer = 0x14;
			lpBuffer[1] = lpOutput[1];
			lpBuffer[2] = lpOutput[2];
			lpBuffer[3] = lpOutput[3];
			lpBuffer[4] = lpOutput[4];
			AddDevCtrlToQueue(dwDeviceDUID, Function, lpBuffer);
		}
		break;
		case 0x10:
		{
			BYTE *lpBuffer = new BYTE[0x1C];
			*lpBuffer = 0x1C;
			lpBuffer[2] = lpOutput[2];
			lpBuffer[3] = lpOutput[3];
			lpBuffer[4] = lpOutput[4];
			lpBuffer[5] = lpOutput[5];
			lpBuffer[6] = lpOutput[6];
			AddDevCtrlToQueue(dwDeviceDUID, Function, lpBuffer);
		}
		break;
		case 0x16:
		{
			BYTE *lpBuffer = new BYTE[0x14];
			lpBuffer[0] = 0x14;
			lpBuffer[1] = lpOutput[1];
			lpBuffer[2] = lpOutput[2];
			lpBuffer[3] = lpOutput[3];
			AddDevCtrlToQueue(dwDeviceDUID, Function, lpBuffer);
		}
		break;
		case 0x1000024:
		{
			BYTE *lpBuffer = new BYTE[0x18];
			*lpBuffer = 0x18;
			lpBuffer[1] = lpOutput[1];
			lpBuffer[2] = lpOutput[2];
			lpBuffer[3] = lpOutput[3];
			lpBuffer[4] = lpOutput[4];
			lpBuffer[5] = lpOutput[5];
			AddDevCtrlToQueue(dwDeviceDUID, Function, lpBuffer);
		}
		break;
		case 0x1000033:
		{
			BYTE *lpBuffer = new BYTE[0xC];
			*lpBuffer = 0xC;
			lpBuffer[1] = lpOutput[1];
			AddDevCtrlToQueue(dwDeviceDUID, Function, lpBuffer);
		}
		break;
		default:
		{
			return CDrvApi::m_LD_DeviceControl(dwDeviceDUID, Function, lpOutput);
		}
		break;
    }
	return 0;
}

int PLD_Close()
{
	if(cKHALMonitor::IsKHALRunning() && CDrvApi::m_LD_Close)
	{
		return CDrvApi::m_LD_Close();
	}
	return 0x8000001A;
}

int PLD_GetDeviceDetails(int a1, void* a2)
{
	if(cKHALMonitor::IsKHALRunning() && CDrvApi::m_LD_GetDeviceDetails)
	{
		return CDrvApi::m_LD_GetDeviceDetails(a1, a2);
	}
	return 0x8000001A;
}

int PLD_GetDeviceInfo(int a1, int a2)
{
	if(cKHALMonitor::IsKHALRunning() && CDrvApi::m_LD_GetDeviceInfo)
	{
		return CDrvApi::m_LD_GetDeviceInfo(a1, a2);
	}
	return 0x8000001A;
}

int PLD_GetDeviceName(int a1, int a2, int a3)
{
	if(cKHALMonitor::IsKHALRunning() && CDrvApi::m_LD_GetDeviceName)
	{
		return CDrvApi::m_LD_GetDeviceName(a1, a2, a3);
	}
	return 0x8000001A;
}

int PLD_GetDriverVersionString(int a1, int a2)
{
	if(cKHALMonitor::IsKHALRunning() && CDrvApi::m_LD_GetDriverVersionString)
	{
		return CDrvApi::m_LD_GetDriverVersionString(a1, a2);
	}
	return 0x8000001A;
}

int PLD_GetFeatureInfo(int a1, int a2, int a3, int a4)
{
	if(cKHALMonitor::IsKHALRunning() && CDrvApi::m_LD_GetFeatureInfo)
	{
		return CDrvApi::m_LD_GetFeatureInfo(a1, a2, a3, a4);
	}
	return 0x8000001A;
}

int PLD_GetFeatureList(int a1, int a2, int a3)
{
	if(cKHALMonitor::IsKHALRunning() && CDrvApi::m_LD_GetFeatureList)
	{
		return CDrvApi::m_LD_GetFeatureList(a1, a2, a3);
	}
	return 0x8000001A;
}

int PLD_Open()
{
	if(cKHALMonitor::IsKHALRunning() && CDrvApi::m_LD_Open)
	{
		return CDrvApi::m_LD_Open();
	}
	return 0x8000001A;
}

int PLD_UnregisterEvent(int a1)
{
	if(cKHALMonitor::IsKHALRunning() && CDrvApi::m_LD_UnregisterEvent)
	{
		return CDrvApi::m_LD_UnregisterEvent(a1);
	}
	return 0x8000001A;
}

size_t PLD_RegisterEvent(LD_EVENT_REGISTRATION *pEvent, bool a2)
{
	if(!cKHALMonitor::IsKHALRunning())
	{
		return -1;
	}
	if(a2 && CDrvApi::m_LD_RegisterEvent)
	{
		return CDrvApi::m_LD_RegisterEvent(pEvent);
	}
	if(CDrvApi::m_LD_RegisterEvent)
	{
		if(pEvent->Flags != 0x10000)
		{
			return CDrvApi::m_LD_RegisterEvent(pEvent);
		}
		AddEventToQueue(pEvent);
	}
	return -1;
}

DWORD InitializeKHALRegQThread()
{
	g_hQueueMutex = CreateMutexW(NULL, FALSE, NULL);
	v0 = CreateEventW(0, 0, 0, 0);
	dword_10008408 = v0;
	Handles = v0;
	v1 = CreateEventW(0, 0, 0, 0);
	g_hItemAddedToQueue = v1;
	g_hItemAddedToQueue2 = (int)v1;
	g_hThreadCreatedEvent = CreateEventW(0, 0, 0, 0);
	DWORD ThreadId = 0;
	g_hKHALRegQThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)ProcessKHALRegQ, 0, 0, &ThreadId);
	return WaitForSingleObject(g_hThreadCreatedEvent, 3000u);
}

DWORD CALLBACK ProcessKHALRegQ(LPVOID)
{
	v73 = 0;
	v72 = 0;
	SetEvent(g_hThreadCreatedEvent);
	do
	{
		while(!g_Queue.empty())
		{
			v17 = WaitQueueMutex(g_hQueueMutex);
			if ( v17 )
			{
				v71 = 0;
				RegQItem item = g_Queue.front();
				g_Queue.pop();
				ReleaseQueueMutex(g_hQueueMutex);
				if(item.bIsEvent)
				{
					
					if(CDrvApi::m_LD_RegisterEvent)
					{
						LD_EVENT_REGISTRATION *pEvent = (LD_EVENT_REGISTRATION*)item.lpBuffer;
						UINT wParam = CDrvApi::m_LD_RegisterEvent(pEvent);
						if(IsWindow(pEvent->hNotifyWnd))
						{
							PostMessageW(pEvent->hNotifyWnd, pEvent->ulNotifyMsg, wParam, 0xFFF5FFF5u);
						}
						delete pEvent;
						pEvent = NULL;
					}
				}
				else
				{
					if(CDrvApi::m_LD_DeviceControl)
					{
						CDrvApi::m_LD_DeviceControl(item.dwDeviceDUID, item.dwFunction, item.lpBuffer);
					}
					if(item.lpBuffer)
					{
						delete item.lpBuffer;
						item.lpBuffer = NULL;
					}
				}
			}
		}
		v73 = WaitForMultipleObjects(2u, &Handles, 0, 0xFFFFFFFFu);
		if ( v73 < 2 )
		{
			v72 = v73;
			if ( v73 != 1 )
			{
				v72 = -1;
			}
		}
	}
	while(v72 != -1);
	return 0;
}