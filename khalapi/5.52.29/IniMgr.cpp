int CALLBACK IniRepositoryWatcherThread(int)
{
  int result; // eax@8
  HANDLE v2; // eax@24
  DWORD v3; // eax@31
  DWORD v4; // eax@51
  char v5; // al@94
  char v6; // al@99
  char v7; // [sp+0h] [bp-2A0h]@7
  DWORD v8; // [sp+6Ch] [bp-234h]@69
  signed int v9; // [sp+70h] [bp-230h]@68
  unsigned int v10; // [sp+74h] [bp-22Ch]@15
  char Dst[268]; // [sp+78h] [bp-228h]@15
  HANDLE hChangeHandle; // [sp+184h] [bp-11Ch]@10
  HANDLE hHandle; // [sp+188h] [bp-118h]@9
  char v14[4]; // [sp+18Ch] [bp-114h]@33
  const CHAR Src; // [sp+190h] [bp-110h]@5
  unsigned int v16; // [sp+298h] [bp-8h]@1
  DWORD v17; // [sp+29Ch] [bp-4h]@33
  int v18; // [sp+2A0h] [bp+0h]@1

    if ( WndGetDevicesFolder((LPSTR)&Src, 0x104u, 0, 0) )
    {
        hHandle = g_pMain->hStopMainProcessThreadEvent;
        while(TRUE)
        {
            hChangeHandle = FindFirstChangeNotificationA(&Src, 1, 1u);
            if ( hChangeHandle == INVALID_HANDLE_VALUE )
            {
                strcpy_s(Dst, MAX_PATH, &Src);
                BYTE *pEnd = &Dst[strlen(Dst)];
                do
                {
                    while(*pEnd != '\\' && pEnd >= Dst)
                    {
                        --pEnd;
                    }
                    if(*pEnd != '\\')
                    {
                        return 0;
                    }
                    *pEnd = NULL;
                    hChangeHandle = FindFirstChangeNotificationA(Dst, 0, 2u);
                }
                while(hChangeHandle == INVALID_HANDLE_VALUE);
                if(hChangeHandle == INVALID_HANDLE_VALUE)
                {
                    return 0;
                }
                v17 = WaitForMultipleObjects(2u, &hChangeHandle, 0, 0xFFFFFFFFu);
                FindCloseChangeNotification(hChangeHandle);
                if ( !v17 )
                {
                    continue;
                }
                if ( v17 != 1 )
                {
                    continue;
                }
                result = 0;
            }
            else
            {
                if(hChangeHandle == INVALID_HANDLE_VALUE)
                {
                    result = 0;
                }
                else
                {
                    while ( 1 )
                    {
                        v17 = WaitForMultipleObjects(2u, &hChangeHandle, 0, 0xFFFFFFFFu);
                        if ( v17 )
                            break;
                        if ( !WaitForSingleObject(hHandle, 0x3E8u) )
                        {
                            break;
                        }
                        v9 = 1;
                        if(g_pMain->dwOndemandReloadStartTime)
                        {
                            OnDemandReloadElapsedTime = GetTickCount() - g_pMain->dwOndemandReloadStartTime;
                            if(OnDemandReloadElapsedTime < 5000)
                            {
                                v9 = 0;
                            }
                        }
                        if(v9)
                        {
                            MainAPIBroadcastControl(5, 0, 0, 0, 1);
                            if(IniLoadFilesFromPath((int *)&g_pMain->ModelTable, &g_pMain->ulModelCount, (void **)&g_pMain->field_1D1C, (int)&g_pMain->field_1D18, &Src, g_pGbl->dwFlags, 0))
                            {
                                if(!WaitForSingleObject(hHandle, 1000) )
                                    break;
                            }
                            MainAPIBroadcastControl(6, 0, 0, 0, 1);
                        }
                        if(!FindNextChangeNotification(hChangeHandle))
                        {
                            break;
                        }
                    }
                    FindCloseChangeNotification(hChangeHandle);
                    result = 0;
                }
            }
            break;
        }
    }
    else
    {
        result = 0;
    }
    return result;
}

