#include "CDrvApi.h"
#include "KHALMonitor.h"

HINSTANCE CDrvApi::m_hInstance;
bool CDrvApi::m_InitTried = false;
LD_GetDeviceInfo CDrvApi::m_LD_GetDeviceInfo;
LD_GetDeviceDetails CDrvApi::m_LD_GetDeviceDetails;
LD_RegisterEvent CDrvApi::m_LD_RegisterEvent;
LD_UnregisterEvent CDrvApi::m_LD_UnregisterEvent;
LD_DeviceControl CDrvApi::m_LD_DeviceControl;
LD_GetDriverVersionString CDrvApi::m_LD_GetDriverVersionString;
LD_Open CDrvApi::m_LD_Open;
LD_Close CDrvApi::m_LD_Close;
LD_GetDeviceName CDrvApi::m_LD_GetDeviceName;
LD_GetFeatureList CDrvApi::m_LD_GetFeatureList;
LD_GetFeatureInfo CDrvApi::m_LD_GetFeatureInfo;
LD_Main CDrvApi::m_LD_Main;
LD_GetDeviceCount CDrvApi::m_LD_GetDeviceCount;
LD_GetFirstDevicePosition CDrvApi::m_LD_GetFirstDevicePosition;
LD_GetNextDevicePosition CDrvApi::m_LD_GetNextDevicePosition;

bool CDrvApi::CheckInit()
{
	if(!m_InitTried)
	{
		m_hInstance = LoadLibraryA("KHALAPI.DLL");
		if(!m_hInstance)
		{
			printf("failed to load KHALAPI.DLL err: %d\n", GetLastError());
			return false;
		}
		else
		{
			m_LD_GetDeviceInfo = (LD_GetDeviceInfo)GetProcAddress(m_hInstance, "LD_GetDeviceInfo");
			m_LD_GetDeviceDetails = (LD_GetDeviceDetails)GetProcAddress(m_hInstance, "LD_GetDeviceDetails");
			m_LD_RegisterEvent = (LD_RegisterEvent)GetProcAddress(m_hInstance, "LD_RegisterEvent");
			m_LD_GetDeviceName = (LD_GetDeviceName)GetProcAddress(m_hInstance, "LD_GetDeviceName");
			m_LD_UnregisterEvent = (LD_UnregisterEvent)GetProcAddress(m_hInstance, "LD_UnregisterEvent");
			m_LD_DeviceControl = (LD_DeviceControl)GetProcAddress(m_hInstance, "LD_DeviceControl");
			m_LD_GetDriverVersionString = (LD_GetDriverVersionString)GetProcAddress(m_hInstance, "LD_GetDriverVersionString");
			m_LD_Open = (LD_Open)GetProcAddress(m_hInstance, "LD_Open");
			m_LD_Close = (LD_Close)GetProcAddress(m_hInstance, "LD_Close");
			m_LD_GetFeatureList = (LD_GetFeatureList)GetProcAddress(m_hInstance, "LD_GetFeatureList");
			m_LD_GetFeatureInfo = (LD_GetFeatureInfo)GetProcAddress(m_hInstance, "LD_GetFeatureInfo");
			m_LD_Main = (LD_Main)GetProcAddress(m_hInstance, "LD_Main");
			m_LD_GetDeviceCount = (LD_GetDeviceCount)GetProcAddress(m_hInstance, "LD_GetDeviceCount");
			m_LD_GetFirstDevicePosition = (LD_GetFirstDevicePosition)GetProcAddress(m_hInstance, "LD_GetFirstDevicePosition");
			m_LD_GetNextDevicePosition = (LD_GetNextDevicePosition)GetProcAddress(m_hInstance, "LD_GetNextDevicePosition");
		}
		m_InitTried = true;
	}
	m_InitTried = true;
	return true;
}

bool CDrvApi::Deinitialize()
{
	m_LD_Close();
	FreeLibrary(m_hInstance);
}

bool CDrvApi::Initialize()
{
	if(CheckInit() && m_LD_Open)
	{
		//m_LD_Main(m_hInstance);
		//m_LD_Open();
		cKHALMonitor::StartMonitoring();	
		return true;
	}
	return false;
}