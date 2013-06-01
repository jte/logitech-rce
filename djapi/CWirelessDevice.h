/**
 */
#pragma once

#include <Windows.h>
#include "CDevice.h"

class CWirelessDevice : public CDevice
{
public:
	CWirelessDevice(class CHPPDevice* pHppDevice, DWORD DevType);
protected:
private:
	DWORD m_HppDevice;
	DWORD pad1;
	BYTE m_DeviceIndex;
	BYTE pad2[3];
	WORD pad3;
	DWORD pad4[2];
	HANDLE m_Event_1;
	HANDLE m_Event_2;
	BYTE pad5[4];
	DWORD m_ProtocolVersion;
};
