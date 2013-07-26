#include "DrvIntf.h"

unsigned int __cdecl AssertMainProcessStarted()
{
  unsigned int result; // eax@7
  signed int v1; // edx@15
  char v2; // al@27
  char v3; // al@46
  char v4; // [sp+0h] [bp-20h]@0
  DWORD v5; // [sp+8h] [bp-18h]@29
  int v6; // [sp+14h] [bp-Ch]@13
  HANDLE hObject; // [sp+18h] [bp-8h]@25
  unsigned int v8; // [sp+1Ch] [bp-4h]@1

  v8 = 0;
  if ( g_pMainShare && g_pGbl )
  {
    if ( g_pMainShare->lMainProcessState == 4 )
    {
      if ( DebugMsg1("AssertMainProcessStarted") )
        DebugMsg2(134217728, "%s - things are on their way down! n\n", "AssertMainProcessStarted");
      result = 0x8000000Cu;
    }
    else
    {
      if ( !g_pGbl->ulProcessType )
      {
        v6 = 0;
        while ( g_pMainShare->lMainProcessState == 3 )
        {
          v1 = v6++;
          if ( v1 >= 30 )
            break;
          if ( dword_1007254C && DebugMsg1("AssertMainProcessStarted") )
            DebugMsg3("Previous instance of the main process is stopping... %d/30", v6);
          Sleep(0x3E8u);
        }
        if ( g_pMainShare->lMainProcessState == 3 )
        {
          if ( DebugMsg1("AssertMainProcessStarted") )
            CheckAssert("Previous main process instance is hanging.", v4);
          v8 = 0x80000005u;
        }
        else
        {
          if ( g_pMainShare->lMainProcessState != 2 )
          {
            hObject = CreateEventA(0, 1, 0, "LD_KHAL_MainProcStartEvent");
            if ( hObject )
            {
              v5 = WaitForSingleObject(hObject, 0x493E0u);
              if ( v5 )
              {
                if ( v5 == 258 )
                {
                  if ( DebugMsg1("AssertMainProcessStarted") )
                    CheckAssert(
                      "Timeout waiting for main process to start. State is %d",
                      g_pMainShare->lMainProcessState);
                  if ( g_pMainShare->lMainProcessState != 2 )
                  {
                    if ( DebugMsg1("AssertMainProcessStarted") )
                      CheckAssert("Main process started event is SET but State is not 'running'.", v4);
                    v8 = 0x80000005u;
                  }
                }
                else
                {
                  v8 = 0x8000000Eu;
                  if ( DebugMsg1("AssertMainProcessStarted") )
                  {
                    v3 = GetLastError();
                    CheckAssert("Wait Event failed. Reason:%d", v3);
                  }
                }
              }
              else
              {
                if ( dword_1007254C )
                  v4 = DebugMsg1("AssertMainProcessStarted") && DebugMsg3("Main process event is SET", v4);
              }
              if ( g_pMainShare->lMainProcessState != 2 )
              {
                if ( DebugMsg1("AssertMainProcessStarted") )
                  CheckAssert("Main process is not in 'running' state.", v4);
                v8 = 0x80000005u;
              }
              CloseHandle(hObject);
            }
            else
            {
              v8 = 0x80000004u;
              if ( DebugMsg1("AssertMainProcessStarted") )
              {
                v2 = GetLastError();
                CheckAssert("Failed to Create event '%s'. Reason:%d", v2);
              }
            }
          }
        }
      }
      result = v8;
    }
  }
  else
  {
    if ( DebugMsg1("AssertMainProcessStarted") )
      DebugMsg2(134217728, "%s - We shouldn't get here in the first place ! n\n", "AssertMainProcessStarted");
    if ( DebugMsg1("AssertMainProcessStarted") )
      CheckAssert(
        "%s - %s line %d: assert %s\n",
        (unsigned int)"AssertMainProcessStarted",
        ".\\DrvIntf.cpp",
        43,
        "FALSE");
    result = 0x8000000Cu;
  }
  return result;
}

