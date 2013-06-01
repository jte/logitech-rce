#include "CDevice.h"
#include "CEnumerator.h"

CDevice::CDevice()
{
	m_pEnumerator = CEnumerator::GetSingleton();
	m_hRawDeviceHandle = 0;
	memset(&m_DeviceInfo, 0, 0x344u);
	m_pad1 = 0;
	m_RefCount = 1;
	m_Event = 0;
	m_pad2 = 0;
	sub_100012C0(&v3);
	sub_10001320((int)&v3, (LPWSTR)m_DeviceInfo.FW);
	sub_10001320((int)&v3, (LPWSTR)m_DeviceInfo.NextFW);
}

DWORD CDevice::GetDFUStatus()
{
	return m_DeviceInfo.DFUStatus;
}

void CDevice::SetDFUStatus(DWORD DFUStatus)
{
	m_DeviceInfo.DFUStatus = DFUStatus;
}

int CDevice::SetNextFW(const wchar_t* pSzNextFw)
{
	int res = wcscmp(m_DeviceInfo.NextFW, pSzNextFw) != 0;
	wsprintfW(m_DeviceInfo.NextFW, L"%s", pSzNextFw);
	return res;
}

int CDevice::SetName(const wchar_t* pSzName)
{
	return wsprintfW(m_DeviceInfo.Name, L"%s", pSzName);
}

BOOL CDevice::WaitForDeletion()
{
	m_Event = CreateEvent(0, 1, 0, 0);
	if(m_RefCount > 0)
	{
		WaitForSingleObject(m_Event, INFINITE);
	}
	return CloseHandle(m_Event);
}

LONG CDevice::DecrementRefCount()
{
	LONG res = InterlockedDecrement(&m_RefCount);
	if(res == 0 && m_Event)
	{
		res = SetEvent(m_Event);
	}
	return res;
}

BOOL CDevice::IncrementRefCount()
{
	if(m_Event)
	{
		return 0;
	}
	InterlockedIncrement(&m_RefCount);
	return 1;
}

WCHAR* CDevice::DevTypeToString()
{
	switch(m_DeviceInfo.DevType)
	{
		case 1:
		case 5:
		case 7:
		case 8:
			return L"mse";
		break;
		case 2:
			return L"kbd";
		break;
		case 0:
			return L"rcv";
		break;
		case 3:
			return L"nmp";
		break;
	}
	return L"dev";
}

void CDevice::GenerateDeviceRemovalEvent()
{
	if(m_DeviceConnected == 1)
	{
		m_DeviceConnected = 0;
		m_pEnumerator->InitiateEventCallback(NotificationEvents::OnDeviceRemoval, m_hRawDeviceHandle, m_DeviceInfo.DevType);
	}
}

void CDevice::GenerateDeviceArrivalEvent()
{
	m_pEnumerator->InitiateEventCallback(0, m_pad2, m_hRawDeviceHandle);
	m_DeviceConnected = 1;
}

DevInfo_t* CDevice::GetDeviceInfo(DevInfo_t* pDevInfo)
{
	memcpy(pDevInfo, &m_DeviceInfo, sizeof(m_DeviceInfo));
	return pDevInfo;
}

void* CDevice::GetRawDeviceHandle()
{
	return m_hRawDeviceHandle;
}

void CDevice::SetRawDeviceHandle(void* hRawDeviceHandle)
{
	m_hRawDeviceHandle = hRawDeviceHandle;
}