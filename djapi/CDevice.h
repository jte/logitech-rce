/**
 */
#pragma once

#include <Windows.h>

struct DevInfo_t
{
	DWORD DevType;
	DWORD Parent;
	DWORD RadioType;
	DWORD ModelID;
	wchar_t Name[80];
	DWORD RadioStatus;
	DWORD Flags;
	DWORD Slider;
	DWORD DFUStatus;
	DWORD Battery;
	wchar_t SN[80];
	wchar_t FW[80];
	wchar_t NextFW[80];
	wchar_t BL[80];
};

class CDevice
{
public:
	CDevice();
	DWORD GetDFUStatus();
	void SetDFUStatus(DWORD DFUStatus);
	int SetNextFW(const wchar_t* pSzNextFw);
	int SetName(const wchar_t* pSzName);
	BOOL WaitForDeletion();
	LONG DecrementRefCount();
	BOOL IncrementRefCount();
	WCHAR* DevTypeToString();
	void GenerateDeviceRemovalEvent();
	void GenerateDeviceArrivalEvent();
	DevInfo_t* GetDeviceInfo(DevInfo_t* pDevInfo);
	void* GetRawDeviceHandle();
	void SetRawDeviceHandle(void* hRawDeviceHandle);
protected:
	DWORD m_DeviceConnected;
	DWORD m_RefCount;
	class CEnumerator* m_pEnumerator;
	HANDLE m_Event;
	void* m_hRawDeviceHandle;
	DWORD m_pad2;
	DevInfo_t m_DeviceInfo;
};
