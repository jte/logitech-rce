#include "CHPPDevice.h"
#include "hid.h"
#include "hidsdi.h"

int CHPPDevice::AddHPPTransporter(DEVINST dnDevInst, HANDLE HidDeviceObject, const wchar_t *DevicePath, PHIDP_CAPS Capabilities, BYTE ReportID, HDEVNOTIFY hDevNotify)
{
	CHPPTransporter* pTransporter = new CHPPTransporter(dnDevInst, HidDeviceObject, ReportID, Capabilities, DevicePath, hDevNotify);
	if(pTransporter)
	{
		for(size_t i = 0; i < 3; i++)
		{
			if(m_pTransporters[i] == NULL)
			{
				m_pTransporters[i] = pTransporter;
				return 1;
			}
		}
	}
	return 0;
}

int CHPPDevice::SendCommand(CHPPTransporter* pTransporter, BYTE* lpCommandBuffer, BYTE* lpResponseBuffer, DWORD TimeOut, int a6)
{
	if(TimeOut > 60000)
	{
		TimeOut = 3000;
	}
	if (!m_Event4)
	{
		return -8;
	}
	size_t RetryCount = 0;
	int ResponseCode = -1;
	while(1)
	{
		v60 = 0;
		HANDLE Handles[2];
		Handles[0] = m_DeviceMutex;
		Handles[1] = m_Event4;
		DWORD
			Status = WaitForMultipleObjects(2, (const HANDLE*)&Handles, 0, m_MutexTimeout);
		if(Status)
		{
			break;
		}
		m_field_390 = a6;
		m_lpCommandBuffer = lpCommandBuffer;
		m_lpResponseBuffer = lpResponseBuffer;
		m_ulOutputLength = pTransporter->m_ulOutputLength;
		OVERLAPPED Overlapped;
		memset(&Overlapped, 0, sizeof(OVERLAPPED));
		Overlapped.hEvent = m_ReadWriteOverlappedEvent;
		ResetEvent(m_ReadWriteOverlappedEvent);
		lpCommandBuffer[0] = pTransporter->m_ReportID;
		v50 = 0;
		do
		{
			ResetEvent(m_Event2);
			QueryPerformanceCounter(&pThis->field_370);
			SIZE_T NumberOfBytesTransferred = 0;
			v61 = WriteFile(pTransporter->m_HidDeviceObject, lpCommandBuffer, pTransporter->m_ulOutputLength, &NumberOfBytesTransferred, &Overlapped);
			v56 = GetLastError();
			if(!v61)
			{
				if ( v56 == 0x3E5 )
				{
					v61 = WaitForSingleObject(Overlapped.hEvent, 500) == 0;
					if ( v61 == 1 )
					{
						v61 = GetOverlappedResult(pTransporter->m_HidDeviceObject, &Overlapped, &NumberOfBytesTransferred, 0);
						if ( !v61 )
						{
							v56 = GetLastError();
						}
					}
				}
				else
				{
					v61 = 0;
				}
			}
			if ( v61 )
			{
				break;
			}
			++v50;
		}
		while ( v50 < 3 );
		if ( !v61 )
		{
			ResponseCode = -4;
			goto LABEL_66;
		}
		if ( !TimeOut )
		{
			ResponseCode = 0;
			goto LABEL_66;
		}
		Handles = (HANDLE)pThis->Event4;
		v54 = pThis->Event2;
		v55 = CHPPDevice::Unknown15(pThis, lpCommandBuffer[1]);
		nCount = (v55 != 0) + 2;
		v48 = 0;
		do
		{
			v61 = 0;
			v41 = WaitForMultipleObjects(nCount, (const HANDLE*)&Handles, 0, TimeOut);
			if ( v41 > 2 )
			{
				if ( v41 != 258 )
				{
					LABEL_59:
						ResponseCode = -1;
						v48 = 0;
					continue;
				}
				if ( v48 == 1 )
				{
					v61 = 1;
					if(m_Threshold > 5)
					{
						--m_Threshold;
					}
				}
				else
				{
					if(RetryCount >= 3)
					{
						ResponseCode = -2;
					}
					else
					{
						RetryCount++;
						v60 = 1;
					}
				}
			}
			else
			{
				if ( v41 == 2 )
				{
					ResponseCode = -9;
					v48 = 0;
				}
				else
				{
					if ( v41 )
					{
						if ( v41 != 1 )
							goto LABEL_59;
						v61 = 1;
						if ( v48 != 1 )
						{
							if ( *(_QWORD *)&pThis->PerformanceCounterFreq.LowPart >= 10000i64 )
							{
								*(_QWORD *)v45 = 10000i64 * (*(_QWORD *)&pThis->field_378.LowPart - *(_QWORD *)&pThis->field_370.LowPart) / *(_QWORD *)&pThis->PerformanceCounterFreq.LowPart;
								if ( *(_QWORD *)v45 < (signed __int64)pThis->Threshold )
								{
									v61 = 0;
									v48 = 1;
									ResetEvent(pThis->Event2);
									TimeOut = 500;
									Format = 0;
								}
							}
						}
					}
					else
					{
						ResponseCode = -3;
					}
				}
			}
		}
		while ( v48 && !v61 );
		if ( v61 )
		{
			ResponseCode = m_ResponseCode;
			if(ResponseCode == 7)
			{
				if(RetryCount < 3)
				{
					Sleep(500);
					RetryCount++;
					v60 = 1;
				}
			}
		}
		LABEL_66:
		pThis->m_ulOutputLength = 0;
		pThis->lpCommandBuffer = 0;
		pThis->field_390 = 0;
		pThis->lpResponseBuffer = &pThis->field_39C;
		ReleaseMutex(pThis->DeviceMutex);
		if ( !v60 )
		{
			return ResponseCode;
		}
	}
	if(Status == ERROR_INVALID_FUNCTION)
	{
		ReleaseMutex(m_DeviceMutex);
		return -8;
	}
	else if(Status == WAIT_TIMEOUT)
	{
		if(m_MutexTimeout < 10000)
		{
			m_MutexTimeout += 2000;
		}
		return -5;
	}
	ReleaseMutex(m_DeviceMutex);
	return -1;
}

