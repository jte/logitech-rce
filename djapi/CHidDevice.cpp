#include "CHidDevice.h"
#include "hid.h"

size_t g_UnifyingReceiverLastID = 0;

int CHidDevice::Setup(HANDLE HidDeviceObject, char ReportID, PHIDP_CAPS Capabilities, PHIDD_ATTRIBUTES Attributes, const wchar_t *DevicePath, HDEVNOTIFY hDevNotify)
{
	m_DeviceInfo.ModelID = (Attributes->VendorID << 16) | Attributes->ProductID;
	wchar_t szName[33 + 1 + 20];
	wsprintfW(szName, L"Unifying receiver [%d] (DFU mode)", g_UnifyingReceiverLastID++);
	SetName(szName);
	wcscpy_s(m_DeviceInfo.SN, 0x50u, &word_100360CC);
	wcscpy_s(m_DevicePath, 260u, DevicePath);
	m_HidDeviceObject = HidDeviceObject;
	m_Event = CreateEventW(0, 0, 0, 0);
	m_OutputReportByteLength = Capabilities->OutputReportByteLength;
	m_InputReportByteLength = Capabilities->InputReportByteLength;
	m_ReportID = ReportID;
	return 1;
}