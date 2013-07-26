
typedef struct
{
    CMainShare *pBase;
    HANDLE hThread;
    DWORD dwThreadId;
    HANDLE hEvent;
    HANDLE hMutex;
    BOOL bRun;
    typedef struct
    {
        typedef void (__cdecl *WorkItemRoutine)(int);
        int Argument;
    } InFlight;
    Fifo WorkItemFifo;
    DWORD __unk; // $$!
} WndList;

HDEVNOTIFY WndRegisterClassChangeNotifications(HANDLE hRecipient, GUID ClassGuid, TCHAR* pName)
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;
    
    memset(&NotificationFilter, 0, sizeof(NotificationFilter));
    
    NotificationFilter.dbcc_size = sizeof(NotificationFilter);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = ClassGuid;
    NotificationFilter.dbcc_name = pName;
    HDEVNOTIFY
        hNotifyDevNode = RegisterDeviceNotificationA(hRecipient, &NotificationFilter, 0);
    return hNotifyDevNode;
}

HDEVNOTIFY WndRegisterHandleChangeNotifications(HANDLE hRecipient, HANDLE hHandle)
{
    DEV_BROADCAST_HANDLE NotificationFilter;

    memset(&NotificationFilter, 0, sizeof(NotificationFilter));
    NotificationFilter.dbch_size = sizeof(NotificationFilter);
    NotificationFilter.dbch_devicetype = DBT_DEVTYP_HANDLE;
    NotificationFilter.dbch_handle = a2;
    HDEVNOTIFY
        hNotifyDevNode = RegisterDeviceNotificationA(hRecipient, &NotificationFilter, 0);
    return hNotifyDevNode;
}

void WndUnregisterDeviceChangeNotifications(HDEVNOTIFY hHandle)
{
    if(hHandle)
    {
        UnregisterDeviceNotification(hHandle);
    }
}

int WndWorkItemThread(WndList *pWList)
{
    while(pWList->bRun)
    {
        if(WaitForSingleObject(pWList->hEvent, 0xFFFFFFFFu) == 0)
        {
            while(pWList->bRun && FifoGet(pWList->pBase, &pWList->WorkItemFifo, &pWList->InFlight.WorkItemRoutine, pWList->hMutex))
            {
                if(pWList->InFlight.WorkItemRoutine)
                {
                    pWList->InFlight.WorkItemRoutine(pWList->InFlight.Argument);
                }
                memset(&pWList->InFlight, 0, sizeof(pWList->InFlight));
            }
        }
    }
    memset(&pWList->InFlight, 0, sizeof(pWList->InFlight));
    return 0;
}

int WndInitWorkItemList(CMainShare *pBase, WndList *pWList)
{
    pWList->pBase = pBase;
    pWList->bRun = 1;
    FifoInit(pBase, &pWList->WorkItemFifo, &pWList->field_38, 512, 8);
    pWList->hEvent = CreateEventA(0, 0, 0, 0);
    pWList->hMutex = CreateMutexA(0, 0, 0);
    pWList->hThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)WndWorkItemThread, pWList, 0, &pWList->dwThreadId);
    return 1;
}

BOOL WndCleanupWorkItemList(WndList *pWList)
{
    BOOL bStatus;

    pWList->bRun = FALSE;
    bStatus = StopNiceThread(pWList->hThread, pWList->hEvent);
    CleanUpHandle(&pWList->hEvent);
    CleanUpHandle(&pWList->hMutex);
    CleanUpHandle(&pWList->hThread);
    return bStatus;
}

BOOL WndQueueWorkItem(WndList *pWList, int pWorkItemRoutine, int pWorkItemRoutineArgument)
{
    if(pWorkItemRoutine)
    {
        if(pWList->bRun == TRUE)
        {
            if(FifoAdd(pWList->pBase, &pWList->WorkItemFifo, (int)&v6, pWList->hMutex) == 1)
            {
                return SetEvent(pWList->hEvent);
            }
        }
        return TRUE;
    }
    return FALSE;
}

int WndFlushWorkItem(WndList *a1, int a2, int a3, HANDLE hHandle)
{
  int result; // eax@14
  char v5; // ST10_1@17
  signed int v6; // eax@37
  char v7; // [sp+0h] [bp-50h]@32
  signed int i; // [sp+38h] [bp-18h]@23
  signed int v9; // [sp+3Ch] [bp-14h]@34
  int src; // [sp+40h] [bp-10h]@25
  int v11; // [sp+44h] [bp-Ch]@27
  _DWORD *v12; // [sp+48h] [bp-8h]@19
  char v13[4]; // [sp+4Ch] [bp-4h]@1

    *(_DWORD *)v13 = 0;
    if ( a1->hMutex )
    {
    if ( CriticalRegionEnter((int)a1->hMutex, 0) )
    {
    v12 = (_DWORD *)FifoGetCount(&a1->WorkItemFifo);
    for ( i = 0; i < (signed int)v12; ++i )
    {
    if ( FifoGet(a1->pBase, &a1->WorkItemFifo, (unsigned int)&src, a1->hMutex) > 0 )
    {
    if ( src == a2 && v11 == a3 )
    ++*(_DWORD *)v13;
    else
    FifoAdd(a1->pBase, &a1->WorkItemFifo, (int)&src, a1->hMutex);
    }
    }
    v9 = 100;
    while ( a1->InFlight_WorkItemRoutine == a2 )
    {
    if ( a1->InFlight_WorkItemRoutineArgument != a3 )
    break;
    v6 = v9--;
    if ( v6 <= 0 )
    break;
    if ( hHandle )
    {
    if ( !WaitForSingleObject(hHandle, 0x64u) )
    {
    break;
    }
    }
    else
    {
    Sleep(0x64u);
    }
    }
    CriticalRegionLeave(a1->hMutex);
    }
    result = *(_DWORD *)v13;
    }
    else
    {
    return FALSE;
    }
    return result;
}