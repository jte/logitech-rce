#include "CHPPTransporter.h"

CHPPTransporter::CHPPTransporter(DEVINST dnDevInst, HANDLE HidDeviceObject, BYTE ReportID, PHIDP_CAPS Capabilities, const wchar_t *DevicePath, HDEVNOTIFY hDevNotify)
{
	m_dnDevInst = dnDevInst;
	m_HidDeviceObject = HidDeviceObject;
	m_hDevNotify = hDevNotify;
	m_ReportID = ReportID;
	m_ulOutputLength = Capabilities->OutputReportByteLength;
	m_ulInputLength = Capabilities->InputReportByteLength;
	wcscpy_s(m_DevicePath, MAX_PATH, DevicePath);
	switch(Capabilities->Usage)
	{
		case 1:
			pad4 = 0x80u;
			m_SubID = 0x81u;
			pad8 = 0;
		break;
		case 2:
			pad4 = 0x82u;
			m_SubID = 0x83u;
		break;
		case 3:
			pad4 = 0x84u;
			m_SubID = 0x85u;
		break;
	}
}

bool CHPPTransporter::MatchDeviceInstance(DEVINST dnDevInst)
{
	return m_dnDevInst == dnDevInst;
}

bool CHPPTransporter::MatchHidDeviceObject(HANDLE HidDeviceObject)
{
	return m_HidDeviceObject == HidDeviceObject;
}

CHPPTransporter::~CHPPTransporter()
{
	if(m_hDevNotify)
	{
		UnregisterDeviceNotification(m_hDevNotify);
	}
	if(m_HidDeviceObject != (HANDLE)-1)
	{
		CloseHandle(m_HidDeviceObject);
	}
}