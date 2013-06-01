/**
 */
#pragma once

#include <Windows.h>
#include "hid.h"
#include "CSingleton.h"
#include "Cfgmgr32.h"
#include "CHIDEnum.h"

#define VENDOR_ID_LOGITECH 0x46D
#define HID_USAGE_PAGE_LOGITECH_1 ((USAGE) 0xff00)
#define HID_USAGE_PAGE_LOGITECH_2 ((USAGE) 0xfb00)

#pragma push_macro("RegisterDeviceNotification")
#undef RegisterDeviceNotification

namespace NotificationEvents
{
	enum Events
	{
		OnDeviceArrival,
		OnDeviceRemoval,
		OnEnumerationComplete,
		OnWirelessStatus,
		OnDFUProgress,
		OnDFUBatteryCheck,
		OnDFUStatusChanged,
		OnBatteryStatus,
		OnUserActivity,
		OnDevicePowerUp,
		OnDFUMilestone,
		OnPairingStatus
	};
};
/**
 *	For notification event OnWirelessStatus
 */
namespace WirelessStatus
{
	enum 
	{
		Active,
		LinkLoss,
		DFUMode,
		Idle,
		Scanning
	};
};

class CEnumerator : public CHIDEnum, public Singleton<CEnumerator>
{
public:
	CEnumerator();
	bool StartEnumeration(HINSTANCE hInstance);
	HDEVNOTIFY RegisterDeviceNotification(GUID HidGuid);
	static DWORD CALLBACK EnumeratorThreadStatic(LPVOID pEnumerator);
	int EnumeratorThread();
	int StartEnum(int a1);
	int EnumHidDevices(GUID HidGuid);
	virtual int OnEnumFindDevice(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData, int a5);
	int PowerUp(int a2, PDEVINST pdnDevInst);
	int AddDevice(HDEVINFO DeviceInfoSet, PSP_DEVINFO_DATA DeviceInfoData, PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData, int a5);
	virtual int OnAddDevice(DWORD DevInst, HANDLE HidDeviceObject, CHAR* DevicePath, HIDD_ATTRIBUTES *Attributes, int a6);
	int ProcessAddedDevice(DWORD DevInst, HANDLE HidDeviceObject, CHAR* DevicePath, HIDD_ATTRIBUTES *Attributes, int a6);
	int ResolveHidDllFunctions(HINSTANCE hInstance);
	virtual int OnAddReceiver(DEVINST dnDevInst, HANDLE HidDeviceObject, const wchar_t *DevicePath, PHIDD_ATTRIBUTES Attributes, int PreparsedData, HIDP_CAPS *Capabilities, int a8);
	virtual int OnAddUnifyingReceiver(DEVINST dnDevInst, HANDLE HidDeviceObject, const wchar_t *DevicePath, PHIDD_ATTRIBUTES Attributes, int PreparsedData, HIDP_CAPS *Capabilities, int a8);
	char GetReportID(PHIDP_PREPARSED_DATA PreparsedData, HIDP_CAPS *a3);
	HDEVNOTIFY GetRegisteredDevNotify(HANDLE HidDeviceObject);
protected:
private:
	DWORD pad1;
	DWORD pad2;
	DWORD pad3;
	HMODULE m_hHidLibrary;
	GUID m_HidGuid;
	HidD_GetPreparsedData* m_pfnHidD_GetPreparsedData;
	HidD_FreePreparsedData* m_pfnHidD_FreePreparsedData;
	HidD_GetAttributes* m_pfnHidD_GetAttributes;
	HidD_GetSerialNumberString* m_pfnHidD_GetSerialNumberString;
	HidD_GetHidGuid* m_pfnHidD_GetHidGuid;
	HidP_GetCaps* m_pfnHidP_GetCaps;
	HidP_GetButtonCaps* m_pfnHidP_GetButtonCaps;
	HidP_GetValueCaps* m_pfnHidP_GetValueCaps;
	HidP_GetUsages* m_pfnHidP_GetUsages;
	HidP_SetUsages* m_pfnHidP_SetUsages;
	HidP_UnsetUsages* m_pfnHidP_UnsetUsages;
	HidP_GetUsagesEx* m_pfnHidP_GetUsagesEx;
	HidP_GetUsageValue* m_pfnHidP_GetUsageValue;
	HidP_GetLinkCollectionNodes* m_pfnHidP_GetLinkCollectionNodes;
	HidP_GetSpecificButtonCaps* m_pfnHidP_GetSpecificButtonCaps;
	HidP_TranslateUsagesToI8042ScanCodes* m_pfnHidP_TranslateUsagesToI8042ScanCodes;
	DWORD m_EnumerateHidDevicesThreadID;
	DWORD pad4;
	CRITICAL_SECTION m_DeviceOpCS;
	Epsilon_t m_Epsilon;
	HDEVNOTIFY m_hDevNotify;
	DWORD pad5;
	HANDLE m_hEnumHidDevicesEvent;
	DWORD pad6;
	DWORD pad7;
	DWORD m_ModelID;
	void* m_hRawDeviceHandle; /* aka hWirelessDevice, but not CWirelessDevice*/
	wchar_t m_szLanguage[80];
	DWORD m_DisableUpdater;
	DWORD pad8;
	DWORD m_EnumerationStarted;
	DWORD pad9;
};

#pragma pop_macro("RegisterDeviceNotification")