unsigned int __cdecl DeviceGetLegacyPowerStatus(LD_DEVICE *pDevice, LD_GET_POWER_STATUS_INFO *pPowerStatusInfo)
{
  unsigned int result; // eax@6
  char v3; // ST18_1@11
  char v4; // ST18_1@16
  int v5; // ST14_4@51
  char *v6; // eax@51
  LD_DEVICE *v7; // [sp+14h] [bp-30h]@46
  unsigned int v8; // [sp+18h] [bp-2Ch]@36
  signed int v9; // [sp+1Ch] [bp-28h]@36
  int v10; // [sp+20h] [bp-24h]@36
  int v11; // [sp+24h] [bp-20h]@18
  int v12; // [sp+28h] [bp-1Ch]@1
  int fn; // [sp+2Ch] [bp-18h]@23
  int v14; // [sp+30h] [bp-14h]@24
  int v15; // [sp+34h] [bp-10h]@24
  int v16; // [sp+38h] [bp-Ch]@22
  int Table; // [sp+3Ch] [bp-8h]@18
  int v18; // [sp+40h] [bp-4h]@22

    v12 = 0x80000015u;
    if(pPowerStatusInfo->dwSize >= 0x18u)
    {
        pPowerStatusInfo->PowerState = 0;
        pPowerStatusInfo->ulBatteryRange = 0;
        pPowerStatusInfo->ulBatteryLevel = 0;
        pPowerStatusInfo->ulRawBatteryValue = 0;
        if ( pPowerStatusInfo->dwSize == 0x1C )
        {
            pPowerStatusInfo->pad1 = 0xAu;
            pPowerStatusInfo->pad2 = 0;
        }
        else
        {
            pPowerStatusInfo->pad1 = 0;
        }
        Table = DeviceGetCapability(pDevice, 0, 0);
        v11 = pDevice->ParentDevice;
        while ( (unsigned int)Table > 1 && Table != -1 )
        {
            if ( pDevice->dwDataValidFlags & 2 )
            {
                v18 = (unsigned __int8)pDevice->ulBatteryStatus;
                v16 = (unsigned __int8)(LOWORD(pDevice->ulBatteryStatus) >> 8);
                v12 = 0;
            }
            else
            {
                fn = 12;
                v12 = LD_DeviceControl(pDevice->DUID, 4, (int)&fn);
                if ( !v12 )
                {
                    v18 = v15;
                    v16 = v14;
                }
            }
            if ( v12 )
            {
                Table = 0;
            }
            else
            {
                pPowerStatusInfo->ulRawBatteryValue = v18;
                pPowerStatusInfo->ulBatteryLevel = -1;
                pPowerStatusInfo->ulBatteryRange = -1;
                if ( (unsigned int)v18 < 8 && v18 || v18 == 129 && Table & 0xFF00 )
                {
                    if ( v18 == 129 )
                    {
                        pPowerStatusInfo->PowerState = 2;
                        v18 = v16;
                        Table = (Table & 0xFF00u) >> 8;
                    }
                    else
                    {
                        pPowerStatusInfo->PowerState = 1;
                        Table = (unsigned __int8)Table;
                    }
                    Table |= 1u;
                    Mask = 1;
                    Raw = 0;
                    Norm = -1;
                    while(Table && Raw < 8)
                    {
                        if ( Table & Mask )
                        {
                            Table &= ~Mask;
                            ++Norm;
                            if(v18 == Raw)
                            {
                                pPowerStatusInfo->ulBatteryLevel = Norm;
                            }
                        }
                        Mask *= 2;
                        ++Raw;
                    }
                    if ( pPowerStatusInfo->ulBatteryLevel == -1 )
                    {
                        pPowerStatusInfo->PowerState = 0;
                        v7 = DeviceGetAndLock(0, 6, v11);
                        if(v7)
                        {
                            v11 = v7->ParentDevice;
                            Table = DeviceGetCapability(v7, 0, 0);
                            ReleaseRemoveLock(0, 6, v7);
                        }
                    }
                    else
                    {
                        pPowerStatusInfo->ulBatteryRange = Norm;
                        Table = 0;
                    }
                }
                else
                {
                    if(Table & 0xFF00)
                    {
                        switch(v18)
                        {
                            case 129:
                            case 132:
                                pPowerStatusInfo->PowerState = 2;
                            break;
                            case 130:
                            case 134:
                                pPowerStatusInfo->PowerState = 3;
                            break;
                            case 131:
                                pPowerStatusInfo->PowerState = 4;
                            break;
                            case 133:
                                pPowerStatusInfo->PowerState = 6;
                            break;
                            default:
                                pPowerStatusInfo->PowerState = 0;
                            break;
                        }
                    }
                    else
                    {
                        pPowerStatusInfo->PowerState = 0;
                    }
                    Table = 0;
                }
            }
        }
        return v12;
    }
    return 0x80000001u;
}

