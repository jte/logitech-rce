#include "CWirelessDevice.h"
#include "CHPPDevice.h"

CWirelessDevice::CWirelessDevice(class CHPPDevice* pHppDevice, DWORD DevType)
{
	m_ProtocolVersion = -1;
	m_Event_1 = CreateEventW(0,1,0,0);
	m_Event_2 = CreateEventW(0,1,0,0);
	m_HppDevice = (DWORD)pHppDevice;
	m_DeviceInfo.DevType = DevType;
	m_DeviceInfo.Parent = (DWORD)GetRawDeviceHandle();
	m_DeviceInfo.Battery = 0;
}