void StartIniRepositoryWatcher()
{
    if(g_pMain->hIniRepositoryWatcherThread == NULL)
    {
        DWORD ThreadId;
        g_pMain->hIniRepositoryWatcherThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)IniRepositoryWatcherThread, 0, 0, &ThreadId);
    }
}

BOOL StopIniRepositoryWatcher()
{
    StopNiceThread(g_pMain->hIniRepositoryWatcherThread, g_pMain->hStopMainProcessThreadEvent);
    return CleanUpHandle(&g_pMain->hIniRepositoryWatcherThread);
}

BOOL ReloadIniFiles()
{
    TCHAR Src[MAX_PATH];
    BOOL bResult = FALSE;
    
    if(WndGetDevicesFolder(&Src, MAX_PATH, 0, 0))
    {
        DWORD dwReloadStartTime = GetTickCount();
        g_pMain->dwOndemandReloadStartTime = dwReloadStartTime ? dwReloadStartTime : -1;
        MainAPIBroadcastControl(5, 0, 0, 0, 1);
        if(IniLoadFilesFromPath((int *)&g_pMain->ModelTable, &g_pMain->ulModelCount, (void **)&g_pMain->field_1D1C, (int)&g_pMain->field_1D18, &Src, g_pGbl->dwFlags, 0))
        {
            bResult = TRUE;
        }
        MainAPIBroadcastControl(6, 0, 0, 0, 1);
    }
    return bResult;
}

char* IniExtractValue(const char *key, char *Dst, size_t SizeInBytes, char *haystack)
{
    if(haystack)
    {
        while(haystack)
        {
            // find key
            haystack = strstr(haystack, key);
            if(haystack)
            {   
                // skip key
                haystack += strlen(key);
                // skip spaces before '=' sign
                for(; *haystack != '=' && *haystack && (*haystack == ' ' || *haystack == '\t'); haystack++)
                    ;
                if(*haystack == '=')
                {
                    // skip spaces after '=' sign
                    for(haystack += 1; *haystack && (*haytack == ' ' || *haystack == '\t'); haystack++)
                    ;
                    // find end of line
                    for(char *pEnd = haystack; *pEnd && j[k] != '\n' && j[k] != '\r'; pEnd++)
                    ;
                    size_t realSize = pEnd - haystack;
                    if(realSize < SizeInBytes)
                    {
                        strncpy_s(Dst, SizeInBytes, haystack, realSize);
                        Dst[k] = 0;
                        return pEnd;
                    }
                }
            }
        }
    }
    return NULL;
}