unsigned int __cdecl ExtrapolateMilageBetweenSamples(LD_DEVICE *pDevice, unsigned int threshold)
{
  char v2; // ST18_1@7
  unsigned int result; // eax@8
  int v4; // ecx@9
  char v5; // [sp+0h] [bp-90h]@31
  unsigned __int64 v6; // [sp+60h] [bp-30h]@9
  unsigned int AverageTimeBetweenSamples; // [sp+6Ch] [bp-24h]@19
  struct _SYSTEMTIME SystemTime; // [sp+70h] [bp-20h]@9
  struct _FILETIME FileTime; // [sp+80h] [bp-10h]@9
  __int64 TimeSinceLastSampleChange; // [sp+88h] [bp-8h]@23

    if(pDevice && threshold && pDevice->LastChangeTime)
    {
        GetSystemTime(&SystemTime);
        SystemTimeToFileTime(&SystemTime, &FileTime);
        CurrentTime = (unsigned int)FileTime.dwLowDateTime + ((unsigned __int64)FileTime.dwHighDateTime << 32);
        v4 = HIDWORD(pDevice->LastChangeTime);
        if ( v4 < HIDWORD(CurrentTime) || v4 <= HIDWORD(CurrentTime) && LODWORD(pDevice->LastChangeTime) <= (_DWORD)CurrentTime )
        {
            if ( pDevice->CurrentDischargeRate < 0.0 && pDevice->ulBatteryRange )
            {
                AverageTimeBetweenSamples = (signed __int64)(-100.0 / ((double)pDevice->ulBatteryRange * pDevice->CurrentDischargeRate));
            }
            else
            {
                AverageTimeBetweenSamples = -1;
            }
            TimeSinceLastSampleChange = (signed __int64)(CurrentTime - pDevice->LastChangeTime) / 10000000;
            if ( TimeSinceLastSampleChange > AverageTimeBetweenSamples )
            {
                TimeSinceLastSampleChange = (TimeSinceLastSampleChange - AverageTimeBetweenSamples) / 2 + AverageTimeBetweenSamples;
            }
            if(TimeSinceLastSampleChange <= threshold)
            {
                threshold -= TimeSinceLastSampleChange;
            }
            else
            {
                threshold = 0;
            }
        }
    }
    return threshold;
}

double __cdecl LeastSquareMethod(int a1, signed int sum_x)
{
  __int64 v2; // qax@2
  double v3; // ST18_8@4
  signed int v4; // ST2C_4@4
  double v6; // [sp+28h] [bp-40h]@2
  double v7; // [sp+30h] [bp-38h]@1
  signed int i; // [sp+3Ch] [bp-2Ch]@2
  double v9; // [sp+40h] [bp-28h]@1
  double v10; // [sp+50h] [bp-18h]@1
  double v11; // [sp+58h] [bp-10h]@1
  double v12; // [sp+60h] [bp-8h]@1

  v10 = 0.0;
  v7 = 0.0;
  v12 = 0.0;
  v9 = 0.0;
  v11 = 0.0;
  if ( sum_x > 1 )
  {
    v2 = -(*(_DWORD *)(a1 + 8 * sum_x - 8) + *(_DWORD *)a1);
    v6 = 2.0 / (double)(*(_DWORD *)(a1 + 8 * sum_x - 8) - *(_DWORD *)a1);
    for ( i = 0; i < sum_x; ++i )
    {
      v3 = (double)((((_DWORD)v2 - HIDWORD(v2)) >> 1) + *(_DWORD *)(a1 + 8 * i)) * v6;
      v4 = *(_DWORD *)(a1 + 8 * i + 4);
      v7 = v7 + v3;
      v12 = (double)v4 + v12;
      v9 = v3 * v3 + v9;
      v11 = (double)v4 * v3 + v11;
    }
    v10 = ((double)sum_x * v11 - v7 * v12) / ((double)sum_x * v9 - v7 * v7) * v6;
  }
  if ( DebugMsg1("LeastSquareMethod") )
    DebugMsg2(4, "LeastSquareMethod - Exit - Slope:%.8f.\n", *(_QWORD *)&v10);
  return v10;
}

