/** 
*/
#pragma once

#include <Windows.h>
#include "hid.h"

class CHPPTransporter
{
public:
	CHPPTransporter(DEVINST dnDevInst, HANDLE HidDeviceObject, BYTE ReportID, PHIDP_CAPS Capabilities, const wchar_t *DevicePath, HDEVNOTIFY hDevNotify);
	bool MatchDeviceInstance(DEVINST dnDevInst);
	bool MatchHidDeviceObject(HANDLE HidDeviceObject);
	~CHPPTransporter();
//protected:
//private:
	DEVINST m_dnDevInst;
	HANDLE m_HidDeviceObject;
	HDEVNOTIFY m_hDevNotify;
	BYTE m_ReportID;
	BYTE pad1;
	wchar_t m_DevicePath[MAX_PATH];
	BYTE pad2;
	BYTE pad3;
	DWORD m_ulOutputLength;
	BYTE pad4;
	BYTE pad5;
	BYTE pad6;
	BYTE pad7;
	DWORD m_ulInputLength;
	BYTE m_SubID;
	BYTE pad8;
	BYTE pad9;
	BYTE pad10;
	BYTE* m_InputBuffer;
	DWORD m_lpNumberOfBytesRead;
	OVERLAPPED m_Overlapped;
	DWORD m_LastError;
};