int __cdecl IniParseIntValue(char *haystack, const char *Name)
{
  char v2; // ST14_1@5
  int result; // eax@6
  char v4; // ST14_1@9
  bool v5; // [sp+0h] [bp-20h]@18
  int v6; // [sp+Ch] [bp-14h]@32
  signed int v7; // [sp+10h] [bp-10h]@27
  int Flag; // [sp+14h] [bp-Ch]@11
  bool v9; // [sp+18h] [bp-8h]@20
  char *haystack; // [sp+1Ch] [bp-4h]@11
  _BYTE *v11; // [sp+1Ch] [bp-4h]@27

    if(haystack)
    {
        if(Name)
        {
            while(haystack)
            {
                haystack = strstr(haystack, Name);
                if(haystack)
                {
                    haystack += strlen(Name);
                    if(!(*haystack == '#' || *haystack == '\\' || !*haystack || *haystack == ' ' || *haystack == '\t'))
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
            }
            if(haystack)
            {
                if(*haystack == '#')
                {
                    int value = 0;
                    size_t base = 10;
                    haystack += 1;
                    // hex
                    if(haystack[0] == '0' && haystack[1] == 'x')
                    {
                        haystack += 2;
                        base = 16;
                    }
                    for(; *haystack; haystack++)
                    {
                        int chval = 0;
                        if(*haystack >= '0' && *haystack <= '9')
                        {
                            chval = *haystack - '0';
                        }
                        else if(*haystack >= 'A' && *haystack <= 'F')
                        {
                            chval = *haystack - '7';
                        }
                        else if(*haystack >= 'a' && *haystack <= 'f')
                        {
                            chval = *haystack - 'W';
                        }
                        else
                        {
                            return value;
                        }
                        value *= base;
                        value += chval;
                    }
                }
                return 1;
            }
        }
    }
    return 0;
}

signed int __cdecl IniLoadUnsupportedSection(void **pUnsupportedList, _DWORD *ulUnsupportedCount, char *a3)
{
  int v3; // ecx@3
  signed int result; // eax@8
  int v5; // [sp+0h] [bp-2Ch]@15
  char *v6; // [sp+4h] [bp-28h]@15
  size_t v7; // [sp+8h] [bp-24h]@15
  int v8; // [sp+Ch] [bp-20h]@15
  char *v9; // [sp+10h] [bp-1Ch]@2
  char v10[4]; // [sp+14h] [bp-18h]@1
  unsigned int v11; // [sp+24h] [bp-8h]@1
  size_t v12; // [sp+28h] [bp-4h]@1
  int v13; // [sp+2Ch] [bp+0h]@1

    char v10[16];
    strcpy(v10, "[Unsupported]");
    if(_strnicmp(a3, v10, strlen(v10)) == 0 )
    {
        v9 = a3;
        do
            v3 = *v9++;
        while ( v3 != '\n' );
        if ( ulUnsupportedCount )
        {
        if ( pUnsupportedList )
        {
        if ( *pUnsupportedList )
        {
        result = 0;
        }
        else
        {
        v5 = 0;
        v6 = v9;
        v7 = 0;
        *ulUnsupportedCount = 0;
        v8 = v7;
        do
        v9 = (char *)ParseUnsupportedListKey(&v5, v9, &v7, &v8, 0);
        while ( !v5 );
        *pUnsupportedList = HeapAlloc(g_hHeap, 8u, 4 * v7);
        if ( *pUnsupportedList )
        {
        v9 = v6;
        v5 = 0;
        v8 = 0;
        do
        v9 = (char *)ParseUnsupportedListKey(&v5, v9, &v7, &v8, pUnsupportedList);
        while ( !v5 );
        }
        qsort(*pUnsupportedList, v7, 4u, (int (__cdecl *)(const void *, const void *))CompareUnknownDevice);
        *ulUnsupportedCount = v7;
        result = 1;
        }
        }
        else
        {
        result = 1;
        }
        }
        else
        {
        result = 0;
        }
    }
    return result;
}

int __cdecl IniLoadFile(int *ModelTable, int *pulModelTableCount, void **a3, int a4, LPCSTR lpFileName, __int16 a6, int a7)
{
  char v7; // ST1C_1@9
  char v9; // ST1C_1@13
  DWORD v10; // eax@17
  DWORD v11; // eax@41
  signed int v12; // eax@44
  int v13; // eax@44
  BlowfishBlock *v14; // eax@50
  int v15; // edx@50
  BlowfishBlock *v16; // eax@59
  int v17; // edx@59
  char v18; // [sp+20h] [bp-31C8h]@59
  int ctx; // [sp+28h] [bp-31C0h]@50
  char v20; // [sp+30h] [bp-31B8h]@44
  BlowfishPad v21; // [sp+1078h] [bp-2170h]@44
  SIZE_T nNumberOfBytesToWrite; // [sp+20C0h] [bp-1128h]@57
  DWORD NumberOfBytesWritten; // [sp+20C4h] [bp-1124h]@57
  HANDLE hFile; // [sp+20C8h] [bp-1120h]@56
  LPCVOID lpBuffer; // [sp+20CCh] [bp-111Ch]@56
  int L; // [sp+20D0h] [bp-1118h]@50
  int R; // [sp+20D4h] [bp-1114h]@50
  void *v28; // [sp+20D8h] [bp-1110h]@54
  unsigned int byteSize; // [sp+20DCh] [bp-110Ch]@45
  int dest; // [sp+20E0h] [bp-1108h]@45
  unsigned int v31; // [sp+20E4h] [bp-1104h]@45
  BlowfishBlock chain; // [sp+20E8h] [bp-1100h]@44
  BlowfishPad pad; // [sp+20F0h] [bp-10F8h]@44
  int cnt; // [sp+3138h] [bp-B0h]@60
  int i; // [sp+313Ch] [bp-ACh]@60
  char key; // [sp+3140h] [bp-A8h]@44
  __int16 v37; // [sp+316Ch] [bp-7Ch]@44
  unsigned int v38; // [sp+3170h] [bp-78h]@1
  unsigned int j; // [sp+3174h] [bp-74h]@85
  DWORD Buffer; // [sp+3178h] [bp-70h]@34
  _BYTE *lpMem; // [sp+317Ch] [bp-6Ch]@1
  ModelTable *v42; // [sp+3180h] [bp-68h]@1
  SIZE_T nNumberOfBytesToRead; // [sp+3184h] [bp-64h]@31
  int ulCount; // [sp+3188h] [bp-60h]@1
  HANDLE hObject; // [sp+318Ch] [bp-5Ch]@30
  ModelTable v46; // [sp+3190h] [bp-58h]@82
  int v47; // [sp+31DCh] [bp-Ch]@85
  char v48[4]; // [sp+31E0h] [bp-8h]@1
  int v49; // [sp+31E4h] [bp-4h]@1
  int v50; // [sp+31E8h] [bp+0h]@1

  v38 = (unsigned int)&v50 ^ dword_10072570;
  lpMem = 0;
  v49 = 0;
  v42 = 0;
  ulCount = 0;
  *(_DWORD *)v48 = 0;
  if ( g_LogEvents )
  {
    if ( DebugMsg1("IniLoadFile") )
      DebugMsg3("Entry - '%s'.", (char)lpFileName);
  }
  if ( !lpFileName || !ModelTable || !pulModelTableCount )
  {
    if ( DebugMsg1("IniLoadFile") )
      CheckAssert("NULL Pointer.", v7);
    return 0;
  }
  if ( !*lpFileName )
  {
    if ( DebugMsg1("IniLoadFile") )
      CheckAssert("EMPTY String.", v9);
    return 0;
  }
  v42 = (ModelTable *)HeapAlloc(g_hHeap, 8u, 0x11000u);
  if ( !v42 )
  {
    if ( DebugMsg1("IniLoadFile") )
    {
      v10 = GetLastError();
      CheckAssert("Failed to allocated %d bytes. Reason:%d.", 0, v10);
    }
    return 0;
  }
  if ( *ModelTable && (unsigned int)*pulModelTableCount < 0x400 )
  {
    ulCount = *pulModelTableCount;
    memcpy_fast((unsigned int)v42, *ModelTable, 0x44 * ulCount);
    HeapFree(g_hHeap, 0, (LPVOID)*ModelTable);
    if ( !ulCount )
    {
      if ( DebugMsg1("IniLoadFile") )
        CheckAssert("%s - %s line %d: assert %s\n", (unsigned int)"IniLoadFile", ".\\IniMgr.cpp", 1598, "ulCount > 0");
    }
    if ( DebugMsg1("IniLoadFile") )
      DebugMsg2(131072, "IniLoadFilesFromPath - Found %d device from previous file(s).\n", ulCount);
  }
  else
  {
    ulCount = 0;
    if ( *pulModelTableCount )
    {
      if ( DebugMsg1("IniLoadFile") )
        CheckAssert(
          "%s - %s line %d: assert %s\n",
          (unsigned int)"IniLoadFile",
          ".\\IniMgr.cpp",
          1604,
          "*pulModelTableCount == 0");
    }
  }
  hObject = CreateFileA(lpFileName, 0x80000000u, 0, 0, 3u, 0x80u, 0);
  if ( hObject != (HANDLE)-1 )
  {
    nNumberOfBytesToRead = GetFileSize(hObject, 0);
    if ( nNumberOfBytesToRead != -1 )
      lpMem = HeapAlloc(g_hHeap, 8u, nNumberOfBytesToRead);
    if ( lpMem )
    {
      if ( ReadFile(hObject, lpMem, nNumberOfBytesToRead, &Buffer, 0) == 1 )
      {
        if ( DebugMsg1("IniLoadFile") )
          DebugMsg2(131072, "IniLoadFilesFromPath - Read %d bytes from file.\n", Buffer);
      }
      else
      {
        if ( DebugMsg1("IniLoadFile") )
          CheckAssert("%s - %s line %d: assert %s\n", (unsigned int)"IniLoadFile", ".\\IniMgr.cpp", 1632, "FALSE");
        if ( DebugMsg1("IniLoadFile") )
        {
          v11 = GetLastError();
          DebugMsg2(131072, "IniLoadFilesFromPath - ReadFile Failed %d.\n", v11);
        }
        HeapFree(g_hHeap, 0, lpMem);
        lpMem = 0;
      }
    }
    CloseHandle(hObject);
    if ( lpMem )
    {
      memcpy(&key, "SetPoint-83918EEE-DB31-4df1-B11D-F93932DBC1B8", 44u);
      v37 = *(_WORD *)&aSetpoint83918e[44];
      v12 = strlen(&key);
      v13 = BlowfishGeneratePad((int)&v20, (int)&key, v12);
      memcpy_fast((unsigned int)&v21, v13, 0x1048u);
      memcpy_fast((unsigned int)&pad, (int)&v21, 0x1048u);
      BlowfishBlock__ctor(&chain, 0xC009F158u, 0x17EC17ECu);
      if ( strncmp(lpMem, ";KHAL", 5u) )
      {
        dest = 0;
        v31 = *(_DWORD *)lpMem;
        byteSize = GetBlockSizeOfBuffer(v31, 8u);
        if ( Buffer != byteSize + 12 || v31 > byteSize )
        {
          HeapFree(g_hHeap, 0, lpMem);
          goto LABEL_101;
        }
        dest = (int)HeapAlloc(g_hHeap, 8u, byteSize + 1);
        if ( !dest )
        {
          HeapFree(g_hHeap, 0, lpMem);
          goto LABEL_101;
        }
        BlowfishDecryptCBC(&pad, (int)(lpMem + 4), dest, byteSize, &chain);
        v14 = BlowfishEncipherBlock((int)&ctx, &pad, &chain);
        v15 = v14->R;
        chain.L = v14->L;
        chain.R = v15;
        L = *(_DWORD *)&lpMem[byteSize + 4];
        R = *(_DWORD *)&lpMem[byteSize + 8];
        if ( chain.L != L || chain.R != R )
        {
          if ( DebugMsg1("IniLoadFile") )
            CheckAssert(
              "%s - %s line %d: assert %s\n",
              (unsigned int)"IniLoadFile",
              ".\\IniMgr.cpp",
              1681,
              "chain.L == L && chain.R == R");
        }
        v28 = lpMem;
        lpMem = (_BYTE *)dest;
        HeapFree(g_hHeap, 0, v28);
        Buffer = v31;
      }
      else
      {
        if ( a6 & 0x400 )
        {
          lpBuffer = 0;
          hFile = CreateFileA(lpFileName, 0x40000000u, 0, 0, 2u, 0x80u, 0);
          if ( hFile != (HANDLE)-1 )
          {
            nNumberOfBytesToWrite = GetBlockSizeOfBuffer(Buffer, 8u);
            lpBuffer = HeapAlloc(g_hHeap, 8u, nNumberOfBytesToWrite);
            WriteFile(hFile, &Buffer, 4u, &NumberOfBytesWritten, 0);
            if ( lpBuffer )
            {
              BlowfishDecryptCFB(&pad, (int)lpMem, (int)lpBuffer, nNumberOfBytesToWrite, &chain);
              WriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, &NumberOfBytesWritten, 0);
              HeapFree(g_hHeap, 0, (LPVOID)lpBuffer);
            }
            v16 = BlowfishEncipherBlock((int)&v18, &pad, &chain);
            v17 = v16->R;
            chain.L = v16->L;
            chain.R = v17;
            WriteFile(hFile, &chain, 4u, &NumberOfBytesWritten, 0);
            WriteFile(hFile, &chain.R, 4u, &NumberOfBytesWritten, 0);
            CloseHandle(hFile);
          }
        }
      }
      cnt = 0;
      i = -1;
      while ( cnt < (signed int)Buffer )
      {
        if ( lpMem[cnt] == ';' )
        {
          for ( i = cnt + 1; i < (signed int)Buffer && lpMem[i] != '\r' && lpMem[i] != '\n'; ++i )
            ;
          while ( i < (signed int)Buffer && (lpMem[i] == '\r' || lpMem[i] == '\n') )
            ++i;
          if ( i < (signed int)Buffer )
            memcpy_fast((unsigned int)&lpMem[cnt], (int)&lpMem[i], Buffer - i);
          Buffer -= i - cnt;
        }
        else
        {
          ++cnt;
        }
      }
      cnt = -1;
      for ( i = 0; i < (signed int)Buffer; ++i )
      {
        if ( lpMem[i] == '[' || i == Buffer - 1 )
        {
          if ( cnt >= 0 )
          {
            lpMem[i - 1] = 0;
            memset(&v46, 0, 0x44u);
            if ( IniLoadUnsupportedSection(a3, (_DWORD *)a4, &lpMem[cnt]) != 1 )
            {
              if ( IniLoadSection(&v46, (int)&lpMem[cnt]) == 1 )
              {
                v47 = 1;
                for ( j = 0; j < ulCount; ++j )
                {
                  if ( v46.field_0 == v42[j].field_0 && v46.field_4 == v42[j].field_4 )
                  {
                    IniFreeModelInfo((unsigned int)&v46, (int)v42, &v46);
                    if ( a7 )
                    {
                      if ( dword_10072548 )
                      {
                        if ( DebugMsg1("IniLoadFile") )
                          sub_1003DBE0("Model Overwrite (%08X-%X).\n", *(_QWORD *)&v46);
                      }
                    }
                    v47 = 0;
                    break;
                  }
                }
                if ( v47 )
                {
                  memcpy(&v42[ulCount++], &v46, sizeof(v42[ulCount++]));
                  ++*(_DWORD *)v48;
                }
              }
            }
          }
          cnt = i;
        }
      }
      HeapFree(g_hHeap, 0, lpMem);
      lpMem = 0;
      goto LABEL_101;
    }
  }
LABEL_101:
  if ( ulCount )
  {
    *ModelTable = (int)HeapAlloc(g_hHeap, 8u, 68 * ulCount);
    if ( *ModelTable )
    {
      memcpy_fast(*ModelTable, (int)v42, 68 * ulCount);
      *pulModelTableCount = ulCount;
    }
    else
    {
      if ( DebugMsg1("IniLoadFile") )
        CheckAssert("%s - %s line %d: assert %s\n", (unsigned int)"IniLoadFile", ".\\IniMgr.cpp", 1840, "FALSE");
      *pulModelTableCount = 0;
    }
  }
  else
  {
    *pulModelTableCount = 0;
    *ModelTable = 0;
  }
  HeapFree(g_hHeap, 0, v42);
  v42 = 0;
  if ( g_LogEvents )
  {
    if ( DebugMsg1("IniLoadFile") )
      DebugMsg3("Exit %d.", v48[0]);
  }
  return *(_DWORD *)v48;
}