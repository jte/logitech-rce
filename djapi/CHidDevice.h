/**
 */
#pragma once

#include <Windows.h>
#include "CDevice.h"
#include "hid.h"

class CHidDevice : public CDevice
{
public:
	int Setup(HANDLE HidDeviceObject, char ReportID, PHIDP_CAPS Capabilities, PHIDD_ATTRIBUTES Attributes, const wchar_t *DevicePath, HDEVNOTIFY hDevNotify);
protected:
private:
	DEVINST m_dnDevInst;
	wchar_t m_DevicePath[MAX_PATH];
	HANDLE m_HidDeviceObject;
	BYTE m_ReportID;
	BYTE pad1[3];
	HANDLE m_Event;
	DWORD m_OutputReportByteLength;
	DWORD m_InputReportByteLength;
};