int CALLBACK CHPPDevice::HPPThreadStatic(LPVOID Context)
{
	CHPPDevice *pHPPDevice = (CHPPDevice*)Context;
	return pHPPDevice->HPPThread();
}

int CHPPDevice::HPPThread()
{
	HANDLE Handles[4];
	Handles[0] = m_Event4;
	size_t ActiveTransporters = 0;
	for(size_t i = 0; i < 3; i++)
	{
		CHPPTransporter* pTransporter = m_pTransporters[i];
		if(pTransporter && pTransporter->m_HidDeviceObject != (HANDLE)-1)
		{
			memset(&pTransporter->m_Overlapped, 0, sizeof(pTransporter->m_Overlapped));
			pTransporter->m_InputBuffer = new BYTE[pTransporter->m_ulInputLength];
			pTransporter->m_Overlapped.hEvent = CreateEventW(0, 0, 0, 0);
			Handles[ActiveTransporters + 1] = pTransporter->m_Overlapped.hEvent;
			pTransporter->m_LastError = 0;
			ActiveTransporters++;
		}
	}
	while(1)
	{
		bool KeepReading = true;
		CHPPTransporter* pTransporter = NULL;
		while(KeepReading)
		{
			for(size_t i = 0; i < ActiveTransporters; ++i )
			{
				pTransporter = m_pTransporters[i];
				if(pTransporter && !pTransporter->m_LastError)
				{
					BOOL Success = ReadFile(pTransporter->m_HidDeviceObject, (LPVOID)pTransporter->m_InputBuffer, pTransporter->m_ulInputLength, &pTransporter->m_lpNumberOfBytesRead, &pTransporter->m_Overlapped);
					pTransporter->m_LastError = GetLastError();
					if(Success)
					{
						pTransporter->m_LastError = 0;
						KeepReading = false;
						OnReadData(pTransporter->m_InputBuffer, pTransporter->m_lpNumberOfBytesRead);
					}
				}
			}
		}
		DWORD res = WaitForMultipleObjects(ActiveTransporters + 1, (const HANDLE*)&Handles, 0, 0xFFFFFFFFu);
		if(res)
		{
			if(res != -1)
			{
				size_t
					i = res - 1;
				if(res - 1 < ActiveTransporters)
				{
					pTransporter = m_pTransporters[i];
					BOOL Success = GetOverlappedResult(pTransporter->m_HidDeviceObject, &pTransporter->m_Overlapped, &pTransporter->m_lpNumberOfBytesRead, 0);
					pTransporter->m_LastError = GetLastError();
					if ( Success == 1 )
					{
						pTransporter->m_LastError = 0;
						OnReadData(pTransporter->m_InputBuffer, pTransporter->m_lpNumberOfBytesRead);
					}
					else if(pTransporter->m_LastError == ERROR_DEVICE_NOT_CONNECTED)
					{
						PostMessageW(m_hEnumNotifyWnd, m_wParamReceiverIdentifier, (WPARAM)pTransporter->m_HidDeviceObject, (LPARAM)0);
						return 1;
					}
				}
			}
		}
		else
		{
			if(m_lpCommandBuffer)
			{
				m_ResponseCode = -3;
				SetEvent(m_Event2);
			}
		}
	}
	return 0;
}