int __cdecl PowerGetMileage(LD_DEVICE *pDevice, signed int Charge)
{
  char *v2; // eax@2
  int result; // eax@6
  char v4; // ST18_1@52
  const CHAR Dst; // [sp+50h] [bp-240h]@44
  const CHAR Src; // [sp+158h] [bp-138h]@43
  unsigned int v7; // [sp+264h] [bp-2Ch]@1
  int v8; // [sp+268h] [bp-28h]@1
  int v9; // [sp+26Ch] [bp-24h]@24
  int v10; // [sp+270h] [bp-20h]@12
  struct _FILETIME LastWriteTime; // [sp+274h] [bp-1Ch]@11
  HANDLE hFile; // [sp+27Ch] [bp-14h]@7
  DWORD NumberOfBytesRead; // [sp+280h] [bp-10h]@21
  unsigned int v14; // [sp+284h] [bp-Ch]@1
  LPVOID lpMem; // [sp+288h] [bp-8h]@32
  int v16; // [sp+28Ch] [bp-4h]@26
  int v17; // [sp+290h] [bp+0h]@1

  v7 = (unsigned int)&v17 ^ dword_10072570;
  v8 = 0;
  v14 = 0;
  if ( DebugMsg1("PowerGetMileage") )
  {
    v2 = GetDeviceModelName(pDevice);
    DebugMsg2(4, "PowerGetMileage - Entry - Charge:%d - DUID:%08X '%s'.\n", Charge, pDevice->DUID, v2);
  }
  if ( DeviceGetCapability(pDevice, 0, 0) & 0x80000000 )
  {
    v14 = 0;
    hFile = (HANDLE)WndOpenBatteryLogFile((int)pDevice, 0, 0);
    if ( hFile == (HANDLE)-1 )
    {
      if ( DebugMsg1("PowerGetMileage") )
        DebugMsg2(4, "PowerGetMileage - LogFile not Open.\n");
    }
    else
    {
      if ( GetFileTime(hFile, 0, 0, &LastWriteTime) )
      {
        if ( CompareFileTime(&LastWriteTime, &pDevice->LogFileLastModifiedTime) )
        {
          pDevice->LogFileLastModifiedTime.dwLowDateTime = LastWriteTime.dwLowDateTime;
          pDevice->LogFileLastModifiedTime.dwHighDateTime = LastWriteTime.dwHighDateTime;
          NumberOfBytesRead = GetFileSize(hFile, 0);
          if ( NumberOfBytesRead <= 0x20000 )
          {
            v9 = 0;
          }
          else
          {
            if ( DebugMsg1("PowerGetMileage") )
              DebugMsg2(
                67108864,
                "PowerGetMileage - File is too Large - %d Bytes are ignored.\n",
                NumberOfBytesRead - 131072);
            NumberOfBytesRead = 131072;
            SetFilePointer(hFile, -131072, 0, 2u);
            v9 = 1;
          }
          v16 = (int)HeapAlloc(g_hHeap, 8u, 0x9C40u);
          if ( v16 )
          {
            v14 = PowerLoadLogFile(hFile, NumberOfBytesRead, v16, 0x1388u, v9, (int)&pDevice->LastChangeTime);
            if ( v14 <= 1 || v14 == -1 )
            {
              pDevice->CurrentDischargeRate = 0.0;
            }
            else
            {
              lpMem = HeapAlloc(g_hHeap, 8u, 8 * v14);
              if ( lpMem )
              {
                v14 = GetPowerMileageDataForLSM(v16, v14, lpMem, pDevice->ulNominalBatteryMileage);
                pDevice->CurrentDischargeRate = LeastSquareMethod(lpMem, v14);
                HeapFree(g_hHeap, 0, lpMem);
                lpMem = 0;
              }
              else
              {
                if ( DebugMsg1("PowerGetMileage") )
                  DebugMsg2(134217728, "PowerGetMileage - Failed to allocate %d bytes for storing samples.\n", 8 * v14);
              }
            }
            HeapFree(g_hHeap, 0, (LPVOID)v16);
            v16 = 0;
          }
          else
          {
            if ( DebugMsg1("PowerGetMileage") )
              DebugMsg2(134217728, "PowerGetMileage - Failed to allocate %d bytes for storing samples.\n", 40000);
          }
        }
        else
        {
          if ( DebugMsg1("PowerGetMileage") )
            DebugMsg2(
              4,
              "PowerGetMileage - LogFile NOT modified. Discharge Rate is:%.8f.\n",
              pDevice->CurrentDischargeRate);
        }
      }
      else
      {
        v10 = GetLastError();
        if ( DebugMsg1("PowerGetMileage") )
          CheckAssert("%s - %s line %d: assert %s\n", (unsigned int)"PowerGetMileage", ".\\PowerMgr.cpp", 915, "FALSE");
        if ( DebugMsg1("PowerGetMileage") )
          DebugMsg2(134217728, "PowerGetMileage - GetFileTime failed - hLogFile:%0x08X. dwLastError:%d.\n", hFile, v10);
      }
      CloseHandle(hFile);
      if ( v14 == -1 )
      {
        if ( DebugMsg1("PowerGetMileage") )
          DebugMsg2(4, "PowerGetMileage - Log File is corrupted - Saving a copy.\n");
        if ( WndGetBatteryLogFileName((int)pDevice, (LPSTR)&Src, 0x104u) )
        {
          strcpy_s((char *)&Dst, 0x104u, &Src);
          strcat_s((char *)&Dst, 0x104u, ".bad");
          if ( !MoveFileExA(&Src, &Dst, 1u) )
          {
            if ( DebugMsg1("PowerGetMileage") )
              DebugMsg2(4, "PowerGetMileage - Backup failed. Deleting File.\n");
            DeleteFileA(&Src);
          }
        }
      }
    }
    if ( !Charge )
    {
      if ( g_LogEvents )
      {
        if ( DebugMsg1("PowerGetMileage") )
          DebugMsg3("Current battery charge modified from 0%% to 1%%", v4);
      }
      Charge = 1;
    }
    if ( pDevice->CurrentDischargeRate >= -0.00000004656612875245797 )
    {
      if ( g_LogEvents )
      {
        if ( DebugMsg1("PowerGetMileage") )
          DebugMsg3(
            "Dischage rate result is abnormal. Using nominal mileage (%d days) instead.",
            pDevice->ulNominalBatteryMileage / 0x15180u);
      }
      v8 = (signed __int64)((double)(unsigned int)Charge * (double)pDevice->ulNominalBatteryMileage / 100.0);
    }
    else
    {
      v8 = (signed __int64)((double)(unsigned int)Charge / -pDevice->CurrentDischargeRate);
      v8 = ExtrapolateMilageBetweenSamples(pDevice, v8);
    }
    if ( DebugMsg1("PowerGetMileage") )
      DebugMsg2(4, "PowerGetMileage - Exit - Mileage:%d Sec.\n", v8);
    result = v8;
  }
  else
  {
    if ( DebugMsg1("PowerGetMileage") )
      DebugMsg2(4, "PowerGetMileage - Device does NOT support battery Mileage'.\n");
    result = 0;
  }
  return result;
}