int CHPPDevice::OnReadData(BYTE *InputBuffer, DWORD lpNumberOfBytesRead)
{
	bool 
		res = OnProcessReadData(InputBuffer, lpNumberOfBytesRead);
	if(!res)
	{
		if ( InputBuffer[2] <= (signed int)0x7Fu )
		{
			if ( pThis->field_338 - pThis->field_33C < 100 )
			{
				*(&pThis->field_18 + pThis->field_338 % 100) = (int)operator_new(lpNumberOfBytesRead);
				if ( *(&pThis->field_18 + pThis->field_338 % 100) )
				{
					*(&pThis->field_1A8 + pThis->field_338 % 100) = lpNumberOfBytesRead;
					FSRepository::Blowfish::defaultPiPad(*((void **)&pThis->field_18 + pThis->field_338 % 100), InputBuffer, lpNumberOfBytesRead);
					pThis->field_338++;
					if ( pThis->field_338 == pThis->field_33C )
					{
						SetEvent(m_Event3);
					}
				}
			}
		}
	}
}

int CHPPDevice::OnProcessReadData(BYTE *InputBuffer, DWORD lpNumberOfBytesRead)
{
	if(!m_lpCommandBuffer || m_lpCommandBuffer[1] != InputBuffer[1])
	{
		return 0;
	}
	if(memcmp(InputBuffer + 2, m_lpCommandBuffer + 2, m_ArgumentsSize))
	{
		if(InputBuffer[2] == 0x8F && InputBuffer[3] == m_lpCommandBuffer[2] && InputBuffer[4] == m_lpCommandBuffer[3])
		{
			QueryPerformanceCounter(&m_PerfCounter);
			m_ResponseCode = InputBuffer[5];
			SetEvent(m_Event2);
			return 1;
		}
		return 0;
	}
	QueryPerformanceCounter(&m_PerfCounter);
	m_ResponseCode = 0;
	if(m_lpResponseBuffer)
	{
		Blowfish::defaultPiPad(m_lpResponseBuffer, InputBuffer, lpNumberOfBytesRead);
	}
	SetEvent(m_Event2);
	return 1;
}

int CHPPDevice::CreateThreadsForReceiver(HWND hEnumNotifyWnd, int wParamReceiverIdentifier)
{
	m_hEnumNotifyWnd = hEnumNotifyWnd;
	m_wParamReceiverIdentifier = wParamReceiverIdentifier;
	if(m_pTransporters[0])
	{
		wchar_t DevicePath[MAX_PATH];
		wcscpy_s(DevicePath, MAX_PATH, L"Global\\HPP");
		wchar_t *pDevPath = m_pTransporters[0]->m_DevicePath;
		wchar_t *pDevEnd = &DevicePath[wcslen(DevicePath)];
		while(*pDevPath)
		{
			if(isalnum(*pDevPath))
			{
				*pDevEnd = toupper(*pDevPath);
				++pDevEnd;
			}
			pDevPath++;
		}
		*pDevEnd = 0;
		m_DeviceMutex = CreateMutexW(0, 0, (LPCWSTR)DevicePath);
	}
	else
	{
		m_DeviceMutex = CreateMutexW(0, 0, 0);
	}
	m_ReadWriteOverlappedEvent = CreateEventW(0, 0, 0, 0);
	m_Event2 = CreateEventW(0, 0, 0, 0);
	m_Event3 = CreateEventW(0, 0, 0, 0);
	m_Count = 0;
	m_FinalCount = 0;
	m_field_340 = 0;
	m_Event4 = CreateEventW(0, 1, 0, 0);
	DWORD ThreadId;
	m_ReadThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CHPPDevice::HPPThreadStatic, (LPVOID)this, 0, &ThreadId);
	m_Thread2 = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)CHPPDevice::Unknown10, pThis_, 0, &ThreadId);
	return 1;
}

int CALLBACK CHPPDevice::ForwardInputToReceiversThreadStatic(LPVOID Context)
{
	CHPPDevice* pHPPDev = (CHPPDevice*)Context;
	return pHPPDev->ForwardInputToReceivers();
}

int CHPPDevice::OnProcessSpontaneous(BYTE *InputBuffer, DWORD lpNumberOfBytesRead)
{
	HANDLE Handles[2];
	Handles[0] = m_Event4;
	Handles[1] = m_Event3;
	while(1)
	{
		DWORD res = WaitForMultipleObjects(2, (const HANDLE*)&Handles, 0, INFINITE);
		if(res) 
		{
			if(res == 1)
			{
				while(m_CurrentInputBuffer < m_NumberInputBuffers )
				{
					ProcessSpontaneous(m_ReadBuffer[m_CurrentInputBuffer % 100], m_ReadBufferBytesRead[m_CurrentInputBuffer % 100]);
					delete[] m_ReadBuffer[m_CurrentInputBuffer++ % 100];
				}
			}
		}
		else
		{
			break;
		}
	}
	return 0;
}